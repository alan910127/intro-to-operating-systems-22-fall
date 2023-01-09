QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += g++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    client.cpp

INCLUDEPATH += ../protos

HEADERS += \
    mainwindow.h \
    client.h

OBJECTS += \
    ../protos/tiny_ftp.grpc.pb.o \
    ../protos/tiny_ftp.pb.o

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

CXX = g++
CXXFLAGS += -std=c++11
CPPFLAGS += `pkg-config --cflags protobuf grpc`

LDFLAGS += -L/usr/local/lib `pkg-config --libs --static protobuf grpc++`\
           -pthread\
           -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed\
           -ldl

PROTOC = protoc
GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH = `which $$GRPC_CPP_PLUGIN`
CPPFLAGS += `pkg-config --cflags protobuf grpc`
PROTOS_PATH = ../protos

%.o.commands = $$CXX $$CXXFLAGS $$CPPFLAGS -c -o $@ $<
%.o.depends = %.cc

LIBS += $$LDFLAGS
QMAKE_CXXFLAGS += $$LDFLAGS $$CPPFLAGS