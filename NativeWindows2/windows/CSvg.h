#pragma once

namespace NativeWindows
{
	class D2dWindow;

	class NATIVEWINDOWS2_API CSvg
	{
	public:

		CSvg(D2dWindow* win, CompositionDrawingSurface const& surface, IStream* stream = nullptr) :
			win_(win), surface_(surface)
		{
			if (stream != nullptr)
			{
				SetSvg(stream);
			}
		}

		virtual ~CSvg() 
		{ 
			ReleaseResources();
		};

		void SetSvg(IStream* stream);
		void WINAPI DrawSvg(int width, int height);

	protected:

		CSvg() = delete;
		// can we make copy operataion?

		inline void ReleaseResources()
		{
			SafeRelease(&root_);
			SafeRelease(&doc_);
			SafeRelease(&stream_);
		}

		D2dWindow* win_ = nullptr;
		CompositionDrawingSurface surface_{ nullptr };

		IStream* stream_ = nullptr;
		ID2D1SvgDocument* doc_ = nullptr;
		ID2D1SvgElement* root_ = nullptr;

		//not used yet.
		D2D1_MATRIX_3X2_F transform_ = D2D1::IdentityMatrix();
	};
}