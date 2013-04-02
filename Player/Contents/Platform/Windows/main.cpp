
#include "PolycodeWindowsPlayer.h"
#include <Polycode.h>
#include "PolycodeView.h"
#include "windows.h"
#include "resource.h"

#define STANDALONE_MODE

using namespace Polycode;


extern Win32Core *core;

void wtoc(char* Dest, TCHAR* Source, int SourceSize)
{
for(int i = 0; i < SourceSize; ++i)
Dest[i] = (char)Source[i];
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	int nArgs;
	LPWSTR *szArglist =  CommandLineToArgvW(GetCommandLineW(), &nArgs);
	
	String fileName = "";
	if(nArgs > 1) {
		fileName = String(szArglist[1]);
	}

	PolycodeView *view = new PolycodeView(hInstance, nCmdShow, L"Polycode Player", false, false);
	PolycodeWindowsPlayer *player = new PolycodeWindowsPlayer(view, fileName.c_str(), false, true);

//	player->addEventListener(view, PolycodeDebugEvent::EVENT_ERROR);
//	player->addEventListener(view, PolycodeDebugEvent::EVENT_PRINT);


	player->runPlayer();

	core = (Win32Core*)player->getCore();

	MSG Msg;

		do {
			if(PeekMessage(&Msg, NULL, 0,0,PM_REMOVE)) {
				TranslateMessage(&Msg);
				DispatchMessage(&Msg);
			}
		} while(player->Update());
	
	return Msg.wParam;
}
