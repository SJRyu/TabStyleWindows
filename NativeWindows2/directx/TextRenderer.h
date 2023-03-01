#pragma once

namespace NativeWindows
{
    class TextDrawingEffectBase;
    class TextDrawingEffect;
    class Win32Window;

    class DECLSPEC_UUID("4327AC14-3172-4807-BF40-02C7475A2520") TextRenderer : 
        public ComBase<QiListSelf<TextRenderer, QiList<IDWriteTextRenderer>>>
    {
    public:

        HRESULT static Create(
            IDWriteFactory7* wrfactory, ID2D1Factory7* d2dfactory, ID2D1DeviceContext6* dc, Win32Window* win,
            OUT TextRenderer** tr, const TextDrawingEffectBase* effect = nullptr);

        // IDWriteTextRenderer implementation
        IFACEMETHOD(DrawGlyphRun)(
            void* clientDrawingContext,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            DWRITE_MEASURING_MODE measuringMode,
            const DWRITE_GLYPH_RUN* glyphRun,
            const DWRITE_GLYPH_RUN_DESCRIPTION* glyphRunDescription,
            IUnknown* clientDrawingEffect
            );

        IFACEMETHOD(DrawUnderline)(
            void* clientDrawingContext,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            const DWRITE_UNDERLINE* underline,
            IUnknown* clientDrawingEffect
            );

        IFACEMETHOD(DrawStrikethrough)(
            void* clientDrawingContext,
            FLOAT baselineOriginX,
            FLOAT baselineOriginY,
            const DWRITE_STRIKETHROUGH* strikethrough,
            IUnknown* clientDrawingEffect
            );

        IFACEMETHOD(DrawInlineObject)(
            void* clientDrawingContext,
            FLOAT originX,
            FLOAT originY,
            IDWriteInlineObject* inlineObject,
            BOOL isSideways,
            BOOL isRightToLeft,
            IUnknown* clientDrawingEffect
            );

        IFACEMETHOD(IsPixelSnappingDisabled)(
            void* clientDrawingContext,
            OUT BOOL* isDisabled
            );

        IFACEMETHOD(GetCurrentTransform)(
            void* clientDrawingContext,
            OUT DWRITE_MATRIX* transform
            );

        IFACEMETHOD(GetPixelsPerDip)(
            void* clientDrawingContext,
            OUT FLOAT* pixelsPerDip
            );

    protected:

        TextRenderer() = delete;
        TextRenderer(IDWriteFactory7* wrfactory, ID2D1Factory7* d2dfactory, ID2D1DeviceContext6* dc, Win32Window* win);
        virtual ~TextRenderer();

        IDWriteFactory7* dwritefactory_ = NULL;
        ID2D1Factory7* d2d1factory_ = NULL; 
        ID2D1DeviceContext6* dc_ = NULL;
        ID2D1SolidColorBrush* brush_ = NULL;
        ID2D1SolidColorBrush* olbrush_ = NULL;

        TextDrawingEffect* effect_ = NULL;
        
        Win32Window* win_ = NULL;
        HMONITOR hmonitor_ = NULL;

        HRESULT CreateBrush();
        void UpdateBrush(const TextDrawingEffectBase* effect);
    };
}
