#ifndef UTIL_H_
#define UTIL_H_

// fix VC++'s broken "for" scope
#define for if(true)for

#include "ByteBuffer.h"
class wxHtmlWinParser;
class wxDateTime;
class wxConfigBase;

#include "wx/hashmap.h"
WX_DECLARE_STRING_HASH_MAP(wxString, StringHashMap);
WX_DECLARE_STRING_HASH_MAP(ByteBuffer, ByteBufferHashMap);

struct HeadTail
{
	wxString head, tail;
};

wxArrayString SplitString(const wxString &text, const wxString &sep);
wxArrayString SplitQuotedString(const wxString &text, const wxString &sep = wxT(" "));
wxString JoinArray(const wxArrayString &array, const wxString &sep, const wxString &prefix = wxEmptyString, const wxString &postfix = wxEmptyString);
bool LeftEq(const wxString &text, const wxString &to_match);
bool RightEq(const wxString &text, const wxString &to_match);
void SplitHeadTail(const wxString &text, wxString &head, wxString &tail, const wxString &sep = wxT(" "));
HeadTail SplitHeadTail(const wxString &text, const wxString &sep = wxT(" "));
void SplitQuotedHeadTail(const wxString &text, wxString &head, wxString &tail, const wxString &sep = wxT(" "));
HeadTail SplitQuotedHeadTail(const wxString &text, const wxString &sep = wxT(" "));
wxString StripQuotes(const wxString &text);
wxString GetLongTimestamp();
wxString GetShortTimestamp();
wxString FormatISODateTime(const wxDateTime &datetime);
void FixBorder(wxWindow *wnd);
void SetHtmlParserFonts(wxHtmlWinParser *parser);
wxString AddCommas(off_t size);
wxString AddCommas(double size);
wxString SizeToString(off_t size);
wxString SizeToLongString(off_t size, wxString suffix = wxEmptyString);
wxString SecondsToMMSS(long seconds, bool milliseconds = false, bool verbose = false);
ByteBuffer Uint64ToBytes(unsigned wxLongLong_t num);
unsigned wxLongLong_t BytesToUint64(const byte *data, int len);
ByteBuffer Uint32ToBytes(wxUint32 num);
wxUint32 BytesToUint32(const byte *data, int len);
ByteBuffer Uint16ToBytes(wxUint16 num);
wxUint16 BytesToUint16(const byte *data, int len);
wxString AppTitle(const wxString &suffix = wxEmptyString);
void ShowAbout();
ByteBuffer EncodeMessage(const wxString &context, const wxString &cmd, const ByteBuffer &data);
bool DecodeMessage(const ByteBuffer &msg, wxString &context, wxString &cmd, ByteBuffer &data);
ByteBuffer Pack(const ByteBuffer &x, const ByteBuffer &y);
bool Unpack(const ByteBuffer &data, ByteBuffer &x, ByteBuffer &y);
ByteBuffer Pack(const ByteBuffer &x, const ByteBuffer &y, const ByteBuffer &z);
bool Unpack(const ByteBuffer &data, ByteBuffer &x, ByteBuffer &y, ByteBuffer &z);
ByteBuffer Pack(const ByteBufferArray &array);
ByteBuffer Pack(const wxArrayString &array);
ByteBufferArray Unpack(const ByteBuffer &packed_array, size_t max_segments = 0);
wxArrayString ByteBufferArrayToArrayString(const ByteBufferArray &array);
ByteBuffer PackStringHashMap(const StringHashMap &hashmap);
StringHashMap UnpackStringHashMap(const ByteBuffer &packed_hashmap);
ByteBuffer PackByteBufferHashMap(const ByteBufferHashMap &hashmap);
ByteBufferHashMap UnpackByteBufferHashMap(const ByteBuffer &packed_hashmap);
const byte* findbytes(const byte *buff, size_t buff_len, const byte *lookfor, size_t lookfor_len);
wxLongLong_t GetMillisecondTicks();
wxString GetPublicListURL();
bool OpenBrowser(wxWindow *parent, const wxString &URL, bool show_error = true);
void ForceForegroundWindow(wxFrame *frm);
void GetWindowState(const wxFrame *frm, wxRect& r, bool& maximized);
void SetWindowState(wxFrame *frm, const wxRect &r, const bool maximized, bool show = false);
void SaveWindowState(const wxFrame *frm, wxConfigBase *cfg, const wxString &name = wxEmptyString);
void RestoreWindowState(wxFrame *frm, wxConfigBase *cfg, const wxString &name = wxEmptyString, bool show = false, bool default_maximized = false);
wxDateTime ParseDateTime(const wxString &str, bool okay_to_presume_future = false);
wxString GetSelf();
bool StringToLongLong(const wxString &str, wxLongLong_t *x);
bool StringToULongLong(const wxString &str, unsigned wxLongLong_t *x);

#endif
