// ######################################################################################
// ## A minimal working sample setting up DirectX 11 and drawing a simple triangle.
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
#include <d3d11.h>
#include <d3dcompiler.h>

// Link to needed lib files. Can also be done by adding these to Properties -> Linker -> Input -> Additional Dependencies
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

// Window forward declarations.
void InitialiseWindow();
void Run();
LRESULT CALLBACK WindowProcedure(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);

// DirectX forward declarations.
void InitialiseDirectX();
void CreateDeviceAndSwapChain();
void CreateRenderTargetView();
void CreateViewport();
void CreateShaders();
void Render();

// Window global variables.
HWND gWindowHandle = 0;
int gWindowWidth = 800;
int gWindowHeight = 600;
ID3D11VertexShader* gVertexShader = nullptr;
ID3D11PixelShader* gPixelShader = nullptr;

// DirectX global variables.
ID3D11Device* gDevice = nullptr;
ID3D11DeviceContext* gContext = nullptr;
IDXGISwapChain* gSwapChain = nullptr;
ID3D11RenderTargetView* gRTV = nullptr;

void main()
{
	InitialiseWindow();
	InitialiseDirectX();
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
	MSG windowMsg = {0};

	while (windowMsg.message != WM_QUIT)
	{
		if (PeekMessage(&windowMsg, NULL, NULL, NULL, PM_REMOVE))
		{
			TranslateMessage(&windowMsg);	// Prepare the message.
			DispatchMessage(&windowMsg);	// Send the message to the window procedure.
		}
		else
		{
			Render();
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

void InitialiseDirectX()
{
	CreateDeviceAndSwapChain();
	CreateRenderTargetView();
	CreateViewport();
	CreateShaders();
}

void CreateDeviceAndSwapChain()
{
	DXGI_SWAP_CHAIN_DESC scDesc;
	scDesc.BufferDesc.Width = gWindowWidth;
	scDesc.BufferDesc.Height = gWindowHeight;
	scDesc.BufferDesc.RefreshRate.Numerator = 60;
	scDesc.BufferDesc.RefreshRate.Denominator = 1;
	scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	scDesc.SampleDesc.Count = 1;
	scDesc.SampleDesc.Quality = 0;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = 1;
	scDesc.OutputWindow = gWindowHandle;
	scDesc.Windowed = true;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scDesc.Flags = 0;

	D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		NULL,
		nullptr,
		NULL,
		D3D11_SDK_VERSION,
		&scDesc,
		&gSwapChain,
		&gDevice,
		nullptr,
		&gContext
		);
}

void CreateRenderTargetView()
{
	ID3D11Texture2D* backbuffer;
	gSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backbuffer));
	gDevice->CreateRenderTargetView(backbuffer, nullptr, &gRTV);
	backbuffer->Release();

	gContext->OMSetRenderTargets(1, &gRTV, nullptr);
}

void CreateViewport()
{
	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = static_cast<float>(gWindowWidth);
	vp.Height = static_cast<float>(gWindowHeight);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	gContext->RSSetViewports(1, &vp);
}

void CreateShaders()
{
	// Define shader contents
	const char* vertexShader = R"(
		float4 main(uint id : SV_VERTEXID) : SV_POSITION
		{
			return float4(-0.5f + (float)(id % 2), -0.5f + (id == 0), 0.0f, 1.0f);
		}
		)";

	const char* pixelShader = R"(
		float4 main(float4 position : SV_POSITION) : SV_TARGET
		{
			return float4(1.0f, 0.0f, 0.0f, 1.0f);
		}
		)";

	// Compile and create vertex shader
	ID3DBlob* compiledShader = nullptr;
	D3DCompile(
		reinterpret_cast<LPCVOID>(vertexShader),
		strlen(vertexShader),
		NULL,
		nullptr,
		nullptr,
		"main",
		"vs_5_0",
		0,
		0,
		&compiledShader,
		nullptr
		);

	gDevice->CreateVertexShader(
		compiledShader->GetBufferPointer(),
		compiledShader->GetBufferSize(),
		NULL,
		&gVertexShader
		);

	// Compile and create pixel shader
	compiledShader = nullptr;
	D3DCompile(
		reinterpret_cast<LPCVOID>(pixelShader),
		strlen(pixelShader),
		NULL,
		nullptr,
		nullptr,
		"main",
		"ps_5_0",
		0,
		0,
		&compiledShader,
		nullptr
		);

	gDevice->CreatePixelShader(
		compiledShader->GetBufferPointer(),
		compiledShader->GetBufferSize(),
		NULL,
		&gPixelShader
		);
}

void Render()
{
	FLOAT bgColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	gContext->ClearRenderTargetView(gRTV, bgColor);

	gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gContext->VSSetShader(gVertexShader, NULL, NULL);
	gContext->PSSetShader(gPixelShader, NULL, NULL);

	gContext->Draw(3, 0);
	gSwapChain->Present(0, 0);
}