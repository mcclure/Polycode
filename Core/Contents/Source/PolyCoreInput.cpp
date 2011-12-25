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

#include "PolyCoreInput.h"
#include "PolyInputEvent.h"

namespace Polycode {
	
	JoystickInfo::JoystickInfo() {
		for(int i=0; i < 32; i++) {
			joystickAxisState[i] = 0.0;
		}

		for(int i=0; i < 64; i++) {
			joystickButtonState[i] = false;
		}	
	}
	
	CoreInput::CoreInput() : EventDispatcher() {
		mouseButtons[0] = false;
		mouseButtons[1] = false;
		mouseButtons[2] = false;
		
		for(int i=0; i < 512; i++) {
			keyboardState[i] = 0;
		}		
	}
	
	CoreInput::~CoreInput() {
	}
	
	unsigned int CoreInput::getNumJoysticks() {
		return joysticks.size();
	}
	
	JoystickInfo *CoreInput::getJoystickInfoByIndex(unsigned int index) {
		return &joysticks[index];
	}	
	
	JoystickInfo *CoreInput::getJoystickInfoByID(unsigned int deviceID) {
		for(int i=0;i<joysticks.size();i++) {
			if(joysticks[i].deviceID == deviceID) {
				return &joysticks[i];
			}
		}
		return NULL;
	}
			
	void CoreInput::addJoystick(unsigned int deviceID) {
		JoystickInfo joystick;
		joystick.deviceID = deviceID;
		joysticks.push_back(joystick);
		InputEvent *evt = new InputEvent();
		evt->joystickDeviceID = deviceID;
		dispatchEvent(evt, InputEvent::EVENT_JOYDEVICE_ATTACHED);				
	}
	
	void CoreInput::removeJoystick(unsigned int deviceID) {
		for(int i=0;i<joysticks.size();i++) {
			if(joysticks[i].deviceID == deviceID) {
				joysticks.erase(joysticks.begin()+i);
				InputEvent *evt = new InputEvent();
				evt->joystickDeviceID = deviceID;
				dispatchEvent(evt, InputEvent::EVENT_JOYDEVICE_DETACHED);
				return;
			}
		}	
	}
	
	void CoreInput::joystickAxisMoved(unsigned int axisID, float value, unsigned int deviceID) {
		JoystickInfo *info = getJoystickInfoByID(deviceID);
		if(info) {
			info->joystickAxisState[axisID] = value;
			InputEvent *evt = new InputEvent();
			evt->joystickDeviceID = deviceID;
			evt->joystickAxis = axisID;
			evt->joystickAxisValue = value;
			dispatchEvent(evt, InputEvent::EVENT_JOYAXIS_MOVED);
		}	
	}
	
	void CoreInput::joystickButtonDown(unsigned int buttonID, unsigned int deviceID) {
		JoystickInfo *info = getJoystickInfoByID(deviceID);
		if(info) {
			info->joystickButtonState[buttonID] = true;
			InputEvent *evt = new InputEvent();
			evt->joystickDeviceID = deviceID;
			evt->joystickButton = buttonID;
			dispatchEvent(evt, InputEvent::EVENT_JOYBUTTON_DOWN);
		}		
	}
	
	void CoreInput::joystickButtonUp(unsigned int buttonID, unsigned int deviceID) {
		JoystickInfo *info = getJoystickInfoByID(deviceID);
		if(info) {
			info->joystickButtonState[buttonID] = false;
			InputEvent *evt = new InputEvent();
			evt->joystickDeviceID = deviceID;
			evt->joystickButton = buttonID;
			dispatchEvent(evt, InputEvent::EVENT_JOYBUTTON_UP);
		}	
	}
	
	
	bool CoreInput::getMouseButtonState(int mouseButton) {
		return mouseButtons[mouseButton];
	}
	
	void CoreInput::setMouseButtonState(int mouseButton, bool state, int ticks) {
		InputEvent *evt = new InputEvent(mousePosition, ticks);
		evt->mouseButton = mouseButton;		
		if(state)
			dispatchEvent(evt, InputEvent::EVENT_MOUSEDOWN);
		else
			dispatchEvent(evt, InputEvent::EVENT_MOUSEUP);
		mouseButtons[mouseButton] = state;
	}
	
	void CoreInput::mouseWheelDown(int ticks) {
		InputEvent *evt = new InputEvent(mousePosition, ticks);
		dispatchEvent(evt, InputEvent::EVENT_MOUSEWHEEL_DOWN);				
	}
	
	void CoreInput::mouseWheelUp(int ticks) {
		InputEvent *evt = new InputEvent(mousePosition, ticks);
		dispatchEvent(evt, InputEvent::EVENT_MOUSEWHEEL_UP);		
	}
	
	void CoreInput::setMousePosition(int x, int y, int ticks) {
		mousePosition.x = x;
		mousePosition.y = y;
		InputEvent *evt = new InputEvent(mousePosition, ticks);
		dispatchEvent(evt, InputEvent::EVENT_MOUSEMOVE);
	}
	
	Vector2 CoreInput::getMouseDelta() {
		return deltaMousePosition;
	}
	
	void CoreInput::setDeltaPosition(int x, int y) {
		deltaMousePosition.x = (Number)x;
		deltaMousePosition.y = (Number)y;
	}

	Vector2 CoreInput::getMousePosition() {
		return mousePosition;
	}
	
	bool CoreInput::getKeyState(PolyKEY keyCode) {
		if(keyCode < 512)
			return keyboardState[keyCode];
		else
			return false;
	}
	
	void CoreInput::setKeyState(PolyKEY keyCode, wchar_t code, bool newState, int ticks) {
		InputEvent *evt = new InputEvent(keyCode, code, ticks);
		if(keyCode < 512)
			keyboardState[keyCode] = newState;
		if(newState) {
			dispatchEvent(evt, InputEvent::EVENT_KEYDOWN);
		} else {
			dispatchEvent(evt, InputEvent::EVENT_KEYUP);
		}
	}
}
