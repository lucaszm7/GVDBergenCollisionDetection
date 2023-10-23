/*
  SOLID - Software Library for Interference Detection
  Copyright (C) 1997-1998  Gino van den Bergen

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

  Please send remarks, questions and bug reports to gino@win.tue.nl,
  or write to:
                  Gino van den Bergen
		  Department of Mathematics and Computing Science
		  Eindhoven University of Technology
		  P.O. Box 513, 5600 MB Eindhoven, The Netherlands
*/

#include <SOLID/solid.h>

#include <algorithm>

#include "Box.h"
#include "Cone.h"
#include "Cylinder.h"
#include "Sphere.h"
#include "Complex.h"
#include "Encounter.h"
#include "Object.h"
#include "Simplex.h"
#include "Polygon.h"
#include "Polyhedron.h"
#include "Response.h"
#include "RespTable.h"

#include <map>
#include <set>
#include <vector>

#include <stdio.h>
// this is for building DLL under win32
#ifdef WIN32
#define EXPORT __declspec(dllexport)
class _IO_ostream_withassign cerr;
class ostream & endl(ostream &) {}
ostream & ostream::operator<<(char const *) {}
#else
#define EXPORT
#endif

typedef vector<Point> PointBuf;
typedef vector<unsigned int> IndexBuf;
typedef vector<const Polytope *> PolyList;
typedef vector<Complex *> ComplexList;
typedef map<DtObjectRef, Object *> ObjectList;
typedef set<Encounter> ProxList;

PointBuf pointBuf;
IndexBuf indexBuf;
PolyList polyList; 
ComplexList complexList;
ObjectList objectList;
RespTable respTable;
ProxList proxList;

DtPolyType currentType;
Complex *currentComplex = NULL;
Object *currentObject = NULL;
bool caching = true;

extern Scalar rel_error;

inline void move() {
  if (caching && currentObject) currentObject->move();
}

EXPORT DtShapeRef dtBox(DtScalar x,DtScalar y,DtScalar z) {
  return new Box(x,y,z);
}

EXPORT DtShapeRef dtCone(DtScalar radius,DtScalar height) {
  return new Cone(radius,height);
}

EXPORT DtShapeRef dtCylinder(DtScalar radius,DtScalar height) {
  return new Cylinder(radius, height);
}

EXPORT DtShapeRef dtSphere(DtScalar radius) {
  return new Sphere(radius);
}

EXPORT DtShapeRef dtNewComplexShape() {
  if (!currentComplex) currentComplex = new Complex;
  return currentComplex;
}

EXPORT void dtEndComplexShape() {
  if (currentComplex->getBase().getPointer() == 0) {
    Point *ptr = new Point[pointBuf.size()];
    copy(pointBuf.begin(),pointBuf.end(),ptr);
    currentComplex->setBase(ptr,true);
    pointBuf.erase(pointBuf.begin(),pointBuf.end());
  }
  currentComplex->finish(polyList.size(),&polyList[0]);
  polyList.erase(polyList.begin(), polyList.end());
  complexList.push_back(currentComplex);
  currentComplex = 0;
}

EXPORT void dtBegin(DtPolyType type) {
  currentType = type;
}

EXPORT void dtEnd() {
  dtVertexIndices(currentType,indexBuf.size(),&indexBuf[0]);
  indexBuf.erase(indexBuf.begin(),indexBuf.end());
}

EXPORT void dtVertex(DtScalar x,DtScalar y,DtScalar z) {
  Point p(x,y,z);
  int i = pointBuf.size()-20;
  if (i < 0) i = 0;
  while (i < (int)pointBuf.size() && !(pointBuf[i] == p)) ++i;
  if (i == (int)pointBuf.size()) pointBuf.push_back(p);
  indexBuf.push_back(i);
}

EXPORT void dtVertexBase(const void *base) {
  currentComplex->setBase(base); 
}

EXPORT void dtVertexIndex(DtIndex idx) {
  indexBuf.push_back(idx);
}

EXPORT void dtVertexIndices(DtPolyType type,DtCount cnt,const DtIndex *indices) {
  if (currentComplex) {
    const Polytope *poly;
    switch (type) {
    case DT_SIMPLEX:
      poly = new Simplex(currentComplex->getBase(), cnt, indices);
      break;
    case DT_POLYGON:
      poly = new Polygon(currentComplex->getBase(), cnt, indices);
      break;
    case DT_POLYHEDRON:
      if (currentComplex->getBase().getPointer()==0) {
	currentComplex->setBase(&pointBuf[0]);
	poly = new Polyhedron(currentComplex->getBase(),cnt,indices);
	currentComplex->setBase(0);
      }
      else poly = new Polyhedron(currentComplex->getBase(),cnt,indices);
      break;
    }
    polyList.push_back(poly);
  }
}

EXPORT void dtVertexRange(DtPolyType type, DtIndex first, DtCount cnt) {
  DtIndex *indices = new DtIndex[cnt];
  for (unsigned int i = 0; i < cnt;++i) indices[i] = first + i;
  dtVertexIndices(type,cnt,indices);
  delete [] indices;
}

EXPORT void dtDeleteShape(DtShapeRef shape) { 
  if (((Shape *)shape)->getType() == COMPLEX) {
    ComplexList::iterator i = 
      find(complexList.begin(), complexList.end(), (Complex *)shape);
    if (i != complexList.end()) complexList.erase(i);
  }
  delete (Shape *)shape; 
}

EXPORT void dtChangeVertexBase(DtShapeRef shape, const void *base) {
  if (((Shape *)shape)->getType() == COMPLEX)
    ((Complex *)shape)->changeBase(base);
  for (ObjectList::const_iterator i = objectList.begin(); 
       i != objectList.end(); ++i) {
    if ((*i).second->shapePtr == (Shape *)shape) {
      (*i).second->move();
    }
  }   
}


// Object instantiation

EXPORT void dtCreateObject(DtObjectRef object, DtShapeRef shape) {
  move();
  currentObject = objectList[object] = new Object(object,(Shape *)shape);
}

EXPORT void dtSelectObject(DtObjectRef object) {
  ObjectList::iterator i = objectList.find(object);
  if (i != objectList.end()) {
    move();
    currentObject = (*i).second;
  }
}

EXPORT void dtDeleteObject(DtObjectRef object) {
  ObjectList::iterator i = objectList.find(object);
  if (i != objectList.end()) { // found
    // Olivier Michel added this patch on 4-JUL-2001
    // cleanup of the proxList
    // The crash when deleting objects in Caching mode appears less often
    // although it is still there. Grrr...
    if (caching) {
      for(ProxList::iterator j=proxList.begin();j!=proxList.end();++j)
       if (((Encounter &)*j).obj1==(*i).second) proxList.erase(j);
       else if (((Encounter &)*j).obj2==(*i).second) proxList.erase(j);
    }
    // end of patch 4-JUL-2001
    if (currentObject == (*i).second) currentObject = NULL;
    delete (*i).second;
    objectList.erase(i);
  }
  else cout << "not found !" << endl;
  respTable.cleanObject(object);
}

EXPORT void dtTranslate(DtScalar x, DtScalar y, DtScalar z) {
  if (currentObject) currentObject->translate(Vector(x, y, z));
}

EXPORT void dtRotate(DtScalar x, DtScalar y, DtScalar z, DtScalar w) {
  if (currentObject) currentObject->rotate(Quaternion(x, y, z, w));
}

EXPORT void dtScale(DtScalar x, DtScalar y, DtScalar z) {
  if (currentObject) currentObject->scale(x, y, z);
}

EXPORT void dtLoadIdentity() { 
  if (currentObject) currentObject->setIdentity();
}

EXPORT void dtLoadMatrixf(const float *m) { 
  if (currentObject) currentObject->setMatrix(m);
}

EXPORT void dtLoadMatrixd(const double *m) { 
  if (currentObject) currentObject->setMatrix(m);
}

EXPORT void dtMultMatrixf(const float *m) { 
  if (currentObject) currentObject->multMatrix(m);
}

EXPORT void dtMultMatrixd(const double *m) { 
  if (currentObject) currentObject->multMatrix(m);
}

// Response

EXPORT void dtSetDefaultResponse(DtResponse response, DtResponseType type,
				  void *client_data) {
  respTable.setDefault(Response(response, type, client_data));
}

EXPORT void dtClearDefaultResponse() {
  respTable.setDefault(Response());
}

EXPORT void dtSetObjectResponse(DtObjectRef object, DtResponse response, 
				 DtResponseType type, void *client_data) {
  respTable.setSingle(object, Response(response, type, client_data));
}

EXPORT void dtClearObjectResponse(DtObjectRef object) {
  respTable.setSingle(object, Response());
}

EXPORT void dtResetObjectResponse(DtObjectRef object) {
  respTable.resetSingle(object);
}

EXPORT void dtSetPairResponse(DtObjectRef object1, DtObjectRef object2, 
			       DtResponse response, DtResponseType type, 
			       void * client_data) {
  respTable.setPair(object1, object2, Response(response, type, client_data));
}

EXPORT void dtClearPairResponse(DtObjectRef object1, DtObjectRef object2) {
  respTable.setPair(object1, object2, Response());
}

EXPORT void dtResetPairResponse(DtObjectRef object1, DtObjectRef object2) {
  respTable.resetPair(object1, object2);
}

// Runtime

EXPORT void dtProceed() {
  for (ComplexList::iterator i = complexList.begin(); 
       i != complexList.end(); ++i) 
    (*i)->proceed();
  for (ObjectList::const_iterator j = objectList.begin(); 
       j != objectList.end(); ++j)
    (*j).second->proceed();
}

EXPORT void dtEnableCaching() {
  for (ObjectList::const_iterator i=objectList.begin();i!=objectList.end();++i)
    (*i).second->move();
  caching = true;
}

EXPORT void dtDisableCaching() { caching = false; }

EXPORT void dtSetTolerance(DtScalar tol) { rel_error = tol; }

EXPORT void addPair(ObjectPtr object1, ObjectPtr object2) {
  proxList.insert(Encounter(object1,object2));
}

EXPORT void removePair(ObjectPtr object1, ObjectPtr object2) {
  proxList.erase(Encounter(object1,object2));
}

bool object_test(Encounter& e) {
  static Point p1, p2;
  const Response& resp = respTable.find(e.obj1->ref, e.obj2->ref);
  switch (resp.type) {
  case DT_SIMPLE_RESPONSE:
    if (intersect(*e.obj1, *e.obj2, e.sep_axis)) { 
      resp(e.obj1->ref, e.obj2->ref);
      return true; 
    }
    break;
  case DT_SMART_RESPONSE:
    if (prev_closest_points(*e.obj1, *e.obj2, e.sep_axis, p1, p2)) {
      Vector v = e.obj1->prev(p1) - e.obj2->prev(p2);
      resp(e.obj1->ref, e.obj2->ref, p1, p2, v);
      return true; 
    }
    break;
  case DT_WITNESSED_RESPONSE:
    if (common_point(*e.obj1, *e.obj2, e.sep_axis, p1, p2)) { 
      resp(e.obj1->ref, e.obj2->ref, p1, p2, Vector(0, 0, 0));
      return true; 
    }
    break;
  default:
    return false;
  }
  return false;
}

EXPORT DtCount dtTest() {
  move();
  DtCount cnt = 0;
  if (caching) {
    for (ProxList::iterator i = proxList.begin(); i != proxList.end(); ++i) 
      if (object_test((Encounter &)*i)) ++cnt;
  }
  else {
    int c=0;
    for (ObjectList::const_iterator j = objectList.begin();
	 j != objectList.end(); ++j)
      for (ObjectList::const_iterator i = objectList.begin();
	   i != j; ++i) {
	Encounter e((*i).second, (*j).second);
	c++;
	if (object_test(e)) ++cnt;
      } 
  }
  return cnt;
}

EXPORT void dtTestObjects(DtObjectRef object1, DtObjectRef object2) {
  // Programmed by Alok Menghrajani.
  // alok.menghrajani@epfl.ch
  static Object *obj1, *obj2;
  static Object *o1, *o2;
  static Vector sep_axis;
  static Point p1, p2;

  o1=(*objectList.find(object1)).second;
  o2=(*objectList.find(object2)).second;

  //  Encounter e(o1, o2);
  //  object_test(e);

  if (o2->shapePtr->getType() < o1->shapePtr->getType() ||
      (o2->shapePtr->getType() == o1->shapePtr->getType() && o2 < o1)) {
    obj1 = o2;
    obj2 = o1;
  }
  else {
    obj1 = o1;
    obj2 = o2;
  }
  sep_axis.setValue(0,0,0);

  const Response& resp = respTable.find(obj1->ref, obj2->ref);
  switch (resp.type) {
  case DT_SIMPLE_RESPONSE:
    if (intersect(*obj1, *obj2, sep_axis))
      resp(obj1->ref, obj2->ref);
    break;
  case DT_SMART_RESPONSE:
    if (prev_closest_points(*obj1, *obj2, sep_axis, p1, p2)) {
      Vector v = obj1->prev(p1) - obj2->prev(p2);
      resp(obj1->ref, obj2->ref, p1, p2, v);
    }
    break;
  case DT_WITNESSED_RESPONSE:
    if (common_point(*obj1, *obj2, sep_axis, p1, p2))
      resp(obj1->ref, obj2->ref, p1, p2, Vector(0, 0, 0));
    break;
  }
}
