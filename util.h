#ifndef UTIL_H_
#define UTIL_H_

class wxHtmlWinParser;
class ByteBuffer;
class wxDateTime;

struct HeadTail
{
	wxString head, tail;
};

wxArrayString SplitString(const wxString &text, const wxString &sep);
bool LeftEq(const wxString &text, const wxString &to_match);
bool RightEq(const wxString &text, const wxString &to_match);
void SplitHeadTail(const wxString &text, wxString &head, wxString &tail, const wxString &sep = " ");
HeadTail SplitHeadTail(const wxString &text, const wxString &sep = " ");
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

#endif
