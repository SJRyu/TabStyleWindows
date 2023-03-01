
/************************************************************************
 *
 * File: CustomTextRenderer.h
 *
 * Description:
 *
 *
 *  This file is part of the Microsoft Windows SDK Code Samples.
 *
 *  Copyright (C) Microsoft Corporation.  All rights reserved.
 *
 * This source code is intended only as a supplement to Microsoft
 * Development Tools and/or on-line documentation.  See these other
 * materials for detailed information regarding Microsoft code samples.
 *
 * THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
 * KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 * PARTICULAR PURPOSE.
 *
 ************************************************************************/

#pragma once
#if 1
namespace NativeWindows
{
	/******************************************************************
	*                                                                 *
	*  CustomTextRenderer                                             *
	*                                                                 *
	*  The IDWriteTextRenderer interface is an input parameter to     *
	*  IDWriteTextLayout::Draw.  This interfaces defines a number of  *
	*  callback functions that the client application implements for  *
	*  custom text rendering.  This sample renderer implementation    *
	*  renders text using text outlines and Direct2D.                 *
	*  A more sophisticated client would also support bitmap          *
	*  renderings.                                                    *
	*                                                                 *
	******************************************************************/
	class NATIVEWINDOWS2_API CustomTextRenderer : public IDWriteTextRenderer
	{
	public:

		static inline void CreateCTextRenderer(
			CustomTextRenderer** renderer, 
			ID2D1Factory* factory = nullptr, ID2D1DeviceContext* dc = nullptr)
		{
			*renderer = new (std::nothrow) CustomTextRenderer(factory, dc);

			(*renderer)->AddRef();
		}

		IFACEMETHOD(IsPixelSnappingDisabled)(
			__maybenull void* clientDrawingContext,
			__out BOOL* isDisabled
			);

		IFACEMETHOD(GetCurrentTransform)(
			__maybenull void* clientDrawingContext,
			__out DWRITE_MATRIX* transform
			);

		IFACEMETHOD(GetPixelsPerDip)(
			__maybenull void* clientDrawingContext,
			__out FLOAT* pixelsPerDip
			);

		IFACEMETHOD(DrawGlyphRun)(
			__maybenull void* clientDrawingContext,
			FLOAT baselineOriginX,
			FLOAT baselineOriginY,
			DWRITE_MEASURING_MODE measuringMode,
			__in DWRITE_GLYPH_RUN const* glyphRun,
			__in DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
			IUnknown* clientDrawingEffect
			);

		IFACEMETHOD(DrawUnderline)(
			__maybenull void* clientDrawingContext,
			FLOAT baselineOriginX,
			FLOAT baselineOriginY,
			__in DWRITE_UNDERLINE const* underline,
			IUnknown* clientDrawingEffect
			);

		IFACEMETHOD(DrawStrikethrough)(
			__maybenull void* clientDrawingContext,
			FLOAT baselineOriginX,
			FLOAT baselineOriginY,
			__in DWRITE_STRIKETHROUGH const* strikethrough,
			IUnknown* clientDrawingEffect
			);

		IFACEMETHOD(DrawInlineObject)(
			__maybenull void* clientDrawingContext,
			FLOAT originX,
			FLOAT originY,
			IDWriteInlineObject* inlineObject,
			BOOL isSideways,
			BOOL isRightToLeft,
			IUnknown* clientDrawingEffect
			);

		IFACEMETHOD_(unsigned long, AddRef) ();
		IFACEMETHOD_(unsigned long, Release) ();
		IFACEMETHOD(QueryInterface) (
			IID const& riid,
			void** ppvObject
			);

		inline void SetFactory(ID2D1Factory* factory)
		{
			factory_ = factory;
		}

		inline void SetDc(ID2D1DeviceContext* dc)
		{
			dc_ = dc;
		}

		inline void SetBrush(
			ID2D1Brush* fill, 
			ID2D1Brush* ulb, 
			ID2D1Brush* mlb, 
			ID2D1Brush* olb, FLOAT olt = 1.0f)
		{
			fillbrush_ = fill;
			ulbrush_ = ulb;
			mlbrush_ = mlb;
			olbrush_ = olb;
			olthickness_ = olt;
		}

		inline void SetFillBrush(ID2D1Brush* brush)
		{
			fillbrush_ = brush;
		}

		inline void SetOutline(ID2D1Brush* brush, FLOAT thickness = 1.0f)
		{
			olbrush_ = brush;
			olthickness_ = thickness;
		}

		inline void SetMidlineBrush(ID2D1Brush* brush)
		{
			mlbrush_ = brush;
		}

		inline void SetUnderlineBrush(ID2D1Brush* brush)
		{
			ulbrush_ = brush;
		}

	protected:
		CustomTextRenderer(ID2D1Factory* factory, ID2D1DeviceContext* dc);
		virtual ~CustomTextRenderer();

	private:
		unsigned long cRefCount_;

		ID2D1Factory* factory_;
		ID2D1DeviceContext* dc_;
		
		ID2D1Brush* fillbrush_;
		ID2D1Brush* olbrush_;
		ID2D1Brush* mlbrush_;
		ID2D1Brush* ulbrush_;

		FLOAT olthickness_ = 1.0f;
	};
}
#endif
