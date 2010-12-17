TEMPLATE = app
TARGET = ExtendMesh     # executable file name

DESTDIR = ./bin/Release
OBJECTS_DIR = ./Release
MOC_DIR = ./GeneratedFiles/Release

CONFIG(debug, debug|release){
    TARGET = ExtendMesh-Debug
    DESTDIR = ./bin/Debug
    OBJECTS_DIR = ./Debug
    MOC_DIR = ./GeneratedFiles/Debug
}

QT += opengl xml

INCLUDEPATH += ./GeneratedFiles \
    ./GraphicsLibrary \
    ./BezierSpline \
    ./Skeleton \
    ./Utility \
    ./TextureSynthesis \
    ./GeometrySynthesis \
    ./GUI \
    ./Solver/SparseLib++/iml \
    ./Solver/SparseLib++/include \
    .

include(ExtendMesh.pri)

win32{
    RC_FILE = ExtendMesh.rc
    LIBS += -llib/GLee \
    -llib/QGLViewer2 \
    -lopengl32 \
    -lglu32 \
    -lSolver/SparseLib++/lib/sparselib
}

CONFIG(debug, debug|release){
    QMAKE_CXXFLAGS+= -ggdb -g3 -O0 -fopenmp
    QMAKE_LFLAGS *= -fopenmp
    LIBS += -lGLEW -lGLU -lGL -lQGLViewer -lsparse
}

CONFIG(release, debug|release){
    QMAKE_CXXFLAGS += -fopenmp
    QMAKE_LFLAGS *= -fopenmp
    LIBS += -lGLEW -lGLU -lGL -lQGLViewer -lsparse
}
