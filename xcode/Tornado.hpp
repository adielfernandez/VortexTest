//
//  Tornado.hpp
//  ParticleSystemTest
//
//  Created by Adiel Fernandez on 1/19/16.
//
//

#ifndef Tornado_hpp
#define Tornado_hpp

#include <stdio.h>

#endif /* Tornado_h */


#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Perlin.h"
#include "cinder/Rand.h"
#include "cinder/gl/VboMesh.h"
#include "cinder/CinderGlm.h"
#include "cinder/CinderMath.h"


#pragma once

using namespace ci;
using namespace ci::app;
using namespace std;

struct Particle{
    
    vec3 pos, ppos, vel, acc;
    
    float mass;
    
    bool peaked;
    bool dying;
    bool dead;
    
    int age, deathTimer;
    
    ColorA col;
    
};


class Tornado{
    
public:
    
    Tornado();
    void setupEnvironment(float rad);
    void setupMesh();
    void update();
    vec3 getCenterPt(float h);
    float getVortexRad(float h);
    void draw();
    
    
    //Draw tornado outline
    bool drawFrame;
    
    //Activate tornado influence
    bool active;
    
    //vortex dynamics
    float liftForce;
    float attractionStrength;
    float rotationStrength;
    float liftStrength;
    float verticalScale;
    float vertTimeScale;
    Perlin mPerlin;

    float centerAmp;
    float influenceRad;

    //Vortex Structure
    int numBackbonePts;
    vector<vec3> backbonePos;
    ColorA ringCol, spineCol;
    float groundRad;
    float height;
    
    //Particles
    vector<Particle> particles;
    int maxParticles;
    float minMass, maxMass;
    float damping;
    
    //VBO and Batch structures
    //using the GPU to draw
    gl::VboRef particleVbo;
    gl::BatchRef particleBatch;
    
    

    
    
    
};