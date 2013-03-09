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

#include "PolyGlobals.h"
#include "PolyUIWindow.h"
#include "PolyUIButton.h"
#include "PolyUIScrollContainer.h"
#include "PolyUITextInput.h"
#include "OSBasics.h"
#include "PolyInputEvent.h"

namespace Polycode {
	
	class CreateFolderWindow : public UIWindow {
		public:
			CreateFolderWindow();
			~CreateFolderWindow();

			UIButton *okButton;
			UIButton *cancelButton;
			UITextInput *nameInput;

	};

	class UIFileDialogEntry : public UIElement {
		public:
			UIFileDialogEntry(OSFileEntry entry, bool canSelect, int width=340, bool isPlace=false);
			~UIFileDialogEntry();
			void Select();
			void Deselect();

			bool canSelect;
			ScreenShape *bg;
			ScreenLabel *label;
			OSFileEntry fileEntry;
			ScreenImage *icon;
	};

	class UIFileDialog : public UIWindow {
		public:
			UIFileDialog(String baseDir, bool foldersOnly, std::vector<String> extensions, bool allowMultiple);
			virtual ~UIFileDialog();

			void onClose();
			void handleEvent(Event *event);
			void clearEntries();
			void showFolder(String folderPath);
	
			bool canOpen(String extension);

			void addToSidebar(String path, String name);

			void Update();

			String getSelection();
		protected:

			String selection;

			String currentFolderPath;

			UIFileDialogEntry *currentEntry;

			bool foldersOnly;
			bool allowMultiple;

			bool doChangeFolder;
			String newPath;

			UIButton *okButton;
			UIButton *cancelButton;
			UIButton *newFolderButton;

			CreateFolderWindow *createFolderWindow;

			UIScrollContainer *scrollContainer;

			std::vector<String> extensions;
			std::vector<UIFileDialogEntry*> entries;
			std::vector<UIFileDialogEntry*> sideBarEntries;
			UIElement *entryHolder;		
	};
}

