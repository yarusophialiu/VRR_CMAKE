add_plugin(DeferredShading)

target_sources(DeferredShading PRIVATE
    DeferredShading.cpp
    DeferredShading.cs.slang
    DeferredShading.h
)

target_copy_shaders(DeferredShading RenderPasses/DeferredShading)

target_source_group(DeferredShading "RenderPasses")
