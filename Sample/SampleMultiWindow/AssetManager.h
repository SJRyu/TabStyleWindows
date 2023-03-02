#pragma once

namespace NativeWindows
{
	class AssetManager
	{
	public:

		enum ASSET_NUMBER
		{
			ASSET_TEST = 0,

			ASSET_END,
		};

		static inline LPCWSTR ASSETS_[ASSET_END] =
		{
			L"svg/test.svg",//0
		};

		static inline void InitAssets()
		{
			int i;
			for (i = 0; i < ASSET_END; i++)
			{
				stream_[i] = CreateMemStream(ASSETS_[i], &heap_[i]);
			}
		}

		static inline void ReleaseAssets()
		{
			int i;
			for (i = 0; i < ASSET_END; i++)
			{
				SafeRelease(&stream_[i]);

				if (!HeapFree(GetProcessHeap(), 0, heap_[i]))
				{
					HR(GetLastError());
				}
			}
		}

		static inline void GetStream(ASSET_NUMBER num, IStream** out)
		{
			HR(stream_[num]->Clone(out));
		}

	protected:

		AssetManager() = delete;
		~AssetManager() = delete;

		static inline IStream* stream_[ASSET_END];
		static inline LPVOID heap_[ASSET_END];

		static inline IStream* CreateMemStream(LPCWSTR path, LPVOID* heapPtr)
		{
			HANDLE fd = CreateFileW(
				path,
				GENERIC_READ,
				0,
				nullptr,
				OPEN_EXISTING,
				FILE_SHARE_READ,
				nullptr);
			if (fd == INVALID_HANDLE_VALUE)
			{
				HR(GetLastError());
			}

			LARGE_INTEGER size;

			if (!GetFileSizeEx(fd, &size))
			{
				HR(GetLastError());
			}

			HANDLE pool = GetProcessHeap();

			*heapPtr = HeapAlloc(pool, 0, size.QuadPart);

			DWORD rbytes;
			if (!ReadFile(fd, *heapPtr, size.LowPart, &rbytes, nullptr))
			{
				HR(GetLastError());
			}

			IStream* ret = SHCreateMemStream((const BYTE*)(*heapPtr), rbytes);

			return ret;
		}
	};
}
