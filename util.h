#ifndef UTIL_H_
#define UTIL_H_

// fix VC++'s broken "for" scope
#define for if(true)for

#include "ByteBuffer.h"
class wxHtmlWinParser;
class wxDateTime;

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
wxString GetLongTimestamp();
wxString GetShortTimestamp();
wxString FormatISODateTime(const wxDateTime &datetime);
void FixBorder(wxWindow *wnd);
void SetHtmlParserFonts(wxHtmlWinParser *parser);
wxString AddCommas(off_t size);
wxString AddCommas(double size);
wxString SizeToString(off_t size);
wxString SizeToLongString(off_t size, wxString suffix = wxEmptyString);
wxString SecondsToMMSS(long seconds);
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
ByteBufferArray Unpack(const ByteBuffer &packed_array, size_t max_segments = 0);
ByteBuffer PackHashMap(const StringHashMap &hashmap);
StringHashMap UnpackHashMap(const ByteBuffer &packed_hashmap);
const byte* findbytes(const byte *buff, size_t buff_len, const byte *lookfor, size_t lookfor_len);
wxLongLong_t GetMillisecondTicks();
wxString GetPublicListURL();
bool OpenBrowser(wxWindow *parent, const wxString &URL, bool show_error = true);
void ForceForegroundWindow(wxFrame *frm);

#endif
