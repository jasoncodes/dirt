/*
    Copyright 2002, 2003 General Software Laboratories
    
    
    This file is part of Dirt Secure Chat.

    Dirt Secure Chat is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Dirt Secure Chat is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Dirt Secure Chat; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#ifndef UTIL_H_
#define UTIL_H_

// fix VC++'s broken "for" scope
#define for if(true)for

// we'll use our own wxPuts function as the built-in fails on some input
#ifdef wxPuts
#undef wxPuts
#endif
#define wxPuts ConsoleOutput

#include "ByteBuffer.h"
class wxDateTime;
class wxConfigBase;

extern const wxString PUBLIC_LIST_URL;

#include "wx/hashmap.h"
WX_DECLARE_STRING_HASH_MAP(wxString, StringHashMap);
WX_DECLARE_STRING_HASH_MAP(ByteBuffer, ByteBufferHashMap);

#include <wx/dynarray.h>
WX_DECLARE_OBJARRAY(wxUint16, Uint16Array);

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
wxString AddCommas(wxLongLong_t size);
wxString AddCommas(double size);
wxString SizeToString(wxLongLong_t size);
wxString SizeToLongString(wxLongLong_t size, wxString suffix = wxEmptyString);
wxString SecondsToMMSS(long seconds, bool milliseconds = false, bool verbose = false);
ByteBuffer Uint64ToBytes(unsigned wxLongLong_t num);
unsigned wxLongLong_t BytesToUint64(const byte *data, int len);
ByteBuffer Uint32ToBytes(wxUint32 num);
wxUint32 BytesToUint32(const byte *data, int len);
ByteBuffer Uint16ToBytes(wxUint16 num);
wxUint16 BytesToUint16(const byte *data, int len);
wxString AppTitle(const wxString &suffix = wxEmptyString);
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
wxDateTime ParseDateTime(const wxString &str, bool okay_to_presume_future = false);
wxString GetSelf();
bool StringToLongLong(const wxString &str, wxLongLong_t *x);
bool StringToULongLong(const wxString &str, unsigned wxLongLong_t *x);
wxString CaseInsensitiveReplace(const wxString &text, const wxString &old_value, const wxString &new_value);
wxString GetOSDescription();
void DebugMsg(const wxString &msg);
void ConsoleOutput(const wxString &line);
void ConsoleOutputUTF8(const wxString &line);

#endif
