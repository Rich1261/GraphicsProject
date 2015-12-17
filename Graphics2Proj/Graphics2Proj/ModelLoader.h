#pragma once
#include "stdafx.h"
void CreateTangents(vector<_OBJ_VERT_> &vertList){
	float3 *tan1 = new float3[vertList.size()];
	float3 *tan2 = new float3[vertList.size()];
	ZeroMemory(tan1, sizeof(float3) * vertList.size());
	ZeroMemory(tan2, sizeof(float3) * vertList.size());
	for (UINT i = 0; i < vertList.size(); i+=3){
		float3 vert0 = float3(vertList[i].pos);
		float3 vert1 = float3(vertList[i + 1].pos);
		float3 vert2 = float3(vertList[i + 2].pos);
		float3 vertEdge0 = vert1 - vert0;
		float3 vertEdge1 = vert2 - vert0;
		float3 tex0 = float3(vertList[i].uvw);
		float3 tex1 = float3(vertList[i + 1].uvw);
		float3 tex2 = float3(vertList[i + 2].uvw);
		float3 texEdge0 = tex1 - tex0;
		float3 texEdge1 = tex2 - tex0;
		float ratio = 1.0f / (texEdge0.x * texEdge1.y - texEdge1.y * texEdge0.y);
		float3 uDirection = float3((texEdge1.y * vertEdge0.x - texEdge0.y * vertEdge1.x) * ratio, (texEdge1.y * vertEdge0.y - texEdge0.y * vertEdge1.y) * ratio, (texEdge1.y * vertEdge0.z - texEdge0.y * vertEdge1.z) * ratio);
		float3 vDirection = float3((texEdge1.x * vertEdge0.x - texEdge0.x * vertEdge1.x) * ratio, (texEdge1.x * vertEdge0.y - texEdge0.x * vertEdge1.y) * ratio, (texEdge1.x * vertEdge0.z - texEdge0.x * vertEdge1.z) * ratio);
		tan1[i] = tan1[i + 1] = tan1[i + 2] = uDirection;
		tan2[i] = tan2[i + 1] = tan2[i + 2] = vDirection;
	}
	for (UINT i = 0; i < vertList.size(); i++){
		const float3 &normal = float3(vertList[i].nrm);
		XMVECTOR u = XMVectorSet(normal.x, normal.y, normal.z, 0.0f);
		u = XMVector3Normalize(u);
		XMVECTOR v = XMVectorSet(tan1[i].x, tan1[i].y, tan1[i].z, 0.0f);
		v = XMVector3Normalize(v);
		XMVECTOR dot = XMVector3Dot(u, v);
		XMVECTOR tan = v - u * dot.m128_f32[0];
		tan = XMVector3Normalize(tan);
		XMStoreFloat4(&vertList[i].tan, tan);
		XMVECTOR cross = XMVector3Cross(u, v);
		XMVECTOR tangent = XMVectorSet(tan2[i].x, tan2[i].y, tan2[i].z, 0.0f);
		tangent = XMVector3Normalize(tangent);
		XMVECTOR dot2 = XMVector3Dot(cross, tangent);
		vertList[i].tan.w = (dot2.m128_f32[0] < 0.0f) ? -1.0f : 1.0f;
	}
	delete[] tan1;
	delete[] tan2;
}
string loadOBJ(const char* filepath, vector<_OBJ_VERT_> &vertList, vector<UINT> &indexarray){
	string error;
	vector<UINT> vertIndices, uvIndices, normIndices;
	vector<float3> tempVertList, tempNormList;
	vector<float2> tempUVList;

	FILE *file;
	errno_t result;
	result = fopen_s(&file, filepath, "r");

	if (file == NULL){
		error = "error in opening file";
		return error;
	}
	while (true){
		char lineHeader[128];
		int res = fscanf_s(file, "%s", lineHeader, 128);
		if (res == EOF) break;
		if (strcmp(lineHeader, "v") == 0){
			float3 vert;
			fscanf_s(file, "%f %f %f\n", &vert.x, &vert.y, &vert.z);
			tempVertList.push_back(vert);
		}
		else if (strcmp(lineHeader, "vt") == 0){
			float2 uv;
			fscanf_s(file, "%f %f\n", &uv.x, &uv.y);
			tempUVList.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0){
			float3 normal;
			fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			tempNormList.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0){
			//string vert1, vert2, vert3;
			UINT vertIndex[3], uvIndex[3], normalIndex[3];
			int matchs = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertIndex[0], &uvIndex[0], &normalIndex[0], &vertIndex[1], &uvIndex[1], &normalIndex[1], &vertIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matchs != 9){
				error = "error in faces reading";
				return error;
			}
			vertIndices.push_back(vertIndex[0]);
			vertIndices.push_back(vertIndex[1]);
			vertIndices.push_back(vertIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normIndices.push_back(normalIndex[0]);
			normIndices.push_back(normalIndex[1]);
			normIndices.push_back(normalIndex[2]);
		}
	}
	//Multi all array sizes by half because the .obj duplicates the image right after itself
	vertList.resize(vertIndices.size());
	indexarray.resize(vertIndices.size());
	//if not checking for unique verts here - index array can just be 0->size of the vertList
	for (UINT i = 0; i < vertIndices.size(); i++){
		UINT vertIndex = vertIndices[i];
		float3 vertex = tempVertList[vertIndex - 1];
		vertList[i].pos = vertex;
	}
	for (UINT i = 0; i < uvIndices.size(); i++){
		UINT uvIndex = uvIndices[i];
		float2 uv = tempUVList[uvIndex - 1];
		uv.y = 1 - uv.y;
		vertList[i].uvw = float3(uv.x, uv.y, 0);
	}
	for (UINT i = 0; i < normIndices.size(); i++){
		UINT normIndex = normIndices[i];
		float3 normal = tempNormList[normIndex - 1];
		vertList[i].nrm = normal;
	}
	CreateTangents(vertList);
	for (UINT i = 0; i < vertList.size(); i++){
		indexarray[i] = i;
	}
	error = "Successful Load";
	return error;
}

