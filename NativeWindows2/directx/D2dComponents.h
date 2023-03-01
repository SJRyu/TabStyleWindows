#pragma once

#include <NativeWindows2/template/templates.h>
#include <NativeWindows2/directx/IndependentRes.h>

namespace NativeWindows
{
	using namespace winrt;
	using namespace Windows::System;
	using namespace Windows::Graphics;
	using namespace Windows::UI;
	using namespace Windows::UI::Composition;
	using namespace Windows::UI::Composition::Desktop;
	namespace abicomp = ABI::Windows::UI::Composition;
	
	class Win32Window;
	class IndependentRes;

	class NATIVEWINDOWS2_API D2dComponents
	{
	public:

		enum : INT32
		{
			VIRTUALSURFACE_MAXPIXEL = 2 << 23,
		};

		Win32Window* window_ = nullptr;
		IndependentRes* refres_ = nullptr;

		DesktopWindowTarget target_{ nullptr };
		VisualCollection parentv_{ nullptr };

		ContainerVisual rootv_{ nullptr };
		ContainerVisual topv_{ nullptr };
		ContainerVisual bottomv_{ nullptr };

		VisualCollection visuals_{ nullptr };

		Vector3KeyFrameAnimation animemove_{ nullptr };
		Vector2KeyFrameAnimation animesize_{ nullptr };

		void InitCompositor();
		void InitCompositor1(D2dComponents* parent);

		void ReleaseCompositor();
		void ReleaseCompositor1();

		inline Windows::UI::Composition::Compositor Compositor()
		{
			return refres_->compositor_;
		}

		inline ShapeVisual AddShapeVisual(float w = 0, float h = 0)
		{
			auto visual = refres_->compositor_.CreateShapeVisual();
			if (w == 0 && h == 0)
			{
				visual.RelativeSizeAdjustment({ 1.0f, 1.0f });
			}
			else
			{
				visual.Size({ w, h });
			}

			visuals_.InsertAtTop(visual);

			return visual;
		}

		template <typename T>
		inline SpriteVisual AddColorVisual(Windows::UI::Color const& color, T w, T h)
		{
			return AddColorVisual(color, (float)w, (float)h);
		}

		inline SpriteVisual AddColorVisual(Windows::UI::Color const& color, float w = 0.f, float h = 0.f)
		{
			auto visual = refres_->compositor_.CreateSpriteVisual();
			if (w == 0 || h == 0)
			{
				visual.RelativeSizeAdjustment({ 1.0f, 1.0f });
			}
			else
			{
				visual.Size({ w, h });
			}

			auto brush = refres_->compositor_.CreateColorBrush(color);
			visual.Brush(brush);
			visuals_.InsertAtTop(visual);

			return visual;
		}

		inline SpriteVisual AddColorVisual1(
			ContainerVisual parent, Color const& color, float w = 0.f, float h = 0.f)
		{
			auto visual = refres_->compositor_.CreateSpriteVisual();
			if (w == 0 || h == 0)
			{
				visual.RelativeSizeAdjustment({ 1.0f, 1.0f });
			}
			else
			{
				visual.Size({ w, h });
			}

			auto brush = refres_->compositor_.CreateColorBrush(color);

			visual.Brush(brush);
			parent.Children().InsertAtTop(visual);

			return visual;
		}

		template <typename T>
		inline SpriteVisual AddD2dVisual(
			OUT CompositionDrawingSurface& surface, 
			T w, T h, bool stretch = true)
		{
			return AddD2dVisual(refout, (float)w, (float)h, stretch);
		}

		inline SpriteVisual AddD2dVisual(
			OUT CompositionDrawingSurface& surface,
			float w, float h, bool stretch = true)
		{
			auto visual = refres_->compositor_.CreateSpriteVisual();
			if (stretch)
			{
				visual.RelativeSizeAdjustment({ 1.0f, 1.0f });
			}
			else
			{
				visual.Size({ w, h });
			}

			surface = CreateDrawingSurface(refres_->graphics_, { w, h });
			auto brush = refres_->compositor_.CreateSurfaceBrush(surface);
			brush.Stretch(CompositionStretch::Fill);

			visual.Brush(brush);
			visuals_.InsertAtTop(visual);

			return visual;
		}

		inline SpriteVisual AddD2dVisual1(
			OUT CompositionDrawingSurface& surface,
			ContainerVisual parent, float w, float h, bool stretch = true)
		{
			auto visual = refres_->compositor_.CreateSpriteVisual();
			if (stretch)
			{
				visual.RelativeSizeAdjustment({ 1.0f, 1.0f });
			}
			else
			{
				visual.Size({ w, h });
			}

			surface = CreateDrawingSurface(refres_->graphics_, { w, h });
			
			auto brush = refres_->compositor_.CreateSurfaceBrush(surface);
			brush.Stretch(CompositionStretch::Fill);
			visual.Brush(brush);

			parent.Children().InsertAtTop(visual);

			return visual;
		}

		inline SpriteVisual AddD2dVirtualSurface(
			OUT CompositionVirtualDrawingSurface& surface,
			INT32 w = VIRTUALSURFACE_MAXPIXEL, INT32 h = VIRTUALSURFACE_MAXPIXEL, bool stretch = true)
		{
			auto visual = refres_->compositor_.CreateSpriteVisual();
			if (stretch)
			{
				visual.RelativeSizeAdjustment({ 1.0f, 1.0f });
			}
			else
			{
				visual.Size({ (float)w, (float)h });
			}

			surface = CreateVirtualDrawingSurface(refres_->graphics_, { w, h });
			auto brush = refres_->compositor_.CreateSurfaceBrush(surface);
			brush.Stretch(CompositionStretch::Fill);
			visual.Brush(brush);
			visuals_.InsertAtTop(visual);

			return visual;
		}

		inline void ResizeD2dSurface(IN CompositionDrawingSurface& surface, LONG width, LONG height)
		{
			width = std::max(1l, width);
			height = std::max(1l, height);

			surface.Resize({ width, height });
		}

		inline void ResizeD2dSurface(IN CompositionVirtualDrawingSurface& surface, LONG width, LONG height)
		{
			width = std::max(1l, width);
			height = std::max(1l, height);

			surface.Resize({ width, height });
		}

		inline auto BeginDraw(
			OUT ID2D1DeviceContext** d2dc, IN CompositionDrawingSurface& surface)
		{
			//surface 사이즈가 0일때, 에러가 리턴된다. 릴리즈에서는 현재의 예외출력이 아닌 드로우가 완료된것으로 처리해야 한다.
			auto interop = surface.as<abicomp::ICompositionDrawingSurfaceInterop>();
			HR(interop->BeginDraw(nullptr, __uuidof(ID2D1DeviceContext), (void**)d2dc, &offset_));

			(*d2dc)->SetTransform(D2D1::Matrix3x2F::Translation((FLOAT)offset_.x, (FLOAT)offset_.y));
		}

		inline auto BeginDraw(
			OUT ID2D1DeviceContext** d2dc, IN CompositionDrawingSurface& surface,
			OUT POINT* offset, OUT const RECT* rect)
		{
			//surface 사이즈가 0일때, 에러가 리턴된다. 릴리즈에서는 현재의 예외출력이 아닌 드로우가 완료된것으로 처리해야 한다.
			auto interop = surface.as<abicomp::ICompositionDrawingSurfaceInterop>();
			HR(interop->BeginDraw(rect, __uuidof(ID2D1DeviceContext), (void**)d2dc, offset));

			(*d2dc)->SetTransform(D2D1::Matrix3x2F::Translation((FLOAT)offset_.x, (FLOAT)offset_.y));
		}

		inline void EndDraw(CompositionDrawingSurface& surface)
		{
			auto interop = surface.as<abicomp::ICompositionDrawingSurfaceInterop>();
			HR(interop->EndDraw());
		}
#if 1
		inline auto BeginDraw1(CompositionDrawingSurface& surface)
		{
			//surface 사이즈가 0일때, 에러가 리턴된다. 릴리즈에서는 현재의 예외출력이 아닌 드로우가 완료된것으로 처리해야 한다.
			auto interop = surface.as<abicomp::ICompositionDrawingSurfaceInterop>();
			HR(interop->BeginDraw(nullptr, __uuidof(bmsurface_.get()), bmsurface_.put_void(), &offset_));
			
			HR(bitmapdc_->CreateBitmapFromDxgiSurface(bmsurface_.get(), bmprops_, bitmap_.put()));
			bitmapdc_->SetTarget(bitmap_.get());
			bitmapdc_->SetTransform(D2D1::Matrix3x2F::Translation((FLOAT)offset_.x, (FLOAT)offset_.y));
			bitmapdc_->BeginDraw();
			return bitmapdc_.get();
		}

		inline auto BeginDraw1(CompositionDrawingSurface& surface,
			POINT* offset, const RECT* rect)
		{
			//surface 사이즈가 0일때, 에러가 리턴된다. 릴리즈에서는 현재의 예외출력이 아닌 드로우가 완료된것으로 처리해야 한다.
			auto interop = surface.as<abicomp::ICompositionDrawingSurfaceInterop>();
			HR(interop->BeginDraw(rect, __uuidof(bmsurface_.get()), bmsurface_.put_void(), offset));

			HR(bitmapdc_->CreateBitmapFromDxgiSurface(bmsurface_.get(), bmprops_, bitmap_.put()));
			bitmapdc_->SetTarget(bitmap_.get());
			bitmapdc_->SetTransform(D2D1::Matrix3x2F::Translation((FLOAT)offset_.x, (FLOAT)offset_.y));
			bitmapdc_->BeginDraw();
			return bitmapdc_.get();
		}

		inline void EndDraw1(CompositionDrawingSurface& surface)
		{
			auto interop = surface.as<abicomp::ICompositionDrawingSurfaceInterop>();
			bitmapdc_->EndDraw();
			HR(interop->EndDraw());
			bitmapdc_->SetTarget(nullptr);
			bitmap_ = nullptr;
			bmsurface_ = nullptr;
		}
#endif

		template <typename T>
		inline auto MoveVisuals(T x, T y)
		{
			rootv_.Offset({ (float)x, (float)y, 0.0f });
		}

		template <typename T>
		inline auto SizeVisuals(T w, T h)
		{
			rootv_.Size({ (float)w, (float)h });
		}

		template <typename T>
		inline auto AnimeMoveVisuals(T x, T y)
		{
			animemove_.InsertKeyFrame(1.0f, { (float)x, (float)y, 0.0f });
			rootv_.StartAnimation(L"Offset", animemove_);
		}

		template <typename T>
		inline auto AnimeSizeVisuals(T w, T h)
		{
			animesize_.InsertKeyFrame(1.0f, { (float)w, (float)h });
			rootv_.StartAnimation(L"Size", animesize_);
		}

		inline auto PlaceAtTop()
		{
			parentv_.Remove(rootv_);
			parentv_.InsertAtTop(rootv_);
		}

	protected:

		D2dComponents(Win32Window* window);
		virtual ~D2dComponents();

		D2dComponents() = delete;
		D2dComponents(const D2dComponents& r) = delete;
		D2dComponents& operator=(const D2dComponents& r) = delete;
		
		POINT offset_;
		D2D1::Matrix3x2F mat_;
#if 1
		com_ptr<ID2D1DeviceContext6> bitmapdc_;
		com_ptr<IDXGISurface2> bmsurface_;
		com_ptr<ID2D1Bitmap1> bitmap_;
		D2D1_BITMAP_PROPERTIES1 bmprops_;

		//DXGI_FORMAT_B8G8R8A8_UNORM
		void CreateBitmapDc();
#endif

	};	

}
