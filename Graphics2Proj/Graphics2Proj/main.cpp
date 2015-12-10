#include "Floor.h"
#include "CameraClass.h"
#include "GridClass.h"
#include "PyramidClass.h"
#include "skybox.h"
#include "StarClass.h"
#include "ModelLoader.h"
#include "Snowflake.h"

#include <crtdbg.h>

class DEMO_APP
{
	HINSTANCE						application;
	WNDPROC							appWndProc;
	HWND							window;
	ID3D11Device *device = nullptr;
	ID3D11DeviceContext *deviceContext = nullptr;
	IDXGISwapChain *swapChain = nullptr;

	DXGI_MODE_DESC displayModeDesc;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;

	ID3D11Texture2D *backBuffer = nullptr;

	D3D11_VIEWPORT viewPort, minimapViewport;

	ID3D11RenderTargetView *renderTargetView = nullptr, *minimapRenderTargetView = nullptr;

	D3D11_SUBRESOURCE_DATA subResourceData;
	ID3D11Texture2D *zBuffer = nullptr, *minimapZBuffer = nullptr;
	D3D11_TEXTURE2D_DESC zBuffDesc;

	ID3D11DepthStencilView *stencilView = nullptr, *minimapStencilView = nullptr;

	ID3D11DepthStencilState *dsState = nullptr;
	D3D11_DEPTH_STENCIL_DESC dsDesc;

	SEND_MATRICIES_TO_VRAM toShader2, minimapToShader2;
	float4x4 viewMatrix, minimapViewMatrix;
	float4x4 projMatrix, minimapProjMatrix;

	D3D11_BUFFER_DESC constBufferDesc;
	ID3D11Buffer *constBuffer2 = nullptr, *minimapConstBuffer2 = nullptr;

	POINT prevPoint;

	ID3D11ShaderResourceView *srv = nullptr, *srvh = nullptr;
	ID3D11ShaderResourceView *srvp = nullptr, *SnowflakeSRV = nullptr;
	ID3D11ShaderResourceView *srvf = nullptr;

	HRESULT hr;
	//OBJECTS//
	Floor floor;
	Pyramid pyramid;
	Skybox skybox;
	Camera myCamera;
	Camera minimapCamera;
	Star3D StarObj;
	Grid myGrid;
	Snowflakes snowflakeEffect;
	Snowflakes blizzard[50];
	//Pyramid helicoptor;

	vector<_OBJ_VERT_> pyramidArray, floorArray, heliArray;
	vector<UINT> pyramidIndexArray, floorIndexArray, heliIndexArray;
public:
	DEMO_APP(HINSTANCE hinst, WNDPROC proc);
	bool Run();
	bool ShutDown();
	void Resize(float width, float height);
};
DEMO_APP * demoapp;
void DEMO_APP::Resize(float width, float height){
	if (swapChain){
		deviceContext->OMSetRenderTargets(0, 0, 0);
		SAFE_RELEASE(renderTargetView);
		HRESULT hr;
		hr = swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
		ID3D11Texture2D *buff = nullptr;
		hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&buff);
		device->CreateRenderTargetView(buff, NULL, &renderTargetView);
		SAFE_RELEASE(buff);
		deviceContext->OMSetRenderTargets(1, &renderTargetView, NULL);
		viewPort.Width = width;
		viewPort.Height = height;
		viewPort.MinDepth = 0.0f;
		viewPort.MaxDepth = 1.0f;
		viewPort.TopLeftX = 0;
		viewPort.TopLeftY = 0;
		deviceContext->RSSetViewports(1, &viewPort);


		ZeroMemory(&zBuffDesc, sizeof(zBuffDesc));
		zBuffDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		zBuffDesc.Format = DXGI_FORMAT_D32_FLOAT;
		zBuffDesc.Width = (UINT)width;
		zBuffDesc.Height = (UINT)height;
		zBuffDesc.MipLevels = 1;
		zBuffDesc.Usage = D3D11_USAGE_DEFAULT;
		zBuffDesc.SampleDesc.Count = 1;
		zBuffDesc.SampleDesc.Quality = 0;
		zBuffDesc.MiscFlags = 0;
		zBuffDesc.CPUAccessFlags = 0;
		zBuffDesc.ArraySize = 1;

		SAFE_RELEASE(zBuffer);
		SAFE_RELEASE(stencilView);

		device->CreateTexture2D(&zBuffDesc, NULL, &zBuffer);
		device->CreateDepthStencilView(zBuffer, NULL, &stencilView);

		XMMATRIX tempProj = XMLoadFloat4x4(&projMatrix);
		tempProj = DirectX::XMMatrixPerspectiveFovLH((3.1415f / 180.0f) * (65 * 0.5f), width / height, 0.1f, 1000.0f);
		XMStoreFloat4x4(&projMatrix, tempProj);
		//projMatrix = BuildPerspectiveProjectionMatrix(65.0f, width / height, 0.1f, 100.0f);

		toShader2.projMatrix = projMatrix;
	}
}

void StartTextureThreads(ID3D11Device *device, const wchar_t *filename, ID3D11ShaderResourceView **srv, int * readyCount, condition_variable *con_var, mutex* lock){
	CreateDDSTextureFromFile(device, filename, NULL, srv);
	lock->lock();
	(*readyCount)++;
	con_var->notify_all();
	lock->unlock();
}
void StartOBJThreads(const char *filename, vector<_OBJ_VERT_> *vertList, vector<UINT> *indexArray, int * readyCount, condition_variable *con_var, mutex* lock){
	loadOBJ(filename, *vertList, *indexArray);
	lock->lock();
	(*readyCount)++;
	con_var->notify_all();
	lock->unlock();
}

DEMO_APP::DEMO_APP(HINSTANCE hinst, WNDPROC proc)
{
	mutex mute;
	int readyCount = 0;
	condition_variable con_var;

	XMMATRIX identity = XMMatrixIdentity();
	XMStoreFloat4x4(&viewMatrix, identity);
	XMStoreFloat4x4(&projMatrix, identity);
	XMStoreFloat4x4(&minimapViewMatrix, identity);
	XMStoreFloat4x4(&minimapProjMatrix, identity);
	srand(static_cast<unsigned int>(time(nullptr)));

#pragma region WINDOWS CODE STUFF
	application = hinst;
	appWndProc = proc;

	WNDCLASSEX  wndClass;
	ZeroMemory(&wndClass, sizeof(wndClass));
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.lpfnWndProc = appWndProc;
	wndClass.lpszClassName = L"DirectXApplication";
	wndClass.hInstance = application;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOWFRAME);
	RegisterClassEx(&wndClass);

	RECT window_size = { 0, 0, BACKBUFFER_WIDTH, BACKBUFFER_HEIGHT };
	AdjustWindowRect(&window_size, WS_OVERLAPPEDWINDOW, false);

	window = CreateWindow(L"DirectXApplication", L"Lab 1a Line Land", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, window_size.right - window_size.left, window_size.bottom - window_size.top,
		NULL, NULL, application, this);

	ShowWindow(window, SW_SHOW);

#pragma endregion
#pragma region swapChain+device+deviceContext Creation
	ZeroMemory(&displayModeDesc, sizeof(displayModeDesc));
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	ZeroMemory(&viewPort, sizeof(viewPort));

	displayModeDesc.Width = BACKBUFFER_WIDTH;
	displayModeDesc.Height = BACKBUFFER_HEIGHT;
	displayModeDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferDesc = displayModeDesc;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapChainDesc.OutputWindow = window;
	swapChainDesc.SampleDesc.Count = 1;
	//chnage count to 2+ on all counts and make sure renderTarget + depth buffer
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Windowed = true;
	HRESULT debugResultCreateDeviceAndSwapChain;
#ifdef _DEBUG
	debugResultCreateDeviceAndSwapChain = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG, NULL, 0, D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, NULL, &deviceContext);
#else
	debugResultCreateDeviceAndSwapChain = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, 0, D3D11_SDK_VERSION, &swapChainDesc, &swapChain, &device, NULL, &deviceContext);
#endif
	swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));

#pragma endregion
#pragma region CreateRenderTargetView
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.Height = BACKBUFFER_HEIGHT;
	viewPort.Width = BACKBUFFER_WIDTH;
	viewPort.MaxDepth = 1;
	viewPort.MinDepth = 0;

	HRESULT debugResultCreateRenderTargetView = device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);

	minimapViewport.TopLeftX = BACKBUFFER_WIDTH * .8;
	minimapViewport.TopLeftY = 0;
	minimapViewport.Height = BACKBUFFER_HEIGHT * .2;
	minimapViewport.Width = BACKBUFFER_WIDTH * .2;
	minimapViewport.MaxDepth = 1;
	minimapViewport.MinDepth = 0;

	device->CreateRenderTargetView(backBuffer, nullptr, &minimapRenderTargetView);

	SAFE_RELEASE(backBuffer);

#pragma endregion
#pragma region CreateDepthBuffer
	ZeroMemory(&zBuffDesc, sizeof(zBuffDesc));
	zBuffDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	zBuffDesc.Format = DXGI_FORMAT_D32_FLOAT;
	zBuffDesc.Width = BACKBUFFER_WIDTH;
	zBuffDesc.Height = BACKBUFFER_HEIGHT;
	zBuffDesc.MipLevels = 1;
	zBuffDesc.Usage = D3D11_USAGE_DEFAULT;
	zBuffDesc.SampleDesc.Count = 1;
	zBuffDesc.SampleDesc.Quality = 0;
	zBuffDesc.MiscFlags = 0;
	zBuffDesc.CPUAccessFlags = 0;
	zBuffDesc.ArraySize = 1;

	device->CreateTexture2D(&zBuffDesc, NULL, &zBuffer);
	device->CreateDepthStencilView(zBuffer, NULL, &stencilView);

	ZeroMemory(&zBuffDesc, sizeof(zBuffDesc));
	zBuffDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	zBuffDesc.Format = DXGI_FORMAT_D32_FLOAT;
	zBuffDesc.Width = UINT(BACKBUFFER_WIDTH * .2);
	zBuffDesc.Height = UINT(BACKBUFFER_HEIGHT * .2);
	zBuffDesc.MipLevels = 1;
	zBuffDesc.Usage = D3D11_USAGE_DEFAULT;
	zBuffDesc.SampleDesc.Count = 1;
	zBuffDesc.SampleDesc.Quality = 0;
	zBuffDesc.MiscFlags = 0;
	zBuffDesc.CPUAccessFlags = 0;
	zBuffDesc.ArraySize = 1;

	device->CreateTexture2D(&zBuffDesc, NULL, &minimapZBuffer);
	device->CreateDepthStencilView(minimapZBuffer, NULL, &minimapStencilView);

	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	dsDesc.StencilEnable = true;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	device->CreateDepthStencilState(&dsDesc, &dsState);

#pragma endregion
#pragma region CreateConstView/ProjMatrixBuffer


	XMMATRIX tempProj = XMLoadFloat4x4(&projMatrix);
	tempProj = DirectX::XMMatrixPerspectiveFovLH((3.1415f / 180.0f) * (65 * 0.5f), (BACKBUFFER_WIDTH / BACKBUFFER_HEIGHT), 0.1f, 1000.0f);
	XMStoreFloat4x4(&projMatrix, tempProj);

	constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constBufferDesc.MiscFlags = NULL;
	constBufferDesc.ByteWidth = sizeof(SEND_MATRICIES_TO_VRAM);
	constBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constBufferDesc.StructureByteStride = sizeof(SEND_MATRICIES_TO_VRAM);
	TwoMatricies4x4 newMatrix(viewMatrix, projMatrix);
	subResourceData.pSysMem = &newMatrix;
	subResourceData.SysMemPitch = 0;
	subResourceData.SysMemSlicePitch = 0;
	device->CreateBuffer(&constBufferDesc, NULL, &constBuffer2);
	toShader2.projMatrix = projMatrix;
	toShader2.viewMatrix = viewMatrix;


	XMMATRIX tempminimapProj = XMLoadFloat4x4(&minimapProjMatrix);
	tempminimapProj = DirectX::XMMatrixPerspectiveFovLH((3.1415f / 180.0f) * (65 * 0.5f), ((BACKBUFFER_WIDTH * .2) / (BACKBUFFER_HEIGHT*.2)), 0.1f, 1000.0f);
	XMStoreFloat4x4(&minimapProjMatrix, tempminimapProj);

	constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constBufferDesc.MiscFlags = NULL;
	constBufferDesc.ByteWidth = sizeof(SEND_MATRICIES_TO_VRAM);
	constBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constBufferDesc.StructureByteStride = sizeof(SEND_MATRICIES_TO_VRAM);
	TwoMatricies4x4 minimapnewMatrix(minimapViewMatrix, minimapProjMatrix);
	subResourceData.pSysMem = &newMatrix;
	subResourceData.SysMemPitch = 0;
	subResourceData.SysMemSlicePitch = 0;
	device->CreateBuffer(&constBufferDesc, NULL, &minimapConstBuffer2);
	minimapToShader2.projMatrix = minimapProjMatrix;
	minimapToShader2.viewMatrix = minimapViewMatrix;

#pragma endregion

#pragma region CreateStarCoords
	SIMPLE_VERTEX star[22];
	star[0].x = 0; star[0].y = 0; star[0].z = -.75f;
	star[1].x = 0; star[1].y = 0.5f; star[1].z = -.75f;
	star[2].x = 0.25f; star[2].y = 0.25f; star[2].z = -.75f;
	star[3].x = .5f; star[3].y = 0.25f; star[3].z = -.75f;
	star[4].x = 0.25f; star[4].y = 0.0f; star[4].z = -.75f;
	star[5].x = 0.25f; star[5].y = -.5f; star[5].z = -.75f;
	star[6].x = 0; star[6].y = -0.25f; star[6].z = -.75f;
	star[7].x = -.25f; star[7].y = -0.5f; star[7].z = -.75f;
	star[8].x = -0.25f; star[8].y = 0; star[8].z = -.75f;
	star[9].x = -.5f; star[9].y = 0.25f; star[9].z = -.75f;
	star[10].x = -0.25f; star[10].y = 0.25f; star[10].z = -.75f;
	star[11].x = 0; star[11].y = 0; star[11].z = -.65f;
	star[12].x = 0; star[12].y = 0.5f; star[12].z = -.65f;
	star[13].x = 0.25f; star[13].y = 0.25f; star[13].z = -.65f;
	star[14].x = .5f; star[14].y = 0.25f; star[14].z = -.65f;
	star[15].x = 0.25f; star[15].y = 0.0f; star[15].z = -.65f;
	star[16].x = 0.25f; star[16].y = -0.5f; star[16].z = -.65f;
	star[17].x = 0.0f; star[17].y = -0.25f; star[17].z = -.65f;
	star[18].x = -.25f; star[18].y = -0.5f; star[18].z = -.65f;
	star[19].x = -0.25f; star[19].y = 0.0f; star[19].z = -.65f;
	star[20].x = -.5f; star[20].y = 0.25f; star[20].z = -.65f;
	star[21].x = -0.25f; star[21].y = 0.25f; star[21].z = -.65f;

	for (int i = 0; i < 22; i++){
		if (i % 2 == 0){
			star[i].r = 1;
		}
		else{
			star[i].b = 1;
		}
	}
	unsigned short starIndexArray[120] = { 0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5, 0, 5, 6, 0, 6, 7, 0, 7, 8, 0, 8, 9, 0, 9, 10, 0, 10, 1/*end face 1*/, 11, 13, 12, 11, 14, 13, 11, 15, 14, 11, 16, 15, 11, 17, 16, 11, 18, 17, 11, 19, 18, 11, 20, 19, 11, 21, 20, 11, 12, 21/*end face 2*/, 1, 12, 13, 1, 13, 2, 2, 13, 14, 2, 14, 3, 3, 14, 15, 3, 15, 4, 4, 15, 16, 4, 16, 5, 5, 16, 17, 5, 17, 6, 6, 17, 18, 6, 18, 7, 7, 18, 19, 7, 19, 8, 8, 19, 20, 8, 20, 9, 9, 20, 21, 9, 21, 10, 10, 21, 12, 10, 12, 1/*end of side face*/ };

#pragma endregion
#pragma region CreateGridVerticies
	SIMPLE_VERTEX fullGrid[84];
	float sX = -1.0f;
	float sY = -1.0f;
	float eX = 1.0f;
	float eY = 1.0f;

	for (int i = 0; i < 42; i += 2){
		fullGrid[i] = { sX, 0.0f, sX - (sY * i / 20) };
		fullGrid[i + 1] = { eX, 0.0f, sX - (sY * i / 20) };
	}
	for (int i = 42; i < 84; i += 2){
		fullGrid[i] = { eX + (sX * (i - 42) / 20), 0.0f, sY };
		fullGrid[i + 1] = { eX + (sX * (i - 42) / 20), 0.0f, eY };
	}
#pragma endregion
#pragma region skybox cube
	SIMPLE_VERTEX cube[8];
	cube[0].x = -70; cube[0].y = 70; cube[0].z = 70;
	cube[1].x = 70; cube[1].y = 70; cube[1].z = 70;
	cube[2].x = -70; cube[2].y = -70; cube[2].z = 70;
	cube[3].x = 70; cube[3].y = -70; cube[3].z = 70;
	cube[4].x = -70; cube[4].y = 70; cube[4].z = -70;
	cube[5].x = 70; cube[5].y = 70; cube[5].z = -70;
	cube[6].x = -70; cube[6].y = -70; cube[6].z = -70;
	cube[7].x = 70; cube[7].y = -70; cube[7].z = -70;

	unsigned int indicies[36] = { 4, 0, 6, 2, 6, 0, 6, 2, 7, 3, 7, 2, 7, 5, 3, 1, 5, 3, 5, 1, 4, 0, 4, 1, 2, 0, 3, 1, 3, 0, 6, 4, 7, 5, 7, 4 };
#pragma endregion
#pragma region floorCoords
	vector<_OBJ_VERT_> floorarray;
	floorarray.resize(4);
	*(float3*)(&floorarray[0].pos) = float3(-10, -10, 10);
	*(float3*)(&floorarray[3].pos) = float3(10, -10, 10);
	*(float3*)(&floorarray[1].pos) = float3(-10, -10, -10);
	*(float3*)(&floorarray[2].pos) = float3(10, -10, -10);

	*(float3*)(&floorarray[0].nrm) = float3(0, 1, 0);
	*(float3*)(&floorarray[3].nrm) = float3(0, 1, 0);
	*(float3*)(&floorarray[1].nrm) = float3(0, 1, 0);
	*(float3*)(&floorarray[2].nrm) = float3(0, 1, 0);

	*(float3*)(&floorarray[0].uvw) = float3(0, 0, 0);
	*(float3*)(&floorarray[3].uvw) = float3(1, 0, 0);
	*(float3*)(&floorarray[1].uvw) = float3(0, 1, 0);
	*(float3*)(&floorarray[2].uvw) = float3(1, 1, 0);

	vector<UINT> floorindexarray;
	floorindexarray.push_back(0);
	floorindexarray.push_back(1);
	floorindexarray.push_back(2);

	floorindexarray.push_back(2);
	floorindexarray.push_back(3);
	floorindexarray.push_back(0);

#pragma endregion

#pragma region InitalSnowflakesPos
	vector<SnowflakeStruct> snowflakes;
	snowflakes.push_back(SnowflakeStruct(float4(0, 2, 0, 1)));
	snowflakes.push_back(SnowflakeStruct(float4(0, 3, 0, 1)));
	snowflakes.push_back(SnowflakeStruct(float4(1, 2, 0, 1)));
	snowflakes.push_back(SnowflakeStruct(float4(1, 3, 0, 1)));
#pragma endregion

	vector<SnowflakeStruct> blizzardFlakes[50];
	for (int i = 0; i < 50; i++){
		blizzardFlakes[i].push_back(SnowflakeStruct(float4(static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 10)), static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 10)), static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 10)), 1), static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / .05f)), static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 15))));
	}
	vector<thread> threads;
	//thread thread1 = thread(StartTextureThreads, device, L"OutputCube.dds", &srv, &readyCount, &con_var, &mute);
	threads.push_back(thread(StartTextureThreads, device, L"OutputCube.dds", &srv, &readyCount, &con_var, &mute));
	threads.push_back(thread(StartTextureThreads, device, L"pyramidTex.dds", &srvp, &readyCount, &con_var, &mute));
	threads.push_back(thread(StartTextureThreads, device, L"pyramidTex.dds", &srvf, &readyCount, &con_var, &mute));
	threads.push_back(thread(StartTextureThreads, device, L"Snowflake.dds", &SnowflakeSRV, &readyCount, &con_var, &mute));
	threads.push_back(thread(StartOBJThreads, "..\\Graphics2Proj\\Pyramid.obj", &pyramidArray, &pyramidIndexArray, &readyCount, &con_var, &mute));
	for (UINT i = 0; i < threads.size(); i++){
		threads[i].detach();
	}
	unique_lock<mutex> lockU(mute);
	con_var.wait(lockU, [&](){return readyCount == threads.size(); });
	lockU.unlock();

	GetCursorPos(&prevPoint);
	myCamera.Instantiate(prevPoint);
	minimapCamera.Instantiate(prevPoint, 1);

	myGrid.Instantiate(device, fullGrid, 84);

	StarObj.Instantiate(device, star, 22, starIndexArray, 120);


	//skybox creation
	skybox.Instantiate(device, cube, 8, indicies, 36, srv);

	//pyramid creation
	pyramid.Instantiate(device, &pyramidArray, &pyramidIndexArray, srvp);

	//floor creation
	floor.Instantiate(device, &floorarray, &floorindexarray, srvf);

	snowflakeEffect.Instantiate(device, &snowflakes, SnowflakeSRV);

	for (int i = 0; i < 50; i++){
		blizzard[i].Instantiate(device, &blizzardFlakes[i], SnowflakeSRV);
	}
}

bool DEMO_APP::Run()
{
	deviceContext->OMSetRenderTargets(1, &renderTargetView, stencilView);
	deviceContext->RSSetViewports(1, &viewPort);

	const FLOAT darkBlueRGBA[4] = { 0, 0, .25f, 0 };

	deviceContext->ClearRenderTargetView(renderTargetView, darkBlueRGBA);
	deviceContext->ClearDepthStencilView(stencilView, D3D11_CLEAR_DEPTH, 1, 0);

	D3D11_MAPPED_SUBRESOURCE mapSubResource;
	ZeroMemory(&mapSubResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

	skybox.Display(deviceContext, constBuffer2, mapSubResource, toShader2, myCamera.GetCameraPos(viewMatrix));

	toShader2.viewMatrix = myCamera.Update(viewMatrix);

	floor.Display(deviceContext, constBuffer2, mapSubResource, toShader2);
	pyramid.Display(deviceContext, constBuffer2, mapSubResource, toShader2);
	myGrid.Display(deviceContext, constBuffer2, mapSubResource, toShader2);
	StarObj.Display(deviceContext, constBuffer2, mapSubResource, toShader2);
	snowflakeEffect.Display(deviceContext, constBuffer2, mapSubResource, toShader2);
	for (int i = 0; i < 50; i++){
		blizzard[i].Display(deviceContext, constBuffer2, mapSubResource, toShader2);
	}

	deviceContext->ClearDepthStencilView(stencilView, D3D11_CLEAR_DEPTH, 1, 0);

	//end main viewport//
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
	//begin minimap viewport//
	//deviceContext->OMSetRenderTargets(1, &minimapRenderTargetView, minimapStencilView);
	deviceContext->RSSetViewports(1, &minimapViewport);

	//deviceContext->ClearRenderTargetView(minimapRenderTargetView, darkBlueRGBA);
	deviceContext->ClearDepthStencilView(minimapStencilView, D3D11_CLEAR_DEPTH, 1, 0);

	skybox.Display(deviceContext, minimapConstBuffer2, mapSubResource, minimapToShader2, myCamera.GetCameraPos(viewMatrix));

	minimapToShader2.viewMatrix = minimapCamera.Update(minimapViewMatrix);

	floor.Display(deviceContext, minimapConstBuffer2, mapSubResource, minimapToShader2);
	pyramid.Display(deviceContext, minimapConstBuffer2, mapSubResource, minimapToShader2);
	myGrid.Display(deviceContext, minimapConstBuffer2, mapSubResource, minimapToShader2);
	StarObj.Display(deviceContext, minimapConstBuffer2, mapSubResource, minimapToShader2);
	snowflakeEffect.Display(deviceContext, minimapConstBuffer2, mapSubResource, minimapToShader2);
	for (int i = 0; i < 50; i++){
		blizzard[i].Display(deviceContext, minimapConstBuffer2, mapSubResource, minimapToShader2);
	}

	swapChain->Present(1, 0);
	return true;
}

bool DEMO_APP::ShutDown()
{
	for (int i = 0; i < 50; i++){
		blizzard[i].Terminate();
	}
	floor.Terminate();
	pyramid.Terminate();
	skybox.Terminate();
	StarObj.Terminate();
	myGrid.Terminate();
	snowflakeEffect.Terminate();
	//helicoptor.Terminate();
	SAFE_RELEASE(minimapRenderTargetView);
	SAFE_RELEASE(minimapConstBuffer2);
	SAFE_RELEASE(minimapStencilView);
	SAFE_RELEASE(minimapZBuffer);
	SAFE_RELEASE(deviceContext);
	SAFE_RELEASE(swapChain);
	SAFE_RELEASE(backBuffer);
	SAFE_RELEASE(renderTargetView);
	SAFE_RELEASE(zBuffer);
	SAFE_RELEASE(stencilView);
	SAFE_RELEASE(dsState);
	SAFE_RELEASE(constBuffer2);
	SAFE_RELEASE(srv);
	SAFE_RELEASE(srvp);
	SAFE_RELEASE(srvf);
	SAFE_RELEASE(SnowflakeSRV);
	ID3D11Debug * debug = nullptr;
	HRESULT hr = device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&debug));
	if (SUCCEEDED(hr)){
		hr = debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	}
	SAFE_RELEASE(debug);
	SAFE_RELEASE(device);

	UnregisterClass(L"DirectXApplication", application);
	return true;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wparam, LPARAM lparam);
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR, int)
{
	srand(unsigned int(time(0)));
	DEMO_APP myApp(hInstance, (WNDPROC)WndProc);
	demoapp = &myApp;
	MSG msg; ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT && myApp.Run())
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	myApp.ShutDown();
	return 0;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (GetAsyncKeyState(VK_ESCAPE))
		message = WM_DESTROY;
	switch (message)
	{
	case (WM_DESTROY) : { PostQuitMessage(0); }
						break;
	case (WM_SIZE) : {
		if (demoapp != nullptr)
			demoapp->Resize((float)LOWORD(lParam), (float)HIWORD(lParam));
		break;
	}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
int main(){

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetBreakAlloc(-1);
	system("pause");
	return EXIT_SUCCESS;
}