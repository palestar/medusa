# Makefile - DisplayD3D.dsp

ifndef CFG
CFG=DisplayD3D - Win32 Debug
endif
CC=gcc
CFLAGS=
CXX=g++
CXXFLAGS=$(CFLAGS)
RC=windres -O COFF
ifeq "$(CFG)"  "DisplayD3D - Win32 Release"
CFLAGS+=-W -g -O2 -I../ -I../Core/DX9/Include -DNDEBUG -D_WINDOWS -DWIN32 -mcpu=pentium -D_M_IX86=500
LD=dllwrap
LDFLAGS=
LDFLAGS+=-Map "../Bin/DisplayD3D.map" -L../Bin -Wl,--subsystem,windows
LIBS+=-lole32 -luser32
else
ifeq "$(CFG)"  "DisplayD3D - Win32 Debug"
CFLAGS+=-W -g -O0 -I../ -I../Core/DX9/Include -D_DEBUG -D_WINDOWS -DWIN32 -mcpu=pentium -D_M_IX86=500
LD=dllwrap
LDFLAGS=
TARGET=../Bin/DisplayD3DD.dll
LDFLAGS+=-L../Bin -Wl,--subsystem,windows
LIBS+=-lole32 -luser32
endif
endif

ifndef TARGET
TARGET=DisplayD3D.dll
endif

.PHONY: all
all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $<

%.o: %.cxx
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ -c $<

%.res: %.rc
	$(RC) $(CPPFLAGS) -o $@ -i $<

SOURCE= \
	DisplayDeviceD3D.cpp \
	PrimitiveFactory.cpp \
	PrimitiveLineListD3D.cpp \
	PrimitiveLineStripD3D.cpp \
	PrimitiveMaterialD3D.cpp \
	PrimitiveSetTransformD3D.cpp \
	PrimitiveSurfaceD3D.cpp \
	PrimitiveTriangleFanD3D.cpp \
	PrimitiveTriangleListD3D.cpp \
	PrimitiveTriangleStripD3D.cpp \
	PrimitiveWindowD3D.cpp

HEADERS= \
	DisplayDeviceD3D.h \
	PrimitiveFactory.h \
	PrimitiveLineListD3D.h \
	PrimitiveLineStripD3D.h \
	PrimitiveMaterialD3D.h \
	PrimitiveSetTransformD3D.h \
	PrimitiveSurfaceD3D.h \
	PrimitiveTriangleFanD3D.h \
	PrimitiveTriangleListD3D.h \
	PrimitiveTriangleStripD3D.h \
	PrimitiveWindowD3D.h

SRCS=$(SOURCE) $(HEADERS)  \
	../Bin/DisplayD3D.map \
	Debug/DisplayD3DD.map \
	../Core/DX9/Lib/dxguid.lib \
	../Core/DX9/Lib/d3dx8.lib \
	../Core/DX9/Lib/d3d8.lib

OBJS=$(patsubst %.rc,%.res,$(patsubst %.cxx,%.o,$(patsubst %.cpp,%.o,$(patsubst %.cc,%.o,$(patsubst %.c,%.o,$(filter %.c %.cc %.cpp %.cxx %.rc,$(SRCS)))))))

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

.PHONY: clean
clean:
	-rm -f $(OBJS) $(TARGET) DisplayD3D.dep

.PHONY: depends
depends:
	-$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MM $(filter %.c %.cc %.cpp %.cxx,$(SRCS)) > DisplayD3D.dep

-include DisplayD3D.dep

