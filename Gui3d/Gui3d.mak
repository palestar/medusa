# Makefile - Gui3d.dsp

CC=gcc
CFLAGS=-g -fPIC -W -O2 -I../ -DNDEBUG
CXX=g++
CXXFLAGS=$(CFLAGS)
LD=gcc
LDFLAGS=-shared -W1,-soname,medusa.gui3d.so
LIBS=
TARGET=../Bin/medusa.gui3d.so

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

CFLAGS+=-fexceptions
SOURCE= \
	ButtonExit.cpp \
	ButtonPopup.cpp \
	ButtonSetScene.cpp \
	Document.cpp \
	EditPassword.cpp \
	InterfaceContext.cpp \
	LayoutCenter.cpp \
	LayoutLeft.cpp \
	LayoutRight.cpp \
	NodeConnector.cpp \
	NodeInterfaceClient.cpp \
	NodeWindow.cpp \
	WindowBar.cpp \
	WindowButton.cpp \
	WindowEdit.cpp \
	WindowHelp.cpp \
	WindowLayout.cpp \
	WindowList.cpp \
	WindowMenu.cpp \
	WindowSlider.cpp \
	WindowStyle.cpp \
	WindowText.cpp \
	WindowTip.cpp \
	WindowTree.cpp \
	WindowView.cpp

HEADERS= \
	ButtonExit.h \
	ButtonPopup.h \
	ButtonSetScene.h \
	Cursor.h \
	Document.h \
	EditPassword.h \
	GUI3DDll.h \
	InterfaceContext.h \
	LayoutCenter.h \
	LayoutLeft.h \
	LayoutRight.h \
	Messages.h \
	NodeConnector.h \
	NodeInterfaceClient.h \
	NodeWindow.h \
	WindowBar.h \
	WindowButton.h \
	WindowEdit.h \
	WindowHelp.h \
	WindowLayout.h \
	WindowList.h \
	WindowMenu.h \
	WindowSlider.h \
	WindowStyle.h \
	WindowText.h \
	WindowTip.h \
	WindowTree.h \
	WindowView.h

SRCS=$(SOURCE) $(HEADERS)  \
	Release/Gui3d.map

OBJS=$(patsubst %.rc,%.res,$(patsubst %.cxx,%.o,$(patsubst %.cpp,%.o,$(patsubst %.cc,%.o,$(patsubst %.c,%.o,$(filter %.c %.cc %.cpp %.cxx %.rc,$(SRCS)))))))

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

.PHONY: clean
clean:
	-rm -f $(OBJS) $(TARGET) GUI3D.dep

.PHONY: depends
depends:
	-$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MM $(filter %.c %.cc %.cpp %.cxx,$(SRCS)) > GUI3D.dep

-include GUI3D.dep

