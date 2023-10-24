#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <vector>

#include "Plugin.h"


struct PluginObject
{
	DT_ObjectHandle object;
	DT_ShapeHandle shape;
	DT_VertexBaseHandle vertexBase;
};

struct PluginScene
{
	DT_SceneHandle scene		   = DT_CreateScene();
	DT_RespTableHandle respTable   = DT_CreateRespTable();
	DT_ResponseClass responseClass = DT_GenResponseClass(respTable);
};

static std::shared_ptr<PluginScene> m_scene;
static std::shared_ptr<PluginObject> m_colonObject;
static std::shared_ptr<PluginObject> m_endoObject;

void ConstructShape(std::shared_ptr<PluginObject> object, const void* vertex, unsigned int count)
{
	object->vertexBase = DT_NewVertexBase(vertex, 0);
	object->shape = DT_NewComplexShape(object->vertexBase);

	for (auto i = 0u; i < count; i += 3)
	{
		DT_Begin();
		DT_VertexIndex(i + 0);
		DT_VertexIndex(i + 1);
		DT_VertexIndex(i + 2);
		DT_End();
	}

	// DT_VertexRange(0, count - 1);
	DT_EndComplexShape();

	object->object = DT_CreateObject(&object, object->shape);
}

void ConstructColonComplexShape(const void* vertex, unsigned int count)
{
	if (!m_scene)
		InitCollisionDetection();

	m_colonObject = std::make_shared<PluginObject>();
	ConstructShape(m_colonObject, vertex, count);

	DT_AddObject(m_scene->scene, m_colonObject->object);
	DT_SetResponseClass(m_scene->respTable, m_colonObject->object, m_scene->responseClass);
}

void ConstructEndoComplexShape(const void* vertex, unsigned int count)
{
	if (!m_scene)
		InitCollisionDetection();

	m_endoObject = std::make_shared<PluginObject>();
	ConstructShape(m_endoObject, vertex, count);

	DT_AddObject(m_scene->scene, m_endoObject->object);
	DT_SetResponseClass(m_scene->respTable, m_endoObject->object, m_scene->responseClass);
}

DT_Bool collResp(void* client_data, void* obj1, void* obj2, const DT_CollData* coll_data)
{
	// Print out the collision data with std::cout
	// std::cout << "Collision detected" << std::endl;
	// std::cout << "Collision Data: " << std::endl;
	/*std::cout << coll_data->normal[0] << " " << coll_data->normal[1] << " " << coll_data->normal[2] << std::endl;
	std::cout << coll_data->point1[0] << " " << coll_data->point1[1] << " " << coll_data->point1[2] << std::endl;
	std::cout << coll_data->point2[0] << " " << coll_data->point2[1] << " " << coll_data->point2[2] << std::endl;*/

	return true;
}

void InitCollisionDetection()
{
	m_scene = std::make_shared<PluginScene>();
	m_scene->scene = DT_CreateScene();
	m_scene->respTable = DT_CreateRespTable();
	m_scene->responseClass = DT_GenResponseClass(m_scene->respTable);

	DT_AddDefaultResponse(m_scene->respTable, &collResp, DT_SIMPLE_RESPONSE, stdout);
}

void UpdateColonComplexShape(const void* vertex)
{
	DT_ChangeVertexBase(m_colonObject->vertexBase, vertex);
}

void UpdateEndoComplexShape(const void* vertex)
{
	DT_ChangeVertexBase(m_endoObject->vertexBase, vertex);
}

DT_ObjectHandle GetColonObject()
{
	return m_colonObject->object;
}

DT_ObjectHandle GetEndoObject()
{
	return m_endoObject->object;
}

bool CollisionCheck()
{
	return DT_Test(m_scene->scene, m_scene->respTable);
}

void* GetCollisionResultPtr()
{
	int* a = new int;
	return a;
}

void Dispose()
{
	DT_RemoveObject(m_scene->scene, m_colonObject->object);
	DT_RemoveObject(m_scene->scene, m_endoObject->object);
	DT_DestroyObject(m_colonObject->object);
	DT_DestroyObject(m_endoObject->object);
	DT_DeleteShape(m_colonObject->shape);
	DT_DeleteShape(m_endoObject->shape);

	DT_DestroyRespTable(m_scene->respTable);
	DT_DestroyScene(m_scene->scene);
}