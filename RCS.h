#ifndef RCS_H_
#define RCS_H_

// use this at the top of every CPP file like: RCS_ID($Id: RCS.h,v 1.3 2003-02-13 14:09:04 jason Exp $)
#define RCS_ID(x,y) static int rcs_id_ ## wxMAKE_UNIQUE_ASSERT_NAME = add_rcs_id( #x, #y );

// call this when you want to know the calculated info
wxString GetRCSDate();
wxString GetRCSAuthor();
wxString GetProductVersion();

// don't call this directly, used by RCS_ID macro
int add_rcs_id(const char *x, const char *y);

#endif
