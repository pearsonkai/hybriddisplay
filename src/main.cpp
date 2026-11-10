#include "Renderer.hpp"

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

using namespace hybriddisplay;

namespace {
constexpr graphics::Resolution resolution = {800, 600};
constexpr uint32_t averageFrameCount = 4;

#ifndef __EMSCRIPTEN__
std::unique_ptr<threading::Pool> pool;
#endif
std::unique_ptr<display::Screen> screen;
std::unique_ptr<rendering::Renderer> renderer;
rendering::Camera camera(90.0f, 0.01f, 10.0f);
std::shared_ptr<const graphics::Image<graphics::Colour>> skybox;
geometry::World world;
std::vector<display::Viewport> viewports;
bool keys[SDL_SCANCODE_COUNT] = {};
bool raytraceEnabled = false;
uint64_t previousTicks = 0;
uint64_t performanceFrequency = 0;
uint64_t previousPresent = 0;
uint64_t accumulatedFrameTime = 0;
uint32_t measuredFrames = 0;
float sensitivity = 0.2f;
constexpr float speed = 2.0f;

void createViewports() {
#ifdef __EMSCRIPTEN__
    const uint32_t viewportCount = 1;
#else
    const uint32_t viewportCount = pool->getNumThreads();
#endif
    std::pair<uint32_t, uint32_t> grid = {1, viewportCount};
    for (uint32_t rows = static_cast<uint32_t>(std::sqrt(viewportCount)); rows > 0; --rows) {
        if (viewportCount % rows == 0) {
            grid = {rows, viewportCount / rows};
            break;
        }
    }

    for (uint32_t x = 0; x < grid.first; ++x) {
        for (uint32_t y = 0; y < grid.second; ++y) {
            const float areaX = static_cast<float>(x) / grid.first;
            const float areaY = static_cast<float>(y) / grid.second;
            const float areaWidth = 1.0f / grid.first;
            const float areaHeight = 1.0f / grid.second;
            viewports.push_back(screen->tieViewport(
                graphics::Region{areaX, areaY, areaWidth, areaHeight},
                graphics::Region{0.0f, 0.0f, 1.0f, 1.0f}));
        }
    }
}

bool processEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT)
            return false;

        if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
            keys[event.key.scancode] = true;
            if (event.key.scancode == SDL_SCANCODE_ESCAPE)
                return false;
        }

        if (event.type == SDL_EVENT_MOUSE_MOTION)
            camera.rotate(-event.motion.yrel * sensitivity, -event.motion.xrel * sensitivity);
        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_RIGHT) {
            camera.setFov(25);
            sensitivity = 0.025f;
        }
        if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_RIGHT) {
            camera.setFov(90);
            sensitivity = 0.2f;
        }
        if (event.type == SDL_EVENT_KEY_UP)
            keys[event.key.scancode] = false;
    }
    return true;
}

void update() {
    const uint64_t currentTicks = SDL_GetTicks();
    const float deltaSeconds = static_cast<float>(currentTicks - previousTicks) / 1000.0f;
    previousTicks = currentTicks;
    if (keys[SDL_SCANCODE_UP])
        raytraceEnabled = true;
    if (keys[SDL_SCANCODE_DOWN])
        raytraceEnabled = false;

    if (keys[SDL_SCANCODE_W]) {
        camera.moveForward(speed * deltaSeconds);
        screen->setWindowType(SDL_WINDOW_BORDERLESS, true);
    }
    if (keys[SDL_SCANCODE_S])
        camera.moveForward(-speed * deltaSeconds);
    if (keys[SDL_SCANCODE_D])
        camera.moveRight(speed * deltaSeconds);
    if (keys[SDL_SCANCODE_A])
        camera.moveRight(-speed * deltaSeconds);

    const bool on = keys[SDL_SCANCODE_P];
    if (keys[SDL_SCANCODE_Y]) {
        camera.moveForward(speed);
        screen->setWindowType(SDL_WINDOW_BORDERLESS, on);
    }
    if (keys[SDL_SCANCODE_U])
        screen->setWindowType(SDL_WINDOW_FULLSCREEN, on);
    if (keys[SDL_SCANCODE_I])
        screen->setWindowType(SDL_WINDOW_RESIZABLE, on);
}

void render() {
    screen->clearFramebuffer();
    screen->clearZBuffer();
    renderer->skybox(viewports, camera, *skybox);
    if (raytraceEnabled)
        renderer->raytrace(viewports, camera, world);
    else
        renderer->rasterize(viewports, camera, world);
    screen->printBuffer();
}

void reportFrameTime() {
    const uint64_t currentPresent = SDL_GetPerformanceCounter();
    accumulatedFrameTime += currentPresent - previousPresent;
    previousPresent = currentPresent;
    ++measuredFrames;
    if (measuredFrames != averageFrameCount)
        return;

    const double averageMilliseconds =
        static_cast<double>(accumulatedFrameTime) * 1000.0 /
        (static_cast<double>(performanceFrequency) * measuredFrames);
    std::cout << "Average frame time: " << averageMilliseconds << " ms" << std::endl;
    accumulatedFrameTime = 0;
    measuredFrames = 0;
}

bool runFrame() {
    if (!processEvents())
        return false;
    update();
    render();
    reportFrameTime();
    return true;
}

void initializeApplication() {
#ifndef __EMSCRIPTEN__
    pool = std::make_unique<threading::Pool>(0);
    renderer = std::make_unique<rendering::Renderer>(pool.get());
#else
    renderer = std::make_unique<rendering::Renderer>(nullptr);
#endif
    screen = std::make_unique<display::Screen>(resolution, resolution);
    screen->fillFramebuffer(
        graphics::Image<graphics::Colour>::load(fs::path("media/hds_splashscreen_grey.png")),
        {0.0f, 0.0f, 1.0f, 1.0f});
    screen->printBuffer();
    skybox = graphics::Image<graphics::Colour>::load(fs::path("skybox.png"));

    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    createViewports();

    camera.goTo(math::Vec3(0.0f, 0.0f, 3.2f));
    camera.pointTowards(math::Vec3(0.0f, 0.0f, -0.2f));

    /*
    world.addModel(geometry::Mesh::load(fs::path("cornell_box/cornell_box.obj")), math::Transform());
    world.addModel(geometry::Mesh::load(fs::path("cornell_box/cornell_short_box.obj")), math::Transform());
    world.addModel(geometry::Mesh::load(fs::path("cornell_box/cornell_tall_box.obj")), math::Transform());

    geometry::Model& myModel = world.addModel(
        geometry::Mesh::load(fs::path("de_dust2-cs-map/de_dust2.obj"), true),
        math::Transform(math::Vec3(0, -2, -1), math::Vec3(0, 0, 0), math::Vec3(0.005, 0.005, 0.005)));
    myModel.transform.rotatePitch(270);
    myModel.transform.rotateYaw(270);

    geometry::Model& room = world.getModel(0);
    geometry::Model& shortBox = world.getModel(1);
    geometry::Model& tallBox = world.getModel(2);
    room.mesh->getMaterial(0)->setReflectiveness(0.0f);
    shortBox.mesh->getMaterial(0)->setReflectiveness(0.35f);
    tallBox.mesh->getMaterial(0)->setReflectiveness(0.7f);
    */
    world.addModel(
        geometry::Mesh::load(fs::path("polywag/model.obj")),
        math::Transform());
    geometry::Model& reflectiveCube = world.addModel(
        geometry::Mesh::load(fs::path("cornell_box/cornell_short_box.obj")),
        math::Transform(
            math::Vec3(-0.44f, 0.60f, -0.30f),
            math::Vec3(),
            math::Vec3(0.55f / 0.65f, 0.55f / 0.95f, 0.55f / 0.80f)));
    reflectiveCube.mesh->getMaterial(0)->setDiffuse(graphics::Colour(190, 200, 220));
    reflectiveCube.mesh->getMaterial(0)->setReflectiveness(0.75f);

    previousTicks = SDL_GetTicks();
    performanceFrequency = SDL_GetPerformanceFrequency();
    previousPresent = SDL_GetPerformanceCounter();
    screen->setWindowRelativeMouseMode(true);
}

void cleanupApplication() {
    if (screen) {
        screen->setWindowRelativeMouseMode(false);
        screen.reset();
    }
    renderer.reset();
#ifndef __EMSCRIPTEN__
    pool.reset();
#endif
    SDL_Quit();
}

#ifdef __EMSCRIPTEN__
void runBrowserFrame(void*) {
    try {
        if (runFrame())
            return;
    } catch (const std::exception& error) {
        std::cerr << "Application frame failed: " << error.what() << std::endl;
    } catch (...) {
        std::cerr << "Application frame failed with an unknown exception" << std::endl;
    }

    emscripten_cancel_main_loop();
    cleanupApplication();
}
#endif
}

int main() {
#ifdef __EMSCRIPTEN__
    if (!SDL_SetHint(SDL_HINT_EMSCRIPTEN_CANVAS_SELECTOR, "#canvas")) {
        std::cerr << "Failed to select the SDL canvas: " << SDL_GetError() << std::endl;
        return 1;
    }
    std::cout << "Welcome to the hybriddisplay demo, use WASD and the mouse to move the camera!" << std::endl;
    emscripten_sleep(2000);
#endif

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    try {
        initializeApplication();
    } catch (const std::exception& error) {
        std::cerr << "Application initialization failed: " << error.what() << std::endl;
        cleanupApplication();
        return 1;
    } catch (...) {
        std::cerr << "Application initialization failed with an unknown exception" << std::endl;
        cleanupApplication();
        return 1;
    }

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop_arg(runBrowserFrame, nullptr, 0, 1);
#else
    while (true) {
        try {
            if (!runFrame())
                break;
        } catch (const std::exception& error) {
            std::cerr << "Application frame failed: " << error.what() << std::endl;
            break;
        } catch (...) {
            std::cerr << "Application frame failed with an unknown exception" << std::endl;
            break;
        }
    }
    cleanupApplication();
#endif
    return 0;
}
