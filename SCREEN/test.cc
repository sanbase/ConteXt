#include "screen.h"

main()
{
	Terminal *term;
	char *mess="Hello World!";

	struct clr color_frame,color_text;
	bzero(&color_frame,sizeof color_frame);
	bzero(&color_text,sizeof color_text);

	color_frame.fg=14;
	color_frame.bg=03;

	color_text.fg=15;
	color_text.bg=6;

	term = new Terminal();  // start the terminal mode
	term->Set_Color(1,14);  // set color, background=blue, foreground=yellow
	term->clean();          // clean the screen

	term->Set_Font(5,3);
	term->dpp(10,3);
	term->dps("Hello World!");

	term->BOX(20,10,strlen(mess)+2,3,' ',&color_frame,&color_text);
	term->dpp(21,11);       // put cursor in position 21,11
	term->dps(mess);        // write this string
	// draw rectangle, x=19,y=9,w=16,h=5,color=12(red)
	term->Put_Object_Scr(RECT,19,9,16,5,12);
	int ch=term->dpi();     // wait user action, ch is code of the pressed button

//        term->screen->

	delete term;            // finish
	return 0;
}
