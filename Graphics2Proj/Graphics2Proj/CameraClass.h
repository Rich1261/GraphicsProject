#pragma once
#include "StarClass.h"

class Camera{
private:
	POINT previous;
	struct Matrix3X3{
		float r[3][3];
		Matrix3X3(float s1 = 1, float s2 = 0, float s3 = 0, float s4 = 0, float s5 = 1, float s6 = 0, float s7 = 0, float s8 = 0, float s9 = 1){
			r[0][0] = s1;
			r[0][1] = s2;
			r[0][2] = s3;
			r[1][0] = s4;
			r[1][1] = s5;
			r[1][2] = s6;
			r[2][0] = s7;
			r[2][1] = s8;
			r[2][2] = s9;
		};
	};
	struct Vertex{
		float x, y, z, w, u, v;
		unsigned int color;
		Vertex(float _x = 0, float _y = 0, float _z = 0, float _w = 1, float _u = 0, float _v = 0, unsigned int _color = 0xFFFFFFFF){
			x = _x;
			y = _y;
			z = _z;
			w = _w;
			u = _u;
			v = _v;
			color = _color;
		};
	};
	void MultiplyVertexByMatrix(Vertex& vertex, Matrix3X3 matrix){
		Vertex temp = vertex;
		temp.x = vertex.x * matrix.r[0][0] + vertex.y * matrix.r[1][0] + vertex.z * matrix.r[2][0];
		temp.y = vertex.x * matrix.r[0][1] + vertex.y * matrix.r[1][1] + vertex.z * matrix.r[2][1];
		temp.z = vertex.x * matrix.r[0][2] + vertex.y * matrix.r[1][2] + vertex.z * matrix.r[2][2];
		vertex = temp;
	}
	void OrthogonalAffineMatrix4x4Inverse(float4x4& m){
		float4x4 t = m;
		t.m[0][1] = t.m[1][0];
		t.m[0][2] = t.m[2][0];
		t.m[1][0] = m.m[0][1];
		t.m[1][2] = t.m[2][1];
		t.m[2][0] = m.m[0][2];
		t.m[2][1] = m.m[1][2];
		Matrix3X3 c(t.m[0][0], t.m[0][1], t.m[0][2], t.m[1][0], t.m[1][1], t.m[1][2], t.m[2][0], t.m[2][1], t.m[2][2]);
		Vertex p(t.m[3][0], t.m[3][1], t.m[3][2]);
		MultiplyVertexByMatrix(p, c);
		t.m[3][0] = -p.x;
		t.m[3][1] = -p.y;
		t.m[3][2] = -p.z;
		m = t;
	}
public:
	void Instantiate(POINT prev){
		previous = prev;
	}
	float4 GetCameraPos(float4x4 viewMatrix){
		return float4(viewMatrix.m[3][0], viewMatrix.m[3][1], viewMatrix.m[3][2], viewMatrix.m[3][3]);
	}
	float4x4 Update(float4x4 &viewMatrix){
		//W/S forward/backward local Z translation
		if (GetAsyncKeyState(0x57)){
			viewMatrix.m[3][0] += viewMatrix.m[2][0] * .05f;
			viewMatrix.m[3][1] += viewMatrix.m[2][1] * .05f;
			viewMatrix.m[3][2] += viewMatrix.m[2][2] * .05f;
			viewMatrix.m[3][3] += viewMatrix.m[2][3] * .05f;
		}
		if (GetAsyncKeyState(0x53)){
			viewMatrix.m[3][0] -= viewMatrix.m[2][0] * .05f;
			viewMatrix.m[3][1] -= viewMatrix.m[2][1] * .05f;
			viewMatrix.m[3][2] -= viewMatrix.m[2][2] * .05f;
			viewMatrix.m[3][3] -= viewMatrix.m[2][3] * .05f;
		}				

		//A/D left/right local X translation			
		if (GetAsyncKeyState(0x44)){
			viewMatrix.m[3][0] += viewMatrix.m[0][0] * .05f;
			viewMatrix.m[3][1] += viewMatrix.m[0][1] * .05f;
			viewMatrix.m[3][2] += viewMatrix.m[0][2] * .05f;
			viewMatrix.m[3][3] += viewMatrix.m[0][3] * .05f;
		}
		if (GetAsyncKeyState(0x41)){
			viewMatrix.m[3][0] -= viewMatrix.m[0][0] * .05f;
			viewMatrix.m[3][1] -= viewMatrix.m[0][1] * .05f;
			viewMatrix.m[3][2] -= viewMatrix.m[0][2] * .05f;
			viewMatrix.m[3][3] -= viewMatrix.m[0][3] * .05f;
		}

		POINT currPoint;
		GetCursorPos(&currPoint);
		if (GetAsyncKeyState(VK_RBUTTON)){

			LONG dx = currPoint.x - previous.x;
			LONG dy = currPoint.y - previous.y;

			if (dy || dx){
				
				XMMATRIX rotX = XMMatrixIdentity();
				XMMATRIX rotY = XMMatrixIdentity();
				rotX = XMMatrixRotationX(dy * -.005f);
				rotY = XMMatrixRotationY(dx * -.005f);
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