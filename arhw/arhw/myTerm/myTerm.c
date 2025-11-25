#include "myTerm.h"
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

int
mt_clrscr (void)
{
  printf ("\033[H\033[J");
  fflush (stdout);
  return 0;
}

int
mt_gotoXY (int row, int col)
{
  if (row < 0 || col < 0)
    return -1;
  printf ("\033[%d;%dH", row, col);
  fflush (stdout);
  return 0;
}

int
mt_getscreensize (int *rows, int *cols)
{
  if (!rows || !cols)
    return -1;
  struct winsize ws;
  if (ioctl (STDOUT_FILENO, TIOCGWINSZ, &ws) < 0)
    return -1;
  *rows = ws.ws_row;
  *cols = ws.ws_col;
  return 0;
}

int
mt_setfgcolor (enum colors color)
{
  if (color < BLACK || color > WHITE)
    return -1;
  printf ("\033[3%dm", color);
  fflush (stdout);
  return 0;
}

int
mt_setbgcolor (enum colors color)
{
  if (color < BLACK || color > WHITE)
    return -1;
  printf ("\033[4%dm", color);
  fflush (stdout);
  return 0;
}

int
mt_setdefaultcolor (void)
{
  printf ("\033[0m");
  fflush (stdout);
  return 0;
}

int
mt_setcursorvisible (int value)
{
  if (value)
    printf ("\033[?25h");
  else
    printf ("\033[?25l");
  fflush (stdout);
  return 0;
}

int
mt_delline (void)
{
  printf ("\033[2K");
  fflush (stdout);
  return 0;
}
