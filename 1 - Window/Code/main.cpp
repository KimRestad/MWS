// ######################################################################################
// ## A minimal working sample creating and showing a basic window in Windows.
// ##
// ## Copyright (c) <2015> <Tim Henriksson and Kim Restad>
// ## 
// ## This software is provided 'as-is', without any express or implied
// ## warranty. In no event will the authors be held liable for any damages
// ## arising from the use of this software.
// ## 
// ## Permission is granted to anyone to use this software for any purpose,
// ## including commercial applications, and to alter it and redistribute it
// ## freely, subject to the following restrictions:
// ## 
// ## 1. The origin of this software must not be misrepresented; you must not
// ## claim that you wrote the original software.If you use this software
// ## in a product, an acknowledgement in the product documentation would be
// ## appreciated but is not required.
// ## 2. Altered source versions must be plainly marked as such, and must not be
// ## misrepresented as being the original software.
// ## 3. This notice may not be removed or altered from any source distribution.
// ##
// ######################################################################################

#include <Windows.h>

// Window forward declarations.
void InitialiseWindow();
void Run();
LRESULT CALLBACK WindowProcedure(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);

// Window global variables.
HWND gWindowHandle = NULL;
int gWindowWidth = 800;
int gWindowHeight = 600;

void main()
{
	InitialiseWindow();
	Run();
}

void InitialiseWindow()
{
	HINSTANCE applicationHandle = GetModuleHandle(NULL);
	WNDCLASS windowClass;
	windowClass.style = CS_HREDRAW | CS_VREDRAW;	// Redraw window if height or width is changed.
	windowClass.lpfnWndProc = WindowProcedure;		// Pointer to the callback function defined (see above).
	windowClass.cbClsExtra = 0;						// Extra memory slots. We don't need this.
	windowClass.cbWndExtra = 0;						// Extra memory slots. We don't need this.
	windowClass.hInstance = applicationHandle;		// Handle to the instance containing the class's window procedure.
	windowClass.hIcon = LoadIcon(0, IDI_APPLICATION);				// Load the default window icon.
	windowClass.hCursor = LoadCursor(0, IDC_ARROW);					// Load the default arrow curser.
	windowClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));	// Set the window background to white.
	windowClass.lpszMenuName = NULL;				// The window has no menu.
	windowClass.lpszClassName = L"WindowClass";		// Define a name for this window class.

	RegisterClass(&windowClass);

	gWindowHandle = CreateWindow(
		L"WindowClass",				// The name of the window class created above.
		L"Window Title",			// Text to go in the window title bar.
		WS_OVERLAPPEDWINDOW,		// Window style: overlapped is a combination of several styles.
		100,						// The window's x position in pixels from the main monitor's upper left corner.
		100,						// The window's y position in pixels from the main monitor's upper left corner.
		gWindowWidth,				// The window's width in pixels.
		gWindowHeight,				// The window's height in pixels.
		NULL,						// No parent - our window is the only one.
		NULL,						// No menu so there is no handle to one.
		applicationHandle,			// Handle to the application owning the window.
		NULL						// Info sent to the WM_CREATE message through its lParam member.
		);

	// Window is not shown by default, so ShowWindow must be called for the window passing in how to show the window 
	// (i.e. maximized, normal, minimized). Default value is SW_SHOWDEFAULT.
	ShowWindow(gWindowHandle, SW_SHOWDEFAULT);
	UpdateWindow(gWindowHandle);					// Refresh window.
}

void Run()
{
	MSG windowMsg = { 0 };

	while (windowMsg.message != WM_QUIT)
	{
		if (PeekMessage(&windowMsg, NULL, NULL, NULL, PM_REMOVE))
		{
			TranslateMessage(&windowMsg);	// Prepare the message.
			DispatchMessage(&windowMsg);	// Send the message to the window procedure.
		}
	}
}

LRESULT CALLBACK WindowProcedure(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:			// When window is destroyed, e.g. by clicking the window's close button (x).
		PostQuitMessage(0);		// Post a WM_QUIT message with the exit code 0.
		return 0;				// When a message has been handled, 0 should be returned.
	}

	// If a message has not been handled, send it to the default window procedure for handling.
	return DefWindowProc(handle, message, wParam, lParam);
}