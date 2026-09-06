#include <iostream>
#include <unordered_map>

#include <iostream>
#include <windows.h>
#include "Renderer.hpp"

using namespace hybriddisplay;

const graphics::Resolution RESOLUTION = {800,600};
const UINT32 THREADCOUNT =  2;

int main()
{
    uint32_t numThreads = THREADCOUNT; // for debugging purposes, limit to 1 thread
    if(numThreads == 0)
    {
        numThreads = std::thread::hardware_concurrency();
    }

    display::Screen screen = display::Screen(RESOLUTION);
    
    std::vector<display::Viewport> viewports;
    
    uint32_t numViewports = std::pow(numThreads,3);
    std::pair<int, int> factors = {1, static_cast<int>(numViewports)};
    for (int i = std::sqrt(numViewports); i >= 1; --i)
    {
        if (numViewports % i == 0)
        {
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

    display::Viewport superPort = screen.tieViewport(graphics::Region{0.0f, 0.0f, 1.0f, 1.0f}, graphics::Region{0.0f, 0.0f, 0.5f, 0.5f});
    
    
    rendering::Renderer renderer = rendering::Renderer();
    threading::Pool pool = threading::Pool(numThreads);

    rendering::Camera camera = rendering::Camera();
    camera.goTo(math::Vec3(0,0,25));
    camera.pointTowards(math::Vec3(0,0,0));
    
    geometry::World mainWorld = geometry::World();


    std::vector<geometry::Vertex> cubeVertices = {
        {{-0.5f, -0.5f, -0.5f}, {0, 0, -1}, {0, 0, 0}},
        {{ 0.5f, -0.5f, -0.5f}, {0, 0, -1}, {1, 0, 0}},
        {{ 0.5f,  0.5f, -0.5f}, {0, 0, -1}, {1, 1, 0}},
        {{-0.5f,  0.5f, -0.5f}, {0, 0, -1}, {0, 1, 0}},
        {{-0.5f, -0.5f,  0.5f}, {0, 0, 1}, {0, 0, 1}},
        {{ 0.5f, -0.5f,  0.5f}, {0, 0, 1}, {1, 0, 1}},
        {{ 0.5f,  0.5f,  0.5f}, {0, 0, 1}, {1, 1, 1}},
        {{-0.5f,  0.5f,  0.5f}, {0, 0, 1}, {0, 1, 1}}
    };
    geometry::Mesh cubeMesh = geometry::Mesh(cubeVertices, {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4}, {}, {});

    geometry::Mesh treeMesh(fs::path("tree.obj"));
    mainWorld.addMesh(treeMesh);
    geometry::Model& treeModel = mainWorld.addModel(&treeMesh,math::Transform());
    treeModel.transform.setPosition(math::Vec3(0,-10,0));
    
    bool running = true;
    SDL_Event event;
    bool keys[SDL_SCANCODE_COUNT] = {};
    const float rotationSpeed = 1.0f;
    uint64_t previousTicks = SDL_GetTicks();

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
                break;
            }

            if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat)
            {
                keys[event.key.scancode] = true;
                if (event.key.scancode == SDL_SCANCODE_ESCAPE)
                {
                    running = false;
                    break;
                }
            }

            if (event.type == SDL_EVENT_KEY_UP)
            {
                keys[event.key.scancode] = false;
            }
        }

        if (!running)
        {
            break;
        }

        const uint64_t currentTicks = SDL_GetTicks();
        const float deltaSeconds = static_cast<float>(currentTicks - previousTicks) / 1000.0f;
        previousTicks = currentTicks;

        float rotationDirection = 0.0f;
        if (keys[SDL_SCANCODE_LEFT])
        {
            rotationDirection -= 1.0f;
        }
        if (keys[SDL_SCANCODE_RIGHT])
        {
            rotationDirection += 1.0f;
        }
        if (keys[SDL_SCANCODE_UP])
        {
            treeModel.transform.setPosition(treeModel.transform.getPosition() + math::Vec3(0, 4, 0) * rotationSpeed * deltaSeconds);
        }
        if (keys[SDL_SCANCODE_DOWN])
        {
            treeModel.transform.setPosition(treeModel.transform.getPosition() + math::Vec3(0, -4, 0) * rotationSpeed * deltaSeconds);
        }

        math::Vec3 rotation = treeModel.transform.getRotation();
        rotation.y += rotationDirection * rotationSpeed * deltaSeconds;
        treeModel.transform.setRotation(rotation);



        screen.clearFramebuffer();
        screen.clearZBuffer();
        
        for(display::Viewport& viewport : viewports) {
            display::Viewport* viewportPtr = &viewport;
            pool.addTask([&renderer, &camera, &mainWorld, viewportPtr](){ renderer.wireframe(*viewportPtr,camera,mainWorld); });
            pool.addTask([&renderer, viewportPtr](){ renderer.outlineViewport(*viewportPtr); });
        }

        pool.waitForCompletion();
        
        //screen.clearZBuffer();
        //pool.addTask([&renderer, &camera, &mainWorld, &superPort](){ renderer.wireframe(superPort,camera,mainWorld); });
        
        //pool.waitForCompletion();
        
        screen.printBuffer();
    };
    

    return 0;
}