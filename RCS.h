#ifndef RCS_H_
#define RCS_H_

// use this at the top of every CPP file like: RCS_ID($Id: RCS.h,v 1.4 2003-02-16 05:09:03 jason Exp $)
#define RCS_ID(x,y) static int rcs_id_ ## wxMAKE_UNIQUE_ASSERT_NAME = add_rcs_id( wxT( #x ), wxT( #y ) );

// call this when you want to know the calculated info
wxString GetRCSDate();
wxString GetRCSAuthor();
wxString GetProductVersion();

// don't call this directly, used by RCS_ID macro
int add_rcs_id(const wxChar *x, const wxChar *y);

#endif
