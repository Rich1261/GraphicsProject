#pragma once
#include "stdafx.h"

class RenderToTexture{
private:
	ID3D11Buffer *VertexBuffer = nullptr, *IndexBuffer = nullptr, *ConstWorldMatrixBuffer = nullptr, *spotlightPosBuffer = nullptr, *pointlightposbuffer = nullptr, *directionalLightBuffer = nullptr;
	ID3D11InputLayout *InputLayout = nullptr;
	ID3D11VertexShader *VertexShader = nullptr;
	ID3D11PixelShader *PixelShader = nullptr;
	ID3D11Texture2D *texture = nullptr;
	ID3D11SamplerState *sampler = nullptr;
	ID3D11BlendState *blendState = nullptr;
	ID3D11ShaderResourceView *shaderResourceView = nullptr, *shaderResourceViewNormal = nullptr;
	ID3D11RasterizerState *front = nullptr, *back = nullptr;

	D3D11_BUFFER_DESC bufferDesc;
	D3D11_SUBRESOURCE_DATA subResourceData;
	D3D11_TEXTURE2D_DESC textureDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BLEND_DESC blendDesc;
	D3D11_RASTERIZER_DESC rasterDesc;
	SEND_WORLD_TO_VRAM WorldToShader;
	float4x4 WorldMatrix;
	HRESULT hr;

	//ID3D11Texture2D *renderTargTexMap = nullptr;
	//ID3D11RenderTargetView *renderTargViewMap = nullptr;
	//ID3D11ShaderResourceView *shaderResourceViewMap = nullptr;
	D3D11_TEXTURE2D_DESC texD;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargViewD;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvD;
	float4x4 mapView;
	float4x4 mapProj;
public:
	HRESULT Instantiate(ID3D11Device *device, vector<_OBJ_VERT_> *coordArray, vector<UINT> *indexArray, ID3D11ShaderResourceView *ColorMap = nullptr, ID3D11ShaderResourceView *NormalMap = nullptr){
		if (ColorMap != nullptr) shaderResourceView = ColorMap;
		if (NormalMap != nullptr) shaderResourceViewNormal = NormalMap;
		//sizeofIndexArray = indexArray->size();

		XMMATRIX identity = XMMatrixIdentity();
		XMStoreFloat4x4(&WorldMatrix, identity);

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

		hr = device->CreateVertexShader(Floor_VS, sizeof(Floor_VS), NULL, &VertexShader);
		if (hr != S_OK) return hr;
		hr = device->CreatePixelShader(Floor_PS, sizeof(Floor_PS), NULL, &PixelShader);
		if (hr != S_OK) return hr;

		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UV", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		hr = device->CreateInputLayout(inputElementDesc, 4, Floor_VS, sizeof(Floor_VS), &InputLayout);
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

		ZeroMemory(&samplerDesc, sizeof(samplerDesc));
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
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
	void Display(){

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
		SAFE_RELEASE(pointlightposbuffer);
		SAFE_RELEASE(spotlightPosBuffer);
		SAFE_RELEASE(directionalLightBuffer);
		SAFE_RELEASE(front);
		SAFE_RELEASE(back);
	}
};