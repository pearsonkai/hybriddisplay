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

All source code for hybriddisplay is organized in 6 namespaces, all under the `hybriddisplay::` parent namespace.

| Namespace | Responsibility |
|----------|------------|
| `math::` | Vec3 and Transform classes. |
| `graphics::` | Materials, colours, and texture mapping. |
| `geometry::` | Vertex, triangle, and mesh storage. |
| `display::` | Wrapper for SDL3 window and texture uploading. |
| `rendering::` | Matrix math related functions and  |
| `threading::` | Multithreading managment objects and functions. |

One of the core goals for hybriddisplay is having a flexible and dynamic rendering pipeline. Consider you're setting up a landscape for a render and you want to angle your shot just right. Ray tracing is slow and doesn't give you an idea of the scene until you wait for it. Rather, you can rasterize the scene and move the camera into an area that looks reasonable, then ray trace to get the desired look.

Most of the impressive code can be found in `include/Renderer.hpp`, where you can see different rendering implimentations such as wireframe, rasterize, and raytrace. 


## 📩 Contact
Author: Kai Pearson  
Email: kaipearson@dal.ca  
GitHub: pearsonkai  
