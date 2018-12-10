#include "arena.hpp"

void initialiseArena(Arena* arena){
  const Real halfLength = 30;

  arena->min[0] = -halfLength;
  arena->min[1] = -halfLength;
  arena->max[0] = halfLength;
  arena->max[1] = halfLength;

  arena->momentum[0] = 0.0;
  arena->momentum[1] = 0.0;
}

void displayArena(Arena* arena){
  glBegin(GL_LINE_LOOP);
  glVertex3f(arena->min[0], arena->min[1], 0.0);
  glVertex3f(arena->max[0], arena->min[1], 0.0);
  glVertex3f(arena->max[0], arena->max[1], 0.0);
  glVertex3f(arena->min[0], arena->max[1], 0.0);
  glEnd();
}
