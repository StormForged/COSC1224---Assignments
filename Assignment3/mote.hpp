#pragma once
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include "global.hpp"
#include "arena.hpp"
#include "shaders.h"

struct Mote {
    Real position[2];
    Real velocity[2];
    Real radius;
    Real mass;
    Real elasticity;
    GLUquadric *quadric;
    int slices, loops;
};

struct ActiveMotes{
    Mote* mote[maxMotes];
    int numMotes;
};

void initialiseMotes(ActiveMotes* activeMotes, Arena arena);
void displayMote(Mote *m, float sx, float sy, float sz, GLuint shaderProgram);
void displayMotes(ActiveMotes* activeMotes, Arena arena, GLuint shaderProgram);
void eulerStepSingleMote(Mote* m, Real h);
void integrateMotionMotes(ActiveMotes* activeMotes, Real h);
void updateMotes(ActiveMotes* activeMotes, Arena arena, Real elapsedTime, Real dt);
void createMote(ActiveMotes* activeMotes, Arena arena, int index, int dir);

void collideMoteWall(Mote* m, Arena &a);
Real calcMass(Mote** mote);
