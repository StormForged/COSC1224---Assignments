/*
 * Simple 3D sine wave animation example using glm
 * $Id: sinewave3D-glm.cpp,v 1.8 2017/08/23 12:56:02 gl Exp gl $
 */

#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#include "shaders.h"
#define GL_GLEXT_PROTOTYPES 

#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>


#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/constants.hpp"


typedef enum {
  d_drawSineWave,
  d_mouse,
  d_key,
  d_animation,
  d_lighting,
  d_OSD,
  d_matrices,
  d_computeLighting,
  d_nflags
} DebugFlags;

bool debug[d_nflags] = { false, false, false, false, false, false, false, false };

typedef struct { float r, g, b; } color3f;

typedef enum { line, fill } polygonMode_t;
typedef enum { grid, wave } shape_t;

typedef struct {
  shape_t shape;
  bool animate;
  float t, lastT;
  polygonMode_t polygonMode;
  bool lighting;
  bool fixed;
  bool twoside;
  bool drawNormals;
  int width, height;
  int tess;
  int waveDim;
  int frameCount;
  float frameRate;
  float displayStatsInterval;
  int lastStatsDisplayT;
  bool displayOSD;
  bool consolePM;
  bool multiView;
  bool shader;
  bool smooth;
  bool perPixel;
} Global;
Global g = { grid, false, 0.0, 0.0, line, true, false, false, false, 0, 0, 8, 2, 0, 0.0, 1.0, 0, false, false, false, false, true, true };

typedef enum { inactive, rotate, pan, zoom } CameraControl;

struct camera_t {
  int lastX, lastY;
  float rotateX, rotateY;
  float scale;
  CameraControl control;
} camera = { 0, 0, 30.0, -30.0, 1.0, inactive };

typedef struct {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
} Material; 

Material waveMaterial = {glm::vec3(0.2, 0.2, 0.2), glm::vec3(0.0, 0.5, 0.5), glm::vec3(0.8, 0.8, 0.8), 20};

glm::vec3 cyan(0.0, 1.0, 1.0);
glm::vec3 magenta(1.0, 0.0, 1.0);
glm::vec3 yellow(1.0, 1.0, 0.0);
glm::vec3 white(1.0, 1.0, 1.0);
glm::vec3 grey(0.8, 0.8, 0.8);
glm::vec3 black(0.0, 0.0, 0.0);
const float shininess = 50.0;

const float milli = 1000.0;

glm::mat4 modelViewMatrix;
glm::mat4 projectionMatrix;
glm::mat3 normalMatrix;

int err;

GLuint shaderProgram;

void printVec(float *v, int n)
{
  int i;

  for (i = 0; i < n; i++)
    printf("%5.3f ", v[i]); 
  printf("\n");
}

void printMatrixLinear(float *m, int n)
{
  int i;

  for (i = 0; i < n; i++)
    printf("%5.3f ", m[i]); 
  printf("\n");
}

void printMatrixColumnMajor(float *m, int n)
{
  int i, j;

  for (j = 0; j < n; j++) {
    for (i = 0; i < n; i++) {
      printf("%5.3f ", m[i*4+j]);
    }
    printf("\n");
  } 
  printf("\n");
}

void init(void) 
{
  glClearColor(0.0, 0.0, 0.0, 1.0);
  if (g.twoside)
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
  glEnable(GL_DEPTH_TEST);
  shaderProgram = getShader("./shader.vert", "./shader.frag");
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, &waveMaterial.ambient[0]);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, &waveMaterial.diffuse[0]);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &waveMaterial.specular[0]);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, waveMaterial.shininess);
}

void reshape(int w, int h)
{
  g.width = w;
  g.height = h;
  glViewport(0, 0, (GLsizei) w, (GLsizei) h); 
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-1.0, 1.0, -1.0, 1.0, -100.0, 100.0);
  projectionMatrix = glm::ortho(-1.0, 1.0, -1.0, 1.0, -100.0, 100.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

void drawAxes(float length)
{
  glm::vec4 v;

  glPushAttrib(GL_CURRENT_BIT);
  glBegin(GL_LINES);
  
  /* x axis */
  glColor3f(1.0, 0.0, 0.0);
  v = modelViewMatrix * glm::vec4(-length, 0.0, 0.0, 1.0);
  glVertex3fv(&v[0]);
  v = modelViewMatrix * glm::vec4(length, 0.0, 0.0, 1.0);
  glVertex3fv(&v[0]);

  /* y axis */
  glColor3f(0.0, 1.0, 0.0);
  v = modelViewMatrix * glm::vec4(0.0, -length, 0.0, 1.0);
  glVertex3fv(&v[0]);
  v = modelViewMatrix * glm::vec4(0.0, length, 0.0, 1.0);
  glVertex3fv(&v[0]);

  /* z axis */
  glColor3f(0.0, 0.0, 1.0);
  v = modelViewMatrix * glm::vec4(0.0, 0.0, -length, 1.0);
  glVertex3fv(&v[0]);
  v = modelViewMatrix * glm::vec4(0.0, 0.0, length, 1.0);
  glVertex3fv(&v[0]);

  glEnd();
  glPopAttrib();
}

void drawVector(glm::vec3 & o, glm::vec3 & v, float s, bool normalize, glm::vec3 & c)
{
  glPushAttrib(GL_CURRENT_BIT);
  glColor3fv(&c[0]);
  glBegin(GL_LINES);
  if (normalize) 
    v = glm::normalize(v);

  glVertex3fv(&o[0]);
  glm::vec3 e(o + s * v);
  glVertex3fv(&e[0]);
  glEnd();
  glPopAttrib();
}

// Console performance meter
void consolePM()
{
  printf("frame rate (f/s):  %5.0f\n", g.frameRate);
  printf("frame time (ms/f): %5.0f\n", 1.0 / g.frameRate * 1000.0);
  printf("tesselation:       %5d\n", g.tess);
}

// On screen display 
void displayOSD()
{
  char buffer[30];
  char *bufp;
  int w, h;
    
  glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();

  /* Set up orthographic coordinate system to match the window, 
     i.e. (0,0)-(w,h) */
  w = glutGet(GLUT_WINDOW_WIDTH);
  h = glutGet(GLUT_WINDOW_HEIGHT);
  glOrtho(0.0, w, 0.0, h, -1.0, 1.0);

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  /* Render mode */
  glColor3f(1.0, 1.0, 0.0);
  glRasterPos2i(10, 100);
  if(g.shader)
    snprintf(buffer, sizeof buffer, "Shader: on");
  else
    snprintf(buffer, sizeof buffer, "Shader: off");
  for(bufp = buffer; *bufp; bufp++)
    glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *bufp);


  /* Lighting calculation mode */
  glColor3f(1.0, 1.0, 0.0);
  glRasterPos2i(10, 80);
  if(g.fixed)
    snprintf(buffer, sizeof buffer, "Lighting calc mode: GPU(Fixed)");
  else
    snprintf(buffer, sizeof buffer, "Lighting calc mode: CPU");
  for(bufp = buffer; *bufp; bufp++)
    glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *bufp);

  /* Frame rate */
  glColor3f(1.0, 1.0, 0.0);
  glRasterPos2i(10, 60);
  snprintf(buffer, sizeof buffer, "frame rate (f/s):  %5.0f", g.frameRate);
  for (bufp = buffer; *bufp; bufp++)
    glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *bufp);

  /* Frame time */
  glColor3f(1.0, 1.0, 0.0);
  glRasterPos2i(10, 40);
  snprintf(buffer, sizeof buffer, "frame time (ms/f): %5.0f", 1.0 / g.frameRate * milli);
  for (bufp = buffer; *bufp; bufp++)
    glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *bufp);

  /* Tesselation */
  glColor3f(1.0, 1.0, 0.0);
  glRasterPos2i(10, 20);
  snprintf(buffer, sizeof buffer, "tesselation:       %5d", g.tess);
  for (bufp = buffer; *bufp; bufp++)
    glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *bufp);

  glPopMatrix();  /* Pop modelview */
  glMatrixMode(GL_PROJECTION);

  glPopMatrix();  /* Pop projection */
  glMatrixMode(GL_MODELVIEW);

glPopAttrib();
}


/* Perform ADS - ambient, diffuse and specular - lighting calculation
 * in eye coordinates (EC).
 */
glm::vec3 computeLighting(glm::vec3 & rEC, glm::vec3 & nEC)
{
  if (debug[d_computeLighting]) {
    printf("rEC ");
    printVec(&rEC[0], 3);
    printf("nEC ");
    printVec(&nEC[0], 3);
  }

  // Used to accumulate ambient, diffuse and specular contributions
  // Note: it is a vec3 being constructed with a single value which
  // is used for all 3 components
  glm::vec3 color(0.0);
  
  // Ambient contribution: A=La×Ma
  // Default light ambient color and default ambient material color
  // are both (0.2, 0.2, 0.2)
  glm::vec3 La(0.2);
  glm::vec3 Ma(0.2);
  glm::vec3 ambient(La * Ma);

  ambient *= waveMaterial.ambient;
  color += ambient;

  // Light direction vector. Default for LIGHT0 is a directional light
  // along z axis for all vertices, i.e. <0, 0, 1>
  glm::vec3 lEC( 0.0, 0.0, 1.0 );

  // Test if normal points towards light source, i.e. if polygon
  // faces toward the light - if not then nodiffuse or specular
  // con tribution
  float dp = glm::dot(nEC, lEC);
  if (dp > 0.0) {
    // Calculate diffuse and specular contribution
    
    // Lambert diffuse: D=Ld×Md×cosθ
    // Ld: default diffuse light color for GL_LIGHT0 is white (1.0, 1.0, 1.0).
    // Md: default diffuse material color is grey (0.8, 0.8, 0.8).
    glm::vec3 Ld(1.0);
    glm::vec3 Md(0.8);
    // Need normalized normal to calculate cosθ,
    // light vector <0, 0, 1> is already normalized 
    nEC = glm::normalize(nEC);
    float NdotL = glm::dot(nEC, lEC);
    glm::vec3 diffuse(Ld * Md * NdotL);
    
    diffuse *= waveMaterial.diffuse;
    color += diffuse;

    // Blinn-Phong specular: S=Ls×Ms×cosⁿα
    // Ls: default specular light color for LIGHT0 is white (1.0, 1.0, 1.0)
    // Ms: specular material color, also set to white (1.0, 1.0, 1.0),
    // but default for fixed pipeline is black, which means can't see
    // specular reflection. Need to set it to same value for fixed
    // pipeline lighting otherwise will look different.
    glm::vec3 Ls(1.0);
    glm::vec3 Ms(1.0);
    // Default viewer is at infinity along z axis <0, 0, 1> i.e. a
    // non local viewer (see glLightModel and GL_LIGHT_MODEL_LOCAL_VIEWER)
    glm::vec3 vEC(0.0, 0.0, 1.0);
    // Blinn-Phong half vector (using a single capital letter for
    // variable name!). Need normalized H (and nEC, above) to calculate cosα.
    glm::vec3 H(lEC + vEC);
    H = glm::normalize(H);
    float NdotH = glm::dot(nEC, H);
    if (NdotH < 0.0)
      NdotH = 0.0;
    glm::vec3 specular(Ls * Ms * powf(NdotH, shininess));

    specular *= waveMaterial.specular;
    color += specular;
  }

  return color;
}

void drawGrid(int tess)
{
  float stepSize = 2.0 / tess;
  glm::vec3 r, n, rEC, nEC;
  int i, j;

  if (g.lighting && g.fixed) {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);

    if(g.smooth)
      glShadeModel(GL_SMOOTH);
    else
      glShadeModel(GL_FLAT);

    if (g.twoside)
      glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    //glMaterialfv(GL_FRONT, GL_SPECULAR, &white[0]);
    //glMaterialf(GL_FRONT, GL_SHININESS, shininess);
  } else {
    glDisable(GL_LIGHTING);
    glColor3fv(&cyan[0]);
  }

  if (g.polygonMode == line)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  else
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  for (j = 0; j < tess; j++) {
    glBegin(GL_QUAD_STRIP);
    for (i = 0; i <= tess; i++) {
      r.x = -1.0 + i * stepSize;
      r.y = 0.0;
      r.z = -1.0 + j * stepSize;

      if (g.lighting) {
	      n.x = 0.0;
	      n.y = 1.0;
	      n.z = 0.0;
      }

      //Checks if shader is doing view calculations
      if(g.shader)
        rEC = glm::vec3(glm::vec4(r, 1.0));
      else
        rEC = glm::vec3(modelViewMatrix * glm::vec4(r, 1.0));

      if (g.lighting) {
        //Checks if shader is doing normal matrix calculations
        if(g.shader)
	        nEC = glm::normalize(n);
        else
          nEC = normalMatrix * glm::normalize(n);

	      if (g.fixed) {
	        glNormal3fv(&nEC[0]);
    	  } else {
	        glm::vec3 c = computeLighting(rEC, nEC);
	        glColor3fv(&c[0]);
	      }
      }
      glVertex3fv(&rEC[0]);

      r.z += stepSize;

      if(g.shader)
        rEC = glm::vec3(glm::vec4(r, 1.0));
      else
        rEC = glm::vec3(modelViewMatrix * glm::vec4(r, 1.0));

      if (g.lighting) {
        if(g.shader)
	        nEC = glm::normalize(n);
        else
          nEC = normalMatrix * glm::normalize(n);

	      if (g.fixed) {
	        glNormal3fv(&nEC[0]);
	      } else {
	        glm::vec3 c = computeLighting(rEC, nEC);
	        glColor3fv(&c[0]);
	      }
      }
      glVertex3fv(&rEC[0]);
    }

    glEnd();

  }

  if (g.lighting) {
    glDisable(GL_LIGHTING);
  }

  // Normals
  if (g.drawNormals) {
    for (j = 0; j <= tess; j++) {
      for (i = 0; i <= tess; i++) {
	      r.x = -1.0 + i * stepSize;
	      r.y = 0.0;
	      r.z = -1.0 + j * stepSize;
	
	      n.y = 1.0;
	      n.x = 0.0;
	      n.z = 0.0;
	      rEC = glm::vec3(modelViewMatrix * glm::vec4(r, 1.0));
	      nEC = normalMatrix * glm::normalize(n);
	      drawVector(rEC, nEC, 0.05, true, yellow);
      }
    }
  }

  while ((err = glGetError()) != GL_NO_ERROR) {
    printf("%s %d\n", __FILE__, __LINE__);
    printf("displaySineWave: %s\n", gluErrorString(err));
  }
}  
  
void drawSineWave(int tess)
{
  const float A1 = 0.25, k1 = 2.0 * M_PI, w1 = 0.25;
  const float A2 = 0.25, k2 = 2.0 * M_PI, w2 = 0.25;
  float stepSize = 2.0 / tess;
  glm::vec3 r, n, rEC, nEC;
  int i, j;
  float t = g.t;

  if (g.lighting && g.fixed) {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);

    if(g.smooth)
      glShadeModel(GL_SMOOTH);
    else
      glShadeModel(GL_FLAT);
  
    if (g.twoside)
      glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    glMaterialfv(GL_FRONT, GL_SPECULAR, &white[0]);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
  } else {
      glDisable(GL_LIGHTING);
      glColor3fv(&cyan[0]);
  }

  if (g.polygonMode == line)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  else
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  // Sine wave
  for (j = 0; j < tess; j++) {
    glBegin(GL_QUAD_STRIP);

    for (i = 0; i <= tess; i++) {
      r.x = -1.0 + i * stepSize;
      r.z = -1.0 + j * stepSize;

      //if(!g.shader){
        if (g.waveDim == 2) {
	        r.y = A1 * sinf(k1 * r.x + w1 * t);
	        if (g.lighting) {
	          n.x = - A1 * k1 * cosf(k1 * r.x + w1 * t);
	          n.y = 1.0;
	          n.z = 0.0;
	        }
        } else if (g.waveDim == 3) {
	        r.y = A1 * sinf(k1 * r.x + w1 * t) + A2 * sinf(k2 * r.z + w2 * t);
	        if (g.lighting) {
	          n.x = - A1 * k1 * cosf(k1 * r.x + w1 * t);
	          n.y = 1.0;
	          n.z = - A2 * k2 * cosf(k2 * r.z + w2 * t);
	        }
        }
      //}else{
      //  r.y = 0;
      //}

      //Checks if shader is doing view calculations
      if(g.shader)
        rEC = glm::vec3(glm::vec4(r, 1.0));
      else
        rEC = glm::vec3(modelViewMatrix * glm::vec4(r, 1.0));

      if (g.lighting) {
        //Checks to see if shader is doing normal matrix calculations
        if(g.shader)
	        nEC = glm::normalize(n);
        else
          nEC = normalMatrix * glm::normalize(n);

	      if (g.fixed) {
	        glNormal3fv(&nEC[0]);
	      } else {
	        glm::vec3 c = computeLighting(rEC, nEC);
	        glColor3fv(&c[0]);
	      }
      }

      glVertex3fv(&rEC[0]);

      r.z += stepSize;

      if (g.waveDim == 3) {
	      r.y = A1 * sinf(k1 * r.x + w1 * t) + A2 * sinf(k2 * r.z + w2 * t);
	      if (g.lighting) {
	        n.z = - A2 * k2 * cosf(k2 * r.z + w2 * t);
	      }
      }

      if(g.shader)
        rEC = glm::vec3(glm::vec4(r, 1.0));
      else
        rEC = glm::vec3(modelViewMatrix * glm::vec4(r, 1.0));

      if (g.lighting) {
        //Checks to see if shader is doing normal matrix calculations
        if(g.shader)
	        nEC = glm::normalize(n);
        else
          nEC = normalMatrix * glm::normalize(n);

	      if (g.fixed) {
	        glNormal3fv(&nEC[0]);
	      } else {
	        glm::vec3 c = computeLighting(rEC, nEC);
	        glColor3fv(&c[0]);
	      }
      }
      glVertex3fv(&rEC[0]);
    }

    glEnd();

  }

  if (g.lighting) {
    glDisable(GL_LIGHTING);
  }

  // Normals
  if (g.drawNormals) {
    for (j = 0; j <= tess; j++) {
      for (i = 0; i <= tess; i++) {
	r.x = -1.0 + i * stepSize;
	r.z = -1.0 + j * stepSize;

	n.y = 1.0;
	n.x = - A1 * k1 * cosf(k1 * r.x + w1 * t);
	if (g.waveDim == 2) {
	  r.y = A1 * sinf(k1 * r.x + w1 * t);
	  n.z = 0.0;
	} else {
	  r.y = A1 * sinf(k1 * r.x + w1 * t) + A2 * sinf(k2 * r.z + w2 * t);
	  n.z = - A2 * k2 * cosf(k2 * r.z + w2 * t);
	}

	rEC = glm::vec3(modelViewMatrix * glm::vec4(r, 1.0));
	nEC = normalMatrix * glm::normalize(n);
	drawVector(rEC, nEC, 0.05, true, yellow);
      }
    }
  }

  while ((err = glGetError()) != GL_NO_ERROR) {
    printf("%s %d\n", __FILE__, __LINE__);
    printf("displaySineWave: %s\n", gluErrorString(err));
  }
}

void idle()
{
  float t, dt;

  t = glutGet(GLUT_ELAPSED_TIME) / milli;

  // Accumulate time if animation enabled
  if (g.animate) {
    dt = t - g.lastT; 
    g.t += dt;
    g.lastT = t;
    if (debug[d_animation])
      printf("idle: animate %f\n", g.t);
  }

  // Update stats, although could make conditional on a flag set interactively
  dt = (t - g.lastStatsDisplayT);
  if (dt > g.displayStatsInterval) {
    g.frameRate = g.frameCount / dt;
    if (debug[d_OSD])
      printf("dt %f framecount %d framerate %f\n", dt, g.frameCount, g.frameRate);
    g.lastStatsDisplayT = t;
    g.frameCount = 0;
  }

  glutPostRedisplay();
}

void displayMultiView()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glm::mat4 modelViewMatrixSave(modelViewMatrix);
  glm::mat3 normalMatrixSave(normalMatrix);

  // Front view
  modelViewMatrix = glm::mat4(1.0);
  glViewport(g.width / 16.0, g.height * 9.0 / 16.0, g.width * 6.0 / 16.0, g.height * 6.0 / 16.0);
  drawAxes(5.0);
  if (g.shape == grid)
    drawGrid(g.tess);
  else
    drawSineWave(g.tess);


  // Top view
  modelViewMatrix = glm::mat4(1.0);
  modelViewMatrix = glm::rotate(modelViewMatrix, glm::pi<float>() / 2.0f, glm::vec3(1.0, 0.0, 0.0));
  glViewport(g.width / 16.0, g.height / 16.0, g.width * 6.0 / 16.0, g.height * 6.0 / 16);
  drawAxes(5.0);
  if (g.shape == grid)
    drawGrid(g.tess);
  else
    drawSineWave(g.tess);

  // Left view
  modelViewMatrix = glm::mat4(1.0);
  modelViewMatrix = glm::rotate(modelViewMatrix, glm::pi<float>() / 2.0f, glm::vec3(0.0, 1.0, 0.0));
  glViewport(g.width * 9.0 / 16.0, g.height * 9.0 / 16.0, g.width * 6.0 / 16.0, g.height * 6.0 / 16.0);
  drawAxes(5.0);
  if (g.shape == grid)
    drawGrid(g.tess);
  else
    drawSineWave(g.tess);

  // General view
  modelViewMatrix = glm::rotate(modelViewMatrix, camera.rotateX * glm::pi<float>() / 180.0f, glm::vec3(1.0, 0.0, 0.0));
  modelViewMatrix = glm::rotate(modelViewMatrix, camera.rotateY * glm::pi<float>() / 180.0f, glm::vec3(0.0, 1.0, 0.0));
  modelViewMatrix = glm::scale(modelViewMatrix, glm::vec3(camera.scale));
  normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelViewMatrix)));
  glViewport(g.width * 9.0 / 16.0, g.width / 16.0, g.width * 6.0 / 16.0, g.height * 6.0 / 16.0);
  drawAxes(5.0);
  if (g.shape == grid)
    drawGrid(g.tess);
  else
    drawSineWave(g.tess);

  if (g.displayOSD)
    displayOSD();

  if (g.consolePM)
    consolePM();

  g.frameCount++;
  
  glutSwapBuffers();

}
    
void display()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);

  glViewport(0, 0, g.width, g.height);

  // General view

  modelViewMatrix = glm::mat4(1.0);
  normalMatrix = glm::mat3(1.0);

  modelViewMatrix = glm::rotate(modelViewMatrix, camera.rotateX * glm::pi<float>() / 180.0f, glm::vec3(1.0, 0.0, 0.0));
  modelViewMatrix = glm::rotate(modelViewMatrix, camera.rotateY * glm::pi<float>() / 180.0f, glm::vec3(0.0, 1.0, 0.0));
  modelViewMatrix = glm::scale(modelViewMatrix, glm::vec3(camera.scale));

  normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelViewMatrix)));

  if (debug[d_matrices]) {
    printf("modelViewMatrix\n");
    printMatrixColumnMajor(&modelViewMatrix[0][0], 4);
    printf("normalMatrix\n");
    printMatrixColumnMajor(&normalMatrix[0][0], 3);
  }

  drawAxes(5.0);
  if(g.shader){
    glUseProgram(shaderProgram);
    
    //All the shaders uniform values
    GLint mvMatLoc = glGetUniformLocation(shaderProgram, "mvMat");
    glUniformMatrix4fv(mvMatLoc, 1, false, &modelViewMatrix[0][0]);
    GLint pMatLoc = glGetUniformLocation(shaderProgram, "pMat");
    glUniformMatrix4fv(pMatLoc, 1, false, &projectionMatrix[0][0]);
    GLint nMatLoc = glGetUniformLocation(shaderProgram, "nMat");
    glUniformMatrix3fv(nMatLoc, 1, false, &normalMatrix[0][0]);
    GLint tLoc = glGetUniformLocation(shaderProgram, "t");
    glUniform1f(tLoc, g.t);
    GLint gridLoc = glGetUniformLocation(shaderProgram, "grid");
    glUniform1i(gridLoc, g.shape);
    GLint perPixelLoc = glGetUniformLocation(shaderProgram, "perPixel");
    glUniform1i(perPixelLoc, g.perPixel);

    //Uniform for material
    GLint waveAmbLoc = glGetUniformLocation(shaderProgram, "waveAmbient");
    glUniform3fv(waveAmbLoc, 1, &waveMaterial.ambient[0]);
    GLint waveDifLoc = glGetUniformLocation(shaderProgram, "waveDiffuse");
    glUniform3fv(waveDifLoc, 1, &waveMaterial.diffuse[0]);
    GLint waveSpecLoc = glGetUniformLocation(shaderProgram, "waveSpecular");
    glUniform3fv(waveSpecLoc, 1, &waveMaterial.specular[0]);
    
    if (g.shape == grid)
      drawGrid(g.tess);
    else
      drawSineWave(g.tess);

    glUseProgram(0);

  }else{
    if (g.shape == grid)
      drawGrid(g.tess);
    else
      drawSineWave(g.tess);
  }

  if (g.displayOSD)
    displayOSD();

  if (g.consolePM)
    consolePM();

  glutSwapBuffers();

  g.frameCount++;

  while ((err = glGetError()) != GL_NO_ERROR) {
    printf("%s %d\n", __FILE__, __LINE__);
    printf("display: %s\n", gluErrorString(err));
  }
}
    
void keyboard(unsigned char key, int x, int y)
{
  switch (key) {
  case 27:
    printf("exit\n");
    exit(0);
    break;
  case 'a':
    g.animate = !g.animate;
    if (g.animate) {
      g.lastT = glutGet(GLUT_ELAPSED_TIME) / milli;
    } 
    break;
  case 'l':
    g.lighting = !g.lighting;
    glutPostRedisplay();
    break;
  case 'f':
    g.fixed = !g.fixed;
    glutPostRedisplay();
    break;
  case 'm':
    printf("%d\n", g.polygonMode);
    if (g.polygonMode == line)
      g.polygonMode = fill;
    else
      g.polygonMode = line;
    glutPostRedisplay();
    break;
  case 'n':
    g.drawNormals = !g.drawNormals;
    glutPostRedisplay();
    break;
  case 'c':
    g.consolePM = !g.consolePM;
    glutPostRedisplay();
    break;
  case 'o':
    g.displayOSD = !g.displayOSD;
    glutPostRedisplay();
    break;
  case 's':
    g.shape = g.shape == grid ? wave : grid;
    g.animate = false;
    break;
  case '4':
    g.multiView = !g.multiView;
    if (g.multiView)
      glutDisplayFunc(displayMultiView);
    else
      glutDisplayFunc(display);
    glutPostRedisplay();
    break;
  case 'v':
    printf("vbos not implemented\n");
    break;
  case '+':
    g.tess *= 2;
    glutPostRedisplay();
    break;
  case '-':
    g.tess /= 2;
    if (g.tess < 8)
      g.tess = 8;
    glutPostRedisplay();
    break;
  case 'z':
    g.waveDim++;
    if (g.waveDim > 3)
      g.waveDim = 2;
    glutPostRedisplay();
    break;
  case 'g':
    g.shader = !g.shader;
    glutPostRedisplay();
    break;
  case 'b':
    g.smooth = !g.smooth;
    glutPostRedisplay();
    break;
  case 'p':
    g.perPixel = !g.perPixel;
    glutPostRedisplay();
    break;
  default:
    break;
  }
}

void mouse(int button, int state, int x, int y)
{
  if (debug[d_mouse])
    printf("mouse: %d %d %d\n", button, x, y);

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

  if (debug[d_mouse]) {
    printf("motion: %d %d\n", x, y);
    printf("camera.rotate: %f %f\n", camera.rotateX, camera.rotateY);
    printf("camera.scale:%f\n", camera.scale);
  }

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
    camera.scale += dy / 100.0;
    break;
  }

  glutPostRedisplay();
}

int main(int argc, char** argv)
{
  glutInit(&argc, argv);
  glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize (1024, 1024); 
  glutInitWindowPosition (100, 100);
  glutCreateWindow (argv[0]);
  init();
  glutDisplayFunc(display); 
  glutReshapeFunc(reshape);
  glutIdleFunc(idle);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutMainLoop();
  return 0;
}
