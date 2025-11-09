TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle

QT -= gui

TARGET = transition_cli

SOURCES += main.cpp \
           video_producer.cpp \
           transitions.cpp

HEADERS += video_producer.h \
           transitions.h

# Chemins d'include (adapter selon ton install)
INCLUDEPATH += /usr/include/opencv4

# nlohmann/json est souvent dans /usr/include ou /usr/include/nlohmann
#INCLUDEPATH += /usr/include/nlohmann

# Lib OpenCV – à adapter à ta version
LIBS += -lopencv_core -lopencv_imgcodecs -lopencv_imgproc -lopencv_videoio -lopencv_highgui

