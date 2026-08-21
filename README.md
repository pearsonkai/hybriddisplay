# <img src="media/hybriddisplayicon4.png" alt="hybriddisplay icon" width="40" height="40"> hybriddisplay
Started: August 8th, 2026  

## 📝 About
hybriddisplay is a C++23 CPU rasterizer/raytracing hybrid rendering engine. It is the predecessor to <a href="https://github.com/pearson-kai/superdisplay">superdisplay</a>. Below you can find some of the features first seen in superdisplay, and the additions to those features that will come in hybriddisplay.


| superdisplay | hybriddisplay |
|----------|------------|
| Full perspective CPU rasterization | CPU rasterization and ray tracing options |
| .obj file compatability | Multithreaded rendering (with load balancing) |
| Modular screen options and rendering settings | Textures and normal mapping |
|   | Mesh normal interpolation |
|   | Viewport camera options |
|   | Lambert diffuse lighting |


## ⚙️ Architecture

### Namespaces

All source code for hybriddisplay is organized in 6 namespaces, all under the `hybriddisplay::` parent namespace.

| Namespace | Responsibility |
|----------|------------|
| `math::` | Vec3 and Transform classes. |
| `graphics::` | Materials, colours, and texture mapping. |
| `geometry::` | Vertex, triangle, and mesh storage. |
| `display::` | Wrapper for SDL3 window and texture uploading. |
| `rendering::` | Matrix math related functions and  |
| `threading::` | Multithreading managment objects and functions. |

### Rendering

The rendering code can be found in `src/Renderer.cpp`, where you can see different implimentations such as wireframe, rasterize, and raytrace. As development of this project continues, more of these functions will have proper implimentations.

One of the core goals for hybriddisplay is having a flexible and dynamic rendering pipeline. With the different rendering options, you can effectively layer effects on top of each other.

Consider a pipeline like this:
1. clearBuffer()
2. rasterize(viewport, camera, world)
3. clearZBuffer()
4. wireframe(viewport, camera, objectworld)
5. presentFrame()

In this pipeline, a scene is being presented, and a wireframe of a single object inside that world is being presented after. Because we clear the z-buffer though between rasterizing and presenting the wireframe, the wireframe will have priority over the rasterization, giving an xray look to the object in the other world.

Rendering functions are passed 3 things:
- Viewport (pointer to the necessary buffers, and boundaries to print in)
- Camera (scene transform information)
- World (collection of models and light sources)

### Display and SDL3 Integration

This project uses SDL3's `SDL_Window` and `SDL_Texture` objects to display graphics. All implimentation code tied to SDL3 can be found in `src/Screen.cpp`. 

The general flow of information to the screen takes place from the frame buffer. The frame buffer in the `display::Screen` class is a vector of colours, the same size as the `SDL_Window`. The `graphics::Colour` struct is standard, consisiting of 4 uint8_t values, corresponding to r, g, b, a. When the screen is ready to present something, the frame buffer is copied into an `SDL_Texture`, which is an optimized data structure specifically designed to then connect and display itself in the `SDL_Window`.






## 📩 Contact
Author: Kai Pearson  
Email: kaipearson@dal.ca  
GitHub: pearsonkai  
