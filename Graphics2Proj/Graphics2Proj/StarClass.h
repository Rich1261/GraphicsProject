#pragma once
#include "stdafx.h"

struct Star3D{
private:
	ID3D11Buffer *VertexBuffer = nullptr;
	ID3D11Buffer *IndexBuffer = nullptr;
	ID3D11InputLayout *InputLayout = nullptr;
	ID3D11VertexShader *VertexShader = nullptr;
	ID3D11PixelShader *PixelShader = nullptr;
	ID3D11Buffer *ConstWorldMatrixBuffer = nullptr;

	D3D11_BUFFER_DESC bufferDesc;
	D3D11_SUBRESOURCE_DATA subResourceData;
	SEND_WORLD_TO_VRAM WorldToShader;
	float4x4 WorldMatrix;
	int sizeofIndexArray = 0;

	HRESULT hr;
public:
	
	HRESULT Instantiate(ID3D11Device *device, SIMPLE_VERTEX *coordArray, int sizeofVertexArray, unsigned short *indexArray, int sizeofIndexArray){
		this->sizeofIndexArray = sizeofIndexArray;
		XMMATRIX identity = XMMatrixIdentity();
		XMStoreFloat4x4(&WorldMatrix, identity);

		ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
		ZeroMemory(&subResourceData, sizeof(D3D11_SUBRESOURCE_DATA));

		bufferDesc.ByteWidth = sizeof(SIMPLE_VERTEX) * sizeofVertexArray;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = NULL;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.MiscFlags = NULL;
		bufferDesc.StructureByteStride = sizeof(SIMPLE_VERTEX);
		subResourceData.pSysMem = coordArray;
		subResourceData.SysMemPitch = 0;
		subResourceData.SysMemSlicePitch = 0;
		hr = device->CreateBuffer(&bufferDesc, &subResourceData, &VertexBuffer);
		if (hr != S_OK) return hr;

		D3D11_INPUT_ELEMENT_DESC starInputElementDesc[2] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		hr = device->CreateInputLayout(starInputElementDesc, 2, Star_VS, sizeof(Star_VS), &InputLayout);
		if (hr != S_OK) return hr;

		hr = device->CreateVertexShader(Star_VS, sizeof(Star_VS), NULL, &VertexShader);
		if (hr != S_OK) return hr;

		hr = device->CreatePixelShader(Star_PS, sizeof(Star_PS), NULL, &PixelShader);
		if (hr != S_OK) return hr;

		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = NULL;
		bufferDesc.ByteWidth = sizeof(unsigned short) * sizeofIndexArray;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.StructureByteStride = sizeof(unsigned short);
		subResourceData.pSysMem = indexArray;
		subResourceData.SysMemPitch = 0;
		subResourceData.SysMemSlicePitch = 0;
		hr = device->CreateBuffer(&bufferDesc, &subResourceData, &IndexBuffer);
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
		return hr;
	}
	void Display(ID3D11DeviceContext *deviceContext, ID3D11Buffer *viewProjBuffer, D3D11_MAPPED_SUBRESOURCE mapSubResource, SEND_MATRICIES_TO_VRAM viewProjMatricies){
		deviceContext->OMSetBlendState(NULL, NULL, 0xffffffff);
		deviceContext->VSSetConstantBuffers(0, 1, &ConstWorldMatrixBuffer);
		deviceContext->VSSetConstantBuffers(1, 1, &viewProjBuffer);

		deviceContext->Map(ConstWorldMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, NULL, &mapSubResource);
		memcpy(mapSubResource.pData, &WorldToShader, sizeof(WorldToShader));
		deviceContext->Unmap(ConstWorldMatrixBuffer, 0);

		deviceContext->Map(viewProjBuffer, 0, D3D11_MAP_WRITE_DISCARD, NULL, &mapSubResource);
		memcpy(mapSubResource.pData, &viewProjMatricies, sizeof(viewProjMatricies));
		deviceContext->Unmap(viewProjBuffer, 0);

		const UINT starSize = sizeof(SIMPLE_VERTEX);
		const UINT starOffset = 0;

		deviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &starSize, &starOffset);
		deviceContext->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R16_UINT, 0);

		deviceContext->VSSetShader(VertexShader, NULL, 0);
		deviceContext->PSSetShader(PixelShader, NULL, 0);

		deviceContext->IASetInputLayout(InputLayout);

		deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		deviceContext->DrawIndexed(sizeofIndexArray, 0, 0);
	}
	void Terminate(){
		SAFE_RELEASE(VertexBuffer);
		SAFE_RELEASE(IndexBuffer);
		SAFE_RELEASE(InputLayout);
		SAFE_RELEASE(VertexShader);
		SAFE_RELEASE(PixelShader);
		SAFE_RELEASE(ConstWorldMatrixBuffer);
	}
};