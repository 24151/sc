#ifndef MYREADKEY_H
#define MYREADKEY_H

typedef enum
{
  KEY_UNKNOWN = 0,
  KEY_UP,
  KEY_DOWN,
  KEY_LEFT,
  KEY_RIGHT,
  KEY_ENTER,
  KEY_ESC,
  KEY_F5,
  KEY_F6,
  KEY_L,
  KEY_S,
  KEY_I,
  KEY_R,
  KEY_T
} keys;

int rk_mytermsave (void);
int rk_mytermrestore (void);
int rk_mytermregime (int regime, int vtime, int vmin, int sigint);
int rk_readkey (keys *key);
int rk_readvalue (int *value, int timeout);

#endif // MYREADKEY_H
