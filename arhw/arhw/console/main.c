#include "keys.h"
#include "myReadKey.h"
#include "mySimpleComputer.h"
#include "myTerm.h"
#include "out.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern InOutEntry inoutBuf[INOUT_SIZE];
extern void initInterrupts (void);

int cell = 0;
int exitFlag = 0;
keys key;

int
main (void)
{
  keyscases_init ();
  sc_regInit ();
  sc_memoryInit ();
  sc_accumulatorInit ();
  sc_icounterInit ();
  sc_regSet (FLAG_IGNORE, 1);
  sc_cacheInit ();

  mt_setcursorvisible (1);
  mt_clrscr ();
  printHints ();

  for (int i = 0; i < 128; ++i)
    {
      sc_memorySet (i, i << 7 | i);
    }

  if (rk_mytermsave () != 0)
    {
      fprintf (stderr, "rk_mytermsave failed\n");
      return -1;
    }

  if (rk_mytermregime (0, 0, 1, 0) != 0)
    {
      fprintf (stderr, "rk_mytermregime failed\n");
      return -1;
    }
  mt_setcursorvisible (0);

  initInterrupts ();

  while (!exitFlag)
    {
      printInterface ();
      if (rk_readkey (&key) != 0)
        continue;
      handlekey (key);
      mt_setcursorvisible (0);
    }

  rk_mytermrestore ();
  mt_setcursorvisible (1);
  mt_setdefaultcolor ();
  mt_clrscr ();
  printf ("Выход из программы.\n");
  return 0;
}
