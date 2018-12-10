#include <math.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <limits.h>
#include <stdlib.h>

#include "global.hpp"
#include "arena.hpp"
#include "mote.hpp"

GLuint shaderProgram;

ActiveMotes activeMotes;
Arena arena;

enum renderMode {wire, solid};
static renderMode renMode = solid;
static Real elapsedTime = 0.0, startTime = 0.0, dt, lastT;
static const int milli = 1000;
static bool win = false;
static bool lose = false;

typedef enum { inactive, rotate, pan, zoom } CameraControl;

struct camera_t {
  int lastX, lastY;
  float rotateX, rotateY;
  float scale;
  float posX, posY;
  CameraControl control;
} camera = { 0, 0, 30.0, -30.0, 0.2, 0, 0, inactive };

void setRenderMode(renderMode rm){
    if(rm == wire){
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_NORMALIZE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else if(rm == solid){
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_NORMALIZE);
        glShadeModel(GL_SMOOTH);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void changeRenderMode(void){
    if(renMode == wire){
        renMode = solid;
    } else{
        renMode = wire;
    }
    setRenderMode(renMode);
}

void displayOSD(int frameNo){
    static const Real interval = 1.0;
    static Real frameRateInterval = 0.0;
    static int frameNoStartInterval = 0;
    static Real elapsedTimeStartInterval = 0.0;
    static char buffer[0];
    int len, i;

    if(elapsedTime < interval)
        return;

    if(elapsedTime > elapsedTimeStartInterval + interval){
        frameRateInterval = (frameNo - frameNoStartInterval) /
            (elapsedTime = elapsedTimeStartInterval);
        elapsedTimeStartInterval = elapsedTime;
        frameNoStartInterval = frameNo;
    }

    sprintf(buffer, "Mass: %f", activeMotes.mote[0]->mass);
    glRasterPos2f(-10,-9);
    len = (int)strlen(buffer);
    for(i = 0; i < len; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, buffer[i]);

    if(win){
        glColor3f(1.0, 0.0, 1.0);
        sprintf(buffer, "WINNER WINNER");
        glRasterPos2f(0,0);
        len = (int)strlen(buffer);
        for(i = 0; i < len; i++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, buffer[i]);   
    }

    if(lose){
        glColor3f(1.0, 0.0, 1.0);
        sprintf(buffer, "GAME OVER");
        glRasterPos2f(0,0);
        len = (int)strlen(buffer);
        for(i = 0; i < len; i++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, buffer[i]);   
    }
}

void update(void){
    if(win != true || lose != true){
        float bestMass;
        elapsedTime = glutGet(GLUT_ELAPSED_TIME) / (Real)milli - startTime;

        dt = elapsedTime - lastT;
        lastT = elapsedTime;

        updateMotes(&activeMotes, arena, elapsedTime, dt);

        //Camera just follows the mote
        camera.posX = activeMotes.mote[0]->position[0];
        camera.posY = activeMotes.mote[0]->position[1];

        for(int i = 1; i < maxMotes; i++)
            if(activeMotes.mote[i] != NULL)
                if(activeMotes.mote[i]->mass > bestMass)
                    bestMass = activeMotes.mote[i]->mass;

        if(activeMotes.mote[0]->mass > bestMass)
            win = true;

        //Due to how ejection works the mote cannot reach 0
        if(activeMotes.mote[0]->mass <= 0.01)
            lose = true;
    }

    glutPostRedisplay();
}

void display(void){
    static int frameNo = 0;
    GLenum err;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glColor3f(0.8, 0.8, 0.8);

    glPushMatrix();

    glScalef(camera.scale, camera.scale, camera.scale);
    glTranslatef(-camera.posX, -camera.posY, 0.0);

    //Display mote and arena
    glPushMatrix();
    displayArena(&arena);
    displayMotes(&activeMotes, arena, shaderProgram);
    glPopMatrix();

    glPopMatrix();
    //Display OSD
    glPushMatrix();
    displayOSD(frameNo);
    glPopMatrix();
    frameNo++;

    glPopMatrix();

    glutSwapBuffers();
}

void myInit(void){
    setRenderMode(renMode);
    initialiseArena(&arena);
    initialiseMotes(&activeMotes, arena);

    shaderProgram = getShader("./shader.vert", "./shader.frag");

    camera.posX = activeMotes.mote[0]->position[0];
    camera.posY = activeMotes.mote[0]->position[1];
}

void keyboardCB(unsigned char key, int x, int y){
    switch(key){
        case 'q':
            exit(EXIT_SUCCESS);
            break;
        case 'w':
            activeMotes.mote[0]->velocity[1] -= 0.1;
            for(int i = 0; i < maxMotes; i++)
              if(activeMotes.mote[i] == NULL){
                  createMote(&activeMotes, arena, i, 0);
                  break;
              }
            break;
        case 's':
            activeMotes.mote[0]->velocity[1] += 0.1;
            for(int i = 0; i < maxMotes; i++)
              if(activeMotes.mote[i] == NULL){
                  createMote(&activeMotes, arena, i, 2);
                  break;
              }
            break;
        case 'a':
            activeMotes.mote[0]->velocity[0] += 0.1;
            for(int i = 0; i < maxMotes; i++)
              if(activeMotes.mote[i] == NULL){
                  createMote(&activeMotes, arena, i, 1);
                  break;
              }
            break;
        case 'd':
            activeMotes.mote[0]->velocity[0] -= 0.1;
            for(int i = 0; i < maxMotes; i++)
              if(activeMotes.mote[i] == NULL){
                  createMote(&activeMotes, arena, i, 3);
                  break;
              }
            break;
        case 'o':
          camera.scale += 0.1;
          break;
        case 'l':
          if(camera.scale > 0.2)
          camera.scale -= 0.1;
          break;
        default:
            break;
    }
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
  camera.lastX = x;
  camera.lastY = y;

  if (state == GLUT_DOWN)
    switch(button) {
    case GLUT_LEFT_BUTTON:
      camera.control = rotate;
      break;
    case GLUT_MIDDLE_BUTTON:
      camera.control = pan;
      break;
    case GLUT_RIGHT_BUTTON:
      camera.control = zoom;
      break;
    }
  else if (state == GLUT_UP)
    camera.control = inactive;
}

void motion(int x, int y)
{
  float dx, dy;

  dx = x - camera.lastX;
  dy = y - camera.lastY;
  camera.lastX = x;
  camera.lastY = y;

  switch (camera.control) {
  case inactive:
    break;
  case rotate:
    camera.rotateX += dy;
    camera.rotateY += dx;
    break;
  case pan:
    break;
  case zoom:
    if(camera.scale >= 0.1)
      camera.scale += dy / 100.0;
    else
      camera.scale = 0.1;
    break;
  }

  glutPostRedisplay();
}

void myReshape(int w, int h){
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(-10.0, 10.0, -10.0, 10.0, -10.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char** argv){
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280, 720);
    glutInitWindowPosition(500, 500);
    glutCreateWindow("Nosmos");
    glutIdleFunc(update);
    glutDisplayFunc(display);
    glutReshapeFunc(myReshape);
    glutKeyboardFunc(keyboardCB);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    myInit();

    glutMainLoop();
}
