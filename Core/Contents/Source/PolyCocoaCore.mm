/*
 Copyright (C) 2011 by Ivan Safrin
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*/

#include "PolyCocoaCore.h"
#include <iostream>
#include <limits.h>

using namespace Polycode;

long getThreadID() {
	return (long)pthread_self();
}

CocoaCore::CocoaCore(PolycodeView *view, int _xRes, int _yRes, bool fullScreen, bool vSync, int aaLevel, int anisotropyLevel, int frameRate, int monitorIndex) : Core(_xRes, _yRes, fullScreen, vSync, aaLevel, anisotropyLevel, frameRate, monitorIndex) {	

	hidManager = NULL;
	initGamepad();

	eventMutex = createMutex();
	
//	NSLog(@"BUNDLE: %@", [[NSBundle mainBundle] bundlePath]);
	chdir([[[[NSBundle mainBundle] bundlePath] stringByAppendingString:@"/Contents/Resources"] UTF8String]);
	
	defaultWorkingDirectory = String([[[[NSBundle mainBundle] bundlePath] stringByAppendingString:@"/Contents/Resources"] UTF8String]);
	
	userHomeDirectory = String([NSHomeDirectory() UTF8String]);
	
	NSOpenGLPixelFormatAttribute attrs[32];
	
	int atindx = 0;
	attrs[atindx++] = NSOpenGLPFADoubleBuffer;
	
	attrs[atindx++] = NSOpenGLPFADepthSize;
	attrs[atindx++] = 32;
	
	if(aaLevel > 0) {
		attrs[atindx++] = NSOpenGLPFASampleBuffers;	
		attrs[atindx++] = 1;	
	
		attrs[atindx++] = NSOpenGLPFASamples;	
		attrs[atindx++] = aaLevel;	
	
		attrs[atindx++] = NSOpenGLPFAMultisample;	
	}
	
	attrs[atindx++] = NSOpenGLPFANoRecovery;		

	if(fullScreen) {
		
//		attrs[atindx++] = NSOpenGLPFAFullScreen;		
//		attrs[atindx++] = NSOpenGLPFAScreenMask;	
//		attrs[atindx++] = CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay);
		
	}
	
	attrs[atindx++] = NSOpenGLPFAAccelerated;			
	
	
	attrs[atindx++] = nil;				
/*	
	NSOpenGLPixelFormatAttribute attrs[] =
	{
			NSOpenGLPFADoubleBuffer,	
		NSOpenGLPFADepthSize, 16,
//		NSOpenGLPFAFullScreen,
//		NSOpenGLPFAScreenMask,
//		CGDisplayIDToOpenGLDisplayMask(kCGDirectMainDisplay),
		NSOpenGLPFASampleBuffers, 1,
		NSOpenGLPFASamples,  aaLevel,
		NSOpenGLPFANoRecovery,	
//		NSOpenGLPFAWindow,
		NSOpenGLPFAMultisample,
//		NSOpenGLPFAAccelerated,
//		NSOpenGLPFAAccumSize, 0,
		nil
	};	
*/

//	[view lockContext];
	
	[view setCore:this];	
	NSOpenGLPixelFormat *format = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
	
	if(!format) {
		NSLog(@"Error creating pixel format!\n");
	}
	
	context = [[NSOpenGLContext alloc] initWithFormat: format shareContext:nil];
	[format release];

	if (context == nil) {
        NSLog(@"Failed to create open gl context");
	}	
	
	if(false) {

//		[view enterFullScreenMode:[NSScreen mainScreen] withOptions:0];
//		[view removeFromSuperview];
//		[[view window] orderOut:nil];
		
//		CGDisplayCapture (kCGDirectMainDisplay ) ;			
//		CGDisplaySwitchToMode (kCGDirectMainDisplay, CGDisplayBestModeForParameters (kCGDirectMainDisplay, 32, xRes, yRes, NULL) );				
		
		
//		[context makeCurrentContext];		
//		[context setFullScreen];
//		[context flushBuffer];	
//		CGDisplayCapture (kCGDirectMainDisplay ) ;		


		
//		[context clearDrawable];
  //      [context release];		
	} else {
		[view clearGLContext];
		[view setOpenGLContext:context];	
		[context setView: (NSView*)view];					
	}
	
	if(fullScreen) {
	//	[view enterFullScreenMode:[NSScreen mainScreen] withOptions:0];	
	}
	
	
	glView = view;
	
	initTime = mach_absolute_time();					
	
//	while(![view isContextReady]) {
		
//	}
	
	renderer = new OpenGLRenderer();
	services->setRenderer(renderer);	
	//[view unlockContext];			
	setVideoMode(xRes,yRes,fullScreen, vSync, aaLevel, anisotropyLevel);		

	CoreServices::getInstance()->installModule(new GLSLShaderModule());	

}

void CocoaCore::copyStringToClipboard(const String& str) {
	NSPasteboard *pb = [NSPasteboard generalPasteboard];
    NSArray *types = [NSArray arrayWithObjects:NSStringPboardType, nil];
    [pb declareTypes:types owner:glView];
	
	//NSString *nsstr = [NSString stringWithCharacters: (unichar*) str.c_str() length: str.length()];
	
	char* data = (char*)str.c_str();
	unsigned size = str.size() * sizeof(char);
	
	NSString* nsstr = [[[NSString alloc] initWithBytes:data length:size encoding:NSUTF32LittleEndianStringEncoding] autorelease];
    [pb setString: nsstr forType:NSStringPboardType];	
}

String CocoaCore::getClipboardString() {
	
}

void CocoaCore::setVideoMode(int xRes, int yRes, bool fullScreen, bool vSync, int aaLeve, int anisotropyLevel) {
	this->xRes = xRes;
	this->yRes = yRes;
	this->fullScreen = fullScreen;
	this->aaLevel = aaLevel;
	renderer->setAnisotropyAmount(anisotropyLevel);
		
	renderer->Resize(xRes, yRes);
//	CoreServices::getInstance()->getMaterialManager()->reloadProgramsAndTextures();	
	dispatchEvent(new Event(), EVENT_CORE_RESIZE);	

	
//	NSRect visibleFrame = [[NSScreen mainScreen] visibleFrame];	
//	NSRect frame = NSMakeRect([[glView window] frame].origin.x, [[glView window] frame].origin.y, xRes, yRes);
	
//	frame.origin.x = (visibleFrame.size.width - frame.size.width) * 0.5;
//	frame.origin.y = (visibleFrame.size.height - frame.size.height) * (9.0/10.0);
	
//	[[glView window] setFrame: frame display: YES animate: NO];
//	if(!fullScreen) {
		[[glView window] setContentSize: NSMakeSize(xRes, yRes)];
//	} else {
//		CGDisplaySwitchToMode (kCGDirectMainDisplay, CGDisplayBestModeForParameters (kCGDirectMainDisplay, 32, xRes, yRes, NULL) );						
//	}
	if(fullScreen) {	
		CGDisplaySwitchToMode (kCGDirectMainDisplay, CGDisplayBestModeForParameters (kCGDirectMainDisplay, 32, xRes, yRes, NULL) );			
		
		if(monitorIndex > -1) {
			if(monitorIndex > [[NSScreen screens] count]-1) {
				Logger::log("Requested monitor index above available screens.\n");
				monitorIndex = -1;
			}
		}
		
	    if(monitorIndex == -1) {		
			[glView enterFullScreenMode:[NSScreen mainScreen] withOptions:[NSDictionary dictionaryWithObjectsAndKeys: nil]];
		} else {
			[glView enterFullScreenMode:[[NSScreen screens] objectAtIndex:1] withOptions:[NSDictionary dictionaryWithObjectsAndKeys: nil]];
		}

		
	}
	
	GLint sync = 0;
	if(vSync) {
		sync =1 ;
	} 
	
	[context setValues:&sync forParameter:NSOpenGLCPSwapInterval];	
				
	/*
	if(aaLevel > 0) {
		glEnable( GL_MULTISAMPLE_ARB );	
	} else {
		glDisable( GL_MULTISAMPLE_ARB );			
	}
	*/
}

void CocoaCore::resizeTo(int xRes, int yRes) {
	this->xRes = xRes;
	this->yRes = yRes;
	renderer->Resize(xRes, yRes);	
	dispatchEvent(new Event(), EVENT_CORE_RESIZE);	
}

vector<Polycode::Rectangle> CocoaCore::getVideoModes() {
	vector<Polycode::Rectangle> retVector;
	return retVector;
}

CocoaCore::~CocoaCore() {
	printf("Shutting down cocoa core\n");
	shutdownGamepad();
	if(fullScreen) {
		[glView exitFullScreenModeWithOptions:nil];
		
	}
	
	[glView clearGLContext];	
	[context release];
}

void *ManagedThreadFunc(void *data) {
	Threaded *target = static_cast<Threaded*>(data);
	target->runThread();
	target->scheduledForRemoval = true;
	return NULL;
}

void CocoaCore::createThread(Threaded *target) {
	Core::createThread(target);
	pthread_t thread;
	pthread_create( &thread, NULL, ManagedThreadFunc, (void*)target);
}

void CocoaCore::lockMutex(CoreMutex *mutex) {
	PosixMutex *m = (PosixMutex*) mutex;
	pthread_mutex_lock(&m->pMutex);	
}

void CocoaCore::unlockMutex(CoreMutex *mutex) {
	PosixMutex *m = (PosixMutex*) mutex;
	pthread_mutex_unlock(&m->pMutex);
}

CoreMutex *CocoaCore::createMutex() {
	PosixMutex *mutex = new PosixMutex();	
	pthread_mutex_init(&mutex->pMutex, NULL);
	return mutex;
}

unsigned int CocoaCore::getTicks() {
	uint64_t time = mach_absolute_time();	
	double conversion = 0.0;
	
	mach_timebase_info_data_t info;
	mach_timebase_info( &info );
	conversion = 1e-9 * (double) info.numer / (double) info.denom;	
	
	return (((double)(time - initTime)) * conversion) * 1000.0f;
}

void CocoaCore::enableMouse(bool newval) {
	
	if(newval) 
		CGDisplayShowCursor(kCGDirectMainDisplay);			
	else
		CGDisplayHideCursor(kCGDirectMainDisplay);	
	Core::enableMouse(newval);
}

void CocoaCore::setCursor(int cursorType) {
	
	NSCursor *newCursor;
	
	switch(cursorType) {
		case CURSOR_TEXT:
			newCursor = [NSCursor IBeamCursor];			
		break;			
		case CURSOR_POINTER:
			newCursor = [NSCursor pointingHandCursor];						
		break;			
		case CURSOR_CROSSHAIR:
			newCursor = [NSCursor crosshairCursor];
		break;			
		case CURSOR_RESIZE_LEFT_RIGHT:
			newCursor = [NSCursor resizeLeftRightCursor];
		break;			
		case CURSOR_RESIZE_UP_DOWN:
			newCursor = [NSCursor resizeUpDownCursor];			
		break;
		default:
			newCursor = [NSCursor arrowCursor];			
		break;
	}
	[glView setCurrentCursor:newCursor];
	[glView resetCursorRects];	
	[[glView window] invalidateCursorRectsForView: (NSView*)glView];
}

void CocoaCore::warpCursor(int x, int y) {

	CGSetLocalEventsSuppressionInterval(0);
	NSArray *theScreens = [NSScreen screens];
	for (NSScreen *theScreen in theScreens) {
		CGPoint CenterOfWindow = CGPointMake([glView window].frame.origin.x+x, (-1)*([glView window].frame.origin.y-theScreen.frame.size.height)-yRes+y);
		CGDisplayMoveCursorToPoint (kCGDirectMainDisplay, CenterOfWindow);		
		break;
	}
	lastMouseX = x;
	lastMouseY = y;
}

void CocoaCore::checkEvents() {
	lockMutex(eventMutex);
	CocoaEvent event;
	for(int i=0; i < cocoaEvents.size(); i++) {
		event = cocoaEvents[i];
		switch(event.eventGroup) {
			case CocoaEvent::INPUT_EVENT:
				switch(event.eventCode) {
					case InputEvent::EVENT_MOUSEMOVE:
						input->setDeltaPosition(lastMouseX - event.mouseX, lastMouseY - event.mouseY);										
						lastMouseX = event.mouseX;
						lastMouseY = event.mouseY;
						input->setMousePosition(event.mouseX, event.mouseY, getTicks());						
						break;
					case InputEvent::EVENT_MOUSEDOWN:
						input->setMouseButtonState(event.mouseButton, true, getTicks());						
						break;
					case InputEvent::EVENT_MOUSEWHEEL_UP:
						input->mouseWheelUp(getTicks());
					break;
					case InputEvent::EVENT_MOUSEWHEEL_DOWN:
						input->mouseWheelDown(getTicks());						
					break;						
					case InputEvent::EVENT_MOUSEUP:
						input->setMouseButtonState(event.mouseButton, false, getTicks());
						break;
					case InputEvent::EVENT_KEYDOWN:
						input->setKeyState(event.keyCode, event.unicodeChar, true, getTicks());
						break;
					case InputEvent::EVENT_KEYUP:
						input->setKeyState(event.keyCode, event.unicodeChar, false, getTicks());
						break;						
				}
				break;
		}
	}
	cocoaEvents.clear();	
	unlockMutex(eventMutex);		
}

void CocoaCore::openURL(String url) {
	[[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:[NSString stringWithUTF8String: url.c_str()]]];
}

void CocoaCore::createFolder(const String& folderPath) {
	[[NSFileManager defaultManager] createDirectoryAtPath:[NSString stringWithUTF8String: folderPath.c_str()] withIntermediateDirectories:YES attributes:nil error:nil];
}

void CocoaCore::copyDiskItem(const String& itemPath, const String& destItemPath) {
	[[NSFileManager defaultManager] copyItemAtPath: [NSString stringWithUTF8String: itemPath.c_str()] toPath: [NSString stringWithUTF8String: destItemPath.c_str()] error: nil];	
}

void CocoaCore::moveDiskItem(const String& itemPath, const String& destItemPath) {
	[[NSFileManager defaultManager] moveItemAtPath: [NSString stringWithUTF8String: itemPath.c_str()] toPath: [NSString stringWithUTF8String: destItemPath.c_str()] error: nil];		
}

void CocoaCore::removeDiskItem(const String& itemPath) {
	[[NSFileManager defaultManager] removeItemAtPath: [NSString stringWithUTF8String: itemPath.c_str()] error:nil];
}
	
String CocoaCore::openFolderPicker() {
	NSOpenPanel *attachmentPanel = [NSOpenPanel openPanel];	
	[attachmentPanel setCanChooseFiles:NO];
	[attachmentPanel setCanCreateDirectories: YES];
	[attachmentPanel setCanChooseDirectories:YES];
	
	if ( [attachmentPanel runModalForDirectory:nil file:nil] == NSOKButton )
	{
		// files and directories selected.
		NSArray* files = [attachmentPanel filenames];
		NSString* fileName = [files objectAtIndex:0];
		return [fileName UTF8String];
	} else {
		return [@"" UTF8String];
	}	
}

vector<String> CocoaCore::openFilePicker(vector<CoreFileExtension> extensions, bool allowMultiple) {
	
	vector<String> retVector;
	
	NSOpenPanel *attachmentPanel = [NSOpenPanel openPanel];	
	[attachmentPanel setCanChooseFiles:YES];
	[attachmentPanel setCanCreateDirectories: YES];
	[attachmentPanel setCanChooseDirectories:NO];
	[attachmentPanel setAllowsMultipleSelection: allowMultiple];
	
	NSMutableArray *types = [[NSMutableArray alloc] init];
	
	for(int i=0; i < extensions.size(); i++) {	
		CoreFileExtension extInfo = extensions[i];
		[types addObject: [NSString stringWithUTF8String: extInfo.extension.c_str()]];
	}
	
	if ( [attachmentPanel runModalForDirectory:nil file:nil types:types] == NSOKButton )
	{
		NSArray* files = [attachmentPanel filenames];
		NSString* fileName = [files objectAtIndex:0];
		retVector.push_back([fileName UTF8String]);
	} else {
		retVector.push_back("");
	}	
	
	return retVector;
}

bool CocoaCore::Update() {
	if(!running)
		return false;
		
	lockMutex(CoreServices::getRenderMutex());	
	checkEvents();
	renderer->BeginRender();
	updateCore();
	renderer->EndRender();
	[context flushBuffer];
	unlockMutex(CoreServices::getRenderMutex());	
	doSleep();	
	return running;
}

static void hatValueToXY(CFIndex value, CFIndex range, int * outX, int * outY) {
	if (value == range) {
		*outX = *outY = 0;
		
	} else {
		if (value > 0 && value < range / 2) {
			*outX = 1;
			
		} else if (value > range / 2) {
			*outX = -1;
			
		} else {
			*outX = 0;
		}
		
		if (value > range / 4 * 3 || value < range / 4) {
			*outY = -1;
			
		} else if (value > range / 4 && value < range / 4 * 3) {
			*outY = 1;
			
		} else {
			*outY = 0;
		}
	}
}


static int IOHIDDeviceGetIntProperty(IOHIDDeviceRef deviceRef, CFStringRef key) {
	CFTypeRef typeRef;
	int value;
	
	typeRef = IOHIDDeviceGetProperty(deviceRef, key);
	if (typeRef == NULL || CFGetTypeID(typeRef) != CFNumberGetTypeID()) {
		return 0;
	}
	
	CFNumberGetValue((CFNumberRef) typeRef, kCFNumberSInt32Type, &value);
	return value;
}	

static void onDeviceValueChanged(void * context, IOReturn result, void * sender, IOHIDValueRef value) {
	IOHIDElementRef element;
	IOHIDElementCookie cookie;
	unsigned int axisIndex, buttonIndex;
	static mach_timebase_info_data_t timebaseInfo;
	
	if (timebaseInfo.denom == 0) {
		mach_timebase_info(&timebaseInfo);
	}
	
	GamepadDeviceEntry *deviceRecord = (GamepadDeviceEntry*) context;
	CoreInput *input = deviceRecord->input;
	JoystickInfo *joystickInfo = input->getJoystickInfoByID(deviceRecord->deviceID);
	if(!joystickInfo)
		return;
	
	element = IOHIDValueGetElement(value);
	cookie = IOHIDElementGetCookie(element);
	
	for (axisIndex = 0; axisIndex < deviceRecord->numAxes; axisIndex++) {
		if (!deviceRecord->axisElements[axisIndex].isHatSwitchSecondAxis &&
		    deviceRecord->axisElements[axisIndex].cookie == cookie) {
			CFIndex integerValue;
			
			if (IOHIDValueGetLength(value) > 4) {
				// Workaround for a strange crash that occurs with PS3 controller; was getting lengths of 39 (!)
				continue;
			}
			integerValue = IOHIDValueGetIntegerValue(value);
			
			if (deviceRecord->axisElements[axisIndex].isHatSwitch) {
				int x, y;
				
				// Fix for Saitek X52
				deviceRecord->axisElements[axisIndex].hasNullState = false;
				if (!deviceRecord->axisElements[axisIndex].hasNullState) {
					if (integerValue < deviceRecord->axisElements[axisIndex].logicalMin) {
						integerValue = deviceRecord->axisElements[axisIndex].logicalMax - deviceRecord->axisElements[axisIndex].logicalMin + 1;
					} else {
						integerValue--;
					}
				}
				
				hatValueToXY(integerValue, deviceRecord->axisElements[axisIndex].logicalMax - deviceRecord->axisElements[axisIndex].logicalMin + 1, &x, &y);
				
				if (x != joystickInfo->joystickAxisState[axisIndex]) {
					input->joystickAxisMoved(axisIndex, x, deviceRecord->deviceID);
				}
				
				if (y != joystickInfo->joystickAxisState[axisIndex + 1]) {
					input->joystickAxisMoved(axisIndex + 1, y, deviceRecord->deviceID);				
				}				
			} else {
				float floatValue;
				
				if (integerValue < deviceRecord->axisElements[axisIndex].logicalMin) {
					deviceRecord->axisElements[axisIndex].logicalMin = integerValue;
				}
				if (integerValue > deviceRecord->axisElements[axisIndex].logicalMax) {
					deviceRecord->axisElements[axisIndex].logicalMax = integerValue;
				}
				
				floatValue = (integerValue - deviceRecord->axisElements[axisIndex].logicalMin) / (float) (deviceRecord->axisElements[axisIndex].logicalMax - deviceRecord->axisElements[axisIndex].logicalMin) * 2.0f - 1.0f;
				input->joystickAxisMoved(axisIndex, floatValue, deviceRecord->deviceID);
			}
			
			return;
		}
	}
	
	for (buttonIndex = 0; buttonIndex < deviceRecord->numButtons; buttonIndex++) {
		if (deviceRecord->buttonElements[buttonIndex].cookie == cookie) {
			bool down;
			
			down = IOHIDValueGetIntegerValue(value);
			if(down) {
				input->joystickButtonDown(buttonIndex, deviceRecord->deviceID);
			} else {
				input->joystickButtonUp(buttonIndex, deviceRecord->deviceID);			
			}
			return;
		}
	}
}

static void onDeviceMatched(void * context, IOReturn result, void * sender, IOHIDDeviceRef device) {
	CocoaCore *core = (CocoaCore*) context;

CFArrayRef elements;
	CFIndex elementIndex;
	IOHIDElementRef element;
	CFStringRef cfProductName;
	IOHIDElementType type;
	char * description;
	
	GamepadDeviceEntry *entry = new GamepadDeviceEntry();
	entry->device = device;
	entry->input  = core->getInput();
	entry->deviceID = core->nextDeviceID++;
	core->gamepads.push_back(entry);	
	core->getInput()->addJoystick(entry->deviceID);
	
	elements = IOHIDDeviceCopyMatchingElements(device, NULL, kIOHIDOptionsTypeNone);
	for (elementIndex = 0; elementIndex < CFArrayGetCount(elements); elementIndex++) {
		element = (IOHIDElementRef) CFArrayGetValueAtIndex(elements, elementIndex);
		type = IOHIDElementGetType(element);
		
		// All of the axis elements I've ever detected have been kIOHIDElementTypeInput_Misc. kIOHIDElementTypeInput_Axis is only included for good faith...
		if (type == kIOHIDElementTypeInput_Misc ||
		    type == kIOHIDElementTypeInput_Axis) {

			entry->axisElements.resize(entry->numAxes+1);
			entry->axisElements[entry->numAxes].cookie = IOHIDElementGetCookie(element);
			entry->axisElements[entry->numAxes].logicalMin = IOHIDElementGetLogicalMin(element);
			entry->axisElements[entry->numAxes].logicalMax = IOHIDElementGetLogicalMax(element);
			entry->axisElements[entry->numAxes].hasNullState = !!IOHIDElementHasNullState(element);
			entry->axisElements[entry->numAxes].isHatSwitch = IOHIDElementGetUsage(element) == kHIDUsage_GD_Hatswitch;
			entry->axisElements[entry->numAxes].isHatSwitchSecondAxis = false;
			entry->numAxes++;
			
			if (entry->axisElements[entry->numAxes - 1].isHatSwitch) {
				entry->axisElements.resize(entry->numAxes+1);			
				entry->axisElements[entry->numAxes].isHatSwitchSecondAxis = true;
				entry->numAxes++;
			}			
		} else if (type == kIOHIDElementTypeInput_Button) {
			entry->buttonElements.resize(entry->numButtons+1);			
			entry->buttonElements[entry->numButtons].cookie = IOHIDElementGetCookie(element);
			entry->numButtons++;
		}
	}
	CFRelease(elements);
		
	IOHIDDeviceRegisterInputValueCallback(device, onDeviceValueChanged, entry);
	
}

static void onDeviceRemoved(void * context, IOReturn result, void * sender, IOHIDDeviceRef device) {
	CocoaCore *core = (CocoaCore*) context;	
	for(int i=0; i < core->gamepads.size();i++) {
		if(core->gamepads[i]->device == device) {
			core->getInput()->removeJoystick(core->gamepads[i]->deviceID);
			delete core->gamepads[i];
			core->gamepads.erase(core->gamepads.begin()+i);
			IOHIDDeviceRegisterInputValueCallback(device, NULL, NULL);
			return;
		}
	}
}

void CocoaCore::shutdownGamepad() {
	if (hidManager != NULL) {
		unsigned int deviceIndex;
		
		IOHIDManagerRegisterDeviceMatchingCallback(hidManager, NULL, NULL);
		IOHIDManagerRegisterDeviceRemovalCallback(hidManager, NULL, NULL);		
		
		IOHIDManagerUnscheduleFromRunLoop(hidManager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
		IOHIDManagerClose(hidManager, 0);
		CFRelease(hidManager);
		hidManager = NULL;
		
		for (int i = 0; i < gamepads.size(); i++) {
			IOHIDDeviceRegisterInputValueCallback(gamepads[i]->device, NULL, NULL);		
			delete gamepads[i];
		}
		
	}
}

void CocoaCore::initGamepad() {
	if (hidManager == NULL) {
		nextDeviceID = 0;
		CFStringRef keys[2];
		int value;
		CFNumberRef values[2];
		CFDictionaryRef dictionaries[3];
		CFArrayRef array;
		
		hidManager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
		IOHIDManagerOpen(hidManager, kIOHIDOptionsTypeNone);
		IOHIDManagerScheduleWithRunLoop(hidManager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
		
		keys[0] = CFSTR(kIOHIDDeviceUsagePageKey);
		keys[1] = CFSTR(kIOHIDDeviceUsageKey);
		
		value = kHIDPage_GenericDesktop;
		values[0] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);
		value = kHIDUsage_GD_Joystick;
		values[1] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);
		dictionaries[0] = CFDictionaryCreate(kCFAllocatorDefault, (const void **) keys, (const void **) values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		CFRelease(values[0]);
		CFRelease(values[1]);
		
		value = kHIDPage_GenericDesktop;
		values[0] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);
		value = kHIDUsage_GD_GamePad;
		values[1] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);
		dictionaries[1] = CFDictionaryCreate(kCFAllocatorDefault, (const void **) keys, (const void **) values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		CFRelease(values[0]);
		CFRelease(values[1]);
		
		value = kHIDPage_GenericDesktop;
		values[0] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);
		value = kHIDUsage_GD_MultiAxisController;
		values[1] = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value);
		dictionaries[2] = CFDictionaryCreate(kCFAllocatorDefault, (const void **) keys, (const void **) values, 2, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		CFRelease(values[0]);
		CFRelease(values[1]);
		
		array = CFArrayCreate(kCFAllocatorDefault, (const void **) dictionaries, 3, &kCFTypeArrayCallBacks);
		CFRelease(dictionaries[0]);
		CFRelease(dictionaries[1]);
		CFRelease(dictionaries[2]);
		IOHIDManagerSetDeviceMatchingMultiple(hidManager, array);
		CFRelease(array);
		
		IOHIDManagerRegisterDeviceMatchingCallback(hidManager, onDeviceMatched, this);
		IOHIDManagerRegisterDeviceRemovalCallback(hidManager, onDeviceRemoved, this);
	}
}
