#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <time.h>
#include <math.h>
#include "vector.h"
//MAC
//#include <GLUT/GLUT.h>
//#include <OpenGL/OpenGL.h>
//Windows
#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <gl/glut.h>

using namespace std;

//Constants
//The scale at which everything is scaled in the application
#define NORMALSIZE 10.0
//The visual offset between both spectaror's eyes
#define EYEOFFSET 0.6
//The dimensions of the frustum
#define X 10.0
#define Y 10.0
#define Z 5
//Number of targets
#define VERSUS 30
//Radius of the target and its shooting interception zone
#define BUBBLE 1.0
//Range length for targets to be created behind the back wall in the animation
#define BARRACKS 40
//Z-offset between both eyes to avoid images mixing with each other.
#define EYESWITCH 0.02
//The final Z destination at which all targets approach. This is done to simulate a conic approach
#define FARBACK 30
//Translation of targets in the line at each renderization of the display
#define VELOCITY 0.05
//Number of textures to be loaded
#define TEXTURES 8

//Control Variables
//Is stereoscopy enabled?
bool stereo = true;

//Screen initial and consequent dimensions
static GLfloat screenWidth = 1920.0;
static GLfloat screenHeight = 1080.0;

//Vector created with FARBACK to approach all targets to a point behind the camera
vector3d farBack = vector3d(0, 0, FARBACK);

//Array which stores the spatial position of all targets
vector3d spheres[VERSUS];

//Array which stores if the target has been shot
bool dead[VERSUS];

//Array which stores the z position at which the sphere was succesfully shot
float firePlace[VERSUS];

//Float value which determines the translation of the targets to their final ddestiantion. It is increases by a small amount everytime the idle function is called
float nearDelta = 0.0;

//Values to determine the postion, direction and up vector of the camera.
vector3d cameraPos = vector3d(0, 0, 0.0001);
vector3d cameraUp = vector3d(0, 1, 0);
vector3d lookAt = vector3d(0, 0, -(NORMALSIZE * 2 - Z) / 2);

//Array which stores the allocation of the .bmp files to be used as textures
static unsigned int texture[TEXTURES];


// Struct of bitmap file.
struct BitMapFile{
   int sizeX;
   int sizeY;
   unsigned char *data;
};


//Methods's Definitions
int main(int argc, char **argv);
void init(int w, int h);
void setup();
void display();
void lights();
void draw();
void reshape(int width, int height);
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void createEnemies();
void incomingEnemies();
void trueColor(int red, int green, int blue);
BitMapFile *getBMPData(string filename);
void loadExternalTextures(string filename, int TexIndex);


//Creates the viewport and set the stereoscopy display mode on.
int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    if (stereo) {
        glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STEREO);
    } else {
        glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    }
    
    glutInitWindowSize(screenWidth, screenHeight);
    
    glutCreateWindow(argv[0]);
    glutDisplayFunc(display);
	glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    init(screenWidth, screenHeight);
    glutMainLoop();
    return 0; 
}


//Initializes cetain components: , defines targets' positions, enables depth test.
void init(int w, int h)
{
	//Creates random seed
    srand((unsigned int)time(NULL));

	//Calls createEnemies function (explained throughly on its implementation)
    createEnemies();
    
	//Sets RGBA color value and initial dimensions of the viewport) 
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glViewport(0, 0, w, h);
    
	//Signals that the following code is to be displayed to the left eye buffer
 	glDrawBuffer(GL_BACK_LEFT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
	//Sets the frustum with its corresponding constants
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

	//Frustum with center in (0, 0, (NORMALSIZE * 2 + Z) / 2))
    glFrustum(-X, X, -Y, Y, Z, NORMALSIZE * 2);

	//Translation to represent the left eye OFFSET constant
    glTranslatef(EYEOFFSET, 0.0f, 0.0f);

	//Setting the camera
    gluLookAt(cameraPos.x + EYEOFFSET, cameraPos.y, cameraPos.z, lookAt.x, lookAt.y, lookAt.z, cameraUp.x, cameraUp.y, cameraUp.z);
    
	//Signals that the following code is to be displayed to the right eye buffer. If stereoscopy is disabled, the right eye is shown the same buffers as the left eye
    if (stereo) {
        glDrawBuffer(GL_BACK_RIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glFrustum(-X, X, -Y, Y, Z, NORMALSIZE * 2);

		//Translation to represent the right eye OFFSET constant and the z EYESWITCH offset to prevent the images merging
        glTranslatef(-EYEOFFSET, 0.0f, -EYESWITCH);
        gluLookAt(cameraPos.x - EYEOFFSET, cameraPos.y, cameraPos.z, lookAt.x, lookAt.y, lookAt.z, cameraUp.x, cameraUp.y, cameraUp.z - EYESWITCH);
    }
    
	//Enables the GL_DEPTH_TEST so that application conceals and shows images depending on their simulated depth
    glEnable(GL_DEPTH_TEST);

	//Calls setup function (explained throughly on its implementation)
	setup();
}


//Setting up textures array for further use
void setup(void){    
   glEnable(GL_DEPTH_TEST);
   glShadeModel(GL_FLAT);
   glClearColor(0.0, 0.0, 0.0, 0.0); 

   glPixelStorei(GL_UNPACK_ALIGNMENT,1);
   glGenTextures(TEXTURES, texture); 

   // Load external texture and generate and load procedural texture.
   loadExternalTextures("Textures/Spot.bmp",0);
   loadExternalTextures("Textures/BloodSpot.bmp",1);
   loadExternalTextures("Textures/BrickWall.bmp",2);
   loadExternalTextures("Textures/Concrete.bmp",3);
   loadExternalTextures("Textures/Ground.bmp",4);
   loadExternalTextures("Textures/Water.bmp",5);
   loadExternalTextures("Textures/Ivory.bmp",6);
   loadExternalTextures("Textures/Grass.bmp",7);

   // Specify how texture values combine with current surface color values.
   glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}


//Display Func: Draws buffer for left and right eyes with appropiate EYEOFFSET and swaps between them.
void display()
{
    //Back buffer for left eye
    glDrawBuffer(GL_BACK_LEFT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Calls lights function (explained throughly on its implementation)
	lights();

	//Enables the use of illumination in the scene
	glEnable(GL_LIGHT4);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	//Calls draw function (explained throughly on its implementation)
    draw();
    
	//Left frustum setting
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-X, X, -Y, Y, Z, NORMALSIZE * 2);
    glTranslatef(EYEOFFSET, 0.0f, 0.0f);
    gluLookAt(cameraPos.x, cameraPos.y, cameraPos.z, lookAt.x, lookAt.y, lookAt.z, cameraUp.x, cameraUp.y, cameraUp.z);
    
    //Back buffer for right eye
    if (stereo) {
        glDrawBuffer(GL_BACK_RIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		lights();
		glEnable(GL_LIGHT4);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        draw();
        
		//Right frustum setting
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glFrustum(-X, X, -Y, Y, Z, NORMALSIZE * 2);
        //Slight translation of right eye to avoid objects drawn in the same z-value
        glTranslatef(-EYEOFFSET, 0.0f, -EYESWITCH);
        gluLookAt(cameraPos.x - EYEOFFSET, cameraPos.y, cameraPos.z, lookAt.x, lookAt.y, lookAt.z, cameraUp.x, cameraUp.y, cameraUp.z - EYESWITCH);
    }
    
    glFlush();
    glutSwapBuffers();
}


//Setting environment illumination
void lights()
{
	//Setting the diffuse light (prevailing) with a white color
	GLfloat diffuse[]  = {1, 1, 1, 1};

	//Setting the specular light (minor tint) with a blue color
	GLfloat specular[]  = {0, 0, 1, 1};

	//Setting lights' position and direction so that it stands near the camera, illuminating towards the center of the frustum
	GLfloat position[]   = {0, NORMALSIZE, -Z, 0};
    GLfloat direction[]  = {0, 0,-1};

	//Setting lights environment
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    GLfloat lmodel_ambient[] = {0.5, 0.5, 0.5, 1};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);

	glLightfv(GL_LIGHT4, GL_DIFFUSE,  diffuse);
	glLightfv(GL_LIGHT4, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT4, GL_POSITION, position);
	glLightf (GL_LIGHT4, GL_SPOT_EXPONENT, 50.0f);
	glLightf (GL_LIGHT4, GL_SPOT_CUTOFF, 100.0);
	glLightfv(GL_LIGHT4, GL_SPOT_DIRECTION, direction);

	GLfloat mat_ambient[]    = {0.7, 0.7, 0.7, 1};
	GLfloat mat_diffuse[]    = {0.8, 0.8, 0.8, 1};
	GLfloat mat_specular[]   = {1, 1, 1, 1};
	GLfloat high_shininess[] = {100};

	glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
}


//Draws Scene
void draw()
{
	//Renders the textures presented in different objects to be shown to wards the camera
	glFrontFace(GL_CCW);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    //Shooting Room
    glPushMatrix();
	//Translates the entire room towards behind the near frustum plane
    glTranslatef(0.0, 0.0, -Z);
    
	//Sets that textured shapes after this point are wrapped in the texture assigned
	glBindTexture(GL_TEXTURE_2D, texture[0]);


	//BACK WALL
	//Begins wrapping texture over shape
	glEnable(GL_TEXTURE_2D);
    glBegin(GL_POLYGON);
    glTexCoord2f(1.0,0.0);              glVertex3f(NORMALSIZE, -NORMALSIZE, -NORMALSIZE);
    glTexCoord2f(1.0,1.0);              glVertex3f(NORMALSIZE, NORMALSIZE, -NORMALSIZE);
    glTexCoord2f(0.0,1.0);              glVertex3f(-NORMALSIZE, NORMALSIZE, -NORMALSIZE);
    glTexCoord2f(0.0,0.0);              glVertex3f(-NORMALSIZE, -NORMALSIZE, -NORMALSIZE);
    glEnd();

	//Ends wrapping texture over shape
	glDisable(GL_TEXTURE_2D);

	//RIGHT WALL
	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glEnable(GL_TEXTURE_2D);    
    glBegin(GL_POLYGON);
    glTexCoord2f(1.0,0.0);              glVertex3f(NORMALSIZE, -NORMALSIZE, -NORMALSIZE);
    glTexCoord2f(1.0,1.0);             glVertex3f(NORMALSIZE, NORMALSIZE, -NORMALSIZE);
    glTexCoord2f(0.0,1.0);            glVertex3f(NORMALSIZE, NORMALSIZE, NORMALSIZE);
    glTexCoord2f(0.0,0.0);            glVertex3f(NORMALSIZE, -NORMALSIZE, NORMALSIZE);
    glEnd();
    
	//LEFT WALL
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_POLYGON);
    glTexCoord2f(0.0,0.0);            glVertex3f(-NORMALSIZE, -NORMALSIZE, NORMALSIZE);
    glTexCoord2f(0.0,1.0);            glVertex3f(-NORMALSIZE, NORMALSIZE, NORMALSIZE);
    glTexCoord2f(1.0,1.0);            glVertex3f(-NORMALSIZE, NORMALSIZE, -NORMALSIZE);
    glTexCoord2f(1.0,0.0);            glVertex3f(-NORMALSIZE, -NORMALSIZE, -NORMALSIZE);
    glEnd();
	glDisable(GL_TEXTURE_2D);
	
	//TOP WALL
	glBindTexture(GL_TEXTURE_2D, texture[3]);
	glEnable(GL_TEXTURE_2D); 
    glBegin(GL_POLYGON);
    glTexCoord2f(1.0,0.0);              glVertex3f(NORMALSIZE, NORMALSIZE, NORMALSIZE);
    glTexCoord2f(1.0,1.0);              glVertex3f(NORMALSIZE, NORMALSIZE, -NORMALSIZE);
    glTexCoord2f(0.0,1.0);              glVertex3f(-NORMALSIZE, NORMALSIZE, -NORMALSIZE);
    glTexCoord2f(0.0,0.0);            glVertex3f(-NORMALSIZE, NORMALSIZE, NORMALSIZE);
    glEnd();
	glDisable(GL_TEXTURE_2D);

	//BOTTOM WALL
    glBindTexture(GL_TEXTURE_2D, texture[5]);
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_POLYGON);
	glColor3f(0.8f,0.8f,0.7f);
	glTexCoord2f(1.0, 1.0);            glVertex3f(NORMALSIZE, -NORMALSIZE, -NORMALSIZE);
    glTexCoord2f(1.0, 0.0);            glVertex3f(NORMALSIZE, -NORMALSIZE, NORMALSIZE);
    glTexCoord2f(0.0, 0.0);            glVertex3f(-NORMALSIZE, -NORMALSIZE, NORMALSIZE);
    glTexCoord2f(0.0, 1.0);            glVertex3f(-NORMALSIZE, -NORMALSIZE, -NORMALSIZE);
    glEnd();
	glDisable(GL_TEXTURE_2D);
    
    
    //Shooting Targets
    for (int i = 0; i < VERSUS; i++)
    {
        //If target has been shot, it is assigned a red texture, otherwise white
        if (dead[i] == true)
        {
            glBindTexture(GL_TEXTURE_2D, texture[1]);
        } else {
            glBindTexture(GL_TEXTURE_2D, texture[0]);
        }
        glPushMatrix();
        
        //Translate entire set of targets with a nearDelta value towards the screen.
		//Gets current target
        vector3d sphere = spheres[i];
        
		//Marks its direction vector toward the FARBACK depth and normalizes it
        vector3d cruiseDirection = farBack - sphere;
        cruiseDirection.normalize();

		//Enables the wrapping of texture with a solid shape
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
        
		//Sets the behavior of the targets movemnt depending of wether it was shot or not
		if (dead[i] == true)
		{
			//If it has been shot, it rotates bottomwards and disappears towards the scene
			glTranslatef(sphere.x + cruiseDirection.x * firePlace[i], sphere.y + cruiseDirection.y * firePlace[i] - (nearDelta - firePlace[i]), sphere.z + cruiseDirection.z * firePlace[i]);
		} else {
			//If it has not been shot, it continues its trayectory determined with its original position and advances its direction vector multiplied by nearDelta
			glTranslatef(sphere.x + cruiseDirection.x * nearDelta, sphere.y + cruiseDirection.y * nearDelta, sphere.z + cruiseDirection.z * nearDelta);
		}

        //Draws a sphere with a BUBBLE radius
        glutSolidSphere(BUBBLE, 16, 16);

		//Disables the wrapping of texture with a solid shape
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_TEXTURE_2D);
		
        glPopMatrix();
    }
    
    glPopMatrix();
}

void reshape(int width, int height)
{
	glViewport (0, 0, (GLsizei) width, (GLsizei) height); 
	screenWidth = width;
	screenHeight = height;
}


//Keyboard Func: Press 's' to start the game and 'p' to pause it
void keyboard(unsigned char key, int x, int y)
{
    switch(key)
    {
		case 'r':
            createEnemies();
			nearDelta = 0;
        case 's':
            glutIdleFunc(incomingEnemies);
            break;
        case 'p':
            glutIdleFunc(NULL);
            break;
        default:
            break;
    }
    glutPostRedisplay();
}


//Mouse Func: Detects mouse clicks. If a left click is executed the selector fires away and processes the targets that were hit
void mouse(int button, int state, int x, int y)
{
    switch (button) {
        case GLUT_LEFT_BUTTON:
            if (state == GLUT_DOWN)
            {
                vector3d view = lookAt - cameraPos;
                view.normalize();
                
                vector3d h = view.crossproduct(cameraUp);
                h.normalize();
                
                vector3d v = h.crossproduct(view);
                v.normalize();
                
                h *= Y;
                v *= X;
                
                float xx = x;
                float yy = y;
                
                xx -= screenWidth / 2;
                yy -= screenHeight / 2;
                
                yy /= (screenHeight / 2);
                xx /= (screenWidth / 2);
                
                vector3d p = cameraPos + view * Z + h*xx + v*yy;
                vector3d d = p - cameraPos;
                d.normalize();
                
                int i;
                
				//Windows
				float longShot = - 100000;
				//MAC
                //float longShot = - INFINITY;
                int nearIndex = VERSUS;
                
                for (i = 0; i < VERSUS; i++) {
                    
                    vector3d sphere = spheres[i];
                    
                    vector3d cruiseDirection = farBack - sphere;
                    
                    cruiseDirection.normalize();
                    
                    vector3d c = vector3d(sphere.x + cruiseDirection.x * nearDelta * (1 - Z / FARBACK), sphere.y + cruiseDirection.y * nearDelta * (1 - Z / FARBACK), sphere.z + cruiseDirection.z * nearDelta - Z);
                    
                    vector3d vpc = c - p;
                    vector3d intersection;
                    
                    if ((vpc.dotproduct(d)) < 0) {
                        if (vpc.length() > BUBBLE) {
                            
                            
                        } else if (vpc.length() == BUBBLE) {
                            
                            intersection = p;
                            
                            if (intersection.z > longShot) {
                                longShot = intersection.z;
                                nearIndex = i;
                            }
                            
                        } else {
                            
                            vector3d pc = d *= (c.dotproduct(d) / d.dotproduct(d));
                            
                            float dist = sqrt(pow(BUBBLE, 2) - pow((pc - c).length(), 2));
                            float di1 = dist - (pc - c).length();
                            intersection = p + d * di1;
                            
                            longShot = intersection.z;
                            nearIndex = i;
                        }
                    } else {
                        
                        vector3d pc = d *= (c.dotproduct(d) / d.dotproduct(d));
                        
                        float di1;
                        
                        if ((c - pc).length() > BUBBLE) {
                            
                            
                        } else {
                            
                            float dist = sqrt(pow(BUBBLE, 2) - pow((pc - c).length(), 2));
                            
                            if (vpc.length() > BUBBLE) {
                                
                                di1 = (pc - p).length() - dist;
                                
                            } else {
                                
                                di1 = (pc - p).length() + dist;
                            }
                            
                            intersection = p + d * di1;
                            
                            
                            if (intersection.z > longShot && dead[i] == false) {
                                longShot = intersection.z;
                                nearIndex = i;
                            }
                            
                        }
                        
                        
                    }
                    
                }
                
                if (nearIndex < VERSUS) {
                    dead[nearIndex] = true;
					firePlace[nearIndex] = nearDelta;
                }
                
                glutPostRedisplay();
            }
    }
}


//At game start, decides randomly the position of all targets to be created inside a certain boundary limit
void createEnemies()
{

    for (int i = 0; i < VERSUS; i++)
    {
		dead[i] = false;
		firePlace[i] = 0;
        float x = rand() / float(RAND_MAX) * (X - BUBBLE) * 2 - (X - BUBBLE);
        float y = rand() / float(RAND_MAX) * (Y - BUBBLE) * 2 - (Y - BUBBLE);
        float z = (rand() / float(RAND_MAX) - 1.5) * BARRACKS - BUBBLE;
        
        spheres[i] = vector3d(x, y, z);
    }
}


//Idle Func: While the game is running every time is called increases the nearDelta value moving the targets closer to the screen
void incomingEnemies()
{
    nearDelta += VELOCITY;
    glutPostRedisplay();
}


//Gets RGB integer value and returns appropiate color.
void trueColor(int red, int green, int blue)
{
    glColor3f(red/255.0, green/255.0, blue/255.0);
}



// Routine to read a bitmap file.
// Works only for uncompressed bmp files of 24-bit color.
BitMapFile *getBMPData(string filename)
{
   BitMapFile *bmp = new BitMapFile;
   unsigned int size, offset, headerSize;
  
   // Read input file name.
   ifstream infile(filename.c_str(), ios::binary);
 
   // Get the starting point of the image data.
   infile.seekg(10);
   infile.read((char *) &offset, 4); 
   
   // Get the header size of the bitmap.
   infile.read((char *) &headerSize,4);

   // Get width and height values in the bitmap header.
   infile.seekg(18);
   infile.read( (char *) &bmp->sizeX, 4);
   infile.read( (char *) &bmp->sizeY, 4);

   // Allocate buffer for the image.
   size = bmp->sizeX * bmp->sizeY * 24;
   bmp->data = new unsigned char[size];

   // Read bitmap data.
   infile.seekg(offset);
   infile.read((char *) bmp->data , size);
   
   // Reverse color from bgr to rgb.
   int temp;
   for (int i = 0; i < size; i += 3){ 
      temp = bmp->data[i];
	  bmp->data[i] = bmp->data[i + 2];
	  bmp->data[i + 2] = temp;
   }

   return bmp;
}

// Load external textures.
void loadExternalTextures(string filename, int TexIndex)
{
	BitMapFile *image[1];		// Local storage for bmp image data.
	image[0] = getBMPData(filename);		// Load the textures.

	// Activate texture index texture[0]. 
    glBindTexture(GL_TEXTURE_2D, texture[TexIndex]);

    // Set texture parameters for wrapping.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,  GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Set texture parameters for filtering.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Specify an image as the texture to be bound with the currently active texture index.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image[0]->sizeX, image[0]->sizeY, 0, 
	            GL_RGB, GL_UNSIGNED_BYTE, image[0]->data);	
}