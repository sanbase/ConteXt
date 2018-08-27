#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#ifdef SPARC
 #include "termios.h"
#else
 #include <termios.h>
#endif
#include <signal.h>
#include <unistd.h>
//void exit(int status);

static struct termios oldtio, newtio;
#define CHANNEL STDERR_FILENO                      /* output file descriptor */

void Ttyset ();
void Ttyreset ();

void exit_prog(int i)
{
	Ttyreset();
#ifdef LINUX
	_exit(1);
#else
	exit(1);
#endif
}

static int _set=0;
#ifndef TCGETA
#define TCGETA TIOCGETA
#define TCSETA TIOCSETA
#define TCFLSH TIOCFLUSH
#endif

void Ttyset ()
{
	static   void      (*fsig)(int);
	if(_set) return;
	if (ioctl (CHANNEL, TCGETA, (char *) &oldtio) < 0)
		return;

	newtio = oldtio;
	newtio.c_iflag &= ~(INLCR | ICRNL | IGNCR | ISTRIP);
	newtio.c_oflag &= ~OPOST;      /* no output postprocessing */
	newtio.c_lflag &= ~(ECHO | ICANON);

	newtio.c_cc [VMIN]   =  1;     /* break input after each character */
	newtio.c_cc [VTIME]  =  0;     /* timeout is 0 msecs */
	newtio.c_cc [VINTR]  =  3;     /* interupt after CTRL/C */
//        newtio.c_cc [VQUIT]  = -1;
	tcsetattr(CHANNEL,TCSANOW,&newtio);

	fsig = signal(SIGINT,SIG_IGN);
	if( fsig == SIG_DFL )
		fsig = exit_prog;
	signal( SIGINT, fsig );
	_set=1;
}

void Ttyreset ()
{
	if(!_set) return;
	ioctl (CHANNEL, TCSETA, (char *) &oldtio);
	signal(SIGINT,SIG_DFL);
	_set=0;
}

void TtyFlush ()
{
	if(_set) return;
	ioctl (CHANNEL, TCFLSH, 0);
}
