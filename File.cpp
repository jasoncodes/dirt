#include "wx/wxprec.h"
#ifdef __BORLANDC__
	#pragma hdrstop
#endif
#ifndef WX_PRECOMP
	#include "wx/wx.h"
#endif
#include "RCS.h"
RCS_ID($Id: File.cpp,v 1.2 2003-05-10 04:34:39 jason Exp $)

#include "File.h"

#if !USE_WIN32_FILE
	#include <sys/stat.h>
#endif

wxLongLong_t File::Length(const wxString &filename)
{
	#if USE_WIN32_FILE
	#else
		wxStructStat st;
		if (wxStat(filename, &st) == 0)
		{
			return st.st_size;
		}
		return -1;
	#endif
}

File::File()
{
	#if USE_WIN32_FILE

	#endif
}

File::~File()
{
	#if USE_WIN32_FILE

	#endif
}

bool File::Create(const wxString &filename, bool overwrite)
{
	#if USE_WIN32_FILE

	#else
		return m_file.Create(filename, overwrite);
	#endif
}

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

bool File::Open(const wxString &filename, OpenMode mode)
{
	#if USE_WIN32_FILE

	#else
		return m_file.Open(filename, TranslateOpenMode(mode));
	#endif
}

bool File::Close()
{
	#if USE_WIN32_FILE

	#else
		return m_file.Close();
	#endif
}

bool File::IsOpen()
{
	#if USE_WIN32_FILE

	#else
		return m_file.IsOpened();
	#endif
}

bool File::IsEof()
{
	#if USE_WIN32_FILE

	#else
		return m_file.Eof();
	#endif
}

size_t File::Read(const byte *ptr, const size_t len)
{
	#if USE_WIN32_FILE

	#else
		return m_file.Read((void*)ptr, len);
	#endif
}

size_t File::Write(const byte *ptr, const size_t len)
{
	#if USE_WIN32_FILE

	#else
		return m_file.Write((void*)ptr, len);
	#endif
}

bool File::Flush()
{
	#if USE_WIN32_FILE

	#else
		return m_file.Flush();
	#endif
}

wxLongLong_t File::Seek(wxLongLong_t pos, wxSeekMode mode)
{
	#if USE_WIN32_FILE

	#else
		return m_file.Seek(pos, mode);
	#endif
}

wxLongLong_t File::Tell()
{
	#if USE_WIN32_FILE

	#else
		return m_file.Tell();
	#endif
}

wxLongLong_t File::Length()
{
	#if USE_WIN32_FILE

	#else
		return m_file.Length();
	#endif
}
