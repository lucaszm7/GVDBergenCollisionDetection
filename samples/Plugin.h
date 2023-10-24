#pragma once
#pragma once

#include <vector>
#include <iostream>
#include <memory>
#include <Eigen/Core>

#include <SOLID.h>

#ifndef _WIN32
#define __stdcall 
#endif

#ifdef _WIN32
#define DLLExport __declspec(dllexport)
#else
#define DLLExport
#endif


extern "C"
{
	// Collision Detection
	DLLExport void InitCollisionDetection();
	DLLExport bool CollisionCheck();
	DLLExport void* GetCollisionResultPtr();

	// Colon
	DLLExport void ConstructColonComplexShape(const void* vertex, unsigned int count);
	DLLExport void UpdateColonComplexShape(const void* vertex);
	DLLExport DT_ObjectHandle GetColonObject();
	
	// Endo
	DLLExport void ConstructEndoComplexShape (const void* vertex, unsigned int count);
	DLLExport void UpdateEndoComplexShape (const void* vertex);
	DLLExport DT_ObjectHandle GetEndoObject();

	// Utility
	DLLExport void Dispose();
}

