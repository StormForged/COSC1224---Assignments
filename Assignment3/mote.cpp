#include "mote.hpp"

void initialiseMotes(ActiveMotes* activeMotes, Arena arena){
  GLUquadric *quadric = gluNewQuadric();
  const Real maxVelocity = 1.0;
  Real n[2], n_mag_sq, sum_radii, sum_radii_sq;
  bool collision, done;
  int i, j;

  // MOTE 0 IS ALWAYS PLAYER
  activeMotes->mote[0] = (Mote*)malloc(sizeof(Mote));
  activeMotes->mote[0]->velocity[0] = 0.0;
  activeMotes->mote[0]->velocity[1] = 0.0;
  activeMotes->mote[0]->mass = 0.1;
  activeMotes->mote[0]->radius = sqrt(activeMotes->mote[0]->mass);
  activeMotes->mote[0]->elasticity = 1.0;
  activeMotes->mote[0]->quadric = quadric;
  activeMotes->mote[0]->slices = 10;
  activeMotes->mote[0]->loops = 3;

  activeMotes->mote[0]->position[0] = 0.5;
  activeMotes->mote[0]->position[1] = 0.5;

  activeMotes->numMotes++;

  for(i = 1; i < moteInit - 1; i++){
    activeMotes->mote[i] = (Mote*)malloc(sizeof(Mote));
    activeMotes->mote[i]->velocity[0] = (random_uniform() - 0.5) * maxVelocity;
    activeMotes->mote[i]->velocity[1] = (random_uniform() - 0.5) * maxVelocity;
    activeMotes->mote[i]->mass = random_uniform() * 0.1;
    activeMotes->mote[i]->radius = sqrt(activeMotes->mote[i]->mass);
    activeMotes->mote[i]->elasticity = 1.0;
    activeMotes->mote[i]->quadric = quadric;
    activeMotes->mote[i]->slices = 10;
    activeMotes->mote[i]->loops = 3;
    done = false;
    while(!done){
      activeMotes->mote[i]->position[0] = random_uniform() *
        (arena.max[0] - arena.min[0] - 2.0 * activeMotes->mote[i]->radius) +
        arena.min[0] + activeMotes->mote[i]->radius + epsilon;
      activeMotes->mote[i]->position[1] = random_uniform() *
        (arena.max[1] - arena.min[1] - 2.0 * activeMotes->mote[i]->radius) +
        arena.min[1] + activeMotes->mote[i]->radius + epsilon;

      //Check for collision with existing motes
      collision = false;
      j = 0;
      while(!collision && j < i){
        sum_radii = activeMotes->mote[i]->radius + activeMotes->mote[i]->radius;
        sum_radii_sq = sum_radii * sum_radii;
        n[0] = activeMotes->mote[j]->position[0] - activeMotes->mote[i]->position[0];
        n[1] = activeMotes->mote[j]->position[1] - activeMotes->mote[i]->position[1];
        n_mag_sq = n[0] * n[0] + n[1] * n[1];
        if(n_mag_sq < sum_radii_sq)
          collision = true;
        else
          j++;
      }
      if(!collision)
        done = true;
    }
    activeMotes->numMotes++;
  }
}

void displayMote(Mote *m, float sx, float sy, float sz, GLuint shaderProgram){
  glUseProgram(shaderProgram);

  GLint massLoc = glGetUniformLocation(shaderProgram, "mass");
  glUniform1f(massLoc, m->mass);

  glPushMatrix();
  glScalef(sx, sy, sz);
  gluDisk(m->quadric, 0.0, m->radius, m->slices, m->loops);
  glPopMatrix();

  glUseProgram(0);
}

void displayMotes(ActiveMotes* activeMotes, Arena arena, GLuint shaderProgram){
  int i;
  ActiveMotes *m;
  m = activeMotes;
  //display particles
  for(i = 0; i < maxMotes; i++){
    if(m->mote[i] != NULL){
      glPushMatrix();
      glTranslatef(m->mote[i]->position[0], m->mote[i]->position[1], 0.0);
      if(i == 0)
        glColor3f(1.0, 0.2, 0.2);
      else
        glColor3f(0.8, 0.8, 0.8);
      displayMote(m->mote[i], 1.0, 1.0, 1.0, shaderProgram);
      glPopMatrix();
    }
  }
}

void eulerStepSingleMote(Mote* m, Real h){
  m->position[0] += h * m->velocity[0];
  m->position[1] += h * m->velocity[1];
}

void integrateMotionMotes(ActiveMotes* activeMotes, Real h){
  for(int i = 0; i < maxMotes; i++)
    if(activeMotes->mote[i] != NULL)
      eulerStepSingleMote(activeMotes->mote[i], h);
}

void collideMoteWall(Mote* m, Arena &a){
  float dp[2];

  dp[0] = dp[1] = 0.0;
  if((m->position[0] - m->radius) < a.min[0]){
    m->position[0] +=
      2.0 * (a.min[0] - (m->position[0] - m->radius));
    m->velocity[0] *= -1.0;
    dp[0] += m->mass * -2.0 * m->velocity[0];
  }
  if((m->position[1] - m->radius) < a.min[1]){
    m->position[1] +=
      2.0 * (a.min[1] - (m->position[1] - m->radius));
    m->velocity[1] *= -1.0;
    dp[1] += m->mass * -2.0 * m->velocity[1];
  }
  if((m->position[0] + m->radius) > a.max[0]){
    m->position[0] -=
      2.0 * (m->position[0] + m->radius - a.max[0]);
    m->velocity[0] *= -1.0;
    dp[0] += m->mass * -2.0 * m->velocity[0];
  }
  if((m->position[1] + m->radius) > a.max[1]){
    m->position[1] -=
      2.0 * (m->position[1] + m->radius - a.max[1]);
    m->velocity[1] *= -1.0;
    dp[1] += m->mass * -2.0 * m->velocity[1];
  }
}

void collideMotesWall(ActiveMotes activeMotes, Arena arena){
  for(int i = 0; i < maxMotes; i++)
    if(activeMotes.mote[i] != NULL)
      collideMoteWall(activeMotes.mote[i], arena);
}

inline bool collisionDetectionMotes(Mote m1, Mote m2){
  Real sum_radii, sum_radii_sq, n[2], n_mag_sq;

  sum_radii = m1.radius + m2.radius;
  sum_radii_sq = sum_radii * sum_radii;
  n[0] = m2.position[0] - m1.position[0];
  n[1] = m2.position[1] - m1.position[1];
  n_mag_sq = n[0] * n[0] + n[1] * n[1];
  if(n_mag_sq <= sum_radii_sq)
    return true;
  else
    return false;
}

void collisionReactionMotes2DbasisChange(Mote* mote1, Mote* mote2){
  Real n[2], t[2], n_mag;
  Real v1_nt[2], v2_nt[2];
  Real m1, m2, v1i, v2i, v1f, v2f, vf;
  Real massChange;

  //Normal vector n between centres
  n[0] = mote2->position[0] - mote1->position[0];
  n[1] = mote2->position[1] - mote1->position[1];

  //Normalise n
  n_mag = sqrt(n[0] * n[0] + n[1] * n[1]);
  n[0] /= n_mag;
  n[1] /= n_mag;

  //Tangent vector t
  t[0] = -n[1];
  t[1] = n[0];

  //Change basis for velocities from standard basis to nt basis
  v1_nt[0] = n[0] * mote1->velocity[0] + n[1] * mote1->velocity[1];
  v1_nt[1] = t[0] * mote1->velocity[0] + t[1] * mote1->velocity[1];
  v2_nt[0] = n[0] * mote2->velocity[0] + n[1] * mote2->velocity[1];
  v2_nt[1] = t[0] * mote2->velocity[0] + t[1] * mote2->velocity[1];

  if(mote1->mass > mote2->mass){
    massChange = mote2->mass * 0.01;
    mote1->mass += massChange;
    mote2->mass -= massChange;
    mote1->radius = sqrt(mote1->mass);
    mote2->radius = sqrt(mote2->mass);
  }else if(mote2->mass > mote1->mass){
    massChange = mote1->mass * 0.01;
    mote2->mass += massChange;
    mote1->mass -= massChange;
    mote1->radius = sqrt(mote1->mass);
    mote2->radius = sqrt(mote2->mass);
  }

  //Use 1D equations to calculate final velocities in n direction
  m1 = mote1->mass;
  m2 = mote2->mass;
  v1i = v1_nt[0];
  v2i = v2_nt[0];

  //Change back to standard basis
  mote1->velocity[0] = n[0] * v1_nt[0] + t[0] * v1_nt[1];
  mote1->velocity[1] = n[1] * v1_nt[0] + t[1] * v1_nt[1];
  mote2->velocity[0] = n[0] * v2_nt[0] + t[0] * v2_nt[1];
  mote2->velocity[1] = n[1] * v2_nt[0] + t[1] * v2_nt[1];
}

void collideMotesBruteForce(ActiveMotes activeMotes, Arena arena, Real h){
  int i, j;

  for(i = 0; i < maxMotes; i++){
    if(activeMotes.mote[i] == NULL)
      break;
    for(j = i + 1; j < maxMotes; j++){
      if(activeMotes.mote[j] == NULL)
        break;
      if(collisionDetectionMotes(*activeMotes.mote[i], *activeMotes.mote[j])){
        //Take step back. Better approaches possible
        eulerStepSingleMote(activeMotes.mote[i], -h);
        eulerStepSingleMote(activeMotes.mote[j], -h);

        collisionReactionMotes2DbasisChange(activeMotes.mote[i], activeMotes.mote[j]);

        //Step forward
        eulerStepSingleMote(activeMotes.mote[i], h);
        eulerStepSingleMote(activeMotes.mote[j], h);

        //Check walls
        collideMoteWall(activeMotes.mote[i], arena);
        collideMoteWall(activeMotes.mote[j], arena);
      }
    }
  }
}

void destroyMote(Mote* mote){
  mote = NULL;
  free(mote);
}

void updateMotes(ActiveMotes* activeMotes, Arena arena, Real elapsedTime, Real dt){
  static Real time = 0.0, h;

  //Adjust the size of the motes before calculation
  for(int i = 0; i < maxMotes; i++)
    if(activeMotes->mote[i] != NULL)
      activeMotes->mote[i]->radius = sqrt(activeMotes->mote[i]->mass);
  //Calculate time increment
  time = elapsedTime;
  h = dt;

  //Compute new positions of motes
  integrateMotionMotes(activeMotes, h);

  //collisions against walls & motes
  collideMotesWall(*activeMotes, arena);
  collideMotesBruteForce(*activeMotes, arena, h);

  //Destroy any motes that have 0 mass
  for(int i = 0; i < maxMotes; i++){
    if(activeMotes->mote[i] != NULL)
      if(activeMotes->mote[i]->mass <= 0){
          destroyMote(activeMotes->mote[i]);
          activeMotes->numMotes--;
      }
  }
}

void createMote(ActiveMotes* activeMotes, Arena arena, int index, int dir){
  GLUquadric *quadric = gluNewQuadric();
  Real n[2], n_mag_sq, sum_radii, sum_radii_sq;
  bool collision, done;

  //W = 0, A = 1, S = 2, D = 3

  activeMotes->mote[index] = (Mote*)malloc(sizeof(Mote));
  activeMotes->mote[index]->mass = activeMotes->mote[0]->mass / 50;
  activeMotes->mote[index]->radius = sqrt(activeMotes->mote[index]->mass);
  activeMotes->mote[index]->elasticity = 1.0;
  activeMotes->mote[index]->quadric = quadric;
  activeMotes->mote[index]->slices = 10;
  activeMotes->mote[index]->loops = 3;

  //THESE ARE ALL ARBITRARY AS HELL

  if(dir == 0){
    activeMotes->mote[index]->position[0] = activeMotes->mote[0]->position[0] + activeMotes->mote[0]->radius - (activeMotes->mote[0]->radius);
    activeMotes->mote[index]->position[1] = activeMotes->mote[0]->position[1] + activeMotes->mote[0]->radius + (activeMotes->mote[0]->radius / 2);
    activeMotes->mote[index]->velocity[0] = 0.0;
    activeMotes->mote[index]->velocity[1] = 1.0;
  }else if(dir == 1){
    activeMotes->mote[index]->position[0] = activeMotes->mote[0]->position[0] + activeMotes->mote[0]->radius - (activeMotes->mote[0]->radius * 2.2);
    activeMotes->mote[index]->position[1] = activeMotes->mote[0]->position[1] + activeMotes->mote[0]->radius - (activeMotes->mote[0]->radius);
    activeMotes->mote[index]->velocity[0] = -1.0;
    activeMotes->mote[index]->velocity[1] = 0.0;
  }else if(dir == 2){
    activeMotes->mote[index]->position[0] = activeMotes->mote[0]->position[0] + activeMotes->mote[0]->radius - (activeMotes->mote[0]->radius);
    activeMotes->mote[index]->position[1] = activeMotes->mote[0]->position[1] + activeMotes->mote[0]->radius - (activeMotes->mote[0]->radius * 2.2);
    activeMotes->mote[index]->velocity[0] = 0.0;
    activeMotes->mote[index]->velocity[1] = -1.0;
  }else if(dir == 3){
    activeMotes->mote[index]->position[0] = activeMotes->mote[0]->position[0] + activeMotes->mote[0]->radius + (activeMotes->mote[0]->radius / 2);
    activeMotes->mote[index]->position[1] = activeMotes->mote[0]->position[1] + activeMotes->mote[0]->radius - (activeMotes->mote[0]->radius);
    activeMotes->mote[index]->velocity[0] = 1.0;
    activeMotes->mote[index]->velocity[1] = 0.0;
  }

  activeMotes->numMotes++;

  activeMotes->mote[0]->mass -= activeMotes->mote[index]->mass;
}