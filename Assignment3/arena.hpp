#pragma once
#include "global.hpp"
#include <GL/gl.h>

struct Arena{
    Real min[2], max[2];
    Real momentum[2];
};

void initialiseArena(Arena* arena);
void displayArena(Arena* arena);
