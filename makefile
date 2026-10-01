CXX = g++
RC  = windres

CXXFLAGS = -std=c++26 -Wall -Iinclude -Iinclude/SDL3  -O2

SRC := $(shell find src -name '*.cpp')
OBJ := $(SRC:.cpp=.o)

OUT = hybriddisplay.exe
OTHER = comparedisplay.exe
LIB = lib/libhybriddisplay.a

SDL_LIB_PATH = -Llib/SDL3 
SDL_LIBS = -lSDL3

RESOURCE_OBJ = resources/resources.o

all: $(OUT)

debug:
	$(CXX) $(CXXFLAGS) -O0 $(SRC) $(RESOURCE_OBJ) $(SDL_LIB_PATH) $(SDL_LIBS) -o $(OUT)

compare:
	$(CXX) $(CXXFLAGS) $(SRC) $(RESOURCE_OBJ) $(SDL_LIB_PATH) $(SDL_LIBS) -o $(OTHER)

release:
	$(CXX) $(CXXFLAGS) -O2 $(SRC) $(RESOURCE_OBJ) $(SDL_LIB_PATH) $(SDL_LIBS) -mwindows -o $(OUT)

$(OUT): $(SRC) $(RESOURCE_OBJ)
	$(CXX) $(CXXFLAGS) $(SRC) $(RESOURCE_OBJ) $(SDL_LIB_PATH) $(SDL_LIBS) -o $(OUT)

library: $(LIB)

$(LIB): $(OBJ)
	ar rcs $(LIB) $(OBJ)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.INTERMEDIATE: $(OBJ)

$(RESOURCE_OBJ): resources/resources.rc
	$(RC) resources/resources.rc -o $(RESOURCE_OBJ)

clean:
	rm -f $(OUT) $(RESOURCE_OBJ)