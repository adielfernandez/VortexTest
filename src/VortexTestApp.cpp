#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Shader.h"
#include "cinder/Utilities.h"
#include "cinder/CameraUi.h"

#include "Tornado.hpp"

using namespace ci;
using namespace ci::app;
using namespace std;

class VortexTestApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
    void keyDown( KeyEvent event ) override;
	void update() override;
	void draw() override;
    
    
    CameraPersp cam;
    CameraUi camUI;

    Tornado vortex;
    
    float groundRad;
    bool full;
    bool cursor;
    
};

void VortexTestApp::setup(){
    
    setWindowSize(1440, 900);
    setFrameRate(60);
    
    gl::enableVerticalSync();
    gl::enableDepthRead();
    gl::enableDepthWrite();
    
    //full screen and cursor toggling
    full = true;
    setFullScreen(full);
    
    cursor = true;
    hideCursor();

    
    //Camera + perspective
    cam.setPerspective( 60.0f, getWindowAspectRatio(), 10, 15000 );
    cam.lookAt( vec3(0, 400, 1400), vec3(0, 300, 0));
    camUI = CameraUi( &cam, getWindow());

    
    groundRad = 300;
    
    vortex.setupEnvironment(groundRad);
    vortex.setupMesh();
    
}

void VortexTestApp::mouseDown( MouseEvent event ){
    
}

void VortexTestApp::update(){

    

    getWindow()->setTitle( "Particle System Test - " + toString( getAverageFps() ) + " FPS" );
    
    
    
    vortex.update();
    
}

void VortexTestApp::draw(){

    float gray = 0.0f;
    gl::clear( Color( gray, gray, gray ) );
    
    gl::setMatrices(cam);
    
    //draw ground radius
    gl::pushMatrices();
    gl::rotate(-M_PI_2, vec3(1, 0, 0));
    gl::color(1, 0, 0, 1.0);
    gl::drawStrokedCircle(vec2(0), groundRad, 50);
    gl::popMatrices();
    
    
    vortex.draw();
    
    
    gl::setMatricesWindow( getWindowSize() );
    gl::color(1, 1, 1, 1);
    gl::pushMatrices();
    gl::scale(vec3(1.5));
    gl::drawString("Framerate: " + toString( getAverageFps() ), vec2(10, 10));

    gl::popMatrices();
    
}

void VortexTestApp::keyDown( KeyEvent event ){

    switch( event.getCode() ) {
        case KeyEvent::KEY_ESCAPE:
            quit();
            return;

        case KeyEvent::KEY_a:
            vortex.active = !vortex.active;
            break;
            
        case KeyEvent::KEY_d:
            vortex.drawFrame = !vortex.drawFrame;
            break;
            
        case KeyEvent::KEY_f:
            full = !full;
            setFullScreen(full);
            break;
            
        case KeyEvent::KEY_c:
            cursor = !cursor;
            
            if(cursor){
                showCursor();
            } else {
                hideCursor();
            }
            break;
    }
    
}


CINDER_APP( VortexTestApp, RendererGl )
