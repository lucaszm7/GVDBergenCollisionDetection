SOURCES = Transform.cpp Convex.cpp Box.cpp Cone.cpp Cylinder.cpp Sphere.cpp \
	Simplex.cpp Polygon.cpp Polyhedron.cpp Complex.cpp \
	BBoxTree.cpp Endpoint.cpp Object.cpp Response.cpp RespTable.cpp \
	C-api.cpp
OBJECTS = $(SOURCES:.cpp=.o)

%.o:%.cpp
	$(CXX) -c $(CFLAGS) -DQHULL -I../include -I../include/qhull $(INCLUDE) $<

default:
	@echo "Specify a target configuration"

clean:
	@$(RM) *.o *~

include ../Make-config

targets: $(TARGETS)

../lib/solid.dll ../lib/libsolid.a: $(OBJECTS)
	gcc -mdll -mwindows -o junk.tmp -Wl,--base-file,base.tmp $(OBJECTS)
	del junk.tmp
	dlltool --dllname solid.dll --base-file base.tmp --output-exp temp.exp --def ../lib/solid.def
	del base.tmp
	gcc -mdll -mwindows -o ../lib/solid.dll $(OBJECTS) -Wl,temp.exp
	del temp.exp
	dlltool --dllname solid.dll --def ../lib/solid.def --output-lib ../lib/libsolid.a

../lib/libsolid.so.2.1.0: $(OBJECTS)
	gcc -shared -Wl,-soname,libsolid.so -o $@ $(OBJECTS) libqhull.a

realclean: clean
	@$(RM) ../lib/*


