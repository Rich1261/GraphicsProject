#pragma once
#include "stdafx.h"

class Instance{
private:
	ID3D11Buffer *VertexBuffer = nullptr, *IndexBuffer = nullptr, *ConstWorldMatrixBuffer = nullptr;
	ID3D11InputLayout *InputLayout = nullptr;
	ID3D11VertexShader *VertexShader = nullptr;
	ID3D11PixelShader *PixelShader = nullptr;
	ID3D11Texture2D *texture = nullptr;
	ID3D11SamplerState *sampler = nullptr;
	ID3D11BlendState *blendState = nullptr;
	ID3D11ShaderResourceView *shaderResourceView = nullptr;
	ID3D11RasterizerState *front = nullptr, *back = nullptr;

	D3D11_BUFFER_DESC bufferDesc;
	D3D11_SUBRESOURCE_DATA subResourceData;
	D3D11_TEXTURE2D_DESC textureDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BLEND_DESC blendDesc;
	D3D11_RASTERIZER_DESC rasterDesc;
	INSTANCED_WORLD_TO_VRAM WorldToShader;
	float4x4 WorldMatrix[100];
	int sizeofIndexArray = 0;
	float flipTime = 1.0f;
	bool tempIndexedFlag = true;
	HRESULT hr;
public:
	HRESULT Instantiate(ID3D11Device *device, vector<_OBJ_VERT_> *coordArray, vector<UINT> *indexArray, ID3D11ShaderResourceView *_srv){
		sizeofIndexArray = indexArray->size();

		for (int i = 0; i < 100; i++){
			XMMATRIX identity = DirectX::XMMatrixIdentity();
			XMStoreFloat4x4(&WorldToShader.worldMatricies[i], identity);
		}

		bufferDesc.ByteWidth = sizeof(_OBJ_VERT_) * coordArray->size();
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = NULL;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.MiscFlags = NULL;
		bufferDesc.StructureByteStride = sizeof(_OBJ_VERT_);
		subResourceData.pSysMem = coordArray->data();
		subResourceData.SysMemPitch = 0;
		subResourceData.SysMemSlicePitch = 0;
		hr = device->CreateBuffer(&bufferDesc, &subResourceData, &VertexBuffer);
		if (hr != S_OK) return hr;

		hr = device->CreateVertexShader(Instanced_VS, sizeof(Instanced_VS), NULL, &VertexShader);
		if (hr != S_OK) return hr;
		hr = device->CreatePixelShader(Pyramid_PS, sizeof(Pyramid_PS), NULL, &PixelShader);
		if (hr != S_OK) return hr;

		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		hr = device->CreateInputLayout(inputElementDesc, 4, Instanced_VS, sizeof(Instanced_VS), &InputLayout);
		if (hr != S_OK) return hr;

		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = NULL;
		bufferDesc.ByteWidth = sizeof(INSTANCED_WORLD_TO_VRAM);
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.StructureByteStride = sizeof(INSTANCED_WORLD_TO_VRAM);
		subResourceData.pSysMem = &WorldMatrix;
		subResourceData.SysMemPitch = 0;
		subResourceData.SysMemSlicePitch = 0;
		hr = device->CreateBuffer(&bufferDesc, NULL, &ConstWorldMatrixBuffer);
		if (hr != S_OK) return hr;

		int index = 0;
		for (int i = 0; i < 10; i++){
			for (int j = 0; j < 10; j++){
				WorldToShader.worldMatricies[index].m[3][0] = 0 + i * 2.0f;
				WorldToShader.worldMatricies[index].m[3][1] = 0;
				WorldToShader.worldMatricies[index].m[3][2] = 0 + j * 2.0f;
				WorldToShader.worldMatricies[index].m[3][3] = 1;
				index++;
			}
		}

		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = NULL;
		bufferDesc.ByteWidth = sizeof(unsigned int) * indexArray->size();
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.StructureByteStride = sizeof(unsigned int);
		subResourceData.pSysMem = indexArray->data();
		subResourceData.SysMemPitch = 0;
		subResourceData.SysMemSlicePitch = 0;
		hr = device->CreateBuffer(&bufferDesc, &subResourceData, &IndexBuffer);
		if (hr != S_OK) return hr;


		shaderResourceView = _srv;

		ZeroMemory(&samplerDesc, sizeof(samplerDesc));
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = 1;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		hr = device->CreateSamplerState(&samplerDesc, &sampler);
		if (hr != S_OK) return hr;

		ZeroMemory(&blendDesc, sizeof(blendDesc));
		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.IndependentBlendEnable = false;
		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		hr = device->CreateBlendState(&blendDesc, &blendState);
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
		hr = device->CreateRasterizerState(&rasterDesc, &back);
		if (hr != S_OK) return hr;

		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.CullMode = D3D11_CULL_FRONT;
		rasterDesc.FrontCounterClockwise = false;
		rasterDesc.DepthBias = 0;
		rasterDesc.SlopeScaledDepthBias = 0.0f;
		rasterDesc.DepthBiasClamp = 0.0f;
		rasterDesc.DepthClipEnable = true;
		rasterDesc.ScissorEnable = false;
		rasterDesc.MultisampleEnable = false;
		rasterDesc.AntialiasedLineEnable = true;
		hr = device->CreateRasterizerState(&rasterDesc, &front);
		if (hr != S_OK) return hr;
		return hr;
	}
	void Display(ID3D11DeviceContext *deviceContext, ID3D11Buffer *viewProjBuffer, D3D11_MAPPED_SUBRESOURCE mapSubResource, SEND_MATRICIES_TO_VRAM viewProjMatricies){
		deviceContext->OMSetBlendState(blendState, NULL, 0xffffffff);

		//rotation = MultiplyMatrix4x4ByMatrix4x4(BuildYRotationMatrix4X4(time), BuildXRotationMatrix4X4(time));
		//toShader.worldMatrix = MultiplyMatrix4x4ByMatrix4x4(rotation, worldMatrix);

		deviceContext->VSSetConstantBuffers(0, 1, &ConstWorldMatrixBuffer);
		deviceContext->VSSetConstantBuffers(1, 1, &viewProjBuffer);

		deviceContext->Map(ConstWorldMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, NULL, &mapSubResource);
		memcpy(mapSubResource.pData, &WorldToShader, sizeof(WorldToShader));
		deviceContext->Unmap(ConstWorldMatrixBuffer, 0);

		deviceContext->Map(viewProjBuffer, 0, D3D11_MAP_WRITE_DISCARD, NULL, &mapSubResource);
		memcpy(mapSubResource.pData, &viewProjMatricies, sizeof(viewProjMatricies));
		deviceContext->Unmap(viewProjBuffer, 0);

		const UINT size = sizeof(_OBJ_VERT_);
		const UINT offset = 0;

		deviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &size, &offset);

		deviceContext->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		deviceContext->VSSetShader(VertexShader, NULL, 0);

		deviceContext->PSSetShader(PixelShader, NULL, 0);
		deviceContext->PSSetSamplers(0, 1, &sampler);
		deviceContext->PSSetShaderResources(0, 1, &shaderResourceView);

		deviceContext->IASetInputLayout(InputLayout);

		deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		deviceContext->RSSetState(back);
		deviceContext->DrawIndexedInstanced(sizeofIndexArray, 100, 0, 0, 0);
	}
	void Terminate(){
		SAFE_RELEASE(VertexBuffer);
		SAFE_RELEASE(IndexBuffer);
		SAFE_RELEASE(ConstWorldMatrixBuffer);
		SAFE_RELEASE(InputLayout);
		SAFE_RELEASE(VertexShader);
		SAFE_RELEASE(PixelShader);
		SAFE_RELEASE(texture);
		SAFE_RELEASE(sampler);
		SAFE_RELEASE(blendState);
		SAFE_RELEASE(front);
		SAFE_RELEASE(back);
	}
};