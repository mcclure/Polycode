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
 
#pragma once

#include "PolycodeEditor.h"
#include <Polycode.h>
#include <PolycodeUI.h>
#include "PolycodeProps.h"

using namespace Polycode;

class EntityBrowserData  {
	public:
		Entity *entity;
};

class EntityTreeView : public UIElement {
	public:
		EntityTreeView(Entity *rootEntity);
		~EntityTreeView();
		
		void Resize(Number width, Number height);
		void handleEvent(Event *event);	
		
		void syncNodeToEntity(UITree *node, Entity *entity);
		
		void Refresh();

		Entity *selectedEntity;
		Entity *targetLayer;
		
		UIImageButton *newLayerButton;
		UIImageButton *targetLayerButton;
		
		Entity *rootEntity;
						
	protected:	
	
		bool dontSendSelectionEvent;
	
		UITreeContainer *treeContainer;
			
		ScreenShape *headerBg;		
		ScreenShape *bg;		
				
};

class PropertiesSizer : public UIElement {
	public:
		PropertiesSizer();
		~PropertiesSizer();
		
		void Resize(Number width, Number height);
		
		UIVSizer *sizer;
};


class ScreenEntityNameDisplay : public ScreenEntity {
	public:
		ScreenEntityNameDisplay(ScreenEntity *targetEntity);
		~ScreenEntityNameDisplay();
		
		void Update();
		
		ScreenLabel *label;
		ScreenLabel *tagsLabel;		
		ScreenEntity *targetEntity;
};

class PolycodeScreenEditorMain : public UIElement {
	public:
		
		PolycodeScreenEditorMain();
		virtual ~PolycodeScreenEditorMain();	
			
		void Resize(Number width, Number height);	
		void syncTransformToSelected();	
		ScreenEntity *addNewLayer(String layerName);	
		void updateCursor();		
		void selectEntity(ScreenEntity *entity);		
		void setMode(int newMode);	
		void handleEvent(Event *event);	
		void resizePreviewScreen();		
		void handleDroppedFile(OSFileEntry file, Number x, Number y);		
		bool hasSelected(ScreenEntity *entity);
	
		void applyEditorOnly(ScreenEntity *entity);
		void applyEditorProperties(ScreenEntity *entity);
			
		void processEventForEntity(ScreenEntity *childEntity, InputEvent *inputEvent);
	
		void createParticleRef(ScreenParticleEmitter *target);
		void createSoundRef(ScreenSound *target);
		void createEntityRef(ScreenEntity *entity);
	
		void handleMouseDown(Vector2 position);
		void handleMouseMove(Vector2 position);
		void handleMouseUp(Vector2 position);
		
		void setGrid(int gridSize);
				
		static const int MODE_SELECT = 0;
		static const int MODE_SHAPE = 1;
		static const int MODE_ZOOM = 2;
		static const int MODE_PAN = 3;
		static const int MODE_IMAGE = 4;
		static const int MODE_TEXT = 5;
		static const int MODE_PARENT = 6;
		static const int MODE_ENTITY = 7;
		static const int MODE_SOUND = 8;
		static const int MODE_LINK = 9;
		static const int MODE_SPRITE = 10;
		static const int MODE_PARTICLES = 11;
																
		std::vector<ScreenEntity*> layers;
		
		ScreenEntity *currentLayer;
		
		PropList *entityProps;
		
		ScreenEntity *layerBaseEntity;		
						
		ScreenEntity *selectedEntity;
		std::vector<ScreenEntity*> selectedEntities;		
		
		Transform2DSheet *transform2dSheet;
		EntitySheet *entitySheet;
		ShapeSheet *shapeSheet;
		ScreenImageSheet *imageSheet;
		ScreenLabelSheet *labelSheet;
		SoundSheet *soundSheet;
		ScreenEntityInstanceSheet *instanceSheet;
		ScreenSpriteSheet *spriteSheet;
		ScreenParticleSheet *particleSheet;
		
		EntityTreeView *treeView;
		
		ScreenEntity *objectBaseEntity;		
		ScreenEntity *placingPreviewEntity;												
	protected:
	
		bool multiSelect;
	
		int gridSize;
		bool gridSnap;
	
		ScreenShape *previewShape;
		ScreenImage *previewImage;
		ScreenEntityInstance *previewInstance;		
		ScreenLabel *previewLabel;							
		ScreenEntity *previewEntity;
		ScreenParticleEmitter *previewEmitter;
							
		ScreenSound *previewSound;
		ScreenSprite *previewSprite;
											
		ScreenMesh *grid;

		bool firstResize;
		
		int placementCount;
	
		
		Number previewAspectRatio;

		bool placing;
		bool moving;
		bool scalingY;
		bool scalingX;		
		bool rotating;
		bool panning;
		bool zooming;
		
		bool parenting;
		ScreenEntity *parentingChild;
		ScreenLine *parentingLine;
		
		
		int lastMode;
		
		Vector2 basePanPosition;
		
		Number baseZoomScale;
		
		Number baseRotateAngle;
		
		int selectedEntityPositionMode;
		
		Vector3 baseScale;
		
		Vector2 mouseBase;
		std::vector<Vector2> baseEntityPositions;
		Number baseAngle;
	
		ScreenShape *placingShape;	
		Vector2 placingAnchor;
		
		UIColorBox *entityColorBox;
		UIWindow *entityInfoWindow;
		
		ScreenShape *screenPreviewShape;
		
		Vector2 dragOffset;
		bool isDraggingEntity;
		bool isScalingEntity;
		
		ScreenEntity *baseEntity;

		
		ScreenEntity *screenTransform;
		ScreenShape *screenTransformShape;
		ScreenImage *transformRotator;

		ScreenImage *transformScalerY;
		ScreenImage *transformScalerX;		
					
		ScreenImage *centerImage;
		
		ScreenEntity *properties;
		ScreenShape *propertiesBg;

		ScreenEntity *viewOptions;
		ScreenShape *viewOptionsBg;
		
		UITextInput *gridSizeInput;
		UICheckBox *gridCheckBox;
		
		UICheckBox *pixelSnapBox;
		UICheckBox *gridSnapBox;
		
		UITextInput *scaleInput;
		
		ScreenEntity *toolPalette;
		ScreenShape *toolPaletteBg;
		
		UIButton *resetZoom;
		
		UIImageButton *arrowToolButton;
		UIImageButton *shapeToolButton;
		UIImageButton *zoomToolButton;
		UIImageButton *panToolButton;
		UIImageButton *imageToolButton;
		UIImageButton *textToolButton;
		UIImageButton *parentToolButton;
		UIImageButton *entityToolButton;
		UIImageButton *soundToolButton;
		UIImageButton *spriteToolButton;
		UIImageButton *particlesToolButton;
				
		UIImageButton *linkToolButton;
						
		UIComboBox *aspectComboBox;
		UIComboBox *zoomComboBox;
		
		UIButton *moveUpButton;
		UIButton *moveDownButton;
		UIButton *moveTopButton;
		UIButton *moveBottomButton;
		UIButton *unparentButton;
								
		Number zooms[16];
						
		Number aspects[16];
				
		ScreenImage *selectorImage;
		int mode;	
};

class PolycodeScreenEditor : public PolycodeEditor {
	public:
		PolycodeScreenEditor();
		virtual ~PolycodeScreenEditor();
	
		bool openFile(OSFileEntry filePath);
		void Resize(int x, int y);
		
		void Activate();		
		
		void saveCurveToObject(ObjectEntry *entry, BezierCurve *curve);
		void saveEntityToObjectEntry(ScreenEntity *entity, ObjectEntry *entry);
		void saveFile();
					
		void handleDroppedFile(OSFileEntry file, Number x, Number y);

		void handleEvent(Event *event);

	protected:

		UIHSizer *mainSizer;
		PropertiesSizer *propSizer;
		EntityTreeView *treeView;
		PolycodeScreenEditorMain *editorMain;
};

class PolycodeScreenEditorFactory : public PolycodeEditorFactory {
	public:
		PolycodeScreenEditorFactory() : PolycodeEditorFactory() { extensions.push_back("entity2d"); }
		PolycodeEditor *createEditor() { return new PolycodeScreenEditor(); }
};
