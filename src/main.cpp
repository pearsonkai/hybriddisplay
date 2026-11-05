#include <iostream>
#include <unordered_map>
#include <ctime>
#include <cmath>
#include "Renderer.hpp"


using namespace hybriddisplay;

constexpr graphics::Resolution PRI_RES = {800,600};

int main()
{    
    std::vector<std::thread> threads;
    
    srand(time(NULL));
    uint32_t numThreads = std::max((unsigned int) 1, std::thread::hardware_concurrency() - 1);

    display::Screen screen = display::Screen(PRI_RES,PRI_RES);
    screen.setWindowType(SDL_WINDOW_HIDDEN, true);

    screen.fillFramebuffer(graphics::Image<graphics::Colour>::load(fs::path("media/hds_splashscreen_grey.png")), {0.0f, 0.0f, 1.0f, 1.0f});
    screen.printBuffer();
    screen.setWindowType(SDL_WINDOW_HIDDEN, false);

    std::vector<display::Viewport> viewports;
    
    uint32_t numViewports = numThreads;
    std::pair<int, int> factors = {1, static_cast<int>(numViewports)};
    for (int i = std::sqrt(numViewports); i >= 1; --i) {
        if (numViewports % i == 0){
            factors = {i, static_cast<int>(numViewports) / i};
            break;
        }
    }

    for(int i = 0; i < factors.first; ++i)
    {
        for(int j = 0; j < factors.second; ++j)
        {
            const float areaX = static_cast<float>(i) / static_cast<float>(factors.first);
            const float areaY = static_cast<float>(j) / static_cast<float>(factors.second);
            const float areaWidth = 1.0f / static_cast<float>(factors.first);
            const float areaHeight = 1.0f / static_cast<float>(factors.second);
            viewports.push_back(screen.tieViewport(graphics::Region{areaX, areaY, areaWidth, areaHeight}, graphics::Region{0.0f, 0.0f, 1.0f, 1.0f}));
        }
    }
    
    threading::Pool pool = threading::Pool(numThreads);
    rendering::Renderer renderer = rendering::Renderer(&pool);

    rendering::Camera camera = rendering::Camera();
    camera.goTo(math::Vec3(0.000000, 2816.000000, 256.000000));
    camera.pointTowards(math::Vec3(0,0,0));
    graphics::Image<graphics::Colour> skybox = graphics::Image<graphics::Colour>(fs::path("skybox.png"));

    geometry::World mainWorld = geometry::World();
    geometry::Model& treeModel = mainWorld.addModel(geometry::Mesh::load(fs::path("de_dust2-cs-map/de_dust2.obj"), true),math::Transform(math::Vec3(0,-10,0),math::Vec3()));
    treeModel.transform.rotatePitch(270);
    treeModel.transform.setScale(math::Vec3(1,1,1));

    bool running = true;
    SDL_Event event;
    bool keys[SDL_SCANCODE_COUNT] = {};
    const float rotationSpeed = 1.0f;
    uint64_t previousTicks = SDL_GetTicks();
    const uint64_t performanceFrequency = SDL_GetPerformanceFrequency();
    uint64_t previousPresent = SDL_GetPerformanceCounter();
    uint64_t accumulatedFrameTime = 0;
    uint32_t measuredFrames = 0;
    constexpr uint32_t averageFrameCount = 4;
    //std::cout << "Num faces: " << treeMesh->getNumFaces() << std::endl;
    //std::cout << "Num vertices: " << treeMesh->getNumVertices() << std::endl;
    float sensitivity = 0.2;
    float speed = 1000.0;

    screen.setWindowRelativeMouseMode(true);
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
                break;
            }

            if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
                keys[event.key.scancode] = true;
                if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
                    running = false;
                    break;
                }
            }

            if (event.type == SDL_EVENT_MOUSE_MOTION) {
                float dx = event.motion.xrel;
                float dy = event.motion.yrel;

                camera.rotate(
                    -dy * sensitivity,
                    -dx * sensitivity
                );
            }
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                if (event.button.button == SDL_BUTTON_RIGHT) {
                    camera.setFov(25);
                    sensitivity = 0.025;
                }
            }
            if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
                if (event.button.button == SDL_BUTTON_RIGHT) {
                    camera.setFov(90);
                    sensitivity = 0.2;
                }
            }

            if (event.type == SDL_EVENT_KEY_UP) {
                keys[event.key.scancode] = false;
            }

        }


        if (!running) {
            break;
        }

        const uint64_t currentTicks = SDL_GetTicks();
        const float deltaSeconds = static_cast<float>(currentTicks - previousTicks) / 1000.0f;
        previousTicks = currentTicks;
        if (keys[SDL_SCANCODE_UP])
        {   
            screen.resizeWindow(screen.getRenderRes());
            for(display::Viewport& port : viewports) {
                port.recalculateBounds();
            }
            camera.setFov(90);
            treeModel.transform.setPosition(treeModel.transform.getPosition() + math::Vec3(0, 4, 0) * rotationSpeed * deltaSeconds);
        }
        if (keys[SDL_SCANCODE_DOWN])
        {
            
            camera.setFov(145);
            treeModel.transform.setPosition(treeModel.transform.getPosition() + math::Vec3(0, -4, 0) * rotationSpeed * deltaSeconds);
        }

    
        if(keys[SDL_SCANCODE_W]) {
            camera.moveForward(speed * deltaSeconds);
            screen.setWindowType(SDL_WINDOW_BORDERLESS, true);
        }
        if(keys[SDL_SCANCODE_S]) {
            camera.moveForward(-speed * deltaSeconds);
        }
        if(keys[SDL_SCANCODE_D]) {
            camera.moveRight(speed * deltaSeconds);
        }
        if(keys[SDL_SCANCODE_A]) {
            camera.moveRight(-speed * deltaSeconds);
        }


        bool on = keys[SDL_SCANCODE_P];
        if(keys[SDL_SCANCODE_Y]) {
            camera.moveForward(speed);
            screen.setWindowType(SDL_WINDOW_BORDERLESS, on);
        }
        if(keys[SDL_SCANCODE_U]) {
            screen.setWindowType(SDL_WINDOW_FULLSCREEN, on);
        }
        if(keys[SDL_SCANCODE_I]) {
            screen.setWindowType(SDL_WINDOW_RESIZABLE, on);
        }
        /*
        if(keys[SDL_SCANCODE_O]) {
            if(on) { screen.resizeRender(screen.getWindowRes()); }
            else {screen.resizeRender(PRI_RES);}
            for(display::Viewport& port : viewports) {
                port.recalculateBounds();
            }
        }*/
        
        pool.wait();

        screen.clearFramebuffer();
        screen.clearZBuffer();
        renderer.skybox(viewports, camera, skybox);
        renderer.rasterize(viewports, camera, mainWorld);

        
        for(display::Viewport& viewport : viewports) {
            display::Viewport* viewportPtr = &viewport;
            
            pool.add([&renderer, viewportPtr](){ renderer.outlineViewport(*viewportPtr); });
        }
        screen.printBuffer();

        const uint64_t currentPresent = SDL_GetPerformanceCounter();
        accumulatedFrameTime += currentPresent - previousPresent;
        previousPresent = currentPresent;
        ++measuredFrames;

        if (measuredFrames == averageFrameCount) {
            const double averageMilliseconds =
                static_cast<double>(accumulatedFrameTime) * 1000.0 /
                (static_cast<double>(performanceFrequency) * measuredFrames);
            std::cout << "Average frame time: " << averageMilliseconds << " ms" << std::endl;
            accumulatedFrameTime = 0;
            measuredFrames = 0;
        }
    }

    screen.setWindowRelativeMouseMode(false);
    pool.requestStop();

    return 0;
}