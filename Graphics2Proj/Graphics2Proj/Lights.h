#pragma once

#include "SharedDefines.h"
#include "XTime.h"
using namespace DirectX;

struct SpotlightStats{
	float4 pos;
	float4 dir;
};

class Spotlight{
private:
	bool left = false;
	XTime timer;
	float time = 0;
	float degree = 0;
public:
	void Update(SpotlightStats &stats){
		timer.Signal();
		time += (float)timer.Delta();
		degree += 0.01f;
		if (stats.pos.x > 10){
			left = true;
		}
		if (stats.pos.x < -10){
			left = false;
		}
		if (left){
			stats.pos.x -= .025f;
		}
		else{
			stats.pos.x += .025f;
		}
		//XMMATRIX rotZ = XMMatrixIdentity();
		//XMMATRIX rotX = XMMatrixIdentity();
		//XMMATRIX temp = XMMatrixIdentity();
		//
		//XMVECTOR z = XMLoadFloat4(&stats.dir);
		//z = XMVector4Normalize(z);
		//
		//float4 tempy(0, 1, 0, 0);
		//XMVECTOR tempY = XMLoadFloat4(&tempy);
		//tempY = XMVector4Normalize(tempY);
		//
		//XMVECTOR x = XMVector3Cross(tempY, z);
		//x = XMVector4Normalize(x);
		//
		//XMVECTOR y = XMVector3Cross(z, x);
		//y = XMVector4Normalize(y);
		//
		//XMVECTOR w = XMLoadFloat4(&stats.pos);
		//temp.r[0] = x;
		//temp.r[1] = y;
		//temp.r[2] = z;
		//temp.r[3] = w;
		//
		//rotZ = XMMatrixRotationZ(time * 0.05f);
		//rotX = XMMatrixRotationX(time * 0.05f);
		//
		//temp = XMMatrixMultiply(temp, rotX);
		//temp = XMMatrixMultiply(rotZ, temp);
		//
		//XMStoreFloat4(&stats.dir, temp.r[2]);
		//XMStoreFloat4(&stats.pos, temp.r[3]);
		stats.dir.x = cos(degree);
		stats.dir.z = sin(degree);
		//if (dx > 0) stats.dir = float4(-stats.dir.x, -stats.dir.y, - stats.dir.z, stats.dir.w);
	}
};

struct DirectionalLightStats{
	float4 dir;
};
class DirectionalLight{
private:
	float degree = 0.0f;
public:
	void Update(DirectionalLightStats &stats){
		degree += 0.01f;
		stats.dir.y = sin(degree);
		stats.dir.x = cos(degree);
	}
};

struct PointLightStats{
	float4 pos;
};
class PointLight{
private:

public:
	void Update(PointLightStats &stats, bool changeDir = false){
		if (changeDir){
			stats.pos.x -= .05f;
		}
		else{
			stats.pos.x += .05f;
		}
	}
};