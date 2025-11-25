#include "myReadKey.h"
#include "mySimpleComputer.h"
#include "myTerm.h"
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

extern int memory[128];
extern void printHints ();

int
sc_memoryInit (void)
{
  for (int i = 0; i < 128; ++i)
    {
      memory[i] = 0;
    }
  return 0;
};

int
sc_memoryLoad (const char *filename)
{
  FILE *f = fopen (filename, "rb");
  if (!f)
    {
      return -1;
    }
  size_t read = fread (memory, sizeof (int), 128, f);
  fclose (f);
  return (read == 128) ? 0 : -1;
};

int
sc_memorySave (const char *filename)
{
  FILE *f = fopen (filename, "wb");
  if (!f)
    {
      return -1;
    }
  size_t written = fwrite (memory, sizeof (int), 128, f);
  fclose (f);
  return (written == 128) ? 0 : -1;
};

int
sc_memorySet (int address, int value)
{
  if (value < -16384 || value > 16384)
    {
      sc_regSet (FLAG_OUTOFRANGE, 1);
      return -1;
    }
  else if (address < 0 || address > 127)
    {
      return -2;
    }
  else if (value < 0)
    {
      int temp = value * -1;
      temp = temp & 0x3FFF;
      invers (&temp);
      temp |= 0x4000;
      temp += 1;
      memory[address] = temp;
    }
  else
    {
      memory[address] = value;
    }
  return 0;
};

int
sc_dataSet (int address, int value)
{
  if (value < -16384 || value > 16384)
    {
      sc_regSet (FLAG_OUTOFRANGE, 1);
      return -1;
    }
  else if (address < 0 || address > 127)
    {
      return -2;
    }
  else if (value < 0)
    {
      int temp = value * -1;
      temp = temp & 0x3FFF;
      invers (&temp);
      temp |= 0x4000;
      temp += 1;
      sc_cacheWrite (address, temp);
    }
  else
    {
      sc_cacheWrite (address, value);
    }
  return 0;
};

int
sc_dataPeek (int address, int *value, int inout)
{
  if (value == NULL || address < 0 || address >= 128)
    {
      return -1;
    }
  if (inout)
    inoutAdd (address, '>', *value);
  return sc_cacheRead (address, value);
}

int
sc_memoryGet (int address, int *value, int inout)
{
  if (value == NULL || address < 0 || address >= 128)
    {
      return -1;
    }
  *value = memory[address];
  if (inout)
    inoutAdd (address, '>', *value);
  return 0;
}

void
sc_editcurrentcell (int address, int inout)
{
  char buf[32];
  int val = 0;
  int row = (address / 10) + 2;
  int col = (address % 10) * 6 + 2;

  mt_gotoXY (26, 1);
  printf ("Ввод . . . ⏳");
  fflush (stdout);

  struct termios oldt, newt;
  tcgetattr (STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag |= (ICANON | ECHO);
  tcsetattr (STDIN_FILENO, TCSANOW, &newt);

  mt_gotoXY (row, col);
  mt_setbgcolor (WHITE);
  mt_setfgcolor (BLACK);
  printf ("     ");
  mt_gotoXY (row, col);
  fflush (stdout);

  if (fgets (buf, sizeof (buf), stdin) != NULL)
    {
      int len = strlen (buf);
      if (len > 0 && buf[len - 1] == '\n')
        buf[--len] = '\0';

      int is_negative = 0;
      int start = 0;
      if (buf[0] == '-')
        {
          is_negative = 1;
          start = 1;
        }

      int count = 0;
      for (int i = start; i + 1 < len && count < 4; i += 2)
        {
          char hexpair[3] = { buf[i], buf[i + 1], '\0' };
          if (!isxdigit (hexpair[0]) || !isxdigit (hexpair[1]))
            break;

          int hexval = (int)strtol (hexpair, NULL, 16);
          if (hexval < 0 || hexval > 0x7F)
            {
              tcsetattr (STDIN_FILENO, TCSANOW, &oldt);
              sc_regSet (FLAG_OUTOFRANGE, 1);
              mt_setdefaultcolor ();
              fflush (stdout);
              return;
            }

          val = (val << 7) | hexval;
          count++;
        }

      if (count > 0)
        {
          if (is_negative && val)
            {
              invers (&val);
              val += 1;
              val *= -1;
              sc_memorySet (address, val);
            }
          else if (is_negative && !val)
            {
              val |= 0x4000;
              memory[address] = val;
            }
          else
            {
              sc_memorySet (address, val);
            }
          if (inout)
            inoutAdd (address, '<', val);
          sc_updateCacheCell (address, val);
          mt_setbgcolor (BLACK);
          mt_setfgcolor (WHITE);
          mt_gotoXY (row, col);
          printf ("+%04X", (unsigned short)(val & 0x3FFF));
        }
    }
  sc_regSet (FLAG_INVALIDCMD, 0);
  tcsetattr (STDIN_FILENO, TCSANOW, &oldt);
  mt_setdefaultcolor ();
  fflush (stdout);
  mt_clrscr ();
  printHints ();
}

void
sc_editaccumulator (void)
{
  int val = 0;
  char buf[32];

  struct termios oldt, newt;
  tcgetattr (STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag |= (ICANON | ECHO);
  tcsetattr (STDIN_FILENO, TCSANOW, &newt);

  mt_gotoXY (2, 67);
  mt_setbgcolor (WHITE);
  mt_setfgcolor (BLACK);
  printf ("     ");
  mt_gotoXY (2, 67);
  fflush (stdout);

  if (fgets (buf, sizeof (buf), stdin) != NULL)
    {
      int len = strlen (buf);
      if (len > 0 && buf[len - 1] == '\n')
        buf[--len] = '\0';

      int is_negative = 0;
      int start = 0;
      if (buf[0] == '-')
        {
          is_negative = 1;
          start = 1;
        }

      int count = 0;
      for (int i = start; i + 1 < len && count < 4; i += 2)
        {
          char hexpair[3] = { buf[i], buf[i + 1], '\0' };
          if (!isxdigit (hexpair[0]) || !isxdigit (hexpair[1]))
            break;

          int hexval = (int)strtol (hexpair, NULL, 16);
          if (hexval < 0 || hexval > 0x7F)
            {
              mt_setdefaultcolor ();
              return;
            }

          val = (val << 7) | hexval;
          count++;
        }

      if (count > 0)
        {
          if (is_negative && val)
            {
              val *= -1;
              sc_accumulatorSet (val);
            }
          else if (is_negative && !val)
            {
              val |= 0x4000;
              sc_accumulatorSet (val);
            }
          else
            {
              sc_accumulatorSet (val);
            }
          mt_setbgcolor (BLACK);
          mt_setfgcolor (WHITE);
          mt_gotoXY (2, 67);
          printf ("+%04X", (unsigned short)(val & 0x3FFF));
        }
    }

  tcsetattr (STDIN_FILENO, TCSANOW, &oldt);
  mt_setdefaultcolor ();
  fflush (stdout);
}

void
sc_editicounter (int *cell)
{
  int val;

  struct termios oldt, newt;
  tcgetattr (STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag |= (ICANON | ECHO);
  tcsetattr (STDIN_FILENO, TCSANOW, &newt);

  mt_gotoXY (5, 77);
  mt_setbgcolor (WHITE);
  mt_setfgcolor (BLACK);
  printf ("     ");
  mt_gotoXY (5, 77);
  fflush (stdout);

  char buf[16];
  if (fgets (buf, sizeof (buf), stdin) != NULL)
    {
      val = atoi (buf);
      sc_icounterSet (val);
      *cell = val;
      mt_setbgcolor (BLACK);
      mt_setfgcolor (WHITE);
      mt_gotoXY (5, 77);
      printf ("+%04X", (unsigned short)(val & 0x3FFF));
    }

  tcsetattr (STDIN_FILENO, TCSANOW, &oldt);
  mt_setdefaultcolor ();
  fflush (stdout);
}