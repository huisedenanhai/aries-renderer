# TODO

A very coarse plan for development.

## Rendering

1. Surface Shading
    + Material Model
        + [x] Metallic Roughness PBR
        + [ ] Sheen
        + [ ] Subsurface Scattering
        + [ ] Clear Coat
        + [ ] Hair
        + [ ] Eye
    + [ ] Shadow
    + [ ] Shader Graph
    + [ ] Dynamic GI
    + [ ] GPU Driven Pipeline
2. Effect
    + [x] Stochastic Screen Space Reflection
        + [ ] Fix precision issue of HiZ tracing
        + [ ] Less ghosting
    + [ ] TAA
    + [ ] DOF
    + [ ] Physical Sky
    + [ ] Volumetric Cloud
    + [ ] Super Resolution
3. Architecture
    + [x] Render Graph
        + [x] Pass Culling
        + [ ] Virtual Resource Management
        + [ ] Scheduling
4. System
   > Build on top of existing rendering features
    + [ ] Particle
    + [ ] Skeleton Animation
    + [ ] Terrain
5. Utilities
    + [x] Profiler

## Editing

+ [ ] Make scene editing workflow clear
+ [ ] Built-in support GLTF as scene file, which can be an extended version. no need to make a wheel
+ [ ] Built-in support for KTX
