#include <stdio.h>
#include <fstream> //alex
#include <SOLID/solid.h>

#include <assert.h>
#include <vector>
#include <chrono>

#include <3D/Point.h>
#include <3D/Quaternion.h>
#include <vector.h>

#define SPACE_SIZE 5
#define NUM_ITER 10000

using PointList = std::vector<Point>;

typedef struct MyObject {
	int id;
} MyObject;

/* ARGSUSED */
void collide1(void* client_data, DtObjectRef obj1, DtObjectRef obj2,
	const DtCollData* coll_data) {
}

/* ARGSUSED */
void collide2(void* client_data, DtObjectRef obj1, DtObjectRef obj2,
	const DtCollData* coll_data) {
	FILE* stream = (FILE*)client_data;
	fprintf(stream, "Object %d interferes with object %d\n",
		(*(MyObject*)obj1).id, (*(MyObject*)obj2).id);
}

int main(int argc, char* argv[]) {
	MyObject object1, object2;

	object1.id = 1;
	object2.id = 2;

	// create a clock
	auto start = std::chrono::high_resolution_clock::now();

	DtShapeRef shape = dtNewComplexShape();

	ifstream arg_s("teapot.vrt");

	if (!arg_s) { cout << "?????" << endl; return -1; }

	char ch;
	arg_s >> ch;
	assert(ch == '[');

	PointList points;  //DtVector points[200000];
	Point point;

	do
	{
		arg_s >> point[0] >> point[1] >> point[2] >> ch;
		points.push_back(point);
	} while (ch == ',');
	assert(ch == ']');
	dtVertexBase(&points[0]);

	arg_s >> ch;
	assert(ch == '[');

	do
	{
		int index;
		dtBegin(DT_POLYGON);
		do
		{
			arg_s >> index >> ch;
			if (index >= 0) 
				dtVertexIndex(index);
		} while (index >= 0);
		dtEnd();
	} while (ch == ',');
	assert(ch == ']');

	dtEndComplexShape();
	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	std::cout << "Time to load teapot: " << duration << "ms" << std::endl;

	fprintf(stderr, "done.\n");

	dtCreateObject(&object1, shape);
	dtCreateObject(&object2, shape);

	dtDisableCaching();

	dtSetDefaultResponse(collide1, DT_SIMPLE_RESPONSE, stdout);

	int col_count = 0;
	Quaternion q;

	printf("Running %d tests at random placements\n", NUM_ITER);
	printf("in a space of size %d...\n", SPACE_SIZE);
	for (int i = 0; i != NUM_ITER; ++i) {
		dtSelectObject(&object1);
		dtLoadIdentity();
		dtTranslate(rnd() * SPACE_SIZE, rnd() * SPACE_SIZE, rnd() * SPACE_SIZE);
		q = Quaternion::random();
		dtRotate(q[X], q[Y], q[Z], q[W]);

		dtSelectObject(&object2);
		dtLoadIdentity();
		dtTranslate(rnd() * SPACE_SIZE, rnd() * SPACE_SIZE, rnd() * SPACE_SIZE);
		q = Quaternion::random();
		dtRotate(q[X], q[Y], q[Z], q[W]);

		if (dtTest()) ++col_count;
	}
	printf("done\n");

	cout << "Number of collisions: " << col_count << endl;

	dtDeleteObject(&object1);
	dtDeleteObject(&object2);
	dtDeleteShape(shape);

	std::cout << "Press enter to exit" << std::endl;
	std::cin.get();
	return 0;
}


