// ######################################################################################
// ## A minimal working sample setting up DirectX 11 and drawing a simple coloured
// ## rectangle using a vertex buffer, input layout and shaders in separate files.
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
#include <DirectXMath.h>

// Link to needed lib files. Can also be done by adding these to Properties -> Linker -> Input -> Additional Dependencies
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

// Define the information contained in each vertex.
struct Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 colour;
	DirectX::XMFLOAT2 uv;
};

// Window forward declarations.
void InitialiseWindow();
void Run();
LRESULT CALLBACK WindowProcedure(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);

// DirectX forward declarations.
void InitialiseDirect3D();
void CreateDeviceAndSwapChain();
void CreateRenderTargetView();
void CreateViewport();

// Remaining forward declarations
void SetupScene();
void CreateVertexBuffer();
void CreateShaders();
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
ID3D11InputLayout* gInputLayout = nullptr;
ID3D11Buffer* gVertexBuffer = nullptr;

void main()
{
	InitialiseWindow();
	InitialiseDirect3D();
	SetupScene();
	Run();
}

void InitialiseWindow()
{
	// Register the window class to create.
	HINSTANCE applicationHandle = GetModuleHandle(NULL);
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

	RegisterClass(&windowClass);

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

	ShowWindow(gWindowHandle, SW_SHOWDEFAULT);
	UpdateWindow(gWindowHandle);
}

void Run()
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

void InitialiseDirect3D()
{
	CreateDeviceAndSwapChain();
	CreateRenderTargetView();
	CreateViewport();
}

void CreateDeviceAndSwapChain()
{
	DXGI_SWAP_CHAIN_DESC scDesc;
	scDesc.BufferDesc.Width = gWindowWidth;			// Using the window's size avoids weird effects. If 0 the window's client width is used.
	scDesc.BufferDesc.Height = gWindowHeight;		// Using the window's size avoids weird effects. If 0 the window's client height is used.
	scDesc.BufferDesc.RefreshRate.Numerator = 0;	// Screen refresh rate as RationalNumber. Zeroing it out makes DXGI calculate it.
	scDesc.BufferDesc.RefreshRate.Denominator = 0;	// Screen refresh rate as RationalNumber. Zeroing it out makes DXGI calculate it.
	scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;						// The most common format. Variations include [...]UNORM_SRGB.
	scDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;	// The order pixel rows are drawn to the back buffer doesn't matter.
	scDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;					// Since the back buffer and window sizes matches, scaling doesn't matter.
	scDesc.SampleDesc.Count = 1;												// Disable multisampling.
	scDesc.SampleDesc.Quality = 0;												// Disable multisampling.
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;						// The back buffer will be rendered to.
	scDesc.BufferCount = 1;							// We only have one back buffer.
	scDesc.OutputWindow = gWindowHandle;			// Must point to the handle for the window used for rendering.
	scDesc.Windowed = true;							// Run in windowed mode. Fullscreen is covered in a later sample.
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;	// This makes the display driver select the most efficient technique.
	scDesc.Flags = 0;								// No additional options.

	D3D11CreateDeviceAndSwapChain(
		nullptr,					// Use the default adapter.
		D3D_DRIVER_TYPE_HARDWARE,	// Use the graphics card for rendering. Other options include software emulation.
		NULL,						// NULL since we don't use software emulation.
		NULL,						// No creation flags.
		nullptr,					// Array of feature levels to try using. With null the following are used 11.0, 10.1, 10.0, 9.3, 9.2, 9.1.
		0,							// The array above has 0 elements.
		D3D11_SDK_VERSION,			// Always use this.
		&scDesc,					// Description of the swap chain.
		&gSwapChain,				// [out] The created swap chain.
		&gDevice,					// [out] The created device.
		nullptr,					// [out] The highest supported feature level (from array).
		&gContext					// [out] The created device context.
		);
}

void CreateRenderTargetView()
{
	// Get the back buffer from the swap chain, create a render target view of it to use as the target for rendering.
	ID3D11Texture2D* backBuffer;
	gSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
	gDevice->CreateRenderTargetView(backBuffer, nullptr, &gRTV);
	backBuffer->Release();

	gContext->OMSetRenderTargets(1, &gRTV, nullptr);
}

void CreateViewport()
{
	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0.0f;		// The top left corner's x coordinate in pixels from the window's top left corner.
	vp.TopLeftY = 0.0f;		// The top left corner's y coordinate in pixels from the window's top left corner.
	vp.Width = static_cast<float>(gWindowWidth);	// This viewport will cover the entire window.
	vp.Height = static_cast<float>(gWindowHeight);	// This viewport will cover the entire window.
	vp.MinDepth = 0.0f;		// Minimum depth value used by Direct3D is 0.0f so this is used.
	vp.MaxDepth = 1.0f;		// Maximum depth value used by Direct3D is 1.0f so this is used.

	gContext->RSSetViewports(1, &vp);				// Set the viewport to use.
}

void SetupScene()
{
	CreateVertexBuffer();
	CreateShaders();
}

void CreateVertexBuffer()
{
	// Create vertices.
	Vertex vertices[] = 
	{
		// First triangle.
		{ DirectX::XMFLOAT3(-0.5f, 0.5f, 0.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },	// Vertex 0, red
		{ DirectX::XMFLOAT3(0.5f, -0.5f, 0.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },	// Vertex 1, green
		{ DirectX::XMFLOAT3(-0.5f, -0.5f, 0.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },	// Vertex 2, blue

		// Second triangle, using two of the same vertices as the first triangle: vertex 1 and vertex 0.
		{ DirectX::XMFLOAT3(0.5f, 0.5f, 0.0f), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },		// Vertex 3, white
		{ DirectX::XMFLOAT3(0.5f, -0.5f, 0.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },	// Vertex 1, green
		{ DirectX::XMFLOAT3(-0.5f, 0.5f, 0.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },	// Vertex 0, red
	};

	// Fill out the buffer description to use when creating our vertex buffer.
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = sizeof(vertices);			// The buffer needs to know the total size of its data, i.e. all vertices.
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;			// A buffer whose contents never change after creation is IMMUTABLE.
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;	// For a vertex buffer, this flag must be specified.
	bufferDesc.CPUAccessFlags = 0;						// The CPU won't access the buffer after creation.
	bufferDesc.MiscFlags = 0;							// The buffer is not doing anything extraordinary.
	bufferDesc.StructureByteStride = 0;					// Only used for structured buffers, which a vertex buffer is not.
	
	// Define what data our buffer will contain.
	D3D11_SUBRESOURCE_DATA bufferContents;
	bufferContents.pSysMem = vertices;

	// Create the buffer.
	gDevice->CreateBuffer(&bufferDesc, &bufferContents, &gVertexBuffer);
}

void CreateShaders()
{
	// Compile and create vertex shader from the file vertexShader.hlsl in the folder Resources/Shaders/.
	ID3DBlob* compiledVS = nullptr;	// A variable to hold the compiled vertex shader data.
	D3DCompileFromFile(
		L"../Resources/Shaders/vertexShader.hlsl",		// The path to the shader file relative to the .vxproj folder.
		nullptr,		// We don't use any defines.
		nullptr,		// We don't have any includes.
		"main",			// The name of the entry function. Must match function in source data.
		"vs_5_0",		// The shader model to use, "vs" specifies it is a vertex shader, 5_0 that it is shader model 5.0.
		0,				// No shader compile options.
		0,				// Ignored when compiling a shader (effect compile options).
		&compiledVS,	// [out] Compiled shader data.
		nullptr			// [out] Compile time error data.
		);

	gDevice->CreateVertexShader(
		compiledVS->GetBufferPointer(),
		compiledVS->GetBufferSize(),
		NULL,
		&gVertexShader
		);

	// Compile and create pixel shader from the file vertexShader.hlsl in the folder Resources/Shaders/.
	// Works the same way asbove.
	ID3DBlob* compiledPS = nullptr;
	D3DCompileFromFile(
		L"../Resources/Shaders/pixelShader.hlsl",
		nullptr,
		nullptr,
		"main",
		"ps_5_0",		// NOTE: This must be changed to ps_5_0 for pixel shader model 5.0
		0,
		0,
		&compiledPS,
		nullptr
		);

	gDevice->CreatePixelShader(
		compiledPS->GetBufferPointer(),
		compiledPS->GetBufferSize(),
		NULL,
		&gPixelShader
		);

	// Define the input description. Semantic names must correspond to the semantic names used in the vertex shader inputs.
	D3D11_INPUT_ELEMENT_DESC inputDesc[] =
	{
		// For each input: Semantic name, semantic index (if multiple with the same name), input format,
		// input slot (usually 0), byte offset (depends on the previous format size), input slot class (usually 
		// INPUT_PER_VERTEX_DATA), instance data step rate (always 0 when using INPUT_PER_VERTEX_DATA).
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	// Create the input layout to go with our vertex shader (the layout is validated against the shader's input signature).
	int inputLayoutSize = sizeof(inputDesc) / sizeof(D3D11_INPUT_ELEMENT_DESC);
	gDevice->CreateInputLayout(inputDesc, inputLayoutSize, compiledVS->GetBufferPointer(), compiledVS->GetBufferSize(), &gInputLayout);
}

void Render()
{
	FLOAT bgColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };		// Back buffer clear colour as an array of floats (rgba).
	gContext->ClearRenderTargetView(gRTV, bgColor);		// Clear the render target view using the specified colour.

	// The stride and offset need to be stored in variables as we need to provide pointers to them when setting the vertex buffer.
	UINT vbStride = sizeof(Vertex);
	UINT vbOffset = 0;

	// Set the input layout, vertex buffer and topology to use when drawing.
	gContext->IASetVertexBuffers(0, 1, &gVertexBuffer, &vbStride, &vbOffset);
	gContext->IASetInputLayout(gInputLayout);
	gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	// The vertices should be interpreted as parts of a triangle.

	gContext->VSSetShader(gVertexShader, NULL, NULL);	// Set the vertex shader to use. No class instances are used.
	gContext->PSSetShader(gPixelShader, NULL, NULL);	// Set the pixel shader to use. No class instances are used.

	gContext->Draw(6, 0);								// Draw 6 vertices, three for each triangle.

	// When everything has been drawn, present the final result on the screen by swapping the back and front buffers.
	gSwapChain->Present(0, 0);
}