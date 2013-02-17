/*
 Copyright (C) 2012 by Ivan Safrin
 
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


#include "PolyUIButton.h"
#include "PolyConfig.h"
#include "PolyInputEvent.h"
#include "PolyLabel.h"
#include "PolyCoreServices.h"

using namespace Polycode;

UIButton::UIButton(String text, Number width, Number height) : UIElement() {

	Config *conf = CoreServices::getInstance()->getConfig();	
	
	String fontName = conf->getStringValue("Polycode", "uiDefaultFontName");
	int fontSize = conf->getNumericValue("Polycode", "uiButtonFontSize");	
	
	Number st = conf->getNumericValue("Polycode", "uiButtonSkinT");
	Number sr = conf->getNumericValue("Polycode", "uiButtonSkinR");
	Number sb = conf->getNumericValue("Polycode", "uiButtonSkinB");
	Number sl = conf->getNumericValue("Polycode", "uiButtonSkinL");

	Number labelOffsetX = conf->getNumericValue("Polycode", "uiButtonLabelOffsetX");
	Number labelOffsetY = conf->getNumericValue("Polycode", "uiButtonLabelOffsetY");
		
	buttonRect = new UIBox(conf->getStringValue("Polycode", "uiButtonSkin"),
						   st,sr,sb,sl,
						   width, height);	
	
	buttonRect->blockMouseInput  = true;
	buttonFocusedRect= new UIBox(conf->getStringValue("Polycode", "uiButtonFocusedSkin"),
								 st,sr,sb,sl,
								 width, height);		
	blockMouseInput = true;
	addChild(buttonRect);
	addChild(buttonFocusedRect);
	
	buttonFocusedRect->visible = false;
	
	buttonRect->addEventListener(this, InputEvent::EVENT_MOUSEOVER);
	buttonRect->addEventListener(this, InputEvent::EVENT_MOUSEOUT);
	buttonRect->addEventListener(this, InputEvent::EVENT_MOUSEUP);
	buttonRect->addEventListener(this, InputEvent::EVENT_MOUSEUP_OUTSIDE);	
	buttonRect->addEventListener(this, InputEvent::EVENT_MOUSEDOWN);
	pressedDown = false;
	
	buttonLabel = new ScreenLabel(text, fontSize, fontName, Label::ANTIALIAS_FULL);
	buttonLabel->positionAtBaseline = false;
	addChild(buttonLabel);
	labelXPos = floor((width-buttonLabel->getWidth())/2.0f) + labelOffsetX;
	labelYPos = floor((height-(buttonLabel->getLabel()->getTextHeight()))/2.0f) + labelOffsetY;
	buttonLabel->setPosition(labelXPos,labelYPos);
	
	this->width = width;
	this->height = height;
	focusable = true;
	
	buttonRect->processInputEvents = true;
	
}

void UIButton::Update() {
	if(hasFocus) {
		buttonFocusedRect->visible = true;
		buttonRect->visible = false;
	} else {
		buttonFocusedRect->visible = false;
		buttonRect->visible = true;
	}
}

UIButton::~UIButton() {
	delete buttonRect;
	delete buttonFocusedRect;
	delete buttonLabel;
}
		
void UIButton::handleEvent(Event *event) {
	if(event->getDispatcher() == buttonRect) {
		switch(event->getEventCode()) {
			case InputEvent::EVENT_MOUSEOVER:
			break;
			case InputEvent::EVENT_MOUSEOUT:
				buttonLabel->setPosition(labelXPos,labelYPos);
				buttonRect->setPosition(0,0);				
				pressedDown = false;
			break;
			case InputEvent::EVENT_MOUSEUP_OUTSIDE:
				buttonLabel->setPosition(labelXPos,labelYPos);
				buttonRect->setPosition(0,0);
			break;			
			case InputEvent::EVENT_MOUSEUP:
				buttonLabel->setPosition(labelXPos,labelYPos);
				buttonRect->setPosition(0,0);
				if(pressedDown) {
					dispatchEvent(new UIEvent(), UIEvent::CLICK_EVENT);
				}
				pressedDown = false;
			break;
			case InputEvent::EVENT_MOUSEDOWN:
				pressedDown = true;
				if(parentEntity)
					((ScreenEntity*)parentEntity)->focusChild(this);
				buttonLabel->setPosition(labelXPos+1,labelYPos+1);
				buttonRect->setPosition(1,1);
			break;
		}
	}
}