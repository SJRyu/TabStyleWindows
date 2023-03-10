#pragma once

#include <type_traits>
#include <windef.h>
#include <dcommon.h>

namespace Ctemplates
{
	struct WINPOS
	{
		int x;
		int y;
		int cx;
		int cy;
	};

	template <typename T>
	struct TypeRect
	{
		typedef D2D1_RECT_F Rect;
	};

	template <>
	struct TypeRect<LONG>
	{
		typedef tagRECT Rect;
	};

	template <typename T = FLOAT>
	using Rect_t = typename TypeRect<T>::Rect;

	template <typename T = FLOAT>
	struct CRECT : public Rect_t<T>
	{
		static_assert(std::is_arithmetic_v<T>, "WRONG RECT TYPE");
	public:

		using Rect_t<T>::left;
		using Rect_t<T>::top;
		using Rect_t<T>::right;
		using Rect_t<T>::bottom;

		inline CRECT() : Rect_t<T>{ 0, } {}

		inline CRECT(T l, T t, T r, T b) : Rect_t<T>{ l, t, r, b } {}

		inline CRECT(const Rect_t<T>& rect) : Rect_t<T>(rect) {}

		template <typename U>
		CRECT(const Rect_t<U>& rect) :
			Rect_t<T>{ (T)rect.left, (T)rect.top, (T)rect.right, (T)rect.bottom } {}

		template <typename U>
		CRECT(const CRECT<U>& rect) :
			Rect_t<T>{ (T)rect.left, (T)rect.top, (T)rect.right, (T)rect.bottom } {}

		inline CRECT(WINPOS const& pos) :
			CRECT{ (T)pos.x, (T)pos.y, (T)(pos.x + pos.cx), (T)(pos.y + pos.cy) } {}

		inline bool operator==(const CRECT<T>& r)
		{
			if ((r.getwidth() == getwidth()) && (r.getheight() == getheight()))
			{
				return true;
			}
			else
			{
				return false;
			}
		}

		inline bool operator!=(const CRECT<T>& r)
		{
			if ((r.getwidth() == getwidth()) && (r.getheight() == getheight()))
			{
				return false;
			}
			else
			{
				return true;
			}
		}

		inline void MoveToXY(T x, T y)
		{
			T dx = x - left; T dy = y - top;
			OffSetXY(dx, dy);
		}

		inline void MoveToX(T x)
		{
			T dx = x - left;
			OffSetX(dx);
		}

		inline void MoveToY(T y)
		{
			T dy = y - top;
			OffSetY(dy);
		}

		inline void OffSetXY(T x, T y)
		{
			OffSetX(x); OffSetY(y);
		}

		inline void OffSetX(T x)
		{
			right += x;
			left += x;
		}

		inline void OffSetY(T y)
		{
			bottom += y;
			top += y;
		}

		inline void Margin(const Rect_t<T>& margin)
		{
			Margin(margin.left, margin.top, margin.right, margin.bottom);
		}

		inline void Margin(T l, T t, T r, T b)
		{
			MarginH(l, r); MarginV(t, b);
		}

		inline void MarginH(T l, T r)
		{
			if ((l + r) <= width)
			{
				left += l; right -= r;
			}
		}

		inline void MarginV(T t, T b)
		{
			if ((t + b) <= height)
			{
				top += t; bottom -= b;
			}
		}

		__declspec(property(get = getwidth, put = putwidth)) T width;
		__declspec(property(get = getheight, put = putheight)) T height;

		inline T getwidth() const
		{
			return (right - left);
		}

		inline T getheight() const
		{
			return (bottom - top);
		}

		inline void putwidth(T i)
		{
			if (i < 0) i = 0;

			right = left + i;
		}

		inline void putheight(T i)
		{
			if (i < 0) i = 0;

			bottom = top + i;
		}
	};
}
