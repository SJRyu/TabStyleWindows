#pragma once
#include <NativeWindows2/windows/D2dWindow.h>

namespace NativeWindows
{
	class CSvg;

	class TestWindow1 : public D2dWindow
	{
	public:

		TestWindow1(WinArgs const& args);
		virtual ~TestWindow1();

	protected:

		std::unique_ptr<CSvg> csvg_;

		SpriteVisual svgvisual_{ nullptr };
		CompositionDrawingSurface svgsurface_{ nullptr };

		virtual LRESULT CALLBACK OnCreate1(LPCREATESTRUCT createstr) override;
		virtual LRESULT CALLBACK OnSize(WPARAM state, int width, int height) override;
		virtual LRESULT CALLBACK OnLbtndown(int state, int x, int y) override;
		virtual VOID CALLBACK OnClose1() override;

	};
}