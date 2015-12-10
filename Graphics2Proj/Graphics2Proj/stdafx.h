#pragma once
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <assert.h>
#include <string>
#include <vector>
#include <Windows.h>
#include <numeric>
#include "SharedDefines.h"
#include "DDSTextureLoader.h"
#include "Lights.h"
#include <comdef.h>
#include <ctime>
#include <cstdlib>
#include <thread>
#include <mutex>
#include <condition_variable>


#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

#include "Grid_PS.csh"
#include "Grid_VS.csh"
#include "Star_PS.csh"
#include "Star_VS.csh"
#include "skybox_VS.csh"
#include "skybox_PS.csh"
#include "Pyramid_PS.csh"
#include "Pyramid_VS.csh"
#include "Floor_VS.csh"
#include "Floor_PS.csh"
#include "Snowflake_VS.csh"
#include "Snowflake_PS.csh"
#include "Snowflake_GS.csh"

using namespace std;
using namespace DirectX;
#define SAFE_RELEASE(x) \
	if(x){\
		x->Release();\
		x = nullptr;\
				}\

#define BACKBUFFER_WIDTH	1000
#define BACKBUFFER_HEIGHT	1000
#define WHITE 0xFFFFFFFF
#define BLACK 0x00000000
#define RED 0x00FF0000
#define GREEN 0x0000FF00
#define BLUE 0x000000FF
#define YELLOW 0x00FFFF00
#define CYAN 0x0000FFFF
#define MAGENTA 0x00FF00FF
#define PURPLE 0x00660099

#define RASTER_WIDTH 500
#define RASTER_HEIGHT 500
#define NUM_RANDOM_LOCATIONS 2500
#define NUM_PIXELS ((RASTER_WIDTH)*(RASTER_HEIGHT))
#define ROTATION_SPEED 3

unsigned int raster[NUM_PIXELS];
float depthBuffer[NUM_PIXELS];
int randomLocs[NUM_RANDOM_LOCATIONS];
float ShaderUValue = 0;
float ShaderVValue = 0;
float nearPlane = .1f;
float farPlane = 10.0f;
float ShaderWValue = 0;

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
struct Matrix4x4{
	float r[4][4];
	Matrix4x4(float s1 = 1, float s2 = 0, float s3 = 0, float s4 = 0, float s5 = 0, float s6 = 1, float s7 = 0, float s8 = 0, float s9 = 0, float s10 = 0, float s11 = 1, float s12 = 0, float s13 = 0, float s14 = 0, float s15 = 0, float s16 = 1){
		r[0][0] = s1;
		r[0][1] = s2;
		r[0][2] = s3;
		r[0][3] = s4;
		r[1][0] = s5;
		r[1][1] = s6;
		r[1][2] = s7;
		r[1][3] = s8;
		r[2][0] = s9;
		r[2][1] = s10;
		r[2][2] = s11;
		r[2][3] = s12;
		r[3][0] = s13;
		r[3][1] = s14;
		r[3][2] = s15;
		r[3][3] = s16;
	};
};
struct SIMPLE_VERTEX{
	float x, y, z, w;
	float r, g, b, a;
	SIMPLE_VERTEX(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f, float _w = 1.0f, float _r = 0.0f, float _g = 0.0f, float _b = 0.0f, float _a = 0.0f){
		x = _x; y = _y; z = _z; w = _w;
		r = _r; g = _g; b = _b; a = _a;
	};
};
struct TwoMatricies4x4{
	float4x4 m1;
	float4x4 m2;
	TwoMatricies4x4(float4x4 _m1, float4x4 _m2){ m1 = _m1; m2 = _m2; };
};
struct SEND_WORLD_TO_VRAM {
	float4x4 worldMatrix;
	float faceNum = 1;
	float3 padding;
};
struct SEND_MATRICIES_TO_VRAM {
	float4x4 viewMatrix;
	float4x4 projMatrix;
};
struct _OBJ_VERT_
{
	float pos[3];
	float uvw[3];
	float nrm[3];
};
struct SnowflakeStruct{
	float4 pos;
	float4 origPos;
	float speed;
	float dist;
	SnowflakeStruct(float4 _pos = float4(0, 1, 0, 1), float dropSpeed = .0025f, float dropDistance = 5){
		pos = _pos; 
		origPos = _pos;
		speed = dropSpeed;
		dist = dropDistance;
	}
};