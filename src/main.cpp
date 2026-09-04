#include <iostream>
#include <unordered_map>

#include <iostream>
#include <windows.h>
#include "Renderer.hpp"

using namespace hybriddisplay;

const graphics::Resolution RESOLUTION = {800,600};

int main()
{
    //uint32_t numThreads = std::thread::hardware_concurrency();
    uint32_t numThreads = 1; // for debugging purposes, limit to 1 thread

    // std::unordered_map<std::string, std::unique_ptr<rendering::Material>> material_library;
    // std::vector<std::unique_ptr<geometry::Mesh>> mesh_pile;

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
            float x = static_cast<float>(i) / static_cast<float>(factors.first);
            float y = static_cast<float>(j) / static_cast<float>(factors.second);
            float width = 1.0f / static_cast<float>(factors.first);
            float height = 1.0f / static_cast<float>(factors.second);
            viewports.push_back(screen.tieViewport(x,y,width,height));
        }
    }

    //display::Viewport singleViewport = screen.tieViewport(0,0,1,1);

    rendering::Renderer renderer = rendering::Renderer();
    rendering::Camera camera = rendering::Camera();
    threading::Pool pool = threading::Pool(numThreads);

    camera.goTo(math::Vec3(0,0,15));
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
            treeModel.transform.setPosition(treeModel.transform.getPosition() + math::Vec3(0, -2, 0) * rotationSpeed * deltaSeconds);
        }
        if (keys[SDL_SCANCODE_DOWN])
        {
            treeModel.transform.setPosition(treeModel.transform.getPosition() + math::Vec3(0, 2, 0) * rotationSpeed * deltaSeconds);
        }

        math::Vec3 rotation = treeModel.transform.getRotation();
        rotation.y += rotationDirection * rotationSpeed * deltaSeconds;
        treeModel.transform.setRotation(rotation);

        screen.clearFramebuffer();
        screen.clearZBuffer();

        //renderer.drawLine(singleViewport, math::Vec3(0,0), math::Vec3(RESOLUTION.width,RESOLUTION.height), graphics::COLOUR_RED);
        //renderer.wireframe(singleViewport,camera,mainWorld);
        for(display::Viewport& viewport : viewports) {
            display::Viewport* viewportPtr = &viewport;
            pool.addTask([&renderer, &camera, &mainWorld, viewportPtr](){ renderer.wireframe(*viewportPtr,camera,mainWorld); });
            pool.addTask([&renderer, viewportPtr](){ renderer.outlineViewport(*viewportPtr); });
        }

        pool.waitForCompletion();
        
        screen.printBuffer();
    };
    

    return 0;
}