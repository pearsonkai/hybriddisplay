CXX = g++
RC  = windres
EMXX = em++

CXXFLAGS = -std=c++26 -Wall -Iinclude -Iinclude/SDL3 -O0
WASMFLAGS = -std=c++26 -Wall -Iinclude -Iinclude/SDL3 -O3 -fexceptions -sASSERTIONS=1 -sUSE_SDL=3 -sALLOW_MEMORY_GROWTH=1
WASM_ASSET_FLAGS = --preload-file media/hds_splashscreen_grey.png@/media/hds_splashscreen_grey.png --preload-file polywag@/polywag --preload-file cornell_box@/cornell_box --preload-file skybox.png@/skybox.png
WASM_HTML_SHELL = web/shell.html

SRC := $(shell find src -name '*.cpp')
WASM_SRC := $(filter-out src/Pool.cpp,$(SRC))
WASM_ASSETS := media/hds_splashscreen_grey.png polywag $(shell find polywag -type f) cornell_box $(shell find cornell_box -type f) skybox.png
OBJ := $(SRC:.cpp=.o)

OUT = hybriddisplay.exe
WASM_OUT = hybriddisplay.html
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
	$(CXX) $(CXXFLAGS) -Ofast -ffast-math  $(SRC) $(RESOURCE_OBJ) $(SDL_LIB_PATH) $(SDL_LIBS) -mwindows -o $(OUT)

release_html: $(WASM_OUT)

$(WASM_OUT): $(WASM_SRC) $(WASM_ASSETS) $(WASM_HTML_SHELL) makefile
	$(EMXX) $(WASMFLAGS) $(WASM_ASSET_FLAGS) --shell-file $(WASM_HTML_SHELL) $(WASM_SRC) -o $@

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

.PHONY: all debug compare release release_html library clean