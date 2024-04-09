
#include "DeferredShading.h"

namespace
{
const char kSrc[] = "src";
const char kDst[] = "dst";

// Scripting options.
const char kEnabled[] = "enabled";
const char kOutputSize[] = "outputSize";
const char kFixedOutputSize[] = "fixedOutputSize";
const char kWipe[] = "wipe";
const char kBloomAmount[] = "bloomAmount";
const char kStarAmount[] = "starAmount";
const char kStarAngle[] = "starAngle";
const char kVignetteAmount[] = "vignetteAmount";
const char kChromaticAberrationAmount[] = "chromaticAberrationAmount";
const char kBarrelDistortAmount[] = "barrelDistortAmount";
const char kSaturationCurve[] = "saturationCurve";
const char kColorOffset[] = "colorOffset";
const char kColorScale[] = "colorScale";
const char kColorPower[] = "colorPower";
const char kColorOffsetScalar[] = "colorOffsetScalar";
const char kColorScaleScalar[] = "colorScaleScalar";
const char kColorPowerScalar[] = "colorPowerScalar";

const char kShaderFile[] = "RenderPasses/DeferredShading/DeferredShading.cs.slang";
} // namespace

static void regDeferredShading(pybind11::module& m)
{
    pybind11::class_<DeferredShading, RenderPass, ref<DeferredShading>> pass(m, "DeferredShading");
    pass.def_property(kEnabled, &DeferredShading::getEnabled, &DeferredShading::setEnabled);
    pass.def_property(kWipe, &DeferredShading::getWipe, &DeferredShading::setWipe);
    pass.def_property(kBloomAmount, &DeferredShading::getBloomAmount, &DeferredShading::setBloomAmount);
    pass.def_property(kStarAmount, &DeferredShading::getStarAmount, &DeferredShading::setStarAmount);
    pass.def_property(kStarAngle, &DeferredShading::getStarAngle, &DeferredShading::setStarAngle);
    pass.def_property(kVignetteAmount, &DeferredShading::getVignetteAmount, &DeferredShading::setVignetteAmount);
    pass.def_property(kChromaticAberrationAmount, &DeferredShading::getChromaticAberrationAmount, &DeferredShading::setChromaticAberrationAmount);
    pass.def_property(kBarrelDistortAmount, &DeferredShading::getBarrelDistortAmount, &DeferredShading::setBarrelDistortAmount);
    pass.def_property(kSaturationCurve, &DeferredShading::getSaturationCurve, &DeferredShading::setSaturationCurve);
    pass.def_property(kColorOffset, &DeferredShading::getColorOffset, &DeferredShading::setColorOffset);
    pass.def_property(kColorScale, &DeferredShading::getColorScale, &DeferredShading::setColorScale);
    pass.def_property(kColorPower, &DeferredShading::getColorPower, &DeferredShading::setColorPower);
    pass.def_property(kColorOffsetScalar, &DeferredShading::getColorOffsetScalar, &DeferredShading::setColorOffsetScalar);
    pass.def_property(kColorScaleScalar, &DeferredShading::getColorScaleScalar, &DeferredShading::setColorScaleScalar);
    pass.def_property(kColorPowerScalar, &DeferredShading::getColorPowerScalar, &DeferredShading::setColorPowerScalar);
}

const ChannelList DeferredShading::kDeferredShadingChannels = {
    // clang-format off
    { "posW",           "gPosW",            "Position in world space",                           true /* optional */, ResourceFormat::RGBA32Float },
    { "normW",          "gNormW",           "Shading normal in world space",                     true /* optional */, ResourceFormat::RGBA32Float },
    { "tangentW",       "gTangentW",        "Shading tangent in world space (xyz) and sign (w)", true /* optional */, ResourceFormat::RGBA32Float },
    { "faceNormalW",    "gFaceNormalW",     "Face normal in world space",                        true /* optional */, ResourceFormat::RGBA32Float },
    { "texC",           "gTexC",            "Texture coordinate",                                true /* optional */, ResourceFormat::RG32Float   },
    { "texGrads",       "gTexGrads",        "Texture gradients (ddx, ddy)",                      true /* optional */, ResourceFormat::RGBA16Float },
    { "mvec",           "gMotionVector",    "Motion vector",                                     true /* optional */, ResourceFormat::RG32Float   },
    { "mtlData",        "gMaterialData",    "Material data (ID, header.x, header.y, lobes)",     true /* optional */, ResourceFormat::RGBA32Uint  },
    // clang-format on
};

extern "C" FALCOR_API_EXPORT void registerPlugin(Falcor::PluginRegistry& registry)
{
    registry.registerClass<RenderPass, DeferredShading>();
    ScriptBindings::registerBinding(regDeferredShading);
}

DeferredShading::DeferredShading(ref<Device> pDevice, const Properties& props) : RenderPass(pDevice)
{
    // Deserialize pass from dictionary.
    for (const auto& [key, value] : props)
    {
        if (key == kEnabled)
            setEnabled(value);
        else if (key == kOutputSize)
            mOutputSizeSelection = value;
        else if (key == kFixedOutputSize)
            mFixedOutputSize = value;
        else if (key == kWipe)
            setWipe(value);
        else if (key == kBloomAmount)
            setBloomAmount(value);
        else if (key == kStarAmount)
            setStarAmount(value);
        else if (key == kStarAngle)
            setStarAngle(value);
        else if (key == kVignetteAmount)
            setVignetteAmount(value);
        else if (key == kChromaticAberrationAmount)
            setChromaticAberrationAmount(value);
        else if (key == kBarrelDistortAmount)
            setBarrelDistortAmount(value);
        else if (key == kSaturationCurve)
            setSaturationCurve(value);
        else if (key == kColorOffset)
            setColorOffset(value);
        else if (key == kColorScale)
            setColorScale(value);
        else if (key == kColorPower)
            setColorPower(value);
        else if (key == kColorOffsetScalar)
            setColorOffsetScalar(value);
        else if (key == kColorScaleScalar)
            setColorScaleScalar(value);
        else if (key == kColorPowerScalar)
            setColorPowerScalar(value);
        else
            logWarning("Unknown property '{}' in DeferredShading properties.", key);
    }

    Sampler::Desc samplerDesc;
    samplerDesc.setFilterMode(TextureFilteringMode::Linear, TextureFilteringMode::Linear, TextureFilteringMode::Point);
    samplerDesc.setAddressingMode(TextureAddressingMode::Border, TextureAddressingMode::Border, TextureAddressingMode::Border);
    mpLinearSampler = mpDevice->createSampler(samplerDesc);

    DefineList defines;
    mpDownsamplePass = ComputePass::create(mpDevice, kShaderFile, "downsample", defines);
    mpUpsamplePass = ComputePass::create(mpDevice, kShaderFile, "upsample", defines);
    mpPostFXPass = ComputePass::create(mpDevice, kShaderFile, "runPostFX", defines);
}

Properties DeferredShading::getProperties() const
{
    Properties props;
    props[kEnabled] = getEnabled();
    props[kOutputSize] = mOutputSizeSelection;
    if (mOutputSizeSelection == RenderPassHelpers::IOSize::Fixed)
        props[kFixedOutputSize] = mFixedOutputSize;
    props[kWipe] = getWipe();
    props[kBloomAmount] = getBloomAmount();
    props[kStarAmount] = getStarAmount();
    props[kStarAngle] = getStarAngle();
    props[kVignetteAmount] = getVignetteAmount();
    props[kChromaticAberrationAmount] = getChromaticAberrationAmount();
    props[kBarrelDistortAmount] = getBarrelDistortAmount();
    props[kSaturationCurve] = getSaturationCurve();
    props[kColorOffset] = getColorOffset();
    props[kColorScale] = getColorScale();
    props[kColorPower] = getColorPower();
    props[kColorOffsetScalar] = getColorOffsetScalar();
    props[kColorScaleScalar] = getColorScaleScalar();
    props[kColorPowerScalar] = getColorPowerScalar();
    return props;
}

RenderPassReflection DeferredShading::reflect(const CompileData& compileData)
{
    RenderPassReflection reflector;
    const uint2 sz = RenderPassHelpers::calculateIOSize(mOutputSizeSelection, mFixedOutputSize, compileData.defaultTexDims);

    reflector.addInput(kSrc, "Source texture").bindFlags(ResourceBindFlags::ShaderResource);
    ;
    reflector.addOutput(kDst, "post-effected output texture")
        .bindFlags(ResourceBindFlags::RenderTarget | ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess)
        .format(ResourceFormat::RGBA32Float)
        .texture2D(sz.x, sz.y);
    
    addRenderPassOutputs(reflector, kDeferredShadingChannels, ResourceBindFlags::UnorderedAccess, sz);

    return reflector;
}

void DeferredShading::execute(RenderContext* pRenderContext, const RenderData& renderData)
{
    auto pSrc = renderData.getTexture(kSrc);
    auto pDst = renderData.getTexture(kDst);
    FALCOR_ASSERT(pSrc && pDst);

    const uint2 resolution = uint2(pSrc->getWidth(), pSrc->getHeight());
    // const float2 dstresolution = float2(pDst->getWidth(), pSrc->getHeight());

    // std::cout << "\n\npDst.x = " << dstresolution.x << std::endl;

    // if we have 'identity' settings, we can just copy input to output
    // clang-format off
    // if (getEnabled() == false || getWipe() >= 1.f || (
    //     getBloomAmount() == 0.f &&
    //     getChromaticAberrationAmount() == 0.f &&
    //     getBarrelDistortAmount() == 0.f &&
    //     all(getSaturationCurve() == float3(1.f)) &&
    //     all(getColorOffset() == float3(0.5f)) &&
    //     all(getColorScale() == float3(0.5f)) &&
    //     all(getColorPower() == float3(0.5f)) &&
    //     getColorOffsetScalar() == 0.f &&
    //     getColorScaleScalar() == 0.f &&
    //     getColorPowerScalar() == 0.f
    //     ))
    // {
    //     // wipe is all the way across, which corresponds to no effect
    //     std::cout << "about to blit" <<"\n";
    //     pRenderContext->blit(pSrc->getSRV(), pDst->getRTV());
    //     return;
    // }
    // clang-format on

    preparePostFX(pRenderContext, resolution.x, resolution.y);
    {
        auto var = mpPostFXPass->getRootVar();
        var["PerFrameCB"]["gResolution"] = resolution;
        var["PerFrameCB"]["gInvRes"] = float2(1.f / resolution.x, 1.f / resolution.y);
        var["PerFrameCB"]["gVignetteAmount"] = getVignetteAmount();
        var["PerFrameCB"]["gChromaticAberrationAmount"] = getChromaticAberrationAmount() * (1.f / 64.f);
        float barrel = getBarrelDistortAmount() * 0.125f;
        // scale factor chosen to keep the corners of a 16:9 viewport fixed
        var["PerFrameCB"]["gBarrelDistort"] = float2(1.f / (1.f + 4.f * barrel), barrel);
        float3 satcurve = getSaturationCurve();
        // fit a quadratic thru the 3 points
        satcurve.y -= satcurve.x;
        satcurve.z -= satcurve.x;
        float A = 2.f * satcurve.z - 4.f * satcurve.y;
        float B = satcurve.z - A;
        float C = satcurve.x;
        var["PerFrameCB"]["gSaturationCurve"] = float3(A, B, C);
        var["PerFrameCB"]["gColorOffset"] = getColorOffset() + getColorOffsetScalar() - 0.5f;
        var["PerFrameCB"]["gColorScale"] = getColorScale() * std::exp2(1.f + 2.f * getColorScaleScalar());
        var["PerFrameCB"]["gColorPower"] = exp2(3.f * (0.5f - getColorPower() - getColorPowerScalar()));
        var["PerFrameCB"]["gWipe"] = mWipe * resolution.x;
        var["gBloomed"] = getBloomAmount() > 0.f ? mpPyramid[0] : pSrc;
        var["gSrc"] = pSrc;
        var["gDst"] = pDst;
        var["gLinearSampler"] = mpLinearSampler;
        mpPostFXPass->execute(pRenderContext, uint3(resolution, 1));
    }
}

void DeferredShading::preparePostFX(RenderContext* pRenderContext, uint32_t width, uint32_t height)
{
    for (int res = 0; res < kNumLevels + 1; ++res)
    {
        ref<Texture>& pBuf = mpPyramid[res];
        if (getBloomAmount() <= 0.f)
        {
            pBuf = nullptr;
        }
        else
        {
            uint32_t w = std::max(1u, width >> res);
            uint32_t h = std::max(1u, height >> res);
            if (!pBuf || pBuf->getWidth() != w || pBuf->getHeight() != h)
            {
                pBuf = mpDevice->createTexture2D(
                    w, h, ResourceFormat::RGBA16Float, 1, 1, nullptr, ResourceBindFlags::ShaderResource | ResourceBindFlags::UnorderedAccess
                );
                FALCOR_ASSERT(pBuf);
            }
        }
    }
}

void DeferredShading::renderUI(Gui::Widgets& widget)
{
    // Controls for output size.
    // When output size requirements change, we'll trigger a graph recompile to update the render pass I/O sizes.
    if (widget.dropdown("Output size", mOutputSizeSelection))
        requestRecompile();
    if (mOutputSizeSelection == RenderPassHelpers::IOSize::Fixed)
    {
        if (widget.var("Size in pixels", mFixedOutputSize, 32u, 16384u))
            requestRecompile();
    }

    // PostFX options.
    widget.checkbox("Enable post fx", mEnabled);
    widget.slider("Wipe", mWipe, 0.f, 1.f);
    if (auto group = widget.group("Lens FX", true))
    {
        group.slider("Bloom", mBloomAmount, 0.f, 1.f);
        group.slider("Bloom Star", mStarAmount, 0.f, 1.f);
        group.slider("Star Angle", mStarAngle, 0.f, 1.f, true);
        group.slider("Vignette", mVignetteAmount, 0.f, 1.f);
        group.slider("Chromatic Aberration", mChromaticAberrationAmount, 0.f, 1.f);
        group.slider("Barrel Distortion", mBarrelDistortAmount, 0.f, 1.f);
        if (group.button("reset this group"))
        {
            mBloomAmount = 0.f;
            mStarAmount = 0.f;
            mStarAngle = 0.1f;
            mVignetteAmount = 0.f;
            mChromaticAberrationAmount = 0.f;
            mBarrelDistortAmount = 0.f;
        }
    }
    if (auto group = widget.group("Saturation", true))
    {
        group.slider("Shadow Saturation", mSaturationCurve.x, 0.f, 2.f);
        group.slider("Midtone Saturation", mSaturationCurve.y, 0.f, 2.f);
        group.slider("Hilight Saturation", mSaturationCurve.z, 0.f, 2.f);
        if (group.button("reset this group"))
        {
            mSaturationCurve = float3(1.f);
        }
    }
    if (auto group = widget.group("Offset/Power/Scale (luma)", true))
    {
        group.slider("Luma Offset (Shadows)", mColorOffsetScalar, -1.f, 1.f);
        group.slider("Luma Power (Midtones)", mColorPowerScalar, -1.f, 1.f);
        group.slider("Luma Scale (Hilights)", mColorScaleScalar, -1.f, 1.f);
        if (group.button("reset this group"))
        {
            mColorOffsetScalar = 0.f;
            mColorPowerScalar = 0.f;
            mColorScaleScalar = 0.f;
        }
    }
    if (auto group = widget.group("Offset/Power/Scale (color)", true))
    {
        if (group.button("reset##1"))
            mColorOffset = float3(0.5f, 0.5f, 0.5f);
        group.rgbColor("Color Offset (Shadows)", mColorOffset, true);

        if (group.button("reset##2"))
            mColorPower = float3(0.5f, 0.5f, 0.5f);
        group.rgbColor("Color Power (Midtones)", mColorPower, true);

        if (group.button("reset##3"))
            mColorScale = float3(0.5f, 0.5f, 0.5f);
        group.rgbColor("Color Scale (Hilights)", mColorScale, true);

        if (group.button("reset this group"))
        {
            mColorOffset = float3(0.5f, 0.5f, 0.5f);
            mColorPower = float3(0.5f, 0.5f, 0.5f);
            mColorScale = float3(0.5f, 0.5f, 0.5f);
        }
    }
}
