#ifndef SpanTag_H_
#define SpanTag_H_

#include <wx/html/winpars.h>

class SpanTagHandler : public wxHtmlWinTagHandler
{

public:

	SpanTagHandler();
	virtual ~SpanTagHandler();
	virtual wxString GetSupportedTags();
	static bool ParseColour(const wxString& str, wxColour *clr);
	static wxString ExtractStyle(const wxString &style_tag, const wxString &style);
	virtual bool HandleTag(const wxHtmlTag& tag);

	DECLARE_NO_COPY_CLASS(SpanTagHandler)

};

#endif
