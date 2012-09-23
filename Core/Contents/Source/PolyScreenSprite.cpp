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

#include "PolyScreenSprite.h"
#include "PolyCore.h"
#include "PolyCoreServices.h"
#include "PolyMesh.h"
#include "PolyPolygon.h"
#include "PolyTexture.h"

using std::vector;
using namespace Polycode;

ScreenSprite::ScreenSprite(const String& fileName, Number spriteWidth, Number spriteHeight) : ScreenShape(ScreenShape::SHAPE_RECT, spriteWidth, spriteHeight) {
	this->spriteWidth = spriteWidth;
	this->spriteHeight = spriteHeight;	
	loadTexture(fileName);
		
	spriteUVWidth = 1.0f / ((Number) texture->getWidth() / spriteWidth);
	spriteUVHeight = 1.0f / ((Number) texture->getHeight() / spriteHeight);
	
	currentFrame = 0;
	currentAnimation = NULL;
	
	paused = false;
}

ScreenSprite::~ScreenSprite() {
	
}

void ScreenSprite::addAnimation(const String& name, const String& frames, Number speed) {
	SpriteAnimation *newAnimation = new SpriteAnimation();
	
	vector<String> frameNumbers = frames.split(",");
	
	int numFramesX = texture->getWidth() / spriteWidth;
	int numFramesY = texture->getHeight() / spriteHeight;
	
	int frameNumber;
	int frameX;
	int frameY;
	
	for(int i=0; i < frameNumbers.size(); i++) {
		frameNumber = atoi(frameNumbers[i].c_str());
		frameX = frameNumber % numFramesX;
		frameY = frameNumber/numFramesX;
		newAnimation->framesOffsets.push_back(Vector2(spriteUVWidth * frameX, spriteUVHeight * frameY));		
	}
	
	newAnimation->speed = speed;
	newAnimation->name = name;
	newAnimation->numFrames = frameNumbers.size();
	animations.push_back(newAnimation);
}

void ScreenSprite::playAnimation(const String& name, int startFrame, bool once) {
	paused = false;
	for(int i=0; i < animations.size(); i++) {
		if(animations[i]->name == name) {
			if(currentAnimation == animations[i] && !playingOnce)
				return;
			currentFrame = 0;			
			currentAnimation = animations[i];
			if(currentFrame < currentAnimation->numFrames)
				currentFrame = startFrame;
			playingOnce = once;
			lastTick = 0;
		}
	}
}

void ScreenSprite::Pause(bool val) {
	paused = val;
}

void ScreenSprite::showFrame(unsigned int frameIndex) {
	if(!currentAnimation)
		return;

	if(frameIndex < currentAnimation->numFrames) {
		currentFrame = frameIndex;
		updateSprite();
	}
}

void ScreenSprite::Update() {
	if(!currentAnimation)
		return;
	
	Number newTick = CoreServices::getInstance()->getCore()->getTicksFloat();
	
	Number elapsed = newTick - lastTick;
	
	if(paused)
		return;
	
	if(elapsed > currentAnimation->speed) {
	currentFrame++;
	if(currentFrame >= currentAnimation->numFrames) {
		if(playingOnce) {
			dispatchEvent(new Event(), Event::COMPLETE_EVENT);
			return;			
		} else {
			currentFrame = 0;
		}
	}
	
	updateSprite();
			
	lastTick = newTick;
		
	}
}

void ScreenSprite::updateSprite() {
	Number xOffset = currentAnimation->framesOffsets[currentFrame].x;
	Number yOffset = 1.0f - currentAnimation->framesOffsets[currentFrame].y - spriteUVHeight;
	
	Polygon *imagePolygon = mesh->getPolygon(0);
		
	imagePolygon->getVertex(0)->setTexCoord(xOffset, yOffset+spriteUVHeight);	
	imagePolygon->getVertex(1)->setTexCoord(xOffset+spriteUVWidth, yOffset+spriteUVHeight);
	imagePolygon->getVertex(2)->setTexCoord(xOffset+spriteUVWidth, yOffset);
	imagePolygon->getVertex(3)->setTexCoord(xOffset, yOffset);	
		
	mesh->arrayDirtyMap[RenderDataArray::TEXCOORD_DATA_ARRAY] = true;

}
