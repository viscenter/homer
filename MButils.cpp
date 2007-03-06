// Updated July 23, 2003
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

#include "MButils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


FILE * MBopenFile(char *name, char *mode)
{

	FILE *fp = fopen(name, mode);
	if (fp == NULL)
	{
		fprintf(stderr, "Can't open file %s \n", name);
		exit(-1);
	}

	return(fp);
}


char *MBupperCase(char *s)
{

	if (!s)
		return "\0";

	for(unsigned int i=0; i < strlen(s); i++)
	{
		s[i] = toupper(s[i]);

		if (s[i] == '\n')
			s[i] = '\0';
	}

	return(s);

}

bool MBcompareStr(char *var, char *string)
{

	if (strncmp(var, string, strlen(string)) == 0)
		return true;
	else
		return false;

}

char * MBlowerCase(char *s)
{

	if (!s)
		return "\0";

	for(unsigned int i=0; i < strlen(s); i++)
	{
		s[i] = tolower(s[i]);
		if (s[i] == '\n')
			s[i] = '\0';
	}

	return(s);
}
