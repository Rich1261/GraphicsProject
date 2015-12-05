#pragma once
#include "CameraClass.h"

class Grid{
private:
	ID3D11Buffer *VertexBuffer = nullptr;
	ID3D11InputLayout *InputLayout = nullptr;
	ID3D11VertexShader *VertexShader = nullptr;
	ID3D11PixelShader *PixelShader = nullptr;
	ID3D11Buffer *ConstWorldMatrixBuffer = nullptr;
	ID3D11RasterizerState *rasterState = nullptr;
	
	D3D11_RASTERIZER_DESC rasterDesc;
	D3D11_BUFFER_DESC bufferDesc;
	D3D11_SUBRESOURCE_DATA subResourceData;
	SEND_WORLD_TO_VRAM WorldToShader;
	float4x4 WorldMatrix;
	int DrawSize = 0;

	HRESULT hr;
public:
	HRESULT Instantiate(ID3D11Device *device, SIMPLE_VERTEX *VertArray, int sizeofVertexArray){
		DrawSize = sizeofVertexArray;
		XMMATRIX identity = XMMatrixIdentity();
		XMStoreFloat4x4(&WorldMatrix, identity);

		bufferDesc.ByteWidth = sizeof(SIMPLE_VERTEX) * sizeofVertexArray;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = NULL;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.MiscFlags = NULL;
		bufferDesc.StructureByteStride = sizeof(SIMPLE_VERTEX);
		subResourceData.pSysMem = VertArray;
		subResourceData.SysMemPitch = 0;
		subResourceData.SysMemSlicePitch = 0;
		hr = device->CreateBuffer(&bufferDesc, &subResourceData, &VertexBuffer);
		if (hr != S_OK) return hr;

		hr = device->CreateVertexShader(Grid_VS, sizeof(Grid_VS), NULL, &VertexShader);
		if (hr != S_OK) return hr;
		hr = device->CreatePixelShader(Grid_PS, sizeof(Grid_PS), NULL, &PixelShader);
		if (hr != S_OK) return hr;

		D3D11_INPUT_ELEMENT_DESC InputDesc[1] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		hr = device->CreateInputLayout(InputDesc, 1, Grid_VS, sizeof(Grid_VS), &InputLayout);
		if (hr != S_OK) return hr;

		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = NULL;
		bufferDesc.ByteWidth = sizeof(SEND_WORLD_TO_VRAM);
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.StructureByteStride = sizeof(SEND_WORLD_TO_VRAM);
		subResourceData.pSysMem = &WorldMatrix;
		subResourceData.SysMemPitch = 0;
		subResourceData.SysMemSlicePitch = 0;
		hr = device->CreateBuffer(&bufferDesc, NULL, &ConstWorldMatrixBuffer);
		WorldToShader.worldMatrix = WorldMatrix;
		if (hr != S_OK) return hr;

		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.CullMode = D3D11_CULL_BACK;
		rasterDesc.FrontCounterClockwise = false;
		rasterDesc.DepthBias = 0;
		rasterDesc.SlopeScaledDepthBias = 0.0f;
		rasterDesc.DepthBiasClamp = 0.0f;
		rasterDesc.DepthClipEnable = true;
		rasterDesc.ScissorEnable = false;
		rasterDesc.MultisampleEnable = false;
		rasterDesc.AntialiasedLineEnable = true;
		hr = device->CreateRasterizerState(&rasterDesc, &rasterState);
		if (hr != S_OK) return hr;

		return hr;
	}
	void Display(ID3D11DeviceContext *deviceContext, ID3D11Buffer *viewProjBuffer, D3D11_MAPPED_SUBRESOURCE mapSubResource, SEND_MATRICIES_TO_VRAM viewProjMatricies){
		deviceContext->VSSetConstantBuffers(0, 1, &ConstWorldMatrixBuffer);
		deviceContext->VSSetConstantBuffers(1, 1, &viewProjBuffer);

		deviceContext->Map(ConstWorldMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, NULL, &mapSubResource);
		memcpy(mapSubResource.pData, &WorldToShader, sizeof(WorldToShader));
		deviceContext->Unmap(ConstWorldMatrixBuffer, 0);

		deviceContext->Map(viewProjBuffer, 0, D3D11_MAP_WRITE_DISCARD, NULL, &mapSubResource);
		memcpy(mapSubResource.pData, &viewProjMatricies, sizeof(viewProjMatricies));
		deviceContext->Unmap(viewProjBuffer, 0);

		const UINT Size = sizeof(SIMPLE_VERTEX);
		const UINT Offset = 0;

		deviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &Size, &Offset);

		deviceContext->VSSetShader(VertexShader, NULL, 0);

		deviceContext->PSSetShader(PixelShader, NULL, 0);

		deviceContext->IASetInputLayout(InputLayout);

		deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);

		if (GetAsyncKeyState(0x33)){
			deviceContext->RSSetState(rasterState);
		}
		if (GetAsyncKeyState(0x34)){
			deviceContext->RSSetState(NULL);
		}
		deviceContext->Draw(DrawSize, 0);
	}
	void Terminate(){
		SAFE_RELEASE(VertexBuffer);
		SAFE_RELEASE(rasterState);
		SAFE_RELEASE(InputLayout);
		SAFE_RELEASE(VertexShader);
		SAFE_RELEASE(PixelShader);
		SAFE_RELEASE(ConstWorldMatrixBuffer);
	}
};