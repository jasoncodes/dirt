#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: File.cpp,v 1.6 2003-05-16 10:19:41 jason Exp $)

#include "File.h"

#if USE_WIN32_FILE
	#include <windows.h>
	#include <wx/msw/winundef.h>
#else
	#include <sys/stat.h>
#endif

#if USE_WIN32_FILE

	// Why is it that people don't run late versions of Platform SDK? :)
	#ifndef INVALID_SET_FILE_POINTER
		#define INVALID_SET_FILE_POINTER ((DWORD)-1)
	#endif

	static inline wxLongLong_t MakeLongLong(long hi, unsigned long lo)
	{
		return wxLongLong((long)hi, (unsigned long)lo).GetValue();
	}

	static inline wxLongLong_t MakeLongLong(unsigned long hi, unsigned long lo)
	{
		return wxLongLong((long)hi, (unsigned long)lo).GetValue();
	}

#endif

wxLongLong_t File::Length(const wxString &filename)
{
	#if USE_WIN32_FILE
		WIN32_FIND_DATA wfd;
		HANDLE hFindFile;
		hFindFile = FindFirstFile(filename, &wfd);
		if (hFindFile != INVALID_HANDLE_VALUE &&
			(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY)
		{
			return MakeLongLong(wfd.nFileSizeHigh, wfd.nFileSizeLow);
		}
	#else
		wxStructStat st;
		if (wxStat(filename, &st) == 0)
		{
			return st.st_size;
		}
	#endif
	return -1;
}

File::File()
{
	#if USE_WIN32_FILE
		m_hFile = INVALID_HANDLE_VALUE;
	#endif
}

File::~File()
{
	#if USE_WIN32_FILE
		Close();
	#endif
}

bool File::Create(const wxString &filename, bool overwrite)
{
	#if USE_WIN32_FILE
		m_hFile = CreateFile(filename, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, overwrite?CREATE_ALWAYS:CREATE_NEW, 0, 0);
		return m_hFile != INVALID_HANDLE_VALUE;
	#else
		return m_file.Create(filename, overwrite);
	#endif
}

#if USE_WIN32_FILE

	static DWORD GetDesiredAccess(File::OpenMode mode)
	{
		switch (mode)
		{

			case File::read:
				return GENERIC_READ;

			case File::write:
			case File::write_append:
			case File::write_excl:
				return GENERIC_WRITE;

			case File::read_write:
				return GENERIC_READ|GENERIC_WRITE;

			default:
				wxFAIL_MSG(wxT("Unknown File::OpenMode"));
				return DWORD(-1);

		}
	}

	static DWORD GetShareMode(File::OpenMode mode)
	{
		switch (mode)
		{

			case File::read:
			case File::write:
			case File::read_write:
			case File::write_append:
				return FILE_SHARE_READ | FILE_SHARE_WRITE;

			case File::write_excl:
				return 0;

			default:
				wxFAIL_MSG(wxT("Unknown File::OpenMode"));
				return DWORD(-1);

		}
	}

#else

	static wxFile::OpenMode TranslateOpenMode(File::OpenMode mode)
	{
		switch (mode)
		{
			case File::read:
				return wxFile::read;
			case File::write:
				return wxFile::write;
			case File::read_write:
				return wxFile::read_write;
			case File::write_append:
				return wxFile::write_append;
			case File::write_excl:
				return wxFile::write_excl;
			default:
				wxFAIL_MSG(wxT("Unknown File::OpenMode"));
				return wxFile::OpenMode(-1);
		}
	}

#endif

bool File::Open(const wxString &filename, OpenMode mode)
{
	#if USE_WIN32_FILE
		m_hFile = CreateFile(filename, GetDesiredAccess(mode), GetShareMode(mode), NULL, OPEN_ALWAYS, 0, 0);
		if (m_hFile != INVALID_HANDLE_VALUE && mode == write_append)
		{
			Seek(0, wxFromEnd);
		}
		return m_hFile != INVALID_HANDLE_VALUE;
	#else
		return m_file.Open(filename, TranslateOpenMode(mode));
	#endif
}

bool File::Close()
{
	#if USE_WIN32_FILE
		BOOL b = CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
		return (b == TRUE);
	#else
		return m_file.Close();
	#endif
}

bool File::IsOpen()
{
	#if USE_WIN32_FILE
		return (m_hFile != INVALID_HANDLE_VALUE);
	#else
		return m_file.IsOpened();
	#endif
}

bool File::IsEof()
{
	#if USE_WIN32_FILE
		return Tell() >= Length();
	#else
		return m_file.Eof();
	#endif
}

off_t File::Read(const byte *ptr, const off_t len)
{
	#if USE_WIN32_FILE
		DWORD num_read;
		if (ReadFile(m_hFile, (LPVOID)ptr, len, &num_read, NULL) != 0)
		{
			return num_read;
		}
		return wxInvalidOffset;
	#else
		return m_file.Read((void*)ptr, len);
	#endif
}

off_t File::Write(const byte *ptr, const off_t len)
{
	#if USE_WIN32_FILE
		DWORD num_written;
		if (WriteFile(m_hFile, (LPVOID)ptr, len, &num_written, NULL) != 0)
		{
			return num_written;
		}
		return wxInvalidOffset;
	#else
		return m_file.Write((void*)ptr, len);
	#endif
}

bool File::Flush()
{
	#if USE_WIN32_FILE
		return (FlushFileBuffers(m_hFile) != 0);
	#else
		return m_file.Flush();
	#endif
}

#if USE_WIN32_FILE
	static DWORD TranslateSeekMode(wxSeekMode mode)
	{
		switch (mode)
		{
			case wxFromStart:
				return FILE_BEGIN;
			case wxFromCurrent:
				return FILE_CURRENT;
			case wxFromEnd:
				return FILE_END;
			default:
				wxFAIL_MSG(wxT("Unknown wxSeekMode"));
				return DWORD(-1);
		}
	}
#endif

wxLongLong_t File::Seek(wxLongLong_t pos, wxSeekMode mode)
{
	#if USE_WIN32_FILE
		wxLongLong ll(pos);
		LONG hi = ll.GetHi();
		LONG lo = SetFilePointer(m_hFile, ll.GetLo(), &hi, TranslateSeekMode(mode));
		if ((DWORD)lo != INVALID_SET_FILE_POINTER || GetLastError() == NO_ERROR)
		{
			return MakeLongLong(hi, lo);
		}
		return -1;
	#else
		return m_file.Seek(pos, mode);
	#endif
}

wxLongLong_t File::Tell()
{
	#if USE_WIN32_FILE
		return Seek(0, wxFromCurrent);
	#else
		return m_file.Tell();
	#endif
}

wxLongLong_t File::Length()
{
	#if USE_WIN32_FILE
		DWORD hi;
		DWORD lo = GetFileSize(m_hFile, &hi);
		if (lo != INVALID_FILE_SIZE || GetLastError() == NO_ERROR)
		{
			return MakeLongLong(hi, lo);
		}
		return -1;
	#else
		return m_file.Length();
	#endif
}
