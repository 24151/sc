#include "keys.h"
#include "myReadKey.h"
#include "mySimpleComputer.h"
#include "myTerm.h"
#include "out.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void startExecutionMode (void);
extern void stopExecutionMode (void);
extern void nextTick (void);
extern char inoutBuf[5];
extern int exitFlag;
extern int cell;

void (*keyscases[14]) (void) = { NULL };

static inline void
ex_KEY_T (void)
{
  stopExecutionMode ();
  sc_regSet (FLAG_IGNORE, 0);
  nextTick ();
  sc_regSet (FLAG_IGNORE, 1);
}

static inline void
ex_KEY_UP (void)
{
  stopExecutionMode ();
  sc_icounterGet (&cell);
  cell = (cell - 10 + (130)) % (130) - ((cell == 9) * 10) - ((cell == 8) * 10);
  sc_icounterSet (cell);
}

static inline void
ex_KEY_DOWN (void)
{
  stopExecutionMode ();
  sc_icounterGet (&cell);
  cell
      = ((cell + 10) < 128) * (cell + 10) + ((cell + 10) >= 128) * (cell % 10);
  sc_icounterSet (cell);
}

static inline void
ex_KEY_LEFT (void)
{
  stopExecutionMode ();
  sc_icounterGet (&cell);
  cell = (cell + 127) % 128;
  sc_icounterSet (cell);
}

static inline void
ex_KEY_RIGHT (void)
{
  stopExecutionMode ();
  sc_icounterGet (&cell);
  cell = (cell + 1) % 128;
  sc_icounterSet (cell);
}

static inline void
ex_KEY_ENTER (void)
{
  stopExecutionMode ();
  sc_icounterGet (&cell);
  sc_editcurrentcell (cell, 0);
  mt_clrscr ();
  printHints ();
}

static inline void
ex_KEY_F5 (void)
{
  stopExecutionMode ();
  sc_editaccumulator ();
}

static inline void
ex_KEY_F6 (void)
{
  stopExecutionMode ();
  sc_editicounter (&cell);
}

static inline void
ex_KEY_L (void)
{
  char name[51], c;
  char path[sizeof (name) + 8];
  stopExecutionMode ();
  mt_gotoXY (26, 1);
  printf ("Загрузка памяти. . .\t(q - выйти)\nИз файла :");
  mt_setcursorvisible (1);
  rk_mytermrestore ();
  mt_gotoXY (28, 1);

  if (system ("ls -1 memory") == -1)
    perror ("Не удалось выполнить ls");

  mt_gotoXY (27, 11);

  if (scanf ("%50s", name) != 1)
    fprintf (stderr, "Ошибка ввода имени файла\n");

  while ((c = getchar ()) != '\n' && c != EOF)
    ;

  if (name[0] == 'q' && name[1] == '\0')
    {
      rk_mytermregime (0, 0, 1, 0);
      mt_clrscr ();
      printHints ();
      return;
    }
  rk_mytermregime (0, 0, 1, 0);
  mt_setcursorvisible (0);
  snprintf (path, sizeof (path), "memory/%s", name);
  sc_memoryLoad (path);
  fflush (stdout);
  mt_clrscr ();
  printHints ();
}

static inline void
ex_KEY_S (void)
{
  char name[51], c;
  char path[sizeof (name) + 8];
  stopExecutionMode ();
  mt_gotoXY (26, 1);
  printf ("Сохранение памяти...\t(q - выйти)\nВ файл :");
  mt_setcursorvisible (1);
  rk_mytermrestore ();
  if (scanf ("%50s", name) != 1)
    fprintf (stderr, "Ошибка ввода имени файла\n");

  while ((c = getchar ()) != '\n' && c != EOF)
    ;

  if (name[0] == 'q' && name[1] == '\0')
    {
      rk_mytermregime (0, 0, 1, 0);
      mt_clrscr ();
      printHints ();
      return;
    }
  rk_mytermregime (0, 0, 1, 0);
  mt_setcursorvisible (0);
  snprintf (path, sizeof (path), "memory/%s", name);
  sc_memorySave (path);
  fflush (stdout);
  mt_clrscr ();
  printHints ();
}

static inline void
ex_KEY_I (void)
{
  stopExecutionMode ();
  memset (inoutBuf, 0, sizeof (inoutBuf));
  sc_memoryInit ();
  sc_accumulatorInit ();
  sc_regInit ();
  sc_cacheInit ();
  sc_regSet (FLAG_IGNORE, 1);
  mt_clrscr ();
  printHints ();
}

static inline void
ex_KEY_ESC (void)
{
  stopExecutionMode ();
  exitFlag = 1;
};

int
handlekey (int key)
{
  if (!keyscases[key])
    return -1;
  keyscases[key]();
  return 0;
}

void
keyscases_init (void)
{
  keyscases[KEY_UP] = ex_KEY_UP;
  keyscases[KEY_DOWN] = ex_KEY_DOWN;
  keyscases[KEY_LEFT] = ex_KEY_LEFT;
  keyscases[KEY_RIGHT] = ex_KEY_RIGHT;
  keyscases[KEY_ENTER] = ex_KEY_ENTER;
  keyscases[KEY_ESC] = ex_KEY_ESC;
  keyscases[KEY_F5] = ex_KEY_F5;
  keyscases[KEY_F6] = ex_KEY_F6;
  keyscases[KEY_L] = ex_KEY_L;
  keyscases[KEY_S] = ex_KEY_S;
  keyscases[KEY_I] = ex_KEY_I;
  keyscases[KEY_R] = startExecutionMode;
  keyscases[KEY_T] = ex_KEY_T;
}