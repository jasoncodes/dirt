#ifndef RCS_H_
#define RCS_H_

int add_rcs_id(const char *rcs_id);

#define RCS_ID(id) static int rcs_id_ ## wxMAKE_UNIQUE_ASSERT_NAME = add_rcs_id( #id );

#endif
