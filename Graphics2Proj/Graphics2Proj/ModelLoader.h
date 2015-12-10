#pragma once
#include "stdafx.h"

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
		*(float3*)(&vertList[i].pos) = vertex;
	}
	for (UINT i = 0; i < uvIndices.size(); i++){
		UINT uvIndex = uvIndices[i];
		float2 uv = tempUVList[uvIndex - 1];
		uv.x = 1 - uv.x;
		*(float3*)(&vertList[i].uvw) = float3(uv.x, uv.y, 0);
	}
	for (UINT i = 0; i < normIndices.size(); i++){
		UINT normIndex = normIndices[i];
		float3 normal = tempNormList[normIndex - 1];
		*(float3*)(&vertList[i].nrm) = normal;
	}
	for (UINT i = 0; i < vertList.size(); i++){
		indexarray[i] = i;
	}
	error = "Successful Load";
	return error;
}