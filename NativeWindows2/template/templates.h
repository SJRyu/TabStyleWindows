#pragma once

#ifdef _DEBUG
#define dbg_msg(s, ...) {\
	char msg__[128] = { 0, }; \
	sprintf_s(msg__, sizeof(msg__), s, __VA_ARGS__); \
	OutputDebugStringA(msg__); OutputDebugStringA("\n"); \
}
#else
#define dbg_msg(...)	
#endif

#include <cmath>
#include <winerror.h>

namespace Ctemplates
{
	template <typename T>
	T Dpiadj(T value, UINT dpi, UINT xdpi = 96)
	{
		if constexpr (std::is_floating_point_v<T>)
		{
			return (T)((double)value * dpi) / xdpi;
		}
		else
		{
			return (T)round(((double)value * dpi) / xdpi);
		}
	}

#if 1
	struct ComException
	{
		HRESULT result;
		ComException(HRESULT const value) :
			result(value)
		{}
	};

	static __forceinline void HR(const HRESULT result)
	{
		if (result != S_OK)
		{
			throw ComException(result);
		}
	}
#else
	static __forceinline void HR(const HRESULT result)
	{
		check_hresult(result);
	}
#endif 

	static __forceinline void Throwif(bool b, HRESULT hr = E_FAIL)
	{
		if (b) HR(hr);
	}

	static __forceinline void Throwifnot(bool b, HRESULT hr = E_FAIL)
	{
		if (!b) HR(hr);
	}

	static __forceinline void Throwifzero(bool b, HRESULT hr = E_FAIL)
	{
		if (!b) HR(hr);
	}

	template <typename T>
	void SafeDelete(T** ppT)
	{
		if (*ppT)
		{
			delete (*ppT);
			*ppT = nullptr;
		}
	}

	template <typename T>
	void SafeClose(T** ppT)
	{
		if (*ppT)
		{
			CloseHandle(*ppT);
			*ppT = nullptr;
		}
	}

	template <typename T>
	void ClearMem(T* pT)
	{
		ZeroMemory(pT, sizeof(T));
	}

	template <typename T> class refptr
	{
	public:

		refptr() : ptr_(nullptr) { }
		refptr(T* p) : ptr_(p) { }

		refptr(const refptr<T>& r) : ptr_(r.ptr) {}

		T* operator->() { return ptr_; }

		//inline T& operator()() { return *ptr_; }
		//inline operator T& () { return *ptr_; }

		//_NODISCARD operator T* () const { return ptr_; }

	protected:
		T* ptr_;
	};

	template <typename T> struct Tmem
	{
		T* ptr;	UINT size;
	};

	template <typename T> struct Tpoint
	{
		T x; T y;
	};

	// Needed text editor backspace deletion.
	inline bool IsSurrogate(UINT32 ch) throw()
	{
		// 0xD800 <= ch <= 0xDFFF
		return (ch & 0xF800) == 0xD800;
	}


	inline bool IsHighSurrogate(UINT32 ch) throw()
	{
		// 0xD800 <= ch <= 0xDBFF
		return (ch & 0xFC00) == 0xD800;
	}


	inline bool IsLowSurrogate(UINT32 ch) throw()
	{
		// 0xDC00 <= ch <= 0xDFFF
		return (ch & 0xFC00) == 0xDC00;
	}

	// Maps exceptions to equivalent HRESULTs,
	inline HRESULT ExceptionToHResult() throw()
	{
		try
		{
			throw;  // Rethrow previous exception.
		}
		catch (std::bad_alloc&)
		{
			return E_OUTOFMEMORY;
		}
		catch (...)
		{
			return E_FAIL;
		}
	}
}

#include <NativeWindows2/template/comhelp.h>
#include <NativeWindows2/template/CRECT2.h>
#include <NativeWindows2/template/concurrency.h>
