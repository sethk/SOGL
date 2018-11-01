Seth's OpenGL
========================
**SOGL** is a toy rasterizer I made for my own exploration of the fixed-function OpenGL pipeline as it existed before the introduction of programmable shaders.

Basic overview
--------------
* Setup and windowing (`glut.c`, `window_cgl.c`)
* API and Primitive assembly (`gl.c`)
* Lighting and shading (`render.c`)
* Clipping (`clip.c`)
* Rasterization (`draw_raster.c`, `raster.c`)
* Math support functions (`matrix.c`, `vector.c`)

Rendering techniques used
-------------------------
* Cyrus-Beck parametric line clipping
* Sutherland-Hodgeman polygon clipping
* Blinn-Phong reflection model
* Gouraud shading
* Bernstein evaluators (Bézier curves/patches, etc.)

Why OpenGL/GLUT?
----------------
It's the API I was already the most familiar with, and there are a bunch of self-contained sample apps exercising its different capabilities.

Surprising things I learned
---------------------------
**Line rasterization is harder than polygon rasterization**

For a floating-point renderer without multisampling, rasterizing lines according to the diamond exit rule is surprisingly complex. It requires significantly more code to determine whether the endpoints should be filled or not, and doesn't seem to be easily generalizable. By comparison, scan-converting a polygon only requires applying the “top-left rule,” which is simple after sorting the edges.

For lines, I settled on generalizing them by the octant of their direction vector, and then by whether they were *x*- or *y*-major. Each octant requires a slightly different test of the diamond exit rule, and some involve calculating the midpoint as the line exits the endpoint pixels. Because I used a span-based rasterizer, it was also more efficient to differentiate between left- and right-moving lines.

What I didn't bother to try was using the fractional position of the endpoints to initialize the decision variable of Bresenham's algorithm. NVIDIA has a patent, [US8482567](https://patents.google.com/patent/US8482567), which describes a technique for “conditioning” the endpoints of a line beforehand, such that the implementation can be simpler, and probably faster. [Note: I did not try to implement this patent, obviously!]

**The eye vector is somewhat incorrect by default**

When calculating the specular irradiance of a vertex, the fixed-function pipeline uses {0, 0, -1} as the view direction for lighting calculations. This leads to a slight error that increases towards the edges of the field of view.

This is mentioned in the OpenGL Redbook, and can be changed with `glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);`, but the default is 0 (disabled). This surprised me while I was testing my light calculations, because they were wrong in this suble way relative to the hardware GL implementation.

Using a view direction parallel to the *z* axis makes sense in the context of the fixed-function pipeline because it saves a subtraction (*eye_pos* - *world_pos*) and a vector normalize per vertex, but the cost savings is infinitesimal by today's standards.

Reference materials
-------------------
* [“Rasterization Rules” from _Programming Guide for Direct3D 11_](https://docs.microsoft.com/en-us/windows/desktop/direct3d11/d3d10-graphics-programming-guide-rasterizer-stage-rules#triangle-rasterization-rules-without-multisampling) by Microsoft Corp.
* [_Computer Graphics: Principles and Practice in C (2nd Edition)_](https://www.pearson.com/us/higher-education/product/Foley-Computer-Graphics-Principles-and-Practice-in-C-2nd-Edition/9780201848403.html) by Foley, van Dam, Feiner, Hughes
