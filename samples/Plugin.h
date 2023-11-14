#pragma once

#include <vector>
#include <list>
#include <iostream>
#include <memory>
#include <Eigen/Core>

#include "SOLID.h"

struct PluginCollisionData
{
	unsigned int id1;
	unsigned int id2;
	float point1[3];
	float point2[3];
	float normal[3];
};

enum class ShapeType
{
	CUBE = 0u,
	SPHERE = 1u,
	CYLINDER = 2u
};


#ifndef _WIN32
#define __stdcall 
#endif

#ifdef _WIN32
#define DLLExport __declspec(dllexport)
#else
#define DLLExport
#endif

struct PluginObject
{
	int id;
	DT_ObjectHandle object;
	DT_ShapeHandle shape;
	DT_VertexBaseHandle vertexBase;
};

struct PluginScene
{
	DT_SceneHandle     scene;
	DT_RespTableHandle respTable;
	DT_ResponseClass   objectClass;
};


static std::shared_ptr<PluginScene> m_Scene;
static std::shared_ptr<std::vector<PluginObject>> m_Objects;
static std::shared_ptr<PluginCollisionData> m_CollResult;

extern "C"
{

	DLLExport void InitCollisionDetection();
	DLLExport void Dispose();
	DLLExport unsigned int AllObjectsCollisionCheck();
	DLLExport void* GetCollisionPtr();

	DLLExport unsigned int ConstructComplexShape(const void* vertex, const unsigned int* indices, unsigned int vertexCount);
	DLLExport unsigned int ConstructRigidBody(unsigned int shapeType, const float* pos, const float radius, const float height);
	
	DLLExport void UpdateComplexShape(unsigned int id, const void* vertex);
	DLLExport void UpdateObjectPosition(unsigned int id, const float* pos);
	DLLExport void UpdateObjectRotation(unsigned int id, const float* rot);
	DLLExport void UpdateObjectScale(unsigned int id, const float* scale);
	DLLExport void UpdateObjectModelMatrix(unsigned int id, const float* mat);
}
