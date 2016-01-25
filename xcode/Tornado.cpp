//
//  Tornado.cpp
//  ParticleSystemTest
//
//  Created by Adiel Fernandez on 1/19/16.
//
//

#include "Tornado.hpp"





Tornado::Tornado():
    //use initialization list for non-trivial types
    mPerlin(Perlin(1, Rand::randInt(Rand::randInt(1000))))
{
    
    drawFrame = false;
    height = 600;
    centerAmp = 400;
    verticalScale = 0.001;
    maxParticles = 200e3;
    numBackbonePts = 2000;
    ringCol = ColorA(1.0, 1.0, 1.0, 0.1);
    spineCol = ColorA(1.0, 0.0, 0.0, 1.0);
    active = false;
    damping = 0.945;
    
    minMass = 0.5;
    maxMass = 2.0;
    
    
    //use backbone to store location of tornado according to noise
    //this prevents each particle from having to call the perlin function
    //which is too slow to allow 200,000 particles
    for(int i = 0; i < numBackbonePts; i++){
        backbonePos.push_back(vec3(0, i, 0));
    }
    
    
    //--------------------PARTICLE DYNAMICS--------------------
    //attractionStrength = 9.2;    <----
    //rotationStrength = 1.06;     <---- these coefficients give good vortex behavior
    //liftStrength = 4.5;          <----
    
    attractionStrength = 9.2;
    rotationStrength = 1.06;
    liftStrength = 4.5;
    
    //speed of undulations of vortex
    vertTimeScale = 0.0999;
    
    //octaves of noise traversed going from bottom to top (bigger = more undulation)
    verticalScale = 0.00065;
    
    
}


void Tornado::setupEnvironment(float rad){
    
    groundRad = rad;
    
    //Dark brown: Red: 0.216 green: 0.169 blue: 0.110 alpha: 1.00
    //Darker brown: Red: 0.149 green: 0.102 blue: 0.063 alpha: 1.00]
    ColorA heavy(0.149, 0.102, 0.063, 0.8);
    
    
    //Lighter brown: Red:0.835 green:0.776 blue:0.667 alpha:1.00
    //light brown: Red: 0.694 green: 0.620 blue: 0.510 alpha:1.00
    ColorA light(0.694, 0.620, 0.510, 0.8);
    
    
    //create all particles in the beginning to front load the CPU time
    for(int i = 0; i < maxParticles; i++){

        //find a random place within a circle
        vec3 place(1, 0, 0);
        place *= Rand::randFloat(groundRad);
        place = glm::rotateY(place, Rand::randFloat(M_PI * 2));
        
        
        Particle p;
        
        p.pos = place;
        p.ppos = place;
        p.vel = vec3(0);
        p.acc = vec3(0);
        
        p.peaked = false;
        p.dying = false;
        p.dead = false;
        
        //give varied masses
        p.mass = Rand::randFloat(minMass,maxMass);
        
        //color according to particle mass
        p.col = light.lerp((p.mass - minMass)/(maxMass - minMass), heavy);

        //start at random ages so they don't all age out at once
        p.age = Rand::randInt(5 * 60);    //5 seconds @ 60 fps
        p.deathTimer = 0;
        
        particles.emplace_back(p);
        
    }
    
    
}


void Tornado::setupMesh(){
    
    //create the vbo with the right type, pass it the vector and set to
    //streaming so we can pass it data dynamically
    particleVbo = gl::Vbo::create(GL_ARRAY_BUFFER, particles, GL_STREAM_DRAW);
 
    //Setup how the information will be laid out within the buffer
    geom::BufferLayout layout;
    layout.append( geom::Attrib::POSITION, 3, sizeof(Particle), offsetof(Particle, pos));
    layout.append( geom::Attrib::COLOR , 4, sizeof(Particle), offsetof(Particle, col));
    
    
    //create the mesh, tell it how big, how to link points and tell
    //it how the buffer info is laid out and which vbo to use
    gl::VboMeshRef mesh;
    mesh = gl::VboMesh::create(particles.size(), GL_POINTS, { { layout, particleVbo } });
    
    //create a shader to feed into the batch. Just get a stock Cinder shader for now
    gl::GlslProgRef shader;
    shader = gl::getStockShader( gl::ShaderDef().color() );
    
    glPointSize(1.0);
    
    //assemble the batch
    particleBatch = gl::Batch::create(mesh, shader);
    
}


void Tornado::update(){
    
    
    //Noise is slow, so instead of calling for noise for every particle (200,000+ times),
    //load the noise positions into the tornado backbone then have particles refer to it there
    for(int i = 0; i < backbonePos.size(); i++){
        backbonePos[i] = getCenterPt(i);
    }
    
    
    for(vector<Particle>::iterator it = particles.begin(); it != particles.end(); ++it){
        
        if(it -> dead){
            
            //erasing is slow, let's just reset them
            
            vec3 place(1, 0, 0);
            place *= Rand::randFloat(groundRad);
            place = glm::rotateY(place, Rand::randFloat(M_PI * 2));
            
            it -> pos = vec3(place);
            it -> ppos = vec3(place);
            it -> vel = vec3(0);
            it -> dying = false;
            it -> dead = false;
            it -> peaked = false;
            it -> age = 0;
            it -> deathTimer = 0;
            
        } else if( it -> pos.y >= 0){
            
            //if we haven't peaked yet and are "undisturbed" count age
            //in order to re-cycle the unused particles on the fringe
            if(it -> peaked == false){
                it -> age ++;
            }
            
            //if we're old enough, we're dead
            if(it -> age > 300){     //300 = 5 seconds @ 60 fps
                it -> dead = true;
            }
            
            //find center of tornado

            //This line uses noise and is slow after 50,000+ particles
//            vec3 centerLoc = getCenterPt(it -> pos.y);
            
            //instead get the noise value stored in the vortex
            vec3 centerLoc = backbonePos[constrain((int)(it -> pos.y), 0, (int)backbonePos.size() - 1)];
            
            //get distance vector to center
            vec3 toCenter = centerLoc - it -> pos;
            
            float distSqToCenter = length2(toCenter);
            
            toCenter = normalize(toCenter);
            
            //first calculate percentage of depth into vortex:
            //0 = at edge, 1 = core
            
            float currentVRad = getVortexRad(it -> pos.y);
            float influenceRadSq = currentVRad * currentVRad;
            
            //normalized distance from edge of vortex to core
            float pct = 1 - constrain(distSqToCenter/influenceRadSq, 0.0f, 1.0f);
            
            //normalized distance from height of particle to ground
            float pctHeight = 1 - (it -> pos.y/height);
            
            //if we're inside the vortex...
            if(pct > 0){
                
                //We need THREE forces:
                //1) attraction to center
                //2) rotation around center
                //3) lift
                
                vec3 attraction = toCenter * attractionStrength * pct * pctHeight;
                vec3 rotation = cross(toCenter, vec3(0, 1, 0)) * rotationStrength * pct;
                vec3 lift = vec3(0, 1, 0) * liftStrength * pct;
                
                //if the vortex is active
                if(active){
                    it -> acc += (attraction + rotation + lift)/it -> mass;
                    
                }
                
            }
            
            
            //if we've been lifted high enough, we've "peaked"
            //and can start the regeneration process
            if(it -> pos.y > 30){
                it -> peaked = true;
            }
            
            //Also add gravity
            vec3 gravity;
            gravity = vec3(0, -3, 0);
            
            it -> acc += (gravity)/it -> mass;
            
            
            //now update all the particle kinematics
            it -> ppos = it -> pos;
            it -> vel += it -> acc;
            it -> pos += it -> vel;
            it -> vel *= damping;
            
            it -> acc = vec3(0);
            
            //if we're at the ground...
            if(it -> pos.y < 0){
                
                //bounce and lose energy
                it -> vel.y *= -0.95;
                it -> pos.y = 0;
                
                //if we've hit the ground after peaking, make it die
                if(it -> peaked){
                    it -> dying = true;
                }
                
            }
            
            //if we're dying (i.e. we've hit the ground after peaking)
            if(it -> dying){
                
                //start the death clock
                it -> deathTimer ++;
                
                //and reset
                if(it -> deathTimer > 100) it -> dead = true;
                
            }
            
        }
        
    }
    
    
    
    //copy the particle data to the GPU via the Vbo
    
    //first clear and map the Vbo to receive new data
    void *gpuMem = particleVbo -> mapReplace();
    
    //copy it over
    memcpy(gpuMem, particles.data(), particles.size() * sizeof(Particle));
    
    //now unmap the buffer
    particleVbo -> unmap();
    
    
    
    
}

void Tornado::draw(){
    
    if(drawFrame){
        float step = 10;
        
        
        //draw tornado envelope
        for(int i = 0; i < height - step; i += step){

            vec3 center = getCenterPt(i);
            vec3 nextCenter = getCenterPt(i + step);

//            vec3 center = backbonePos[i];
//            vec3 nextCenter = backbonePos[i + step];
            
            gl::color(spineCol);
            gl::drawLine(center, nextCenter);
            
            gl::pushMatrices();
            gl::rotate(-M_PI_2, vec3(1, 0, 0));
            gl::translate(0, 0, i);
            
            float thisRad = getVortexRad(i);
         
            gl::color(ringCol);
            gl::drawStrokedCircle(vec2(center.x, -center.z), thisRad, (int)(thisRad * 0.7));
            
            gl::popMatrices();
            
        }
        
        
        
    }
    
    //draw the "batch", i.e. the particle info in the Vbo
    //with the shader we set in the beginning
    particleBatch -> draw();
    
    
}

//This method sets the outside envelope of the tornado from an equation
//I created from an exponential + an inverse relationship in height.
//See Grapher file in assets for more details
float Tornado::getVortexRad(float h){
    
    float rad;
    
    float coreWidth = 17;
    
    rad = coreWidth + 5000/(h + 25) + 0.0008 * pow(h, 2);
    
    return rad;
    
}


//function returns the center point of the tornado at a given height
vec3 Tornado::getCenterPt(float h){
    
    float x, z;
    float amp = centerAmp * (1 + h/height);

    x = amp * mPerlin.noise(0, 0, getElapsedSeconds() * vertTimeScale - (h * verticalScale));
    //shift by 1000 to decouple x & y
    z = amp * mPerlin.noise(0, 1000, getElapsedSeconds() * vertTimeScale - (h * verticalScale));

    vec3 ctrPt(x, h, z);
    return ctrPt;
    
    
}






