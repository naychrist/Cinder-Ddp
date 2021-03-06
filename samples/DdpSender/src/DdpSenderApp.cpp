#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/CinderMath.h"
#include "cinder/Timeline.h"

#include "DdpSender.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class DdpSenderApp : public App {
  public:
	void setup() override;
	void mouseMove( MouseEvent event ) override;
    void mouseDown( MouseEvent event ) override;
	void keyDown( KeyEvent event ) override;
	void update() override;
	void draw() override;
    
    ddp::Sender     sender;
    ddp::Sender     broadcaster;
    unsigned char   *data;
    int             dataLength;
    int             mouseX;
};

void DdpSenderApp::setup()
{

    //For single NDB:
    sender.setup("10.0.0.101");
    sender.connect();


    //For syncing multiple NDBs:
//    sender.setup("10.0.0.100", true);
//    sender.connect();
//    //sender2.setup(...);
//    //sender2.connect();//etc.
//    broadcaster.setup("10.0.0.255",true);//broadcast address for subnet containing NDBs
//    broadcaster.connect();



    dataLength = 510;
    data = new unsigned char[dataLength];
    
    mouseX = 0;
    
    setWindowSize(dataLength/3, 255);
    setFrameRate(30);
}

void DdpSenderApp::mouseMove( MouseEvent event )
{
    mouseX = math<int>::clamp(event.getPos().x,0, getWindowWidth());
}

void DdpSenderApp::mouseDown( MouseEvent event )
{
//    broadcaster.getStatus(); //untested
}

void DdpSenderApp::keyDown( KeyEvent event )
{
	if (event.getChar() == 'r') {
		sender.reset();
		timeline().add([&]{
			console() << "Attempting to reconnect..." <<endl;
			sender.setup("10.0.0.101");
			sender.connect();
		}, timeline().getCurrentTime() + 60);
	}
}

void DdpSenderApp::update()
{

    for (int i=0; i<dataLength; i++) data[i] = 0;
    int bright = 255;
    data[mouseX*3] = bright;
    data[mouseX*3+1] = bright;
    data[mouseX*3+2] = bright;

    sender.update( data, dataLength );

//    broadcaster.push();//if syncing multiple NDBs
}

void DdpSenderApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    gl::color( Color( 1,1,1) );
    gl::drawLine(glm::vec2(mouseX,0), glm::vec2(mouseX,getWindowHeight()));
}

CINDER_APP( DdpSenderApp, RendererGl )
