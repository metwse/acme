#ifndef COMMON_H
#define COMMON_H

#include <ctype.h>
#include <stdio.h>
#include <string.h>


void print_tolower(const char *str)
{
	int len = strlen(str);
	char lowercase[len + 1];

	lowercase[len] = '\0';
	for (int i = 0; i < len; i++)
		lowercase[i] = tolower(str[i]);

	printf("%s", lowercase);
}


#endif
