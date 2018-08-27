#include "StdAfx.h" 
#include <string.h>

char *tgoto(char *CM,int x,int y)
{ 
	static char result[64]; 
	static char added[10]; 
	char *cp = CM; 
	register char *dp = result; 
	register int c; 
	int oncol = 0; 
	register int which = y;

	if (cp == 0)
	{
toohard: 
		return ("OOPS"); 
	} 
	added[0] = 0; 
	while (c = *cp++)
	{
		if (c != '%')
		{
			*dp++ = c; 
			continue; 
		} 
		c = *cp++;
		switch (c)
		{

		case 'd': 
			if (which < 10) 
				goto one; 
			if (which < 100) 
				goto two; 
		case '3': 
			*dp++ = (which / 100) | '0'; 
			which %= 100; 
		case '2': 
two:	 
			*dp++ = which / 10 | '0'; 
one: 
			*dp++ = which % 10 | '0'; 
swap: 
			oncol = 1 - oncol; 
setwhich: 
			which = oncol ? x : y;
			continue; 

		case '+': 
			which += *cp++; 
		case '.': 
			*dp++ = which; 
			goto swap; 

		case 'r': 
			oncol = 1; 
			goto setwhich; 

		case 'i': 
			x++;
			y++;
			which++; 
			continue; 

		case '%': 
			*dp++ = c; 
			continue; 

		default: 
			goto toohard; 
		} 
	} 
	strcpy(dp, added); 
	return (result); 
} 
