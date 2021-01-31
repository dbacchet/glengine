#include "gl_debug_hooks.h"

#include <stdio.h>

// pipelines
void trace_make_pipeline(const sg_pipeline_desc *desc, sg_pipeline pip, void *data) {
    printf("PIP make pipeline %s [%d]\n", desc->label, pip.id);
}
void trace_alloc_pipeline(sg_pipeline pip, void *data) {
    printf("PIP     alloc pipeline [%d]\n", pip.id);
}
void trace_dealloc_pipeline(sg_pipeline pip, void *data) {
    printf("PIP         dealloc pipeline [%d]\n", pip.id);
}
void trace_destroy_pipeline(sg_pipeline pip, void *data) {
    printf("PIP             destroy pipeline [%d]\n", pip.id);
}

// passes
void trace_make_pass(const sg_pass_desc *desc, sg_pass pass, void *data) {
    printf("PASS make pass %s [%d]\n", desc->label, pass.id);
}
void trace_alloc_pass(sg_pass pass, void *data) {
    printf("PASS     alloc pass [%d]\n", pass.id);
}
void trace_dealloc_pass(sg_pass pass, void *data) {
    printf("PASS         dealloc pass [%d]\n", pass.id);
}
void trace_destroy_pass(sg_pass pass, void *data) {
    printf("PASS             destroy pass [%d]\n", pass.id);
}

// images
void trace_make_image(const sg_image_desc *desc, sg_image image, void *data) {
    printf("IMAGE make image %s [%d]\n", desc->label, image.id);
}
void trace_alloc_image(sg_image image, void *data) {
    printf("IMAGE     alloc image [%d]\n", image.id);
}
void trace_dealloc_image(sg_image image, void *data) {
    printf("IMAGE         dealloc image [%d]\n", image.id);
}
void trace_destroy_image(sg_image image, void *data) {
    printf("IMAGE             destroy image [%d]\n", image.id);
}

// buffers
void trace_make_buffer(const sg_buffer_desc *desc, sg_buffer buffer, void *data) {
    printf("BUFFER make buffer %s [%d]\n", desc->label, buffer.id);
}
void trace_alloc_buffer(sg_buffer buffer, void *data) {
    printf("BUFFER     alloc buffer [%d]\n", buffer.id);
}
void trace_dealloc_buffer(sg_buffer buffer, void *data) {
    printf("BUFFER         dealloc buffer [%d]\n", buffer.id);
}
void trace_destroy_buffer(sg_buffer buffer, void *data) {
    printf("BUFFER             destroy buffer [%d]\n", buffer.id);
}

namespace glengine {
void install_trace_hooks(GLEngine *eng) {

    sg_trace_hooks hooks = {0};
    hooks.user_data = (void *)eng;
    // pipelines
    hooks.make_pipeline = trace_make_pipeline;
    hooks.alloc_pipeline = trace_alloc_pipeline;
    hooks.dealloc_pipeline = trace_dealloc_pipeline;
    hooks.destroy_pipeline = trace_destroy_pipeline;
    // passes
    hooks.make_pass = trace_make_pass;
    hooks.alloc_pass = trace_alloc_pass;
    hooks.dealloc_pass = trace_dealloc_pass;
    hooks.destroy_pass = trace_destroy_pass;
    // images
    hooks.make_image = trace_make_image;
    hooks.alloc_image = trace_alloc_image;
    hooks.dealloc_image = trace_dealloc_image;
    hooks.destroy_image = trace_destroy_image;
    // buffers
    hooks.make_buffer = trace_make_buffer;
    hooks.alloc_buffer = trace_alloc_buffer;
    hooks.dealloc_buffer = trace_dealloc_buffer;
    hooks.destroy_buffer = trace_destroy_buffer;
    sg_install_trace_hooks(&hooks);
}

} // namespace glengine
