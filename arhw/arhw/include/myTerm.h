#ifndef MYTERM_H
#define MYTERM_H

enum colors
{
  BLACK = 0,
  RED,
  GREEN,
  YELLOW,
  BLUE,
  MAGENTA,
  CYAN,
  WHITE
};

// Функции для красивого вывода

int mt_clrscr (void);
int mt_gotoXY (int row, int col);
int mt_getscreensize (int *rows, int *cols);
int mt_setfgcolor (enum colors color);
int mt_setbgcolor (enum colors color);
int mt_setdefaultcolor (void);
int mt_setcursorvisible (int value);
int mt_delline (void);

#endif
