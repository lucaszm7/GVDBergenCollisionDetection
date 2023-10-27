#pragma once
#pragma once

#include <vector>
#include <list>
#include <iostream>
#include <memory>
#include <Eigen/Core>

#include "SOLID.h"

struct PluginCollisionData
{
	float point1[3];
	float point2[3];
	float normal[3];
};

enum class ShapeType
{
	CUBE,
	SPHERE,
	CYLINDER
};

void InitCollisionDetection();
void Dispose();

unsigned int CollisionCheck();
void* GetCollisionResultPtr();
std::list<PluginCollisionData>* GetVectorCollisionResult();

unsigned int ConstructComplexShape(const void* vertex, const unsigned int* indices, unsigned int count);
unsigned int ConstructRigidBody(ShapeType shapeType, const float* pos, const float radius, const float height);
void UpdateComplexShape(unsigned int id, const void* vertex);
void UpdateObjectPosition(unsigned int id, const float* pos);
void UpdateObjectRotation(unsigned int id, const float* rot);
void UpdateObjectScale(unsigned int id, const float* scale);
void UpdateObjectModelMatrix(unsigned int id, const float* mat);


