#ifndef RCS_H_
#define RCS_H_

int add_rcs_id(const char *x, const char *y);

#define RCS_ID(x,y) static int rcs_id_ ## wxMAKE_UNIQUE_ASSERT_NAME = add_rcs_id( #x, #y );

#endif
