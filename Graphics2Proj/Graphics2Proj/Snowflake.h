#pragma once
#include "stdafx.h"

class Snowflakes{
private:
	ID3D11Buffer *VertexBuffer = nullptr, *IndexBuffer = nullptr, *ConstWorldMatrixBuffer = nullptr, *constActualStatsBuffer = nullptr;

	ID3D11InputLayout *InputLayout = nullptr;

	ID3D11VertexShader *VertexShader = nullptr;
	ID3D11PixelShader *PixelShader = nullptr;
	ID3D11GeometryShader *GeometryShader = nullptr;

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
	SEND_WORLD_TO_VRAM WorldToShader;
	float4x4 WorldMatrix;
	UINT VertCount = 0;
	HRESULT hr;
	XTime timer;
	float time = 0;
	XMMATRIX identity;
	vector<SnowflakeStruct> points;
public:
	HRESULT Instantiate(ID3D11Device *device, vector<SnowflakeStruct> *coordArray, ID3D11ShaderResourceView *_srv = nullptr){
		VertCount = coordArray->size();
		identity = XMMatrixIdentity();

		for (UINT i = 0; i < VertCount; i++){
			points.push_back((*coordArray)[i]);
		}

		XMMATRIX identity = DirectX::XMMatrixIdentity();
		XMStoreFloat4x4(&WorldMatrix, identity);

		bufferDesc.ByteWidth = sizeof(SnowflakeStruct) * coordArray->size();
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = NULL;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.MiscFlags = NULL;
		bufferDesc.StructureByteStride = sizeof(SnowflakeStruct);
		subResourceData.pSysMem = coordArray->data();
		subResourceData.SysMemPitch = 0;
		subResourceData.SysMemSlicePitch = 0;
		hr = device->CreateBuffer(&bufferDesc, &subResourceData, &VertexBuffer);
		if (hr != S_OK){
			return hr;
		}

		hr = device->CreateVertexShader(Snowflake_VS, sizeof(Snowflake_VS), NULL, &VertexShader);
		if (hr != S_OK) return hr;
		hr = device->CreatePixelShader(Snowflake_PS, sizeof(Snowflake_PS), NULL, &PixelShader);
		if (hr != S_OK) return hr;
		hr = device->CreateGeometryShader(Snowflake_GS, sizeof(Snowflake_GS), NULL, &GeometryShader);
		if (hr != S_OK) return hr;

		D3D11_INPUT_ELEMENT_DESC inputElementDesc[4] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "ORIGPOS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "SPEED", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "DISTANCE", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		hr = device->CreateInputLayout(inputElementDesc, 4, Snowflake_VS, sizeof(Snowflake_VS), &InputLayout);
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

		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = NULL;
		bufferDesc.ByteWidth = sizeof(points) * points.size();
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.StructureByteStride = sizeof(points);
		subResourceData.pSysMem = points.data();
		subResourceData.SysMemPitch = 0;
		subResourceData.SysMemSlicePitch = 0;
		hr = device->CreateBuffer(&bufferDesc, NULL, &constActualStatsBuffer);
		if (hr != S_OK) return hr;

		//bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		//bufferDesc.CPUAccessFlags = 0;
		//bufferDesc.MiscFlags = NULL;
		//bufferDesc.ByteWidth = sizeof(unsigned int) * indexArray->size();
		//bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		//bufferDesc.StructureByteStride = sizeof(unsigned int);
		//subResourceData.pSysMem = indexArray->data();
		//subResourceData.SysMemPitch = 0;
		//subResourceData.SysMemSlicePitch = 0;
		//hr = device->CreateBuffer(&bufferDesc, &subResourceData, &IndexBuffer);
		//if (hr != S_OK) return hr;

		shaderResourceView = _srv;

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
	void Display(ID3D11DeviceContext *deviceContext, ID3D11Buffer *viewProjBuffer, D3D11_MAPPED_SUBRESOURCE mapSubResource, SEND_MATRICIES_TO_VRAM viewProjMatricies){
		deviceContext->OMSetBlendState(blendState, NULL, 0xffffffff);
		timer.Signal();
		time += (float)timer.Delta();

		//float3 pos = float3(WorldToShader.worldMatrix.m[3][0], WorldToShader.worldMatrix.m[3][1], WorldToShader.worldMatrix.m[3][2]);
		//WorldToShader.worldMatrix.m[3][0] = WorldToShader.worldMatrix.m[3][1] = WorldToShader.worldMatrix.m[3][2] = 0;

		//XMMATRIX world = XMLoadFloat4x4(&WorldToShader.worldMatrix); 
		//XMMATRIX rotation = XMMatrixRotationY(.0025f);
		//XMMATRIX temp = XMMatrixMultiply(rotation, world);
		//XMStoreFloat4x4(&WorldToShader.worldMatrix, temp);

		//WorldToShader.worldMatrix.m[3][0] = pos.x;
		//WorldToShader.worldMatrix.m[3][1] = pos.y;
		//WorldToShader.worldMatrix.m[3][2] = pos.z;

		for (UINT i = 0; i < points.size(); i++){
			points[i].pos.y -= points[i].speed;
			if (points[i].pos.y <= (points[i].origPos.y - points[i].dist))
				points[i].pos.y = points[i].origPos.y;
		}
		//WorldToShader.faceNum -= .0025f;
		//if (WorldToShader.faceNum < -3) WorldToShader.faceNum = 0;

		deviceContext->GSSetConstantBuffers(0, 1, &ConstWorldMatrixBuffer);
		deviceContext->GSSetConstantBuffers(1, 1, &viewProjBuffer);
		deviceContext->GSSetConstantBuffers(2, 1, &constActualStatsBuffer);

		deviceContext->Map(constActualStatsBuffer, 0, D3D11_MAP_WRITE_DISCARD, NULL, &mapSubResource);
		memcpy(mapSubResource.pData, points.data(), sizeof(points));
		deviceContext->Unmap(constActualStatsBuffer, 0);

		deviceContext->Map(ConstWorldMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, NULL, &mapSubResource);
		memcpy(mapSubResource.pData, &WorldToShader, sizeof(WorldToShader));
		deviceContext->Unmap(ConstWorldMatrixBuffer, 0);

		deviceContext->Map(viewProjBuffer, 0, D3D11_MAP_WRITE_DISCARD, NULL, &mapSubResource);
		memcpy(mapSubResource.pData, &viewProjMatricies, sizeof(viewProjMatricies));
		deviceContext->Unmap(viewProjBuffer, 0);

		const UINT size = sizeof(SnowflakeStruct);
		const UINT offset = 0;

		deviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &size, &offset);

		deviceContext->VSSetShader(VertexShader, NULL, 0);

		deviceContext->GSSetShader(GeometryShader, NULL, 0);

		deviceContext->PSSetShader(PixelShader, NULL, 0);
		deviceContext->PSSetSamplers(0, 1, &sampler);
		deviceContext->PSSetShaderResources(0, 1, &shaderResourceView);

		deviceContext->IASetInputLayout(InputLayout);

		deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
		deviceContext->RSSetState(back);
		//deviceContext->DrawIndexed(sizeofIndexArray, 0, 0);
		deviceContext->Draw(VertCount, 0);
		deviceContext->RSSetState(front);
		deviceContext->Draw(VertCount, 0);
		deviceContext->RSSetState(back);
		//setbackface culling
		deviceContext->GSSetShader(NULL, NULL, 0);
	}
	void Terminate(){
		SAFE_RELEASE(VertexBuffer);
		SAFE_RELEASE(IndexBuffer);
		SAFE_RELEASE(ConstWorldMatrixBuffer);
		SAFE_RELEASE(constActualStatsBuffer);
		SAFE_RELEASE(InputLayout);
		SAFE_RELEASE(VertexShader);
		SAFE_RELEASE(PixelShader);
		SAFE_RELEASE(GeometryShader);
		SAFE_RELEASE(texture);
		SAFE_RELEASE(sampler);
		//SAFE_RELEASE(shaderResourceView);
		SAFE_RELEASE(blendState);
		SAFE_RELEASE(front);
		SAFE_RELEASE(back);
	}
};