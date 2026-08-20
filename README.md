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

All source code for hybriddisplay is organized in 6 namespaces, all under the `hybriddisplay::` parent namespace:

| Namespace | Responsibility |
|----------|------------|
| `math::` | Vec3 and Transform classes. |
| `graphics::` | Materials, colours, and texture mapping. |
| `geometry::` | Vertex, triangle, and mesh storage. |
| `display::` | Wrapper for SDL3 window and texture uploading. |
| `rendering::` | Matrix math related functions and  |
| `threading::` | Multithreading managment objects and functions. |




## 📩 Contact
Author: Kai Pearson  
Email: kaipearson@dal.ca  
GitHub: pearsonkai  
