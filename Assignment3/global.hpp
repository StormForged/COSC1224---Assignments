#pragma once
#include <math.h>
#include "shaders.h"

typedef double Real;
const Real epsilon = 1.0e-6;
const int maxMotes = 2000;
const int moteInit = 1000;

static Real random_uniform(){
    return rand()/(float)RAND_MAX;
}
