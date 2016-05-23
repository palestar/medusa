# Makefile - MovieDS.dsp

ifndef CFG
CFG=MovieDS - Win32 Debug
endif
CC=gcc
CFLAGS=
CXX=g++
CXXFLAGS=$(CFLAGS)
RC=windres -O COFF
ifeq "$(CFG)"  "MovieDS - Win32 Release"
CFLAGS+=-W -g -O2 -I../ -DNDEBUG -D_USRDLL -D_WINDOWS -DWIN32 -mcpu=pentium -D_M_IX86=500
LD=dllwrap
LDFLAGS=
LDFLAGS+=-Map "../Bin/MovieDS.map" -L../Bin
LIBS+=-lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid
else
ifeq "$(CFG)"  "MovieDS - Win32 Debug"
CFLAGS+=-W -g -O0 -I../ -D_DEBUG -D_USRDLL -D_WINDOWS -DWIN32 -mcpu=pentium -D_M_IX86=500
LD=dllwrap
LDFLAGS=
TARGET=../Bin/MovieDSD.dll
LDFLAGS+=-L../Bin
LIBS+=-lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid
endif
endif

ifndef TARGET
TARGET=MovieDS.dll
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
	MovieDS.cpp

HEADER_FILES= \
	MovieDS.h \
	MovieDSDll.h

RESOURCE_FILES=

LIBS=

SRCS=$(SOURCE_FILES) $(HEADER_FILES) $(RESOURCE_FILES) $(LIBS)  \
	ddraw.lib \
	strmiids.lib

OBJS=$(patsubst %.rc,%.res,$(patsubst %.cxx,%.o,$(patsubst %.cpp,%.o,$(patsubst %.cc,%.o,$(patsubst %.c,%.o,$(filter %.c %.cc %.cpp %.cxx %.rc,$(SRCS)))))))

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

.PHONY: clean
clean:
	-rm -f $(OBJS) $(TARGET) MovieDS.dep

.PHONY: depends
depends:
	-$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MM $(filter %.c %.cc %.cpp %.cxx,$(SRCS)) > MovieDS.dep

-include MovieDS.dep

