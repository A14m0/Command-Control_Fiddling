/* Defines authentication related constants */

// defines authentication structures
typedef struct _auth {
    char uname[16];
    char passwd[64];
} auth_t, *pauth_t;