/*
MB's Untilites
  Chuck Pisula taught me that if you can't think up a name for your objects, just use your
  initials.  Thus MButils.

  Functions:
  FILE *MBopenFile(char *name, char *mode);
  char *MBupperCase(char *s)
  char *MBlowerCase(char *s)
  bool  MBcompareStr(char *var, char *string);

*/

#ifndef _MButils_
#define _MButils_
#include <stdio.h>
#include <ctype.h>


FILE * MBopenFile(char *name, char *mode);
char * MBupperCase(char *s);
char * MBlowerCase(char *s);
bool MBcompareStr(char *var, char *string);

#ifndef uint
typedef unsigned int MBuint;
#endif

#endif
