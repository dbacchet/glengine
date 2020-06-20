GLEngine
========

basic visualizer for rapid prototyping


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

### Forward Rendering Stage
Objects with a material that requires forward shading will be rendered in this stage. This includes VertexColor, Flat color, etc.
Objects in this stage will be selectable, with the possibility to query the object ID for a specific screen coordinate.

### Debug Annotations
Additional rendering that can be useful as debug/annotations, like text, bounding boxes, etc.
Objects in this stage will *not* be selectable.


