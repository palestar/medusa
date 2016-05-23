# Makefile - AudioDS.dsp

ifndef CFG
CFG=AudioDS - Win32 Debug
endif
CC=gcc
CFLAGS=
CXX=g++
CXXFLAGS=$(CFLAGS)
RC=windres -O COFF
ifeq "$(CFG)"  "AudioDS - Win32 Release"
CFLAGS+=-W -fexceptions -g -O2 -I../ -I../Core/DX9/Include -DNDEBUG -D_WINDOWS -DWIN32 -mcpu=pentium -D_M_IX86=500
LD=dllwrap
LDFLAGS=
LDFLAGS+=-Map "../Bin/AudioDS.map" -L../Bin -Wl,--subsystem,windows
LIBS+=-lmsacm32 -lole32
else
ifeq "$(CFG)"  "AudioDS - Win32 Debug"
CFLAGS+=-W -fexceptions -g -O0 -I../ -I../Core/DX9/Include -D_DEBUG -D_WINDOWS -DWIN32 -mcpu=pentium -D_M_IX86=500
LD=dllwrap
LDFLAGS=
TARGET=../Bin/AudioDSD.dll
LDFLAGS+=-L../Bin -Wl,--subsystem,windows
LIBS+=-lmsacm32 -lole32
endif
endif

ifndef TARGET
TARGET=AudioDS.dll
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
	AudioBufferDS.cpp \
	AudioCodecSpeech.cpp \
	AudioDeviceDS.cpp \
	JukeboxDS.cpp

HEADER= \
	AudioBufferDS.h \
	AudioCodecSpeech.h \
	AudioDeviceDS.h \
	JukeboxDS.h

SRCS=$(SOURCE) $(HEADER)  \
	Release/AudioDS.map \
	../Core/DX9/Lib/strmiids.lib \
	../Core/DX9/Lib/dxguid.lib \
	../Core/DX9/Lib/dsound.lib

OBJS=$(patsubst %.rc,%.res,$(patsubst %.cxx,%.o,$(patsubst %.cpp,%.o,$(patsubst %.cc,%.o,$(patsubst %.c,%.o,$(filter %.c %.cc %.cpp %.cxx %.rc,$(SRCS)))))))

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

.PHONY: clean
clean:
	-rm -f $(OBJS) $(TARGET) AudioDS.dep

.PHONY: depends
depends:
	-$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MM $(filter %.c %.cc %.cpp %.cxx,$(SRCS)) > AudioDS.dep

-include AudioDS.dep

