#ifndef File_H_
#define File_H_

#ifdef __WXMSW__
	#define USE_WIN32_FILE 0
#else
	#define USE_WIN32_FILE 0
#endif

#if USE_WIN32_FILE
	#include <windows.h>
	#include <wx/msw/winundef.h>
#else
	#include <wx/file.h>
#endif


class File
{

public:
	enum OpenMode { read, write, read_write, write_append, write_excl };

	File();
	~File();

	bool Create(const wxString &filename, bool overwrite = false);
	bool Open(const wxString &filename, OpenMode mode = read);
	bool Close();

	bool IsOpen();
	bool IsEof();

	size_t Read(const byte *ptr, const size_t len);
	size_t Write(const byte *ptr, const size_t len);

	bool Write(const wxString& s, wxMBConv& conv = wxConvUTF8)
	{
		const wxWX2MBbuf buf = s.mb_str(conv);
		size_t size = strlen(buf);
		return Write((const byte*)(const char*)buf, size) == size;
	}

	bool Flush();

	wxLongLong_t Seek(wxLongLong_t pos, wxSeekMode mode = wxFromStart);
	wxLongLong_t Tell();
	wxLongLong_t Length();

	static wxLongLong_t Length(const wxString &filename);

protected:
	#if USE_WIN32_FILE
		HANDLE *m_hFile;
	#else
		wxFile m_file;
	#endif

private:
	DECLARE_NO_COPY_CLASS(File)

};

#endif
