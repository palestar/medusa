# Makefile - SystemWin.dsp

ifndef CFG
CFG=SystemWin - Win32 Debug
endif
CC=gcc
CFLAGS=
CXX=g++
CXXFLAGS=$(CFLAGS)
RC=windres -O COFF
ifeq "$(CFG)"  "SystemWin - Win32 Release"
CFLAGS+=-W -fexceptions -O2 -I../ -I../Core/DX9/Include -DNDEBUG -D_USRDLL -D_WINDOWS -DWIN32 -mcpu=pentium -D_M_IX86=500
LD=dllwrap
LDFLAGS=
LDFLAGS+=-Map "../Bin/SystemWin.map" -L../Bin
LIBS+=-luser32 -lgdi32
else
ifeq "$(CFG)"  "SystemWin - Win32 Debug"
CFLAGS+=-W -fexceptions -g -O0 -I../ -I../Core/DX9/Include -D_DEBUG -D_USRDLL -D_WINDOWS -DWIN32 -mcpu=pentium -D_M_IX86=500
LD=dllwrap
LDFLAGS=
TARGET=../Bin/SystemWinD.dll
LDFLAGS+=-L../Bin -Wl,--subsystem,windows
LIBS+=-luser32 -lgdi32
endif
endif

ifndef TARGET
TARGET=SystemWin.dll
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

SOURCE_FILES= \
	PlatformWin.cpp

HEADER_FILES= \
	PlatformWin.h \
	SystemWin.h

RESOURCE_FILES=

SRCS=$(SOURCE_FILES) $(HEADER_FILES) $(RESOURCE_FILES)  \
	../Core/DX9/Lib/dxguid.lib \
	../Core/DX9/Lib/dinput.lib

OBJS=$(patsubst %.rc,%.res,$(patsubst %.cxx,%.o,$(patsubst %.cpp,%.o,$(patsubst %.cc,%.o,$(patsubst %.c,%.o,$(filter %.c %.cc %.cpp %.cxx %.rc,$(SRCS)))))))

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

.PHONY: clean
clean:
	-rm -f $(OBJS) $(TARGET) SystemWin.dep

.PHONY: depends
depends:
	-$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MM $(filter %.c %.cc %.cpp %.cxx,$(SRCS)) > SystemWin.dep

-include SystemWin.dep

