#include "HelloPolycodeApp.h"

HelloPolycodeApp::HelloPolycodeApp(PolycodeView *view) {

#ifdef __APPLE__
	core = new CocoaCore(view, 640,480,false,false,0,0,90);	  
#else
	core = new SDLCore(view, 640,480,false,false,0,0,90);	  
#endif
	  
	CoreServices::getInstance()->getResourceManager()->addArchive("Resources/default.pak");
	CoreServices::getInstance()->getResourceManager()->addDirResource("default", false);

	Screen *screen = new Screen();			
	image = new ScreenImage("Resources/polycode_logo.png");
	screen->addChild(image);	
}

HelloPolycodeApp::~HelloPolycodeApp() {
}

bool HelloPolycodeApp::Update() {
	Number elapsed = core->getElapsed();
	image->setRotation(image->getRotation()+(elapsed*100));
    return core->Update();
}
