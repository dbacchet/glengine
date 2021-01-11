GLEngine
========

Note: the code in this branch is not maintained, and kept for compatibility reasons. Please use the `master` branch instead.

This branch contains a version of GLEngine with a reduced set of functionalities and a simplified API

-----------------

Basic visualizer for rapid prototyping

## Renderables and scene graph
The engine basic unit is the Renderable, that is composed by a mesh and a material. All what the engine is doing is
manipulating, transforming and passing those renderables to the Renderer.
The renderables are typically attached to objects organized in a scene graph; each node of the graph has a parent and
zero or more children.

## Renderer
The Renderer has a fixed pipeline, implementing basic materials with forward rendering

### Forward Rendering Stage
Objects with a material that requires forward shading will be rendered in this stage. This includes VertexColor, Flat color, etc.

### Imgui
UI widgents will be drawn on top of the scene


