// ######################################################################################
// ## A minimal working sample debugging windows creation and Direct3D 
// ## initialisation.
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
#include <iostream>

// Link to needed lib files. Can also be done by adding these to Properties -> Linker -> Input -> Additional Dependencies
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

// Window forward declarations.
bool InitialiseWindow();
int Run();
LRESULT CALLBACK WindowProcedure(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);

// DirectX forward declarations.
bool InitialiseDirect3D();			// Changed return type to indicate whether function succeeded or failed.
bool CreateDeviceAndSwapChain();	// Changed return type to indicate whether function succeeded or failed.
bool CreateRenderTargetView();		// Changed return type to indicate whether function succeeded or failed.
void CreateViewport();
bool CreateShaders();				// Changed return type to indicate whether function succeeded or failed.
void Render();

// Window global variables.
HWND gWindowHandle = NULL;
int gWindowWidth = 800;
int gWindowHeight = 600;

// DirectX global variables.
ID3D11Device* gDevice = nullptr;
ID3D11DeviceContext* gContext = nullptr;
IDXGISwapChain* gSwapChain = nullptr;
ID3D11RenderTargetView* gRTV = nullptr;
ID3D11VertexShader* gVertexShader = nullptr;
ID3D11PixelShader* gPixelShader = nullptr;

int main(int argc, char* argv)
{
	// If window cannot be created, it's a terminal error. Show a message box alerting the user that something went wrong.
	if (!InitialiseWindow())
	{
		MessageBox(NULL, L"TERMINAL ERROR: Window initialisation failed\nClosing application...", L"ERROR", MB_OK);
		return -1;
	}

	// If DirectX cannot be initialised, nothing can be drawn. Thus, it's a terminal error.
	// Show a message box alerting the user that something went wrong.
	if (!InitialiseDirect3D())
	{
		MessageBox(NULL, L"TERMINAL ERROR: DirectX initialisation failed\nClosing application...", L"ERROR", MB_OK);
		return -1;
	}

	return Run();		// Run returns the exit code of the program.
}

bool InitialiseWindow()
{
	HINSTANCE applicationHandle = GetModuleHandle(NULL);
	if (applicationHandle == NULL)
	{
		// GetModuleHandle failed and we have no application handle. Error - return false.
		std::cout << "Error: Application handle could not be retreieved (code " << GetLastError() << ")." << std::endl;
		return false;
	}

	WNDCLASS windowClass;
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProcedure;
	windowClass.cbClsExtra = 0;	
	windowClass.cbWndExtra = 0;	
	windowClass.hInstance = applicationHandle;
	windowClass.hIcon = LoadIcon(0, IDI_APPLICATION);
	windowClass.hCursor = LoadCursor(0, IDC_ARROW);	
	windowClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = L"WindowClass";	

	if (RegisterClass(&windowClass) == 0)
	{
		// RegisterClass failed and we can't create our window. Error - return false.
		std::cout << "Error: Window class could not be registered (code " << GetLastError() << ")." << std::endl;
		return false;
	}

	gWindowHandle = CreateWindow(
		L"WindowClass",
		L"Window Title",
		WS_OVERLAPPEDWINDOW,
		100,
		100,
		gWindowWidth,
		gWindowHeight,
		NULL,
		NULL,
		applicationHandle,
		NULL
		);

	if (gWindowHandle == NULL)
	{
		// Window could not be created. Error - return false.
		std::cout << "Error: Window class could not be created (code " << GetLastError() << ")." << std::endl;
		return false;
	}

	ShowWindow(gWindowHandle, SW_SHOWDEFAULT);
	UpdateWindow(gWindowHandle);
	return true;			// No errors - return true.
}

int Run()
{
	MSG windowMsg = {0};

	while (windowMsg.message != WM_QUIT)
	{
		if (PeekMessage(&windowMsg, NULL, NULL, NULL, PM_REMOVE))
		{
			TranslateMessage(&windowMsg);
			DispatchMessage(&windowMsg);
		}
		else
		{
			// If there are no more messages to handle, run a frame 
			// Usually Update() + Render() but in this sample only the latter is needed.
			Render();
		}
	}

	return static_cast<int>(windowMsg.wParam); // WM_QUIT's wParam contains the exit code.
}

LRESULT CALLBACK WindowProcedure(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	// If a message has not been handled, send it to the default window procedure for handling.
	return DefWindowProc(handle, message, wParam, lParam);
}

bool InitialiseDirect3D()
{
	// If any resource can't be created, initialisation has failed - return false.
	if (!CreateDeviceAndSwapChain())
		return false;
	if (!CreateRenderTargetView())
		return false;
	
	CreateViewport();
	
	// Shader creation failure may not be a terminal error, depending on error handling.
	if (!CreateShaders())
		return false;

	return true;			// No errors - return true.
}

bool CreateDeviceAndSwapChain()
{
	// Fill out the swap chain description.
	DXGI_SWAP_CHAIN_DESC scDesc;
	scDesc.BufferDesc.Width = gWindowWidth;
	scDesc.BufferDesc.Height = gWindowHeight;
	scDesc.BufferDesc.RefreshRate.Numerator = 0;
	scDesc.BufferDesc.RefreshRate.Denominator = 0;
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

	// If we're running a Debug build, either DEBUG or _DEBUG should be defined. In that case,
	// the Direct3D device should be created with the debug flag set so that we can take advantage of the debug layer
	UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL supportedFeatureLevel;

	// Saving the function's return code in hr to check whether function failed or succeeded.
	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		createDeviceFlags,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&scDesc,
		&gSwapChain,
		&gDevice,
		&supportedFeatureLevel,
		&gContext
		);

	// FAILED() is a macro checking the hr value, returning true if the hr code indicates failure.
	if (FAILED(hr))
	{
		// Device, DeviceContext and Swap Chain creation failed. Write error message to console and return false.
		std::cout << "Error: Device, DeviceContext and Swap Chain could not be created." << std::endl;
		return false;
	}

	// If the graphics card's highest supported feature level is not 11 or above, our application ends in error.
	if (supportedFeatureLevel < D3D_FEATURE_LEVEL_11_0)
	{
		std::cout << "Error: DirectX 11 is not supported." << std::endl;
		return false;
	}

	return true;			// No errors - return true.
}

bool CreateRenderTargetView()
{
	ID3D11Texture2D* backBuffer;

	// Save the function's return code in hr to check whether function failed or succeeded.
	HRESULT hr = gSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
	
	// FAILED() is a macro checking the hr value, returning true if the hr code indicates failure.
	if (FAILED(hr))
	{
		// Back buffer could not be retrieved. Write error message to console and return false.
		std::cout << "Error: Back buffer could not be retreieved." << std::endl;
		return false;
	}

	// Saving the function's return code in hr to check whether function failed or succeeded.
	hr = gDevice->CreateRenderTargetView(backBuffer, nullptr, &gRTV);

	// FAILED() is a macro checking the hr value, returning true if the hr code indicates failure.
	if (FAILED(hr))
	{
		// Render target view creation failed. Write error message to console and return false.
		std::cout << "Error: Render target view could not be created." << std::endl;
		return false;
	}

	backBuffer->Release();
	gContext->OMSetRenderTargets(1, &gRTV, nullptr);

	return true;			// No errors - return true.
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

bool CreateShaders()
{
	// Define shader contents. More on writing shaders is covered in a later sample.
	const char* vertexShader = R"(
		float4 main(uint id : SV_VERTEXID) : SV_POSITION
		{
			// Magic to create a triangle vertex given an id [0-2].
			return float4(-0.5f + (float)(id % 2), -0.5f + (id == 0), 0.0f, 1.0f);
		}
		)";

	const char* pixelShader = R"(
		float4 main(float4 position : SV_POSITION) : SV_TARGET
		{
			// If the pixel is covered by the triangle, give the pixel red colour.
			return float4(1.0f, 0.0f, 0.0f, 1.0f);
		}
		)";

	// Compile and create vertex shader.
	ID3DBlob* compiledVS = nullptr;	// A variable to hold the compiled vertex shader data.
	ID3DBlob* errorMsg = nullptr;	// A variable to hold the function error messages, if any.

	// Saving the function's return code in hr to check whether function failed or succeeded.
	HRESULT hr = D3DCompile(
		reinterpret_cast<LPCVOID>(vertexShader),
		strlen(vertexShader),
		NULL,
		nullptr,
		nullptr,
		"main",
		"vs_5_0",
		0,
		0,
		&compiledVS,
		&errorMsg				// Pass in the ID3DBlob* pointer for error messages, if any.
		);

	// FAILED() is a macro checking the hr value, returning true if the hr code indicates failure.
	if (FAILED(hr))
	{
		// Shader compilation failed. Write error message to console.
		std::cout << "Error: Vertex shader could not be compiled." << std::endl;
		if (errorMsg != nullptr)
			OutputDebugStringA(static_cast<char*>(errorMsg->GetBufferPointer()));
		return false;		// Terminal error - return false.
	}

	// Saving the function's return code in hr to check whether function failed or succeeded.
	hr = gDevice->CreateVertexShader(
		compiledVS->GetBufferPointer(),
		compiledVS->GetBufferSize(),
		NULL,
		&gVertexShader
		);

	// FAILED() is a macro checking the hr value, returning true if the hr code indicates failure.
	if (FAILED(hr))
	{
		// Shader creation failed. Write error message to console.
		std::cout << "Error: Vertex shader could not be created." << std::endl;
		return false;		// Terminal error - return false.
	}

	// Compile and create pixel shader.
	ID3DBlob* compiledPS = nullptr;	// A variable to hold the compiled pixel shader data.
	errorMsg = nullptr;				// Clear errorMsg so that the vertex shader errors (if any) are removed.

	// Saving the function's return code in hr to check whether function failed or succeeded.
	hr = D3DCompile(
		reinterpret_cast<LPCVOID>(pixelShader),
		strlen(pixelShader),
		NULL,
		nullptr,
		nullptr,
		"main",
		"ps_5_0",
		0,
		0,
		&compiledPS,
		&errorMsg
		);

	// FAILED() is a macro checking the hr value, returning true if the hr code indicates failure.
	if (FAILED(hr))
	{
		// Shader compilation failed. Write error message to console.
		std::cout << "Error: Pixel shader could not be compiled." << std::endl;
		if (errorMsg != nullptr)
			OutputDebugStringA(static_cast<char*>(errorMsg->GetBufferPointer()));
		return false;		// Terminal error - return false.
	}

	// Saving the function's return code in hr to check whether function failed or succeeded.
	hr = gDevice->CreatePixelShader(
		compiledPS->GetBufferPointer(),
		compiledPS->GetBufferSize(),
		NULL,
		&gPixelShader
		);

	// FAILED() is a macro checking the hr value, returning true if the hr code indicates failure.
	if (FAILED(hr))
	{
		// Shader creation failed. Write error message to console.
		std::cout << "Error: Pixel shader could not be created." << std::endl;
		return false;		// Terminal error - return false.
	}

	return true;			// No errors - return true.
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