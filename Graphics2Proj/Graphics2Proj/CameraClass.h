#pragma once
#include "stdafx.h"

class Camera{
private:
	POINT previous;
	int type;
	UINT up, down, left, right, look;
public:
	void Instantiate(POINT prev, int ControlsType = 0){
		previous = prev;
		type = ControlsType;
	}
	float4 GetCameraPos(float4x4 viewMatrix){
		return float4(viewMatrix.m[3][0], viewMatrix.m[3][1], viewMatrix.m[3][2], viewMatrix.m[3][3]);
	}
	float4x4 Update(float4x4 &viewMatrix){
		//W/S forward/backward local Z translation
		switch (type){
		case 0:{
			up = 0x57;
			down = 0x53;
			left = 0x44;
			right = 0x41;
			look = VK_RBUTTON;
			break;
		}
		case 1:{
			up = 0x54;
			down = 0x47;
			left = 0x46;
			right = 0x48;
			look = VK_LBUTTON;
			break;
		}
		}//end switch statement
		if (GetAsyncKeyState(up)){
			viewMatrix.m[3][0] += viewMatrix.m[2][0] * .05f;
			viewMatrix.m[3][1] += viewMatrix.m[2][1] * .05f;
			viewMatrix.m[3][2] += viewMatrix.m[2][2] * .05f;
			viewMatrix.m[3][3] += viewMatrix.m[2][3] * .05f;
		}
		if (GetAsyncKeyState(down)){
			viewMatrix.m[3][0] -= viewMatrix.m[2][0] * .05f;
			viewMatrix.m[3][1] -= viewMatrix.m[2][1] * .05f;
			viewMatrix.m[3][2] -= viewMatrix.m[2][2] * .05f;
			viewMatrix.m[3][3] -= viewMatrix.m[2][3] * .05f;
		}

		//A/D left/right local X translation			
		if (GetAsyncKeyState(left)){
			viewMatrix.m[3][0] += viewMatrix.m[0][0] * .05f;
			viewMatrix.m[3][1] += viewMatrix.m[0][1] * .05f;
			viewMatrix.m[3][2] += viewMatrix.m[0][2] * .05f;
			viewMatrix.m[3][3] += viewMatrix.m[0][3] * .05f;
		}
		if (GetAsyncKeyState(right)){
			viewMatrix.m[3][0] -= viewMatrix.m[0][0] * .05f;
			viewMatrix.m[3][1] -= viewMatrix.m[0][1] * .05f;
			viewMatrix.m[3][2] -= viewMatrix.m[0][2] * .05f;
			viewMatrix.m[3][3] -= viewMatrix.m[0][3] * .05f;
		}

		POINT currPoint;
		GetCursorPos(&currPoint);
		if (GetAsyncKeyState(look)){

			LONG dx = currPoint.x - previous.x;
			LONG dy = currPoint.y - previous.y;

			if (dy || dx){

				XMMATRIX rotX = XMMatrixIdentity();
				XMMATRIX rotY = XMMatrixIdentity();
				rotX = XMMatrixRotationX(dy * .005f);
				rotY = XMMatrixRotationY(dx * .005f);
				float3 pos = float3(viewMatrix.m[3][0], viewMatrix.m[3][1], viewMatrix.m[3][2]);

				viewMatrix.m[3][0] = viewMatrix.m[3][1] = viewMatrix.m[3][2] = 0;

				XMMATRIX temp = XMLoadFloat4x4(&viewMatrix);
				temp = XMMatrixMultiply(temp, rotY);
				temp = XMMatrixMultiply(rotX, temp);
				//viewMatrix = MultiplyMatrix4x4ByMatrix4x4(viewMatrix, rotY);
				//viewMatrix = MultiplyMatrix4x4ByMatrix4x4(rotX, viewMatrix);
				DirectX::XMStoreFloat4x4(&viewMatrix, temp);
				viewMatrix.m[3][0] = pos.x;
				viewMatrix.m[3][1] = pos.y;
				viewMatrix.m[3][2] = pos.z;
			}
		}

		previous = currPoint;
		float4x4 trueview = viewMatrix;
		//OrthogonalAffineMatrix4x4Inverse(trueview);
		XMMATRIX view = XMLoadFloat4x4(&trueview);
		view = XMMatrixInverse(&XMMatrixDeterminant(view), view);
		DirectX::XMStoreFloat4x4(&trueview, view);
		return trueview;
	}
};