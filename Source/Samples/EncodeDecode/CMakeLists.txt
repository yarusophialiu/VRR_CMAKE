enable_language(CUDA)

add_falcor_executable(EncodeDecode)

find_package(CUDAToolkit REQUIRED)

add_dependencies(EncodeDecode GBuffer DeferredShading PythonImporter USDImporter)
# add_dependencies(EncodeDecode DeferredShading)

target_sources(EncodeDecode PRIVATE
    EncodeDecode.cpp
    EncodeDecode.h
    stb_image_write.h
    ColorSpace.h
    # FramePresenterD3D11.h
    # FramePresenterD3D.h
    EncodeDecode.3d.slang
    EncodeDecode.rt.slang
)

set(NVENC_SDK_ROOT "C:/Video_Codec_SDK_12.1.14/Video_Codec_SDK_12.1.14")
set(NVENC_SDK_LIB_DIR "${NVENC_SDK_ROOT}/Lib/x64")

add_library(ColorSpace STATIC ColorSpace.cu)

target_include_directories(EncodeDecode PRIVATE ${NVENC_SDK_ROOT}/Interface ${NVENC_SDK_ROOT}/Samples/Utils)

target_link_libraries(EncodeDecode PRIVATE ${NVENC_SDK_LIB_DIR}/nvcuvid.lib ${NVENC_SDK_LIB_DIR}/nvencodeapi.lib CUDA::cudart ColorSpace d3d11.lib dxgi.lib)

target_copy_shaders(EncodeDecode Samples/EncodeDecode)

target_source_group(EncodeDecode "Samples")
