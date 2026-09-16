CXX = g++
RC  = windres

CXXFLAGS = -std=c++17 -Wall -Iinclude -Iinclude/SDL3 -Iinclude/SDL_gpu -O0

SRC = \
	src/main.cpp \
	src/Vec3.cpp \
	src/Transform.cpp\
	src/Material.cpp \
	src/Mesh.cpp \
	src/World.cpp \
	src/Camera.cpp \
	src/Screen.cpp \
	src/Pool.cpp \
	src/Rendering/Renderer.cpp \
	src/Rendering/Wireframe.cpp

OUT = hybriddisplay.exe
OTHER = comparedisplay.exe

SDL_LIB_PATH = -Llib/SDL3 -Llib/SDL_gpu
SDL_LIBS = -lSDL3

RESOURCE_OBJ = resources/resources.o

all: $(OUT)

debug:
	$(CXX) $(CXXFLAGS) -O0 $(SRC) $(RESOURCE_OBJ) $(SDL_LIB_PATH) $(SDL_LIBS) -o $(OUT)

compare:
	$(CXX) $(CXXFLAGS) -O0 $(SRC) $(RESOURCE_OBJ) $(SDL_LIB_PATH) $(SDL_LIBS) -o $(OTHER)

releaseO0:
	$(CXX) $(CXXFLAGS) -O0 $(SRC) $(RESOURCE_OBJ) $(SDL_LIB_PATH) $(SDL_LIBS) -mwindows -o $(OUT)

releaseO2:
	$(CXX) $(CXXFLAGS) -O2 $(SRC) $(RESOURCE_OBJ) $(SDL_LIB_PATH) $(SDL_LIBS) -mwindows -o $(OUT)

$(OUT): $(SRC) $(RESOURCE_OBJ)
	$(CXX) $(CXXFLAGS) $(SRC) $(RESOURCE_OBJ) $(SDL_LIB_PATH) $(SDL_LIBS) -o $(OUT)

$(RESOURCE_OBJ): resources/resources.rc
	$(RC) resources/resources.rc -o $(RESOURCE_OBJ)

clean:
	rm -f $(OUT) $(RESOURCE_OBJ)