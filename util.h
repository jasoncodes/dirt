#ifndef _UTIL_H
#define _UTIL_H

struct HeadTail
{
	wxString head, tail;
};

wxArrayString SplitString(const wxString &text, const wxString &sep);
bool LeftEq(const wxString &text, const wxString &to_match);
bool RightEq(const wxString &text, const wxString &to_match);
void SplitHeadTail(const wxString &text, wxString &head, wxString &tail);
HeadTail SplitHeadTail(const wxString &text);
wxString Timestamp();

#endif
