/*
			 DBMS ConteXt V.6.0

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Tue Oct 13 03:22:04 2015
			Module:Methods.cc
*/
/*
			 DBMS ConteXt V.5.7

		Autor: Alexander Lashenko, Toronto, Canada
		Reply to: info@unixspace.com
		Last modification:Mon Sep 15 11:54:44 2003
			Module:Methods.cc
*/
/*
==========================================================================
		      Defined variables:

#define OUTPUT  base->share->output     // output message
#define BG      base->share->color.bg   // background color
#define FG      base->share->color.fg   // foreground color
#define RECORD  base->share->record     // current Object number
#define RETURN  base->share->ret        // returned value
#define TAG     base->share->slot       // descripton of the current slot
#define LEN     base->share->slot.l     // length of the current slot
#define SLA     base->share->slot.sla   // structute of the current slot
#define CMD     base->share->cmd        // returned command
#define BLANK   base->share->form.blank // current form/blank name
*/

#include <CX_Methods.h>

// request of value of the virtual field
void Methods::Virtual(long record,struct sla *slot)
{
}

// Methods process ending
void Methods::Abort()
{
}

/*------------------------------------------------------------------*/
/* the next methods are using only by DB-browser.  */

// operations of the user
int Methods::Action(int act)
{
	return(0);
}

// recording value in the current slot
int Methods::Write_Slot(char *str)
{
	return(0);
}

// recording current Object in the ConteXt File
int Methods::Write_Cadr()
{
	return(0);
}
