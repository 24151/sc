#include "out.h"
#include "myBigChars.h"
#include "mySimpleComputer.h"
#include "myTerm.h"
#include <stdio.h>

extern int t_count;
extern int reg_flags;
InOutEntry inoutBuf[INOUT_SIZE];
int inoutCount = 0;

int
printInterface (void)
{
  int cell;
  sc_icounterGet (&cell);
  printMem (cell);
  printAccumulator ();
  printFlags ();
  printCounters ();
  printCMD ();
  printInOut ();
  printCache ();
  mt_gotoXY (30, 1);
  fflush (stdout);
  return 0;
}

int
printHints (void)
{
  int row = 19, col = 78;
  bc_box (row, col, 7, 29, WHITE, BLACK, "Клавиши", RED, BLACK);
  mt_gotoXY (row + 1, col + 1);
  printf ("l - load s - save i - reset");
  mt_gotoXY (row + 2, col + 1);
  printf ("r - run t - step");
  mt_gotoXY (row + 3, col + 1);
  printf ("ESC - выход");
  mt_gotoXY (row + 4, col + 1);
  printf ("F5 - accumulator");
  mt_gotoXY (row + 5, col + 1);
  printf ("F6 - instruction counter");
  return 0;
}

int
printCache (void)
{
  int row = 19, col = 1;
  int line[10], tag;
  int sign, com, op;
  bc_box (row, col, 7, 65, WHITE, BLACK, "Кеш процессора", RED, BLACK);
  for (int i = 0; i < 5; ++i)
    {
      sc_getcacheline (line, &tag, i);
      mt_gotoXY (row + 1 + i, col + 1);
      printf ("%02d:", tag * 1);
      for (int x = 0; x < (10 * (tag < 12) + ((tag == 12) * 8)); ++x)
        {
          sc_commandDecode (line[x], &sign, &com, &op);
          printf (" %c%02X%02X", sign ? '-' : '+', com, op);
        }
      if (tag == 12)
        printf ("            ");
    }
  return 0;
}

void
printFlags (void)
{
  bc_box (1, 84, 3, 23, WHITE, BLACK, "Регистр флагов", RED, BLACK);
  mt_setfgcolor (RED);
  mt_gotoXY (2, 89);
  printf ("%c  %c  %c  %c  %c\n", (reg_flags >> 4) & 1 ? 'P' : '_',
          (reg_flags >> 3) & 1 ? '0' : '_', (reg_flags >> 2) & 1 ? 'M' : '_',
          (reg_flags >> 1) & 1 ? 'T' : '_', reg_flags & 1 ? 'E' : '_');
  mt_setdefaultcolor ();
}

void
printAccumulator (void)
{
  int value, sign, cmd, oper;
  sc_accumulatorGet (&value);
  sc_commandDecode (value, &sign, &cmd, &oper);
  bc_box (1, 62, 3, 22, WHITE, BLACK, "Аккумулятор", RED, BLACK);
  mt_gotoXY (2, 63);
  printf ("sc: %c%02X%02X hex: %c%04X", sign ? '-' : '+', cmd, oper,
          sign ? '-' : '+', value);
}

void
printCounters (void)
{
  int value, dec = t_count;
  char sign;
  sc_icounterGet (&value);
  bc_box (4, 62, 3, 22, WHITE, BLACK, "Счетчик команд", RED, BLACK);
  mt_gotoXY (5, 63);
  if (value >> 14)
    {
      sign = '-';
    }
  else
    {
      sign = '+';
    }
  printf ("T: %02d\tIC: %c%04X", dec, sign, value);
}

void
printDecodedCommand (int value)
{
  int sign, command, operand;
  sc_commandDecode (value, &sign, &command, &operand);
  printf ("%c%02X%02X", sign ? '-' : '+', command, operand);
  return;
}

void
printCMD (void)
{
  int ic;
  bc_box (4, 84, 3, 23, WHITE, BLACK, "Команда", RED, BLACK);
  mt_gotoXY (5, 90);
  if (sc_icounterGet (&ic) != 0)
    {
      printf ("!+FF : FF");
      return;
    }

  if (ic < 0 || ic >= 128)
    {
      printf ("!+FF : FF");
      return;
    }

  int value;
  if (sc_memoryGet (ic, &value, 0) != 0)
    {
      printf ("!+FF : FF");
      return;
    }
  int sign, cmd, operand;
  if (sc_commandDecode (value, &sign, &cmd, &operand) != 0)
    {
      printf ("!+FF : FF");
      return;
    }
  char s = (sign == 0) ? '+' : '-';
  printf ("%c %02X : %02X", s, cmd, operand);
}

void
inoutAdd (int address, char type, int value)
{
  if (inoutCount == INOUT_SIZE)
    {
      for (int i = 0; i < INOUT_SIZE - 1; ++i)
        {
          inoutBuf[i] = inoutBuf[i + 1];
        }
      inoutCount = INOUT_SIZE - 1;
    }
  inoutBuf[inoutCount].address = address;
  inoutBuf[inoutCount].type = type;
  inoutBuf[inoutCount].value = value;
  inoutCount++;
}

void
printInOut (void)
{
  int row = 19, col = 66;
  bc_box (row, col, 7, 12, WHITE, BLACK, "IN--OUT", RED, BLACK);
  for (int i = 0; i < inoutCount; ++i)
    {
      int address = inoutBuf[i].address;
      char type = inoutBuf[i].type;
      int value = inoutBuf[i].value;

      mt_gotoXY (row + 1 + i, col + 1);
      if (type)
        {
          printf ("%03d%c ", address, type);
          printDecodedCommand (value);
        }
    }
}

void
printCell (int address, enum colors fg, enum colors bg)
{
  int value, com, op, sign;
  if (((int)fg > -1) & ((int)fg < 8))
    {
      mt_setfgcolor (fg);
    }

  if (((int)bg > -1) & ((int)bg < 8))
    {
      if (bg)
        mt_setbgcolor (bg);
    }

  if (sc_memoryGet (address, &value, 0) == -1)
    {
      printf ("ERROR");
      return;
    }
  sc_commandDecode (value, &sign, &com, &op);
  printf ("%c%02X%02X", sign ? '-' : '+', com, op);
  fflush (stdout);
  mt_setdefaultcolor ();
}

void
printMem (int edit)
{
  bc_box (1, 1, 15, 61, WHITE, BLACK, "Оперативная память", RED, BLACK);
  mt_gotoXY (2, 2);
  int count = 0;
  int BG = BLACK;
  int FG = WHITE;
  for (int i = 0; i < 128; ++i)
    {
      if (i == edit)
        {
          printCell (i, BG, FG);
        }
      else
        {
          printCell (i, FG, BG);
        }

      if ((i + 1) % 10 == 0 && i != 0)
        {
          count += 1;
          mt_gotoXY (2 + count, 2);
        }
      else
        {
          printf (" ");
        }
    }
  isvalidcommands ();
  printEditCell (edit);
  bc_printeditbig (edit);
}

void
printBin (int value)
{
  int bits = 15;
  if (value < 0)
    {
      value *= -1;
      value |= 0x4000;
    }
  for (int i = bits - 1; i >= 0; --i)
    {
      if (i == bits - 1)
        {
          mt_setfgcolor (RED);
        }
      else if (i >= bits - 8)
        {
          mt_setfgcolor (CYAN);
        }
      else
        {
          mt_setfgcolor (MAGENTA);
        }
      printf ("%d", (value >> i) & 1);
    }

  mt_setdefaultcolor ();
}

void
printOct (int value)
{
  int res = value;
  printf ("%05o", res);
}

void
printHex (int value)
{
  int res = value;
  printf ("%04X", res);
}

void
printEditCell (int address)
{
  int value = 0, sign, temp, cmd, op;
  sc_memoryGet (address, &value, 0);
  sc_commandDecode (value, &sign, &cmd, &op);
  temp = value;
  if (sign)
    {
      temp -= 1;
      invers (&temp);
    }
  bc_box (16, 1, 3, 61, WHITE, BLACK, "Редактируемая ячейка (формат)", WHITE,
          YELLOW);
  mt_gotoXY (17, 2);
  printf ("dec: %c%05d | oct: ", sign ? '-' : '+', temp);
  printOct (value);
  printf (" | hex: ");
  printHex (value);
  printf ("\tbin: ");
  printBin (value);
}

void
invers (int *value)
{
  int out = 0;
  for (int i = 0; i < 15; ++i)
    {
      if (((*value) & (1 << i)) == 0)
        {
          out += (1 << i);
        }
    }
  out = out & 0x3FFF;
  *value = out;
}