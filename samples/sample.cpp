#include <stdio.h>
#include <numbers>
#include <iostream>

#define USE_QUADS

#include "MT_Point3.h"
#include "MT_Vector3.h"
#include "MT_Quaternion.h"
#include <chrono>	
#include "Plugin.h"

#define SPACE_SIZE 20
#define NUM_ITER 10000

void CreateTorusVertices(std::vector<float>& vertices);

void main() {
	auto start = std::chrono::high_resolution_clock::now();

	std::vector<float> vertices;
	vertices.reserve(1000000);

	CreateTorusVertices(vertices);
	InitCollisionDetection();
	ConstructColonComplexShape(vertices.data(), vertices.size());
	ConstructEndoComplexShape(vertices.data(), vertices.size());

	DT_ObjectHandle object1 = GetColonObject();
	DT_ObjectHandle object2 = GetEndoObject();

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
	std::cout << "Time to build: " << duration.count() << "ms" << std::endl;

	int col_count = 0;
	DT_Quaternion q;

	printf("Running %d tests at random placements\n", NUM_ITER);
	printf("in a space of size %d...\n", SPACE_SIZE);

	for (int k = 0; k != NUM_ITER; ++k) 
	{
		MT_Point3 p(MT_random() * SPACE_SIZE, MT_random() * SPACE_SIZE, MT_random() * SPACE_SIZE);
		DT_SetPosition(object1, p);
		DT_SetOrientation(object1, MT_Quaternion::random());

		MT_Point3 pp(MT_random() * SPACE_SIZE, MT_random() * SPACE_SIZE, MT_random() * SPACE_SIZE);
		DT_SetPosition(object2, pp);
		DT_SetOrientation(object2, MT_Quaternion::random());

        col_count += CollisionCheck();
    }
    printf("done\n");

	std::cout << "Number of collisions: " << col_count << std::endl;

	Dispose();

	std::cout << "Press enter to continue...";
	std::cin.get();

	return;
}

void CreateTorusVertices(std::vector<float>& vertices)
{
	double a = 10;
	double b = 2;

	fprintf(stderr, "Loading a torus with a major radius of %d and a minor radius of %d \n", (int)a, (int)b);

	const int n1 = 50;
	const int n2 = 50;

	for (int uc = 0; uc < n1; uc++)
	{
		for (int vc = 0; vc < n2; vc++) {
			double u1 = (2.0 * std::numbers::pi * uc) / n1;
			double u2 = (2.0 * std::numbers::pi * (uc + 1)) / n1;
			double v1 = (2.0 * std::numbers::pi * vc) / n2;
			double v2 = (2.0 * std::numbers::pi * (vc + 1)) / n2;

			double p1[3], p2[3], p3[3], p4[3];

			p1[0] = (a - b * cos(v1)) * cos(u1);
			p2[0] = (a - b * cos(v1)) * cos(u2);
			p3[0] = (a - b * cos(v2)) * cos(u1);
			p4[0] = (a - b * cos(v2)) * cos(u2);
			p1[1] = (a - b * cos(v1)) * sin(u1);
			p2[1] = (a - b * cos(v1)) * sin(u2);
			p3[1] = (a - b * cos(v2)) * sin(u1);
			p4[1] = (a - b * cos(v2)) * sin(u2);
			p1[2] = b * sin(v1);
			p2[2] = b * sin(v1);
			p3[2] = b * sin(v2);
			p4[2] = b * sin(v2);

			vertices.push_back(p1[0]);
			vertices.push_back(p1[1]);
			vertices.push_back(p1[2]);

			vertices.push_back(p2[0]);
			vertices.push_back(p2[1]);
			vertices.push_back(p2[2]);

			vertices.push_back(p3[0]);
			vertices.push_back(p3[1]);
			vertices.push_back(p3[2]);


			vertices.push_back(p4[0]);
			vertices.push_back(p4[1]);
			vertices.push_back(p4[2]);

			vertices.push_back(p1[0]);
			vertices.push_back(p1[1]);
			vertices.push_back(p1[2]);

			vertices.push_back(p2[0]);
			vertices.push_back(p2[1]);
			vertices.push_back(p2[2]);
		}
	}
}
