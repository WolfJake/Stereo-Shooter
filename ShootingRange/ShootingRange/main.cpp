#include <stdio.h>
#include <time.h>
#include <math.h>
#include "vector.h"
#include <GLUT/GLUT.h>
#include <OpenGL/OpenGL.h>


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


//Draws Scene
void draw()
{
    //Shooting Room
    glPushMatrix();
    glTranslatef(0.0, 0.0, -Z);
    
    glBegin(GL_POLYGON);
    trueColor(77, 52, 15);              glVertex3f(NORMALSIZE, -NORMALSIZE, -NORMALSIZE);
    trueColor(77, 52, 15);              glVertex3f(NORMALSIZE, NORMALSIZE, -NORMALSIZE);
    trueColor(77, 52, 15);              glVertex3f(-NORMALSIZE, NORMALSIZE, -NORMALSIZE);
    trueColor(77, 52, 15);              glVertex3f(-NORMALSIZE, -NORMALSIZE, -NORMALSIZE);
    glEnd();
    
    glBegin(GL_POLYGON);
    trueColor(77, 52, 15);              glVertex3f(NORMALSIZE, -NORMALSIZE, -NORMALSIZE);
    trueColor(77, 52, 15);              glVertex3f(NORMALSIZE, NORMALSIZE, -NORMALSIZE);
    trueColor(218, 148, 44);            glVertex3f(NORMALSIZE, NORMALSIZE, NORMALSIZE);
    trueColor(218, 148, 44);            glVertex3f(NORMALSIZE, -NORMALSIZE, NORMALSIZE);
    glEnd();
    
    glBegin(GL_POLYGON);
    trueColor(218, 148, 44);            glVertex3f(-NORMALSIZE, -NORMALSIZE, NORMALSIZE);
    trueColor(218, 148, 44);            glVertex3f(-NORMALSIZE, NORMALSIZE, NORMALSIZE);
    trueColor(77, 52, 15);              glVertex3f(-NORMALSIZE, NORMALSIZE, -NORMALSIZE);
    trueColor(77, 52, 15);              glVertex3f(-NORMALSIZE, -NORMALSIZE, -NORMALSIZE);
    glEnd();
    
    glBegin(GL_POLYGON);
    trueColor(218, 148, 44);            glVertex3f(NORMALSIZE, NORMALSIZE, NORMALSIZE);
    trueColor(77, 52, 15);              glVertex3f(NORMALSIZE, NORMALSIZE, -NORMALSIZE);
    trueColor(77, 52, 15);              glVertex3f(-NORMALSIZE, NORMALSIZE, -NORMALSIZE);
    trueColor(218, 148, 44);            glVertex3f(-NORMALSIZE, NORMALSIZE, NORMALSIZE);
    glEnd();
    
    glBegin(GL_POLYGON);
    trueColor(77, 52, 15);              glVertex3f(NORMALSIZE, -NORMALSIZE, -NORMALSIZE);
    trueColor(218, 148, 44);            glVertex3f(NORMALSIZE, -NORMALSIZE, NORMALSIZE);
    trueColor(218, 148, 44);            glVertex3f(-NORMALSIZE, -NORMALSIZE, NORMALSIZE);
    trueColor(77, 52, 15);              glVertex3f(-NORMALSIZE, -NORMALSIZE, -NORMALSIZE);
    glEnd();
    
    //Edge Bars
    trueColor(0, 0, 0);
    glPushMatrix();
    glTranslatef(NORMALSIZE, NORMALSIZE, 0);
    glScalef(1.0, 1.0, NORMALSIZE * 2 / 0.05);
    glutSolidCube(0.05);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(-NORMALSIZE, NORMALSIZE, 0);
    glScalef(1.0, 1.0, NORMALSIZE * 2 / 0.05);
    glutSolidCube(0.05);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(NORMALSIZE, -NORMALSIZE, 0);
    glScalef(1.0, 1.0, NORMALSIZE * 2 / 0.05);
    glutSolidCube(0.05);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(-NORMALSIZE, -NORMALSIZE, 0);
    glScalef(1.0, 1.0, NORMALSIZE * 2 / 0.05);
    glutSolidCube(0.05);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(NORMALSIZE, 0, -NORMALSIZE);
    glScalef(1.0, NORMALSIZE * 2 / 0.05, 1.0);
    glutSolidCube(0.05);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(-NORMALSIZE, 0, -NORMALSIZE);
    glScalef(1.0, NORMALSIZE * 2 / 0.05, 1.0);
    glutSolidCube(0.05);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(0, NORMALSIZE, -NORMALSIZE);
    glScalef(NORMALSIZE * 2 / 0.05, 1.0, 1.0);
    glutSolidCube(0.05);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(0, -NORMALSIZE, -NORMALSIZE);
    glScalef(NORMALSIZE * 2 / 0.05, 1.0, 1.0);
    glutSolidCube(0.05);
    glPopMatrix();
    
    
    //Shooting Targets
    for (int i = 0; i < VERSUS; i++)
    {
        //If target has been shot, it is colored red; otherwise colored white
        if (dead[i] == true)
        {
            glColor3f(1.0, 0.0, 0.0);
        } else {
            glColor3f(1.0, 1.0, 1.0);
        }
        glLoadName(i + 1);
        glPushMatrix();
        
        //Translate entire set of targets with a nearDelta value towards the screen.
        
        vector3d sphere = spheres[i];
        
        vector3d cruiseDirection = farBack - sphere;
        cruiseDirection.normalize();
        
        glTranslatef(sphere.x + cruiseDirection.x * nearDelta, sphere.y + cruiseDirection.y * nearDelta, sphere.z + cruiseDirection.z * nearDelta);
        glutSolidSphere(BUBBLE, 16, 16);
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
                
                float longShot = - INFINITY;
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

