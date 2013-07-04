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

#include "PolyUIBox.h"

using namespace Polycode;

UIBox::UIBox(String imageFile, Number t, Number r, Number b, Number l, Number boxWidth, Number boxHeight) : UIElement() {
	
	setPositionMode(ScreenEntity::POSITION_TOPLEFT);	
	
	width=boxWidth;
	height = boxHeight;
	setHitbox(boxWidth, boxHeight);
	
	tlImage = new ScreenImage(imageFile);
	tlImage->setImageCoordinates(0,0,l,t);
	addChild(tlImage);
	tlImage->setPosition(0, 0);
	
	trImage = new ScreenImage(imageFile);
	trImage->setImageCoordinates(trImage->getWidth()-r,0,r,t);
	addChild(trImage);	
	trImage->setPosition(boxWidth-r, 0);

	blImage = new ScreenImage(imageFile);
	blImage->setImageCoordinates(0,blImage->getHeight()-b,l,b);
	addChild(blImage);	
	blImage->setPosition(0, boxHeight-b);

	brImage = new ScreenImage(imageFile);
	brImage->setImageCoordinates(brImage->getWidth()-r,brImage->getHeight()-b,r,b);
	addChild(brImage);	
	brImage->setPosition(boxWidth-r, boxHeight-b);	
	
	centerImage = new ScreenImage(imageFile);
	centerImage->setImageCoordinates(l,t,centerImage->getWidth()-l-r, centerImage->getHeight()-t-b);
	addChild(centerImage);	
	centerImage->setPosition(l,t);	
	centerImage->setShapeSize(boxWidth-l-r, boxHeight-t-b);
	
	tImage = new ScreenImage(imageFile);
	tImage->setImageCoordinates(l,0,tImage->getWidth()-l-r,t);
	addChild(tImage);
	tImage->setPosition(l,0);	
	tImage->setShapeSize(boxWidth-l-r, t);

	bImage = new ScreenImage(imageFile);
	bImage->setImageCoordinates(l,bImage->getHeight()-b,bImage->getWidth()-l-r,b);
	addChild(bImage);
	bImage->setPosition(l,boxHeight-b);	
	bImage->setShapeSize(boxWidth-l-r, b);

	lImage = new ScreenImage(imageFile);
	lImage->setImageCoordinates(0,t,l,lImage->getHeight()-t-b);
	addChild(lImage);
	lImage->setPosition(0,t);	
	lImage->setShapeSize(l, boxHeight-t-b);

	rImage = new ScreenImage(imageFile);
	rImage->setImageCoordinates(rImage->getWidth()-r,t,r,rImage->getHeight()-t-b);
	addChild(rImage);
	rImage->setPosition(boxWidth-r,t);	
	rImage->setShapeSize(r, boxHeight-t-b);	
	
	this->t = t;
	this->r = r;
	this->b = b;
	this->l = l;
	
	ownsChildren = true;	
}

void UIBox::resizeBox(Number newWidth, Number newHeight) {
	brImage->setPosition(newWidth-brImage->getWidth(), newHeight-brImage->getHeight());
	trImage->setPosition(newWidth-trImage->getWidth(), 0);
	blImage->setPosition(0, newHeight-blImage->getHeight());
	
	centerImage->setShapeSize(newWidth-l-r, newHeight-t-b);
		
	lImage->setShapeSize(l, newHeight-t-b);	
	rImage->setShapeSize(r, newHeight-t-b);		
	bImage->setShapeSize(newWidth-l-r, b);
	tImage->setShapeSize(newWidth-l-r, t);	
	
	bImage->setPosition(bImage->getPosition().x, newHeight-bImage->getHeight());	
	rImage->setPosition(newWidth-rImage->getWidth(),rImage->getPosition().y);
	
	width=newWidth;
	height = newHeight;
	setHitbox(newWidth, newHeight);
	
	this->rebuildTransformMatrix();
}

UIBox::~UIBox() {
	if(!ownsChildren) {
		delete tlImage;
		delete trImage;		
		delete blImage;		
		delete brImage;					
		delete centerImage;		
		delete tImage;
		delete rImage;
		delete bImage;
		delete lImage;		
	}
}
