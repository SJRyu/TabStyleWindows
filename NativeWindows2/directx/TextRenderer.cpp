#include <pch.h>
#include <NativeWindows2/directx/TextRenderer.h>
#include <NativeWindows2/directx/TextDrawingEffect.h>
#include <NativeWindows2/windows/Win32Window.h>

HRESULT TextRenderer::Create(
    IDWriteFactory7* wrfactory, ID2D1Factory7* d2dfactory, ID2D1DeviceContext6* dc, Win32Window* win,
    OUT TextRenderer** tr, const TextDrawingEffectBase* effect)
{
    HRESULT hr = S_OK;
    *tr = NULL;

    auto renderer = new(std::nothrow) TextRenderer(wrfactory, d2dfactory, dc, win);
    if (renderer == NULL)
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        hr = renderer->CreateBrush();
        renderer->UpdateBrush(effect);
        if (hr == S_OK)
        {
            *tr = SafeDetach(&renderer);
        }
        else
        {
            renderer->Release();
        }
    }

    return hr;
}

TextRenderer::TextRenderer(
    IDWriteFactory7* wrfactory, ID2D1Factory7* d2dfactory, ID2D1DeviceContext6* dc, Win32Window* win) :
    dwritefactory_(SafeAcquire(wrfactory)), d2d1factory_(SafeAcquire(d2dfactory)), dc_(SafeAcquire(dc)),
    win_(win)
{
    
}

TextRenderer::~TextRenderer()
{
    SafeRelease(&brush_);
    SafeRelease(&olbrush_);
    SafeRelease(&dc_);
    SafeRelease(&dwritefactory_);
    SafeRelease(&d2d1factory_);
    SafeRelease(&effect_);
}

HRESULT TextRenderer::CreateBrush()
{
    HRESULT hr;
    
    effect_ = new(std::nothrow) TextDrawingEffect(
        D2D1::ColorF(D2D1::ColorF::Black),
        D2D1::ColorF(D2D1::ColorF::White), 0.0f);

    hr = dc_->CreateSolidColorBrush(effect_->color_, &brush_);
    if (hr == S_OK)
    {
        hr = dc_->CreateSolidColorBrush(effect_->olcolor_, &olbrush_);
    }
    return hr;
}

void TextRenderer::UpdateBrush(const TextDrawingEffectBase* effect)
{
    if (effect != NULL)
    {
        *effect_ = *effect;
        brush_->SetColor(effect_->color_);
        olbrush_->SetColor(effect_->olcolor_);
    }
}

HRESULT STDMETHODCALLTYPE TextRenderer::DrawGlyphRun(
    void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    DWRITE_MEASURING_MODE measuringMode,
    const DWRITE_GLYPH_RUN* glyphRun,
    const DWRITE_GLYPH_RUN_DESCRIPTION* glyphRunDescription,
    IUnknown* clientDrawingEffect
)
{
    // need to tell if this is inlineboject by drawingcontext?

    HRESULT hr = S_OK;
    // Since we use our own custom renderer and explicitly set the effect
    // on the layout, we know exactly what the parameter is and can
    // safely cast it directly.
    TextDrawingEffect* effect = (TextDrawingEffect*)(clientDrawingEffect);
    UpdateBrush(effect);
#if 1
    if (effect_->olthickness_ > 0)
    {
        ID2D1PathGeometry* pPathGeometry = NULL;
        ID2D1GeometrySink* pSink = NULL;
        ID2D1TransformedGeometry* pTransformedGeometry = NULL;

        D2D1::Matrix3x2F const matrix = D2D1::Matrix3x2F(
            1.0f, 0.0f,
            0.0f, 1.0f,
            baselineOriginX, baselineOriginY
        );

        hr = d2d1factory_->CreatePathGeometry(&pPathGeometry);
        if (hr != S_OK) goto failed;

        hr = pPathGeometry->Open(&pSink);
        if (hr != S_OK) goto failed;

        hr = glyphRun->fontFace->GetGlyphRunOutline(
            glyphRun->fontEmSize,
            glyphRun->glyphIndices,
            glyphRun->glyphAdvances,
            glyphRun->glyphOffsets,
            glyphRun->glyphCount,
            glyphRun->isSideways,
            glyphRun->bidiLevel % 2,
            pSink
        );
        pSink->Close();
        if (hr != S_OK) goto failed;

        hr = d2d1factory_->CreateTransformedGeometry(
            pPathGeometry,
            &matrix,
            &pTransformedGeometry);
        if (hr != S_OK) goto failed;

        auto thick = win_->DpiVal(effect_->olthickness_);
        dc_->DrawGeometry(pTransformedGeometry,
            olbrush_, thick);
        dc_->FillGeometry(pTransformedGeometry, brush_);
failed:
        SafeRelease(&pPathGeometry);
        SafeRelease(&pSink);
        SafeRelease(&pTransformedGeometry);
    }
    else
    {
        dc_->DrawGlyphRun(
            D2D1::Point2(baselineOriginX, baselineOriginY),
            glyphRun,
            glyphRunDescription,
            brush_,
            measuringMode);
    }
#else
    ID2D1PathGeometry* pPathGeometry = NULL;
    ID2D1GeometrySink* pSink = NULL;
    ID2D1TransformedGeometry* pTransformedGeometry = NULL;

    D2D1::Matrix3x2F const matrix = D2D1::Matrix3x2F(
        1.0f, 0.0f,
        0.0f, 1.0f,
        baselineOriginX, baselineOriginY
    );

    hr = d2d1factory_->CreatePathGeometry(&pPathGeometry);
    if (hr != S_OK) goto failed;

    hr = pPathGeometry->Open(&pSink);
    if (hr != S_OK) goto failed;

    hr = glyphRun->fontFace->GetGlyphRunOutline(
        glyphRun->fontEmSize,
        glyphRun->glyphIndices,
        glyphRun->glyphAdvances,
        glyphRun->glyphOffsets,
        glyphRun->glyphCount,
        glyphRun->isSideways,
        glyphRun->bidiLevel % 2,
        pSink);
    pSink->Close();
    if (hr != S_OK) goto failed;

    hr = d2d1factory_->CreateTransformedGeometry(
        pPathGeometry,
        &matrix,
        &pTransformedGeometry);
    if (hr != S_OK) goto failed;
    
    if (effect_->olthickness_ > 0)
    {
        dc_->DrawGeometry(
            pTransformedGeometry,
            olbrush_, effect_->olthickness_);
    }
    dc_->FillGeometry(
        pTransformedGeometry, brush_);
failed:
    SafeRelease(&pPathGeometry);
    SafeRelease(&pSink);
    SafeRelease(&pTransformedGeometry);
#endif
    return hr;
}


HRESULT STDMETHODCALLTYPE TextRenderer::DrawUnderline(
    void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    const DWRITE_UNDERLINE* underline,
    IUnknown* clientDrawingEffect
)
{
    TextDrawingEffect* effect = (TextDrawingEffect*)(clientDrawingEffect);
    UpdateBrush(effect);

    // We will always get a strikethrough as a LTR rectangle
    // with the baseline origin snapped.
    auto thick = win_->DpiVal(underline->thickness);
    D2D1_RECT_F rectangle =
    {
        baselineOriginX,
        baselineOriginY + underline->offset,
        baselineOriginX + underline->width,
        baselineOriginY + underline->offset + thick
    };

    // Draw this as a rectangle, rather than a line.
    dc_->FillRectangle(&rectangle, brush_);

    return S_OK;
}


HRESULT STDMETHODCALLTYPE TextRenderer::DrawStrikethrough(
    void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    const DWRITE_STRIKETHROUGH* strikethrough,
    IUnknown* clientDrawingEffect
)
{
    TextDrawingEffect* effect = (TextDrawingEffect*)(clientDrawingEffect);
    UpdateBrush(effect);

    // We will always get an underline as a LTR rectangle
    // with the baseline origin snapped.
    auto thick = win_->DpiVal(strikethrough->thickness);
    D2D1_RECT_F rectangle =
    {
        baselineOriginX,
        baselineOriginY + strikethrough->offset,
        baselineOriginX + strikethrough->width,
        baselineOriginY + strikethrough->offset + thick
    };

    // Draw this as a rectangle, rather than a line.
    dc_->FillRectangle(&rectangle, brush_);

    return S_OK;
}


HRESULT STDMETHODCALLTYPE TextRenderer::DrawInlineObject(
    void* clientDrawingContext,
    FLOAT originX,
    FLOAT originY,
    IDWriteInlineObject* inlineObject,
    BOOL isSideways,
    BOOL isRightToLeft,
    IUnknown* clientDrawingEffect
)
{
    //call DrawGlyphRun? then we need drawingcontext?
    inlineObject->Draw(
        nullptr,
        this,
        originX,
        originY,
        false,
        false,
        clientDrawingEffect);

    return S_OK;
}


HRESULT STDMETHODCALLTYPE TextRenderer::IsPixelSnappingDisabled(
    void* clientDrawingContext,
    OUT BOOL* isDisabled
)
{
    // Enable pixel snapping of the text baselines,
    // since we're not animating and don't want blurry text.
    *isDisabled = FALSE;
    return S_OK;
}


HRESULT STDMETHODCALLTYPE TextRenderer::GetCurrentTransform(
    void* clientDrawingContext,
    OUT DWRITE_MATRIX* transform
)
{
    // Simply forward what the real renderer holds onto.
    dc_->GetTransform(reinterpret_cast<D2D1_MATRIX_3X2_F*>(transform));
    return S_OK;
}


HRESULT STDMETHODCALLTYPE TextRenderer::GetPixelsPerDip(
    void* clientDrawingContext,
    OUT FLOAT* pixelsPerDip
)
{
    // Any scaling will be combined into matrix transforms rather than an
    // additional DPI scaling. This simplifies the logic for rendering
    // and hit-testing. If an application does not use matrices, then
    // using the scaling factor directly is simpler.
    *pixelsPerDip = 1;
    return S_OK;
}
