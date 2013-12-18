#include <cstdlib>
#include <iostream>
#include <fstream>

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif




#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "time.h"
#include "windows.h"
#include "gl/glu.h"
#include "vector.h"

using namespace std;

//Constants
#define NORMALSIZE 10.0
#define EYEOFFSET 0.5
#define X 10.0
#define Y 10.0
#define Z 5
#define VERSUS 20
#define BUBBLE 1.0
#define BARRACKS 30
#define EYESWITCH 0.02
#define FARBACK 30

#define BUFFER_LENGTH 64
#define checkImageX 64
#define checkImageY 64
#define PI 3.14159265
#define TEXTURAS 11

//////VARIABLES PARA TEXTURA///////////////////////
static GLubyte checkImage[checkImageX][checkImageY][3];
static GLuint texName;
static unsigned int texture[TEXTURAS];
////////////////////////////////////////////////////

//Control Variables
bool stereo = false;

static GLfloat screenWidth = 800.0;
static GLfloat screenHeight = 800.0;

vector3d farBack = vector3d(0, 0, FARBACK);

vector3d spheres[VERSUS];
bool dead[VERSUS];
float nearDelta = 0.0;

vector3d cameraPos = vector3d(0, 0, 0.0001);
vector3d cameraUp = vector3d(0, 1, 0);
vector3d lookAt = vector3d(0, 0, -(NORMALSIZE * 2 - Z) / 2);


//Methods's Definitions
int main(int argc, char **argv);
void init(int w, int h);
void display();
void draw();
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void createEnemies();
void incomingEnemies();
void trueColor(int red, int green, int blue);
void ligths();

//////////////Fucniones para cargar textura/////////////////

// Struct of bitmap file.
struct BitMapFile{
   int sizeX;
   int sizeY;
   unsigned char *data;
};

// Routine to read a bitmap file.
// Works only for uncompressed bmp files of 24-bit color.
BitMapFile *getBMPData(string filename){

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
	  bmp->data[i] = bmp->data[i+2];
	  bmp->data[i+2] = temp;
   }

   return bmp;
}

// Load external textures.
void loadExternalTextures(string filename, int TexIndex)	{
   
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

void setup(void){    
   glEnable(GL_DEPTH_TEST);
   glShadeModel(GL_FLAT);
   glClearColor(0.0, 0.0, 0.0, 0.0); 

   glPixelStorei(GL_UNPACK_ALIGNMENT,1);
   glGenTextures(TEXTURAS, texture); 

   // Load external texture and generate and load procedural texture.
   loadExternalTextures("Texturas/fantasma3.bmp",0);
   loadExternalTextures("Texturas/fantasma4.bmp",1);
   loadExternalTextures("Texturas/ladrillo.bmp",2);
   loadExternalTextures("Texturas/cemento.bmp",3);
   loadExternalTextures("Texturas/suelo.bmp",4);
   loadExternalTextures("Texturas/agua.bmp",5);
   loadExternalTextures("Texturas/marmol.bmp",6);
   loadExternalTextures("Texturas/grass.bmp",7);

   // Specify how texture values combine with current surface color values.
   glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}


////////////////////////////////////////////////////////////


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
	setup();
	glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    init(screenWidth, screenHeight);
    glutMainLoop();
    return 0; 
}




//Initializes cetain components: creates random seed, defines targets' positions, enables depth test.
void init(int w, int h)
{
    srand((unsigned int)time(NULL));
    createEnemies();
    
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glViewport(0, 0, w, h);
    
 	glDrawBuffer(GL_BACK_LEFT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-X, X, -Y, Y, Z, NORMALSIZE * 2);
    glTranslatef(EYEOFFSET, 0.0f, 0.0f);
    gluLookAt(cameraPos.x + EYEOFFSET, cameraPos.y, cameraPos.z, lookAt.x, lookAt.y, lookAt.z, cameraUp.x, cameraUp.y, cameraUp.z);
    
    if (stereo) {
        glDrawBuffer(GL_BACK_RIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glFrustum(-X, X, -Y, Y, Z, NORMALSIZE * 2);
        glTranslatef(-EYEOFFSET, 0.0f, -EYESWITCH);
        gluLookAt(cameraPos.x - EYEOFFSET, cameraPos.y, cameraPos.z, lookAt.x, lookAt.y, lookAt.z, cameraUp.x, cameraUp.y, cameraUp.z - EYESWITCH);
    }
    
    glEnable(GL_DEPTH_TEST);
}


//Display Func: Draws buffer for left and right eyes with appropiate EYEOFFSET and swaps between them.
void display()
{
	
    //Back buffer for left eye
    glDrawBuffer(GL_BACK_LEFT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
	ligths();
	glEnable(GL_LIGHT4);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    draw();
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-X, X, -Y, Y, Z, NORMALSIZE * 2);
    glTranslatef(EYEOFFSET, 0.0f, 0.0f);
    gluLookAt(cameraPos.x, cameraPos.y, cameraPos.z, lookAt.x, lookAt.y, lookAt.z, cameraUp.x, cameraUp.y, cameraUp.z);
    
    
	
	//Back buffer for right eye
    
    if (stereo) {
        glDrawBuffer(GL_BACK_RIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        draw();
        
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

void ligths(){
	GLfloat luz5Difusa[]     = { 1.0f, 1.0f, 0.0f, 1.0f };
	GLfloat luz5Especular[]  = { 1.0f, 1.0f, 0.0f, 1.0f };  //luz AMARILLA
	GLfloat luz5posicion[]   = { 0.0, NORMALSIZE, -Z, 0.0f }; //Posición luz 5
    GLfloat luz5Direccion[]  = {0.0f, 0.0f,-1.0f};

	
    
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    GLfloat lmodel_ambient[] = { 0.5, 0.5, 0.5, 1.0 };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);

	glLightfv(GL_LIGHT4, GL_DIFFUSE,  luz5Difusa);
	glLightfv(GL_LIGHT4, GL_SPECULAR, luz5Especular);
	glLightfv(GL_LIGHT4, GL_POSITION, luz5posicion);
	glLightf (GL_LIGHT4, GL_SPOT_EXPONENT, 50.0f);
	glLightf (GL_LIGHT4, GL_SPOT_CUTOFF, 100.0);
	glLightfv(GL_LIGHT4, GL_SPOT_DIRECTION, luz5Direccion);

	GLfloat mat_ambient[]    = { 0.7f, 0.7f, 0.7f, 1.0f };
	GLfloat mat_diffuse[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
	GLfloat mat_specular[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat high_shininess[] = { 100.0f };

	glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
}

//Draws Scene
void draw()
{

	glEnable(GL_DEPTH_TEST);	// Hidden surface removal
	glFrontFace(GL_CCW);		// Counter clock-wise polygons face out
	//glEnable(GL_CULL_FACE);		// Do not calculate inside of jet
    //glDepthFunc(GL_LESS);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); 
	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE); 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(1.0f,1.0f,1.0f,1.0f );
	glMatrixMode(GL_MODELVIEW);

     //Shooting Room
    glPushMatrix();
    glTranslatef(0.0, 0.0, -Z);


	//glBindTexture(GL_TEXTURE_2D, texture[0]);
    glBindTexture(GL_TEXTURE_2D, texture[3]);
	glEnable(GL_TEXTURE_2D);
    glBegin(GL_POLYGON);
    glTexCoord2f(1.0,0.0);              glVertex3f(NORMALSIZE, -NORMALSIZE, -NORMALSIZE);
    glTexCoord2f(1.0,1.0);              glVertex3f(NORMALSIZE, NORMALSIZE, -NORMALSIZE);
    glTexCoord2f(0.0,1.0);              glVertex3f(-NORMALSIZE, NORMALSIZE, -NORMALSIZE);
    glTexCoord2f(0.0,0.0);              glVertex3f(-NORMALSIZE, -NORMALSIZE, -NORMALSIZE);
    glEnd();
	glDisable(GL_TEXTURE_2D);

	//WALL RIGHT
	glBindTexture(GL_TEXTURE_2D, texture[6]);
	glEnable(GL_TEXTURE_2D);    
    glBegin(GL_POLYGON);
    glTexCoord2f(1.0,0.0);              glVertex3f(NORMALSIZE, -NORMALSIZE, -NORMALSIZE);
    glTexCoord2f(0.0,0.0);             glVertex3f(NORMALSIZE, NORMALSIZE, -NORMALSIZE);
    glTexCoord2f(0.0,1.0);            glVertex3f(NORMALSIZE, NORMALSIZE, NORMALSIZE);
    glTexCoord2f(1.0,1.0);            glVertex3f(NORMALSIZE, -NORMALSIZE, NORMALSIZE);
    glEnd();
	glDisable(GL_TEXTURE_2D);
    
	//WALL LEFT
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_POLYGON);
    glTexCoord2f(0.0,0.0);            glVertex3f(-NORMALSIZE, -NORMALSIZE, NORMALSIZE);
    glTexCoord2f(0.0,1.0);            glVertex3f(-NORMALSIZE, NORMALSIZE, NORMALSIZE);
    glTexCoord2f(1.0,1.0);            glVertex3f(-NORMALSIZE, NORMALSIZE, -NORMALSIZE);
    glTexCoord2f(1.0,0.0);            glVertex3f(-NORMALSIZE, -NORMALSIZE, -NORMALSIZE);
    glEnd();
	glDisable(GL_TEXTURE_2D);
    
	//roof
    glBegin(GL_POLYGON);
    trueColor(218, 148, 44);            glVertex3f(NORMALSIZE, NORMALSIZE, NORMALSIZE);
    trueColor(77, 52, 15);              glVertex3f(NORMALSIZE, NORMALSIZE, -NORMALSIZE);
    trueColor(77, 52, 15);              glVertex3f(-NORMALSIZE, NORMALSIZE, -NORMALSIZE);
    trueColor(218, 148, 44);            glVertex3f(-NORMALSIZE, NORMALSIZE, NORMALSIZE);
    glEnd();
	/*glBindTexture(GL_TEXTURE_2D, texture[3]);
	glEnable(GL_TEXTURE_2D);
    glBegin(GL_POLYGON);
    glTexCoord2f(1.0,0.0);              glVertex3f(NORMALSIZE, NORMALSIZE, NORMALSIZE);
    glTexCoord2f(1.0,1.0);              glVertex3f(NORMALSIZE, NORMALSIZE, -NORMALSIZE);
    glTexCoord2f(0.0,1.0);              glVertex3f(-NORMALSIZE, NORMALSIZE, -NORMALSIZE);
    glTexCoord2f(0.0,0.0);            glVertex3f(-NORMALSIZE, NORMALSIZE, NORMALSIZE);
    glEnd();
	glDisable(GL_TEXTURE_2D);*/
    
	//FLOOR
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
		
			
        //If target has been shot, it is colored red; otherwise colored white
        if (dead[i] == true)
        {

			glBindTexture(GL_TEXTURE_2D, texture[1]);
			
            
        } else {
			glBindTexture(GL_TEXTURE_2D, texture[0]);
            //glColor3f(1.0, 1.0, 1.0);
        }
        glLoadName(i + 1);
        glPushMatrix();
        
        //Translate entire set of targets with a nearDelta value towards the screen.
        
        vector3d sphere = spheres[i];
        
        vector3d cruiseDirection = farBack - sphere;
        cruiseDirection.normalize();
        
		//Initialization
		

		glEnable(GL_TEXTURE_2D);
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);


        glTranslatef(sphere.x + cruiseDirection.x * nearDelta, sphere.y + cruiseDirection.y * nearDelta, sphere.z + cruiseDirection.z * nearDelta);
        glutSolidSphere(BUBBLE, 16, 16);

		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_TEXTURE_2D);

        
        glPopMatrix();
    }
    
    glPopMatrix();
}


//Keyboard Func: Press 's' to start the game and 'p' to pause it
void keyboard(unsigned char key, int x, int y)
{
    switch(key)
    {
        case 's':
            glutIdleFunc(incomingEnemies);
            break;
        case 'r':
            createEnemies();
            for (int i = 0; i < VERSUS; i++) {
                dead[i] = false;
            }
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
                
                float longShot = - 99999999;
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
        float x = rand() / float(RAND_MAX) * (X - BUBBLE) * 2 - (X - BUBBLE);
        float y = rand() / float(RAND_MAX) * (Y - BUBBLE) * 2 - (Y - BUBBLE);
        float z = (rand() / float(RAND_MAX) - 1.5) * BARRACKS - BUBBLE;
        
        spheres[i] = vector3d(x, y, z);
    }
}


//Idle Func: While the game is running every time is called increases the nearDelta value moving the targets closer to the screen
void incomingEnemies()
{
    nearDelta += 0.035;
    glutPostRedisplay();
}


//Gets RGB integer value and returns appropiate color.
void trueColor(int red, int green, int blue)
{
    glColor3f(red/255.0, green/255.0, blue/255.0);
}

