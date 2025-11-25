#include "myBigChars.h"
#include "mySimpleComputer.h"
#include "myTerm.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int bigchars[18][2] = {
  { 0xC3C3C37E, 0x7EC3C3C3 }, // 0
  { 0x18181E1C, 0x18181818 }, // 1
  { 0xC0C3663C, 0xFF061860 }, // 2
  { 0x7EC0C07E, 0x7EC0C0C0 }, // 3
  { 0xFFC3C3C3, 0xC0C0C0C0 }, // 4
  { 0x7F0303FF, 0x7FC0C0C0 }, // 5
  { 0x7F03037E, 0x7EC3C3C3 }, // 6
  { 0x3060C0FF, 0x0C0C0C18 }, // 7
  { 0x7EC3C37E, 0x7EC3C3C3 }, // 8
  { 0xC3C3C37E, 0x7EC0C0FE }, // 9
  { 0xC3C36618, 0xC3C3FFC3 }, // A
  { 0x1F63637F, 0x7F636363 }, // B
  { 0x03C3C37E, 0x7EC3C303 }, // C
  { 0xC3C3633F, 0x3F63C3C3 }, // D
  { 0xFF0303FF, 0xFF030303 }, // E
  { 0xFF0303FF, 0x03030303 }, // F
  { 0x18FF1818, 0x18181818 }, // +
  { 0x00000000, 0x000000FF }  // -
};

int
bc_strlen (char *str)
{
  if (!str)
    return 0;
  int len = 0;
  unsigned char *s = (unsigned char *)str;
  while (*s)
    {
      if ((*s & 0x80) == 0)
        {
          s++;
        }
      else if ((*s & 0xE0) == 0xC0)
        {
          s += 2;
        }
      else if ((*s & 0xF0) == 0xE0)
        {
          s += 3;
        }
      else if ((*s & 0xF8) == 0xF0)
        {
          s += 4;
        }
      else
        {
          return 0;
        }
      len++;
    }
  return len;
}

int
bc_printA (char *str)
{
  if (!str)
    return -1;
  const char *enter_alt = "\033(0";
  const char *exit_alt = "\033(B";
  printf ("%s", enter_alt);
  fflush (stdout);
  printf ("%s", str);
  fflush (stdout);
  printf ("%s", exit_alt);
  fflush (stdout);
  return 0;
}

int
bc_box (int x1, int y1, int x2, int y2, enum colors box_fg, enum colors box_bg,
        char *header, enum colors header_fg, enum colors header_bg)
{
  if (x2 < 2 || y2 < 2 || x1 < 1 || y1 < 1)
    return -1;

  mt_setfgcolor (box_fg);
  if (box_bg)
    mt_setbgcolor (box_bg);

  mt_gotoXY (x1, y1);
  bc_printA (ACS_ULCORNER);
  for (int i = 1; i < y2 - 1; ++i)
    bc_printA (ACS_HLINE);
  bc_printA (ACS_URCORNER);

  for (int row = 1; row < x2 - 1; row++)
    {
      mt_gotoXY (x1 + row, y1);
      bc_printA (ACS_VLINE);
      mt_gotoXY (x1 + row, y1 + y2 - 1);
      bc_printA (ACS_VLINE);
    }

  mt_gotoXY (x1 + x2 - 1, y1);
  bc_printA (ACS_LLCORNER);
  for (int i = 1; i < y2 - 1; ++i)
    bc_printA (ACS_HLINE);
  bc_printA (ACS_LRCORNER);

  if (header)
    {
      int header_len = bc_strlen (header);
      int start = y1 + (y2 - header_len) / 2;
      char line_buf[128] = { 0 };

      mt_setfgcolor (header_fg);
      if (box_bg)
        mt_setbgcolor (header_bg);

      snprintf (line_buf, sizeof (line_buf), "\033[%d;%dH%s", x1, start,
                header);
      fputs (line_buf, stdout);
      fflush (stdout);
    }

  mt_setdefaultcolor ();
  return 0;
}

int
bc_setbigcharpos (int *big, int x, int y, int value)
{
  if (!big || x < 0 || x > 7 || y < 0 || y > 7 || (value != 0 && value != 1))
    return -1;
  int pos = x * 8 + y;
  if (pos < 32)
    {
      if (value)
        big[0] |= (1 << pos);
      else
        big[0] &= ~(1 << pos);
    }
  else
    {
      pos -= 32;
      if (value)
        big[1] |= (1 << pos);
      else
        big[1] &= ~(1 << pos);
    }
  return 0;
}

int
bc_getbigcharpos (int *big, int x, int y, int *value)
{
  if (!big || !value || x < 0 || x > 7 || y < 0 || y > 7)
    return -1;
  int pos = x * 8 + y;
  if (pos < 32)
    {
      *value = (big[0] >> pos) & 1;
    }
  else
    {
      pos -= 32;
      *value = (big[1] >> pos) & 1;
    }
  return 0;
}

int
bc_printbigchar (int big[2], int x, int y, enum colors fg, enum colors bg)
{
  int bit = 0;
  if (!big)
    return -1;
  for (int i = 0; i < 8; ++i)
    {
      mt_gotoXY (x + i, y);
      mt_setfgcolor (fg);
      if (bg)
        mt_setbgcolor (bg);
      for (int j = 0; j < 8; j++)
        {
          bc_getbigcharpos (big, i, j, &bit);
          if (bit)
            bc_printA (ACS_CKBOARD);
          else
            printf (" ");
          fflush (stdout);
        }
    }
  mt_setdefaultcolor ();
  return 0;
}

int
bc_bigcharwrite (int fd, int *big, int count)
{
  if (fd < 0 || !big || count <= 0)
    return -1;
  ssize_t bytes = write (fd, big, count * 2 * sizeof (int));
  if (bytes < count * 2 * (ssize_t)sizeof (int))
    return -1;
  return 0;
}

int
bc_bigcharread (int fd, int *big, int need_count, int *count)
{
  if (fd < 0 || !big || need_count <= 0 || !count)
    return -1;
  ssize_t bytes = read (fd, big, need_count * 2 * sizeof (int));
  if (bytes < 0)
    return -1;
  *count = bytes / (2 * sizeof (int));
  return 0;
}

int
bc_printeditbig (int address)
{
  int value = 0, com, op, sign;
  if (sc_memoryGet (address, &value, 0) != 0)
    return -1;
  sc_commandDecode (value, &sign, &com, &op);

  int fg = MAGENTA, bg = BLACK;

  bc_box (7, 62, 12, 45, WHITE, BLACK, "Редактируемая ячейка (увеличено)",
          WHITE, YELLOW);

  int digits[4];
  digits[3] = (op >> 0) & 0xF;
  digits[2] = (op >> 4) & 0xF;
  digits[1] = (com >> 0) & 0xF;
  digits[0] = (com >> 4) & 0xF;

  int x = 9, y = 63;

  if (sign)
    bc_printbigchar (bigchars[17], x, y, RED, BLACK);
  else
    bc_printbigchar (bigchars[16], x, y, RED, BLACK);

  y += 8;
  for (int i = 0; i < 4; ++i)
    {
      bc_printbigchar (bigchars[digits[i]], x, y + i * 9, fg, bg);
    }

  return 0;
}
