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

#include "PolyScreen.h"
#include "PolyCoreServices.h"
#include "PolyInputEvent.h"
#include "PolyScreenManager.h"
#include "PolyResourceManager.h"
#include "PolyCore.h"
#include "PolyMaterial.h"
#include "PolyRenderer.h"
#include "PolyScreenEntity.h"
#include "PolyScreenEvent.h"
#include "PolyShader.h"

using namespace Polycode;

Screen::Screen() : EventDispatcher() {
	offset.x = 0;
	offset.y = 0;
	enabled = true;
	ownsChildren = false;
	focusChild = NULL;
	originalSceneTexture = NULL;
	CoreServices::getInstance()->getScreenManager()->addScreen(this);
	filterShaderMaterial = NULL;
	_hasFilterShader = false;
	useNormalizedCoordinates = false;
	rootEntity = new ScreenEntity();
	addChild(rootEntity);
}

Screen::~Screen() {
	if (ownsChildren) {
		for(int i=0; i<children.size();i++) {
			delete children[i];
		}
	} else {
		delete rootEntity; // Must delete this at minimum
	}
	CoreServices::getInstance()->getScreenManager()->removeScreen(this);	
	
	for(int i=0; i < localShaderOptions.size(); i++)
		delete localShaderOptions[i];
	delete originalSceneTexture;
}

void Screen::setNormalizedCoordinates(bool newVal, Number yCoordinateSize) {
	useNormalizedCoordinates = newVal;
	this->yCoordinateSize = yCoordinateSize;
}

void Screen::handleInputEvent(InputEvent *inputEvent) {
	
	for(int i=children.size()-1; i >= 0; i--) {
		switch(inputEvent->getEventCode()) {
			case InputEvent::EVENT_MOUSEDOWN:
				if(children[i]->_onMouseDown(inputEvent->mousePosition.x-offset.x, inputEvent->mousePosition.y-offset.y, inputEvent->mouseButton, inputEvent->timestamp) &&
				children[i]->blockMouseInput)
					return;
			break;
			case InputEvent::EVENT_MOUSEMOVE:
				children[i]->_onMouseMove(inputEvent->mousePosition.x-offset.x, inputEvent->mousePosition.y-offset.y, inputEvent->timestamp);
			break;
			case InputEvent::EVENT_MOUSEUP:
				if(children[i]->_onMouseUp(inputEvent->mousePosition.x-offset.x, inputEvent->mousePosition.y-offset.y, inputEvent->mouseButton, inputEvent->timestamp) &&
				children[i]->blockMouseInput)
					return;
			break;
			case InputEvent::EVENT_MOUSEWHEEL_UP:
				children[i]->_onMouseWheelUp(inputEvent->mousePosition.x-offset.x, inputEvent->mousePosition.y-offset.y, inputEvent->timestamp);
			break;
			case InputEvent::EVENT_MOUSEWHEEL_DOWN:
				children[i]->_onMouseWheelDown(inputEvent->mousePosition.x-offset.x, inputEvent->mousePosition.y-offset.y,inputEvent->timestamp);				
			break;				
			case InputEvent::EVENT_KEYDOWN:
				children[i]->_onKeyDown(inputEvent->key, inputEvent->charCode);
			break;
			case InputEvent::EVENT_KEYUP:
				children[i]->_onKeyUp(inputEvent->key, inputEvent->charCode);
			break;			
		}
	}
}

void Screen::setRenderer(Renderer *renderer) {
	this->renderer = renderer;
}

void Screen::setScreenOffset(Number x, Number y) {
	offset.x = x;
	offset.y = y;
}

Vector2 Screen::getScreenOffset() const {
	return offset;
}

int Screen::getHighestZIndex() const {
	int highestZ = 1;
	for(int i=0; i<children.size();i++) {
		if(children[i]->zindex > highestZ)
			highestZ = children[i]->zindex;
	}
	return highestZ;
}

bool Screen::cmpZindex(const ScreenEntity *left, const ScreenEntity *right) {
	return (left->zindex < right->zindex);
}

void Screen::sortChildren() {
	std::sort(children.begin(), children.end(), Screen::cmpZindex);
	int newz = 1;
	for(int i=0; i<children.size();i++) {
		children[i]->zindex = newz;
		newz++;
	}
}

void Screen::handleEvent(Event *event) {
	if(event->getEventType() == "ScreenEvent") {
	for(int i=0; i<children.size();i++) {
		if(children[i] == event->getDispatcher()) {
			ScreenEvent *screenEvent = (ScreenEvent*)event;
			int highestZ;
			switch(screenEvent->getEventCode()) {
				case ScreenEvent::ENTITY_MOVE_TOP:
					highestZ = getHighestZIndex();
					children[i]->zindex = highestZ+1;
					sortChildren();
				break;
				case ScreenEvent::ENTITY_MOVE_BOTTOM:
					children[i]->zindex = 0;
					sortChildren();
				break;
				case ScreenEvent::ENTITY_MOVE_UP:
					children[i]->zindex++;
					sortChildren();
				break;
				case ScreenEvent::ENTITY_MOVE_DOWN:
					children[i]->zindex--;
					sortChildren();
				break;
			}
		}
	}
	}
}

void Screen::setScreenShader(const String& shaderName) {
	filterShaderMaterial = (Material*)CoreServices::getInstance()->getResourceManager()->getResource(Resource::RESOURCE_MATERIAL, shaderName);
	if(!filterShaderMaterial)
		return;
	
	if(!originalSceneTexture) {
	CoreServices::getInstance()->getRenderer()->createRenderTextures(&originalSceneTexture, NULL, CoreServices::getInstance()->getCore()->getXRes(), CoreServices::getInstance()->getCore()->getYRes(), filterShaderMaterial->fp16RenderTargets);
	}
	
	for(int i=0; i < filterShaderMaterial->getNumShaders(); i++) {
		ShaderBinding* binding = filterShaderMaterial->getShader(i)->createBinding();		
		if( i == 0) 
			binding->addTexture("screenColorBuffer", originalSceneTexture);
			
		localShaderOptions.push_back(binding);
	}
		
	_hasFilterShader = true;
	
}

void Screen::clearScreenShader() {
	if(_hasFilterShader) {
		_hasFilterShader = false;
		filterShaderMaterial = NULL;
	}
}


void Screen::drawFilter() {
	
	if(!filterShaderMaterial)
		return;
	
	CoreServices::getInstance()->getRenderer()->bindFrameBufferTexture(originalSceneTexture);
	
	Render();
	//CoreServices::getInstance()->getRenderer()->renderToTexture(originalSceneTexture);	
	
	CoreServices::getInstance()->getRenderer()->unbindFramebuffers();
	
	ShaderBinding* materialBinding;		
	for(int i=0; i < filterShaderMaterial->getNumShaders(); i++) {
		materialBinding = filterShaderMaterial->getShaderBinding(i);
		CoreServices::getInstance()->getRenderer()->applyMaterial(filterShaderMaterial, localShaderOptions[i], i);	
			
		if(i==filterShaderMaterial->getNumShaders()-1) {
	//		CoreServices::getInstance()->getRenderer()->clearScreen();
			CoreServices::getInstance()->getRenderer()->loadIdentity();
			CoreServices::getInstance()->getRenderer()->drawScreenQuad(CoreServices::getInstance()->getRenderer()->getXRes(), CoreServices::getInstance()->getRenderer()->getYRes());		
		} else {
			for(int j=0; j < materialBinding->getNumOutTargetBindings(); j++) {
				//				CoreServices::getInstance()->getRenderer()->clearScreen();
				//				CoreServices::getInstance()->getRenderer()->loadIdentity();
				CoreServices::getInstance()->getRenderer()->bindFrameBufferTexture(materialBinding->getOutTargetBinding(j)->texture);
				//				Logger::log("drawing quad (%s) %f,%f\n", materialBinding->getOutTargetBinding(j)->texture->getResourceName().c_str(), materialBinding->getOutTargetBinding(j)->width, materialBinding->getOutTargetBinding(j)->height);		
				
				CoreServices::getInstance()->getRenderer()->drawScreenQuad(materialBinding->getOutTargetBinding(j)->width, materialBinding->getOutTargetBinding(j)->height);
				CoreServices::getInstance()->getRenderer()->unbindFramebuffers();
				
				//				CoreServices::getInstance()->getRenderer()->renderToTexture(materialBinding->getOutTargetBinding(j)->texture);
			}						
		}
		CoreServices::getInstance()->getRenderer()->clearShader();
		CoreServices::getInstance()->getRenderer()->loadIdentity();
		
		CoreServices::getInstance()->getRenderer()->setOrthoMode();
		/*
		 CoreServices::getInstance()->getRenderer()->renderToTexture(filterTexture);
		 CoreServices::getInstance()->getRenderer()->clearScreen();
		 CoreServices::getInstance()->getRenderer()->loadIdentity();
		 CoreServices::getInstance()->getRenderer()->drawScreenQuad();
		 CoreServices::getInstance()->getRenderer()->clearShader();
		 CoreServices::getInstance()->getRenderer()->loadIdentity();
		 */
	}
	
}

bool Screen::hasFilterShader() const {
	return _hasFilterShader;
}

ScreenEntity* Screen::addChild(ScreenEntity *newEntity) {
	if(!newEntity)
		return NULL;
	children.push_back(newEntity);
	newEntity->setRenderer(renderer);
	newEntity->addEventListener(this, ScreenEvent::ENTITY_MOVE_TOP);
	newEntity->addEventListener(this, ScreenEvent::ENTITY_MOVE_BOTTOM);
	newEntity->addEventListener(this, ScreenEvent::ENTITY_MOVE_DOWN);
	newEntity->addEventListener(this, ScreenEvent::ENTITY_MOVE_UP);
	newEntity->zindex = getHighestZIndex()+1;
	sortChildren();
	return newEntity;
}

ScreenEntity* Screen::removeChild(ScreenEntity *entityToRemove) {
	if(!entityToRemove)
		return NULL;
	
	entityToRemove->removeEventListener(this, ScreenEvent::ENTITY_MOVE_TOP);
	entityToRemove->removeEventListener(this, ScreenEvent::ENTITY_MOVE_BOTTOM);
	entityToRemove->removeEventListener(this, ScreenEvent::ENTITY_MOVE_DOWN);
	entityToRemove->removeEventListener(this, ScreenEvent::ENTITY_MOVE_UP);
	for(int i=0;i<children.size();i++) {
		if(children[i] == entityToRemove) {
			children.erase(children.begin()+i);
		}
	}
	return entityToRemove;
}

void Screen::Shutdown() {
	
}

void Screen::Update() {

}

ScreenEntity *Screen::getEntityAt(Number x, Number y) {
	for(int i=children.size()-1; i >= 0;i--) {
		if(children[i]->hitTest(x,y))
			return children[i];
	}
	return NULL;
}

void Screen::Render() {
	Update();
	renderer->loadIdentity();
	renderer->translate2D(offset.x, offset.y);
	
	renderer->multModelviewMatrix(rootEntity->getConcatenatedMatrix());
	
	for(int i=0; i<children.size();i++) {
		if(children[i]->hasFocus && focusChild != children[i] && children[i]->isFocusable()) {
			if(focusChild != NULL) {
				focusChild->hasFocus = false;
				focusChild->onLoseFocus();
			}
			focusChild = children[i];
			focusChild->onGainFocus();			
		}
		children[i]->doUpdates();
		children[i]->updateEntityMatrix();
		children[i]->transformAndRender();
	}
}
