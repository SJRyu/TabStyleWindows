#pragma once

namespace NativeWindows
{
    class TextDrawingEffectBase
    {
    public:
        D2D1_COLOR_F color_;
        D2D1_COLOR_F olcolor_;
        FLOAT olthickness_;
    };

    class DECLSPEC_UUID("1CD7C44F-526B-492a-B780-EF9C4159B653") TextDrawingEffect
        : public TextDrawingEffectBase, public ComBase<QiList<IUnknown> >
    {
    public:
        TextDrawingEffect(D2D1_COLOR_F const& color)
        {
            color_ = color;
            olcolor_ = { 0x00, 0x00, 0x00, 0x00 };
            olthickness_ = 0.f;
        };

        TextDrawingEffect(D2D1_COLOR_F const& color, 
            D2D1_COLOR_F const& olcolor, FLOAT olthickness)
        {
            color_ = color;
            olcolor_ = olcolor_;
            olthickness_ = olthickness;
        };

        TextDrawingEffect& operator=(TextDrawingEffect const& r)
        {
            color_ = r.color_;
            olcolor_ = r.olcolor_;
            olthickness_ = r.olthickness_;

            return *this;
        }

        TextDrawingEffect& operator=(TextDrawingEffectBase const& r)
        {
            color_ = r.color_;
            olcolor_ = r.olcolor_;
            olthickness_ = r.olthickness_;

            return *this;
        }

        TextDrawingEffect& operator=(TextDrawingEffect const&& r) = delete;
    };
}
