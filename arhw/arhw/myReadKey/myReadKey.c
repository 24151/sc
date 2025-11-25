#include "myReadKey.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

static struct termios saved_attributes;

int
rk_mytermsave (void)
{
  return tcgetattr (STDIN_FILENO, &saved_attributes);
}

int
rk_mytermrestore (void)
{
  return tcsetattr (STDIN_FILENO, TCSANOW, &saved_attributes);
}

int
rk_mytermregime (int regime, int vtime, int vmin, int sigint)
{
  struct termios new_attr;
  if (tcgetattr (STDIN_FILENO, &new_attr) < 0)
    return -1;

  if (regime == 0)
    {
      new_attr.c_lflag &= ~(ICANON | ECHO);
    }
  else
    {
      new_attr.c_lflag |= (ICANON | ECHO);
    }
  new_attr.c_cc[VTIME] = vtime;
  new_attr.c_cc[VMIN] = vmin;

  if (!sigint)
    new_attr.c_lflag &= ~ISIG;

  return tcsetattr (STDIN_FILENO, TCSANOW, &new_attr);
}

int
rk_readkey (keys *key)
{
  if (!key)
    return -1;

  char buf[16] = { 0 };
  int n = read (STDIN_FILENO, buf, sizeof (buf));
  if (n <= 0)
    return -1;

  *key = KEY_UNKNOWN;

  if (buf[0] != '\033')
    {
      if (buf[0] == '\n' || buf[0] == '\r')
        *key = KEY_ENTER;
      else if (buf[0] == 'l' || buf[0] == 'L')
        *key = KEY_L;
      else if (buf[0] == 's' || buf[0] == 'S')
        *key = KEY_S;
      else if (buf[0] == 'i' || buf[0] == 'I')
        *key = KEY_I;
      else if (buf[0] == 'r' || buf[0] == 'R')
        *key = KEY_R;
      else if (buf[0] == 't' || buf[0] == 'T')
        *key = KEY_T;
      else
        *key = KEY_UNKNOWN;
    }
  else
    {
      if (n == 1)
        {
          *key = KEY_ESC;
        }
      else if (n >= 3 && buf[1] == '[')
        {
          if (buf[2] == 'A')
            *key = KEY_UP;
          else if (buf[2] == 'B')
            *key = KEY_DOWN;
          else if (buf[2] == 'C')
            *key = KEY_RIGHT;
          else if (buf[2] == 'D')
            *key = KEY_LEFT;
          else if (n >= 5 && buf[2] == '1' && buf[3] == '5' && buf[4] == '~')
            *key = KEY_F5;
          else if (n >= 5 && buf[2] == '1' && buf[3] == '7' && buf[4] == '~')
            *key = KEY_F6;
          else
            *key = KEY_UNKNOWN;
        }
    }

  return 0;
}

int
rk_readvalue (int *value, int timeout)
{
  if (!value)
    return -1;

  char buffer[64] = { 0 };
  size_t pos = 0;

  time_t start = time (NULL);

  while (1)
    {
      fd_set fds;
      struct timeval tv;

      FD_ZERO (&fds);
      FD_SET (STDIN_FILENO, &fds);

      tv.tv_sec = 0;
      tv.tv_usec = 100000; // 100ms опрос

      int ret = select (STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
      if (ret < 0)
        return -1;

      if (ret > 0 && FD_ISSET (STDIN_FILENO, &fds))
        {
          char ch;
          ssize_t n = read (STDIN_FILENO, &ch, 1);
          if (n <= 0)
            return -1;

          if (ch == '\n' || ch == '\r')
            break;

          if (pos < sizeof (buffer) - 1)
            buffer[pos++] = ch;
        }

      if (timeout > 0 && (time (NULL) - start) >= timeout)
        return -1;
    }

  buffer[pos] = '\0';
  *value = atoi (buffer);
  return 0;
}
