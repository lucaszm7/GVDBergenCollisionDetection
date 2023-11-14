#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <vector>

#include "Plugin.h"

DT_Bool collResp(void* client_data, void* obj1, void* obj2, const DT_CollData* coll_data)
{
	const auto& myObj1 = *(PluginObject*)obj1;
	const auto& myObj2 = *(PluginObject*)obj2;
		
	m_CollResult->id1 = myObj1.id;
	m_CollResult->id2 = myObj2.id;

	m_CollResult->point1[0] = coll_data->point1[0];
	m_CollResult->point1[1] = coll_data->point1[1];
	m_CollResult->point1[2] = coll_data->point1[2];

	m_CollResult->point2[0] = coll_data->point2[0];
	m_CollResult->point2[1] = coll_data->point2[1];
	m_CollResult->point2[2] = coll_data->point2[2];

	m_CollResult->normal[0] = coll_data->normal[0];
	m_CollResult->normal[1] = coll_data->normal[1];
	m_CollResult->normal[2] = coll_data->normal[2];

	return DT_CONTINUE;
}

void InitCollisionDetection()
{
	m_Objects = std::make_shared<std::vector<PluginObject>>();
	m_Objects->reserve(100);
	m_CollResult = std::make_shared<PluginCollisionData>();

	m_Scene = std::make_shared<PluginScene>();
	m_Scene->scene = DT_CreateScene();

	m_Scene->respTable = DT_CreateRespTable();
	m_Scene->objectClass = DT_GenResponseClass(m_Scene->respTable);

	DT_AddPairResponse(m_Scene->respTable, m_Scene->objectClass, m_Scene->objectClass, collResp, DT_DEPTH_RESPONSE, nullptr);
}

unsigned int AllObjectsCollisionCheck()
{
	return DT_Test(m_Scene->scene, m_Scene->respTable);
}

void* GetCollisionPtr()
{
	return m_CollResult.get();
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

unsigned int ConstructComplexShape(const void* vertex, const unsigned int* indices, unsigned int vertexCount)
{
	if (!m_Scene)
		InitCollisionDetection();

	m_Objects->emplace_back();
	auto& object = m_Objects->back();

	object.vertexBase = DT_NewVertexBase(vertex, 0);
	object.shape = DT_NewComplexShape(object.vertexBase);
	object.id = (int)m_Objects->size() - 1;

	for (auto i = 0u; i < vertexCount; i += 3)
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

unsigned int ConstructRigidBody(unsigned int shapeType, const float* pos, const float radius, const float height)
{
	m_Objects->emplace_back();
	auto& object = m_Objects->back();
	object.id = (int)m_Objects->size() - 1;

	switch (shapeType)
	{
	case static_cast<unsigned int>(ShapeType::CUBE):
		object.shape = DT_NewBox(radius, radius, radius);
		break;
	case static_cast<unsigned int>(ShapeType::SPHERE):
		object.shape = DT_NewSphere(radius);
		break;
	case static_cast<unsigned int>(ShapeType::CYLINDER):
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
