GLEngine
========

basic visualizer for rapid prototyping

## Renderables and scene graph
The engine basic unit is the Renderable, that is composed by a mesh and a material. All what the engine is doing is
manipulating, transforming and passing those renderables to the Renderer.
The renderables are typically attached to objects organized in a scene graph; each node of the graph has a parent and
zero or more children.

## Renderer
The Renderer has a fixed pipeline, composed by several stages:

1. Deferred rendering
2. Forward rendering
3. Debug/Annotations

The basic idea is to walk the scene graph(s) and create a sequence of rendering commands that will be submitted to the GPU.
Each stage has his own command queue, and the commands in the queue can be sorted in order to:

* minimize the state changes (shaders, etc)
* sort back-to-front or front-to-back 

### Deferred Rendering Stage
Objects with a material that requires access to the g-buffer will be rendered in this stage.
The pipeline has a fixed g-buffer definition.
Objects in this stage will be selectable, with the possibility to query the object ID for a specific screen coordinate.

### Forward Rendering Stage (to be implemented)
Objects with a material that requires forward shading will be rendered in this stage. This includes VertexColor, Flat color, etc.
Objects in this stage will be selectable, with the possibility to query the object ID for a specific screen coordinate.

### Debug Annotations (to be implemented)
Additional rendering that can be useful as debug/annotations, like text, bounding boxes, etc.
Objects in this stage will *not* be selectable.


TO BE ADDED
===========

resource ownership
------------------
if created by the engine is managed by the engine
if created by the user has to be managed by the user
renderables are ephemeral and _copied_ into the glengine::Object. Make sure to always update the one in the object, not a temporary object created when _constructing_ the object itself 

why sokol
---------
* better support more than one platform/gfx api
* passes and pipelines were implemented very well
* low-level enough to have total flexibility on the architecture
* very good validation layer in debug (useful for troubleshooting tricky opengl buffers/pipelines incompatibilities, for example)

TODO
----
- [] dynamic meshes (update mesh data)
- [] generate mipmaps
- [] gltf import
- [] support PBR materials (using gltf as reference)
- [] object selection (ID buffer)
- [] consistent color spaces (sRGB for both meshes and textures)
- [] better object/mesh/material update semantics. Now it's tricky with shared and copied components (ex. in the renderable material and mesh are pointers to values created externally, but `bind` is a local object, and since the renderable is _copied_ it cannot be updated in the original Renderable instance created when constructing the Object, but has to be modified in the object itself)
