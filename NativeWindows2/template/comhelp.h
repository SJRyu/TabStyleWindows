#pragma once

namespace Ctemplates
{
	template <typename T>
	void SafeRelease(T** ppT)
	{
		if (*ppT)
		{
			(*ppT)->Release();
			*ppT = nullptr;
		}
	}

	template <typename InterfaceType>
	inline InterfaceType* SafeAcquire(InterfaceType* newObject)
	{
		if (newObject != NULL)
			newObject->AddRef();

		return newObject;
	}

	// Sets a new COM object, releasing the old one.
	template <typename InterfaceType>
	inline void SafeSet(InterfaceType** currentObject, InterfaceType* newObject)
	{
		SafeAcquire(newObject);
		SafeRelease(currentObject);
		*currentObject = newObject;
	}


	// Releases a COM object and nullifies pointer.
	template <typename InterfaceType>
	inline InterfaceType* SafeDetach(InterfaceType** currentObject)
	{
		InterfaceType* oldObject = *currentObject;
		*currentObject = NULL;
		return oldObject;
	}


	// Sets a new COM object, acquiring the reference.
	template <typename InterfaceType>
	inline void SafeAttach(InterfaceType** currentObject, InterfaceType* newObject)
	{
		SafeRelease(currentObject);
		*currentObject = newObject;
	}

	// Generic COM base implementation for classes, since DirectWrite uses
// callbacks for several different kinds of objects, particularly the
// text renderer and inline objects.
//
// Example:
//
//  class RenderTarget : public ComBase<QiList<IDWriteTextRenderer> >
//
	template <typename InterfaceChain>
	class ComBase : public InterfaceChain
	{
	public:
		explicit ComBase() throw()
			: refValue_(0)
		{
			AddRef();
		}

		// IUnknown interface
		IFACEMETHOD(QueryInterface)(IID const& iid, OUT void** ppObject)
		{
			*ppObject = NULL;
			InterfaceChain::QueryInterfaceInternal(iid, ppObject);
			if (*ppObject == NULL)
				return E_NOINTERFACE;

			AddRef();
			return S_OK;
		}

		IFACEMETHOD_(ULONG, AddRef)()
		{
			return InterlockedIncrement(&refValue_);
		}

		IFACEMETHOD_(ULONG, Release)()
		{
			ULONG newCount = InterlockedDecrement(&refValue_);
			if (newCount == 0)
				delete this;

			return newCount;
		}

	protected:

		virtual ~ComBase()
		{ }

		ULONG refValue_;

	private:
		// No copy construction allowed.
		ComBase(const ComBase& b);
		ComBase& operator=(ComBase const&);
	};

	struct QiListNil
	{
	};

	// When the QueryInterface list refers to itself as class,
	// which hasn't fully been defined yet.
	template <typename InterfaceName, typename InterfaceChain>
	class QiListSelf : public InterfaceChain
	{
	public:
		inline void QueryInterfaceInternal(IID const& iid, OUT void** ppObject) throw()
		{
			if (iid != __uuidof(InterfaceName))
				return InterfaceChain::QueryInterfaceInternal(iid, ppObject);

			*ppObject = static_cast<InterfaceName*>(this);
		}
	};


	// When this interface is implemented and more follow.
	template <typename InterfaceName, typename InterfaceChain = QiListNil>
	class QiList : public InterfaceName, public InterfaceChain
	{
	public:
		inline void QueryInterfaceInternal(IID const& iid, OUT void** ppObject) throw()
		{
			if (iid != __uuidof(InterfaceName))
				return InterfaceChain::QueryInterfaceInternal(iid, ppObject);

			*ppObject = static_cast<InterfaceName*>(this);
		}
	};


	// When the this is the last implemented interface in the list.
	template <typename InterfaceName>
	class QiList<InterfaceName, QiListNil> : public InterfaceName
	{
	public:
		inline void QueryInterfaceInternal(IID const& iid, OUT void** ppObject) throw()
		{
			if (iid != __uuidof(InterfaceName))
				return;

			*ppObject = static_cast<InterfaceName*>(this);
		}
	};
}
