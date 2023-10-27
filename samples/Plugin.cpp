#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <vector>

#include "Plugin.h"

struct PluginObject
{
	unsigned int id;
	DT_ObjectHandle object;
	DT_ShapeHandle shape;
	DT_VertexBaseHandle vertexBase;
};

struct PluginScene
{
	DT_SceneHandle     scene;
	DT_RespTableHandle respTable;
	DT_ResponseClass   objectClass;

	DT_RespTableHandle fix_respTable;
	DT_ResponseClass   fix_objectClass;
};

static std::shared_ptr<PluginScene> m_Scene;
static std::shared_ptr<std::vector<PluginObject>> m_Objects;
static std::list<PluginCollisionData> m_CollResult;

DT_Bool collResp(void* client_data, void* obj1, void* obj2, const DT_CollData* coll_data)
{
	PluginCollisionData data;
	data.point1[0] = coll_data->point1[0];
	data.point1[1] = coll_data->point1[1];
	data.point1[2] = coll_data->point1[2];
	data.point2[0] = coll_data->point2[0];
	data.point2[1] = coll_data->point2[1];
	data.point2[2] = coll_data->point2[2];
	data.normal[0] = coll_data->normal[0];
	data.normal[1] = coll_data->normal[1];
	data.normal[2] = coll_data->normal[2];
	m_CollResult.push_back(data);

	return DT_CONTINUE;
}

void InitCollisionDetection()
{
	m_Objects = std::make_shared<std::vector<PluginObject>>();

	m_Scene = std::make_shared<PluginScene>();
	m_Scene->scene = DT_CreateScene();

	m_Scene->respTable = DT_CreateRespTable();
	m_Scene->objectClass = DT_GenResponseClass(m_Scene->respTable);
	DT_AddPairResponse(m_Scene->respTable, m_Scene->objectClass, m_Scene->objectClass, collResp, DT_DEPTH_RESPONSE, nullptr);
}  

unsigned int CollisionCheck()
{
	return DT_Test(m_Scene->scene, m_Scene->respTable);
}

void* GetCollisionResultPtr()
{
	return &m_CollResult;
}

std::list<PluginCollisionData>* GetVectorCollisionResult()
{
	return &m_CollResult;
}

void Dispose()
{
	for (auto& obj : *m_Objects)
	{
		DT_RemoveObject(m_Scene->scene, obj.object);
		DT_DestroyObject(obj.object);
		DT_DeleteShape(obj.shape);
		DT_DeleteVertexBase(obj.vertexBase);
	}

	DT_DestroyRespTable(m_Scene->respTable);
	DT_DestroyScene(m_Scene->scene);
}

unsigned int ConstructComplexShape(const void* vertex, const unsigned int* indices, unsigned int count)
{
	if (!m_Scene)
		InitCollisionDetection();

	m_Objects->emplace_back();
	auto& object = m_Objects->back();

	object.vertexBase = DT_NewVertexBase(vertex, 0);
	object.shape = DT_NewComplexShape(object.vertexBase);

	for (auto i = 0u; i < count; i += 3)
	{
		DT_Begin();
		if (indices != nullptr)
		{
			DT_VertexIndex(indices[i + 0]);
			DT_VertexIndex(indices[i + 1]);
			DT_VertexIndex(indices[i + 2]);
		}
		else
		{
			DT_VertexIndex(i + 0);
			DT_VertexIndex(i + 1);
			DT_VertexIndex(i + 2);
		}
		DT_End();
	}
	DT_EndComplexShape();

	object.object = DT_CreateObject(&object, object.shape);

	DT_AddObject(m_Scene->scene, object.object);
	DT_SetResponseClass(m_Scene->respTable, object.object, m_Scene->objectClass);

	DT_SetMargin(object.object, 0.1f);

	return (unsigned int)m_Objects->size() - 1u;
}

unsigned int ConstructRigidBody(ShapeType shapeType, const float* pos, const float radius, const float height)
{
	m_Objects->emplace_back();
	auto& object = m_Objects->back();

	switch (shapeType)
	{
		case ShapeType::CUBE:
			object.shape = DT_NewBox(radius, radius, radius);
			break;
		case ShapeType::SPHERE:
			object.shape = DT_NewSphere(radius);
			break;
		case ShapeType::CYLINDER:
			object.shape = DT_NewCylinder(radius, height);
			break;

	}

	object.object = DT_CreateObject(&object, object.shape);

	DT_AddObject(m_Scene->scene, object.object);
	DT_SetResponseClass(m_Scene->respTable, object.object, m_Scene->objectClass);

	DT_SetPosition(object.object, pos);

	DT_SetMargin(object.object, 0.1f);

	return (unsigned int)m_Objects->size() - 1u;
}

void UpdateComplexShape(unsigned int id, const void* vertex)
{
	auto& object = m_Objects->at(id);
	DT_ChangeVertexBase(object.vertexBase, vertex);
}

void UpdateObjectPosition(unsigned int id, const float* pos)
{
	auto& object = m_Objects->at(id);
	DT_SetPosition(object.object, pos);
}

void UpdateObjectRotation(unsigned int id, const float* rot)
{
	auto& object = m_Objects->at(id);
	DT_SetOrientation(object.object, rot);
}

void UpdateObjectScale(unsigned int id, const float* scale)
{
	auto& object = m_Objects->at(id);
	DT_SetScaling(object.object, scale);
}

void UpdateObjectModelMatrix(unsigned int id, const float* mat)
{
	auto& object = m_Objects->at(id);
	DT_SetMatrixf(object.object, mat);
}