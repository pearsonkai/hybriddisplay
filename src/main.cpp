#include <iostream>
#include <unordered_map>
#include <windows.h>
#include <conio.h>
#include <ctime>
#include <cmath>
#include "Renderer.hpp"


using namespace hybriddisplay;

static graphics::Resolution PRI_RES = {800,600};
constexpr float radiansToDegrees = 180.0f / 3.14159265359f;

int main()
{
    srand(time(NULL));
    uint32_t numThreads = std::thread::hardware_concurrency();
    

    display::Screen screen = display::Screen(PRI_RES,PRI_RES);
    screen.setWindowType(SDL_WINDOW_HIDDEN, true);

    graphics::Image<graphics::Colour> splash = graphics::Material::loadImage(fs::path("media/hds_splashscreen_grey.png"));
    screen.fillFramebuffer(splash, {0,0,1,1});
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
    camera.goTo(math::Vec3(0.4,0.68,0.5));
    camera.pointTowards(math::Vec3(0,0.2,0));

    geometry::Mesh* exampleMesh = new geometry::Mesh(fs::path("polywag/model.obj"));  
    geometry::World mainWorld = geometry::World();
    mainWorld.addModel(exampleMesh,math::Vec3(0,0,0));
    graphics::Image<graphics::Colour> whiteImage = graphics::Image<graphics::Colour>();
    whiteImage.data.at(0) = graphics::COLOUR_GREY;
    for(int i = 0; i < exampleMesh->getNumMaterials(); i++) {
        exampleMesh->getIndexedMaterial(i)->loadTextureMap(whiteImage);
    }

    bool running = true;
    SDL_Event event;
    bool keys[SDL_SCANCODE_COUNT] = {};
    std::cout << "Num faces: " << exampleMesh->getNumFaces() << std::endl;
    std::cout << "Num vertices: " << exampleMesh->getNumVertices() << std::endl;
    float sensitivity = 0.2;
    constexpr float speed = 0.007f;

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
        if(keys[SDL_SCANCODE_M]) {
            for(int i = 0; i < exampleMesh->getNumMaterials(); i++) {
                exampleMesh->getIndexedMaterial(i)->lightType = graphics::lighting::none;
            }
        }
        if(keys[SDL_SCANCODE_N]) {
            for(int i = 0; i < exampleMesh->getNumMaterials(); i++) {
                exampleMesh->getIndexedMaterial(i)->lightType = graphics::lighting::face;
            }
        }
        if(keys[SDL_SCANCODE_B]) {
            for(int i = 0; i < exampleMesh->getNumMaterials(); i++) {
                exampleMesh->getIndexedMaterial(i)->lightType = graphics::lighting::face_clamp;
            }
        }
        if(keys[SDL_SCANCODE_V]) {
            for(int i = 0; i < exampleMesh->getNumMaterials(); i++) {
                exampleMesh->getIndexedMaterial(i)->lightType = graphics::lighting::normal_face;
            }
        }

        if(keys[SDL_SCANCODE_L]) {
            std::cout << camera.getTransform().getPosition().to_string() << std::endl;
        }
        

        if(keys[SDL_SCANCODE_W]) {
            const math::Transform& transform = camera.getTransform();
            const math::Vec3 forward = transform.applyRotation(math::Vec3(0, 0, -1));
            camera.moveTowards(transform.getPosition() + forward, speed);
        }
        if(keys[SDL_SCANCODE_S]) {
            const math::Transform& transform = camera.getTransform();
            const math::Vec3 forward = transform.applyRotation(math::Vec3(0, 0, 1));
            camera.moveTowards(transform.getPosition() + forward, speed);
        }
        if(keys[SDL_SCANCODE_D]) {
            const math::Transform& transform = camera.getTransform();
            const math::Vec3 forward = transform.applyRotation(math::Vec3(1, 0, 0));
            camera.moveTowards(transform.getPosition() + forward, speed);
        }
        if(keys[SDL_SCANCODE_A]) {
            const math::Transform& transform = camera.getTransform();
            const math::Vec3 forward = transform.applyRotation(math::Vec3(-1, 0, 0));
            camera.moveTowards(transform.getPosition() + forward, speed);
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
        if(keys[SDL_SCANCODE_O]) {
            if(on) { screen.resizeRender(screen.getWindowRes()); }
            else {screen.resizeRender(PRI_RES);}
            for(display::Viewport& port : viewports) {
                port.recalculateBounds();
            }
        }
        
        pool.waitForCompletion();

        screen.clearFramebuffer();
        screen.clearZBuffer();
        renderer.rasterize(viewports, camera, mainWorld);

        pool.waitForCompletion();
        
        screen.printBuffer();
    }

    screen.setWindowRelativeMouseMode(false);
    pool.requestStop();

    return 0;
}