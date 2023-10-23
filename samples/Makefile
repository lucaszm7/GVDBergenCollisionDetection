include ../Make-config

INCDIR = ../include
LIBDIR = ../lib

CPPFLAGS = -I$(INCDIR) 
LDFLAGS = -L$(LIBDIR)  
LDLIBS = -lsolid $(QHULL_LD) -lm

default: sample x_wing teapot

clean:
	-rm -f *.o gmon.out core

realclean: clean
	-rm -f sample x_wing teapot


