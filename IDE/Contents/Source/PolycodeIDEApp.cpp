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

#include "PolycodeIDEApp.h"


using namespace Polycode;

PolycodeIDEApp::PolycodeIDEApp(PolycodeView *view) : EventDispatcher() {
	core = new CocoaCore(view, 900,700,false,true, 0, 0,60);	
	core->addEventListener(this, Core::EVENT_CORE_RESIZE);	
	CoreServices::getInstance()->getRenderer()->setClearColor(0.2,0.2,0.2);
	
	CoreServices::getInstance()->getRenderer()->setTextureFilteringMode(Renderer::TEX_FILTERING_NEAREST);
				
	CoreServices::getInstance()->getResourceManager()->addArchive("default.pak");
	CoreServices::getInstance()->getResourceManager()->addDirResource("default");	

	CoreServices::getInstance()->getResourceManager()->addArchive("api.pak");

	CoreServices::getInstance()->getConfig()->loadConfig("Polycode", RESOURCE_PATH"UIThemes/default/theme.xml");
	CoreServices::getInstance()->getResourceManager()->addDirResource(RESOURCE_PATH"UIThemes/default/", false);
	CoreServices::getInstance()->getResourceManager()->addDirResource(RESOURCE_PATH"Images/", false);	
	
	CoreServices::getInstance()->getRenderer()->setTextureFilteringMode(Renderer::TEX_FILTERING_LINEAR);
		
	printf("creating font editor\n"); 
	
	Screen *screen = new Screen();	
	screen->snapToPixelsByDefault = true;
	
	editorManager = new PolycodeEditorManager();
	
	frame = new PolycodeFrame();
	frame->setPositionMode(ScreenEntity::POSITION_TOPLEFT);

	frame->console->backtraceWindow->addEventListener(this, BackTraceEvent::EVENT_BACKTRACE_SELECTED);

	frame->textInputPopup->addEventListener(this, UIEvent::OK_EVENT);	
	frame->newProjectWindow->addEventListener(this, UIEvent::OK_EVENT);
	frame->newFileWindow->addEventListener(this, UIEvent::OK_EVENT);	
	frame->exampleBrowserWindow->addEventListener(this, UIEvent::OK_EVENT);
	
	frame->playButton->addEventListener(this, UIEvent::CLICK_EVENT);
	frame->stopButton->addEventListener(this, UIEvent::CLICK_EVENT);
		
	screen->addChild(frame);
	
	projectManager = new PolycodeProjectManager();
	projectManager->setProjectBrowser(frame->getProjectBrowser());
	
	frame->getProjectBrowser()->addEventListener(this, Event::CHANGE_EVENT);
	frame->getProjectBrowser()->addEventListener(this, PolycodeProjectBrowserEvent::SHOW_MENU);
	
	frame->Resize(core->getXRes(), core->getYRes());	
	core->setVideoMode(1000, 700, false, false, 0, 0);
	
	debugger = new PolycodeRemoteDebugger(projectManager);
	frame->console->setDebugger(debugger);
	
	editorManager->registerEditorFactory(new PolycodeImageEditorFactory());
	editorManager->registerEditorFactory(new PolycodeScreenEditorFactory());	
	editorManager->registerEditorFactory(new PolycodeFontEditorFactory());
	editorManager->registerEditorFactory(new PolycodeTextEditorFactory());
	editorManager->registerEditorFactory(new PolycodeProjectEditorFactory(projectManager));
		
	
	
//	CoreServices::getInstance()->getResourceManager()->addArchive(RESOURCE_PATH"tomato.polyapp");
	
//	ScreenImage *img = new ScreenImage("tomato.png");
//	screen->addChild(img);
	
	loadConfigFile();
}

void PolycodeIDEApp::renameFile() {
	if(projectManager->selectedFile != "") {
		frame->textInputPopup->setValue(projectManager->selectedFileEntry.name);
		frame->showModal(frame->textInputPopup);
	}
}

void PolycodeIDEApp::removeFile() {
	if(projectManager->selectedFile != "") {
		core->removeDiskItem(projectManager->selectedFile);
		if(projectManager->getActiveProject()) {
			frame->projectBrowser->refreshProject(projectManager->getActiveProject());
		}
	}
}

void PolycodeIDEApp::newProject() {
	frame->newProjectWindow->ResetForm();
	frame->showModal(frame->newProjectWindow);
	
}

void PolycodeIDEApp::newFile() {
	if(projectManager->getActiveProject()) {
		frame->newFileWindow->resetForm();
		frame->showModal(frame->newFileWindow);
	}
}

void PolycodeIDEApp::refreshProject() {
	if(projectManager->getActiveProject()) {
		frame->getProjectBrowser()->refreshProject(projectManager->getActiveProject());
	}
}

void PolycodeIDEApp::closeProject() {
	if(projectManager->getActiveProject()) {
		frame->getProjectBrowser()->removeProject(projectManager->getActiveProject());
		projectManager->removeProject(projectManager->getActiveProject());
	}
}

void PolycodeIDEApp::newGroup() {
	if(projectManager->activeFolder != "") {
		core->createFolder(projectManager->activeFolder+"/New Folder");
		if(projectManager->getActiveProject()) {
			frame->getProjectBrowser()->refreshProject(projectManager->getActiveProject());
		}
	}
}

void PolycodeIDEApp::openProject() {
	
	vector<CoreFileExtension> extensions;
	CoreFileExtension ext;
	ext.extension = "polyproject";
	ext.description = "Polycode Project File";
	extensions.push_back(ext);
	std::vector<String> paths = core->openFilePicker(extensions, false);
	if(paths[0] != "") {
		PolycodeProject *project = projectManager->openProject(paths[0]);
		if(project) {
			projectManager->setActiveProject(project);
		}
	}		
}

void PolycodeIDEApp::browseExamples() {
	frame->newProjectWindow->ResetForm();
	frame->showModal(frame->exampleBrowserWindow);

}

void PolycodeIDEApp::stopProject() {
	printf("Disconnecting clients...\n");
	if(debugger->isConnected()) {
		debugger->Disconnect();
	}
}

void PolycodeIDEApp::runProject() {
	printf("Running project...\n");
	stopProject();

	if(projectManager->getActiveProject()) {
		String outPath = PolycodeToolLauncher::generateTempPath(projectManager->getActiveProject()) + ".polyapp";
		PolycodeToolLauncher::buildProject(projectManager->getActiveProject(), outPath);
		PolycodeToolLauncher::runPolyapp(outPath);
	} else {
		PolycodeConsole::print("No active project!\n");
	}
}

void PolycodeIDEApp::findText() {
	if(editorManager->getCurrentEditor()) {
		if(editorManager->getCurrentEditor()->getEditorType() == "PolycodeTextEditor") {
			PolycodeTextEditor *textEditor = (PolycodeTextEditor*) editorManager->getCurrentEditor();
			textEditor->showFindBar();
		}
	}
}

void PolycodeIDEApp::saveFile() {
	if(editorManager->getCurrentEditor()) {
		editorManager->getCurrentEditor()->saveFile();
	}
}

void PolycodeIDEApp::openFileInProject(PolycodeProject *project, String filePath) {
	OSFileEntry fileEntry = OSFileEntry(project->getRootFolder()+"/"+filePath, OSFileEntry::TYPE_FILE);	
	OSFILE *file = OSBasics::open(project->getRootFolder()+"/"+filePath,"r");
	
	if(file) {
		OSBasics::close(file);
		openFile(fileEntry);		
	} else {
		fileEntry = OSFileEntry(filePath, OSFileEntry::TYPE_FILE);	
		file = OSBasics::open(filePath,"r");	
		if(file) {
			OSBasics::close(file);
			openFile(fileEntry);							
		} else {
			PolycodeConsole::print("File not available.\n");
		}
	}

}

void PolycodeIDEApp::openFile(OSFileEntry file) {
	PolycodeEditor *editor;
	editor = editorManager->getEditorForPath(file.fullPath);
	if(editor) {
		frame->showEditor(editor);				
	} else {
		editor = editorManager->createEditorForExtension(file.extension);
		if(editor) {
			if(editor->openFile(file)) {
				frame->addEditor(editor);					
				frame->showEditor(editor);
			} else {
				delete editor;
				editor = NULL;
			}
		}
	}
							
	if(editor) {
		editorManager->setCurrentEditor(editor);
	}
		
}

void PolycodeIDEApp::handleEvent(Event *event) {

	if(event->getDispatcher() == frame->console->backtraceWindow) {
		if(event->getEventType() == "BackTraceEvent" && event->getEventCode() == BackTraceEvent::EVENT_BACKTRACE_SELECTED) {
			BackTraceEvent *btEvent = (BackTraceEvent*) event;
			openFileInProject(btEvent->project, btEvent->fileName);
			
			PolycodeEditor *editor = editorManager->getCurrentEditor();
			if(editor) {
				if(editor->getEditorType() == "PolycodeTextEditor") {
					PolycodeTextEditor *textEditor = (PolycodeTextEditor*) editor;
					textEditor->highlightLine(btEvent->lineNumber);
				}
				
			}	
		}
	}

	if(event->getDispatcher() == core) {
		switch(event->getEventCode()) {
			case Core::EVENT_CORE_RESIZE:
				frame->Resize(core->getXRes(), core->getYRes());
			break;
		}
	}
	
	if(event->getDispatcher()  == frame->getProjectBrowser()) {
		
		if(event->getEventType() == "PolycodeProjectBrowserEvent") {
			switch(event->getEventCode()) {
				case PolycodeProjectBrowserEvent::SHOW_MENU:
					dispatchEvent(new Event(), EVENT_SHOW_MENU);
				break;
			}
		}
		
		if(event->getEventCode() == Event::CHANGE_EVENT) {
			BrowserUserData *selectedData = frame->getProjectBrowser()->getSelectedData();
						
			if(selectedData->type == 3) {
				projectManager->activeFolder = selectedData->parentProject->getRootFolder();
				projectManager->selectedFile = "";				
			} else if(selectedData->type == 0) {
				projectManager->activeFolder = "";
				projectManager->selectedFile = "";
			} else {
				projectManager->selectedFileEntry = selectedData->fileEntry;
				projectManager->selectedFile = selectedData->fileEntry.fullPath;
				if(selectedData->fileEntry.type == OSFileEntry::TYPE_FILE) {
					projectManager->activeFolder = selectedData->fileEntry.basePath;
				} else {
					projectManager->activeFolder = selectedData->fileEntry.fullPath;
				}			
			}
			
			projectManager->setActiveProject(selectedData->parentProject);
			
			if(selectedData->type == 0)
				return;			
			
			if(selectedData) {
				openFile(selectedData->fileEntry);
			}
		}
	}
	
	if(event->getDispatcher() == frame->playButton) {	
		if(event->getEventType() == "UIEvent" && event->getEventCode() == UIEvent::CLICK_EVENT) {
			runProject();
		}
	}

	if(event->getDispatcher() == frame->stopButton) {	
		if(event->getEventType() == "UIEvent" && event->getEventCode() == UIEvent::CLICK_EVENT) {
			stopProject();
		}
	}

	
	if(event->getDispatcher() == frame->textInputPopup) {
		if(event->getEventType() == "UIEvent" && event->getEventCode() == UIEvent::OK_EVENT) {
			core->moveDiskItem(projectManager->selectedFileEntry.fullPath, projectManager->selectedFileEntry.basePath + "/" + frame->textInputPopup->getValue());			
			if(projectManager->getActiveProject()) {
				frame->getProjectBrowser()->refreshProject(projectManager->getActiveProject());
			}
			
			PolycodeEditor *editor = editorManager->getEditorForPath(projectManager->selectedFileEntry.fullPath);
			if(editor) {
				editor->setFilePath(projectManager->selectedFileEntry.basePath + "/" + frame->textInputPopup->getValue());
			}
			
			projectManager->selectedFileEntry.fullPath = projectManager->selectedFileEntry.basePath + "/" + frame->textInputPopup->getValue();
			projectManager->selectedFileEntry.name = frame->textInputPopup->getValue();
			
			
			frame->hideModal();			
		}
	}	
	
	if(event->getDispatcher() == frame->newProjectWindow) {
		if(event->getEventType() == "UIEvent" && event->getEventCode() == UIEvent::OK_EVENT) {
			projectManager->createNewProject(frame->newProjectWindow->getTemplateFolder(), frame->newProjectWindow->getProjectName(), frame->newProjectWindow->getProjectLocation());
			frame->hideModal();			
		}
	}

	if(event->getDispatcher() == frame->newFileWindow) {
		if(event->getEventType() == "UIEvent" && event->getEventCode() == UIEvent::OK_EVENT) {
			projectManager->createNewFile(frame->newFileWindow->getTemplatePath(), frame->newFileWindow->getFileName());
			frame->hideModal();			
			
			if(projectManager->getActiveProject()) {
				frame->projectBrowser->refreshProject(projectManager->getActiveProject());
			}			
		}
	}
	
	if(event->getDispatcher() == frame->exampleBrowserWindow) {
		if(event->getEventType() == "UIEvent" && event->getEventCode() == UIEvent::OK_EVENT) {
			String fullPath = String(core->getDefaultWorkingDirectory()+"/"+frame->exampleBrowserWindow->getExamplePath());
			projectManager->openProject(fullPath);
			frame->hideModal();			
		}
	}	
}

void PolycodeIDEApp::saveConfigFile() {
	Object configFile;
	configFile.root.name = "config";
	configFile.root.addChild("open_projects");
	for(int i=0; i < projectManager->getProjectCount(); i++) {
		PolycodeProject *project = projectManager->getProjectByIndex(i);		
		ObjectEntry *projectEntry = configFile.root["open_projects"]->addChild("project");
		projectEntry->addChild("name", project->getProjectName());
		projectEntry->addChild("path", project->getProjectFile());
	}
	core->createFolder(core->getUserHomeDirectory()+"/Library/Application Support/Polycode");
	configFile.saveToXML(core->getUserHomeDirectory()+"/Library/Application Support/Polycode/config.xml");	
}

void PolycodeIDEApp::loadConfigFile() {

	Object configFile;
	configFile.loadFromXML(core->getUserHomeDirectory()+"/Library/Application Support/Polycode/config.xml");		
	if(configFile.root["open_projects"]) {
		ObjectEntry *projects = configFile.root["open_projects"];
		if(projects) {
		for(int i=0; i < projects->length; i++) {
			ObjectEntry *entry = (*(*projects)[i])["path"];
			if(entry) {
				projectManager->openProject(entry->stringVal);	
			}
		}
		}
	}
}


PolycodeIDEApp::~PolycodeIDEApp() {
	
	saveConfigFile();
	
	delete core;
}

bool PolycodeIDEApp::Update() {

	if(debugger->isConnected()) {
			frame->stopButton->visible = true;
			frame->stopButton->enabled = true;			
			
			frame->playButton->visible = false;
			frame->playButton->enabled = false;			
			
	} else {
			frame->stopButton->visible = false;
			frame->stopButton->enabled = false;			
			
			frame->playButton->visible = true;
			frame->playButton->enabled = true;				
	}

	if(projectManager->getProjectCount() == 1) {
		projectManager->setActiveProject(projectManager->getProjectByIndex(0));
	}
	
	if(projectManager->getProjectCount() > 0) {
		frame->welcomeEntity->enabled =  false;
		frame->projectBrowser->enabled =  true;		
		frame->mainSizer->enabled = true;
	} else {
		frame->welcomeEntity->enabled =  true;
		frame->projectBrowser->enabled =  false;			
		frame->mainSizer->enabled = false;		
	}

	return core->Update();
}

