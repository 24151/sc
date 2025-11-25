#include "mySimpleComputer.h"
#include "myTerm.h"
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#define ROTL15(x, n) ((((x) << (n)) | ((x) >> (15 - (n)))) & 0x7FFF)
#define ROTR15(x, n) ((((x) >> (n)) | ((x) << (15 - (n)))) & 0x7FFF)

extern int timeout;
extern int t_count;
extern int memory[128];
extern int printInterface (void);

static int temp;
static int initialized = 0;
static int (*commands[128]) (int operand) = { NULL };

// IN_OUT FUNC START

static inline int
op_NOP (int operand)
{
  (void)operand;
  return 0;
}

static inline int
op_CPUINFO (int operand)
{
  (void)operand;
  mt_gotoXY (27, 1);
  printf ("ИА");
  return 0;
}

static inline int
op_READ (int operand)
{
  int val;
  if (sc_dataPeek (operand, &val, 0) != 0)
    {
      temp -= 1;
      return;
    }
  sc_editcurrentcell (operand, 1);
  return 0;
}

static inline int
op_WRITE (int operand)
{
  int a;
  mt_gotoXY (27, 1);
  printf ("                                               \n                  "
          "                         ");
  mt_gotoXY (27, 1);
  sc_dataPeek (operand, &a, 1);
  printf ("Ячейка №%d hex: %04X, dec:%d", operand + 1, a, a);
  return sc_icounterSet (operand);
}

static inline int
op_LOAD (int operand)
{
  int value = 0;
  sc_dataPeek (operand, &value, 0);
  return sc_accumulatorSet (value);
}

static inline int
op_STORE (int operand)
{
  int value = 0;
  if (sc_accumulatorGet (&value) != 0)
    return -1;
  if (value >> 14)
    {
      invers (&value);
      value = value + 1;
      value = value & 0x3FFF;
      value = -value;
    }
  return sc_dataSet (operand, value);
}

// IN_OUT FUNC END

// ALU FUNC START

static inline int
com_ADD (int operand)
{
  int a, b, res;
  sc_accumulatorGet (&a);
  sc_dataPeek (operand, &b, 0);

  res = a + b;
  if (res < -0x7FFF || res > 0x7FFF)
    {
      sc_regSet (FLAG_OVERFLOW, 1);
      res &= 0x7fff;
    }
  return sc_accumulatorSet (res);
}

static inline int
com_SUB (int operand)
{
  int a, b, res;
  sc_accumulatorGet (&a);
  sc_dataPeek (operand, &b, 0);

  res = a - b;
  if (res < -0x7FFF || res > 0x7FFF)
    {
      sc_regSet (FLAG_OVERFLOW, 1);
      res &= 0x7fff;
    }
  return sc_accumulatorSet (res);
}

static inline int
com_DIV (int operand)
{
  int a, b, res;
  sc_accumulatorGet (&a);
  sc_dataPeek (operand, &b, 0);
  if (!b)
    {
      sc_regSet (FLAG_DIVZERO, 1);
      return -1;
    }
  res = a / b;
  if (res < -0x7FFF || res > 0x7FFF)
    {
      sc_regSet (FLAG_OVERFLOW, 1);
      return -1;
    }
  return sc_accumulatorSet (res);
}

static inline int
com_MUL (int operand)
{
  int a, b, res;
  sc_accumulatorGet (&a);
  sc_dataPeek (operand, &b, 0);

  res = a * b;
  if (res < -0x7FFF || res > 0x7FFF)
    {
      sc_regSet (FLAG_OVERFLOW, 1);
      return -1;
    }
  return sc_accumulatorSet (res);
}

// ALU FUNC END

// TRANSFER FUNC START

static inline int
op_JUMP (int operand)
{
  int val;
  if (sc_dataPeek (operand, &val, 0) != 0)
    {
      temp = operand - 1;
      return -1;
    }
  temp = operand - 1;
  return sc_icounterSet (operand);
}

static inline int
op_JNEG (int operand)
{
  int a, val;
  sc_dataPeek (operand, &val, 0);
  sc_accumulatorGet (&a);
  if (a >> 14)
    {
      temp = operand - 1;
      return sc_icounterSet (operand);
    }
  return 0;
}

static inline int
op_JZ (int operand)
{
  int a;
  sc_accumulatorGet (&a);
  if (a == 0)
    return sc_icounterSet (operand);
  return 0;
}

static inline int
op_HALT (int operand)
{
  (void)operand;
  sc_cacheFlush (-1);
  temp -= 1;
  t_count = 0;
  sc_regSet (FLAG_IGNORE, 1);
  stopExecutionMode ();
  return sc_regSet (FLAG_IGNORE, 1);
}

// TRANSFER FUNC END

// USER FUNC START

static inline int
op_NOT (int operand)
{
  int a;
  sc_accumulatorGet (&a);
  if (a >> 14)
    {
      invers (&a);
      a &= 0x3fff;
    }
  else
    {
      invers (&a);
      a |= 0x4000;
    }
  memory[operand] = a;
  return 0;
}

static inline int
op_AND (int operand)
{
  int a, b;
  sc_accumulatorGet (&a);
  sc_dataPeek (operand, &b, 0);
  return sc_accumulatorSet ((a & b) & 0x7FFF);
}

static inline int
op_RCR (int operand)
{
  int b;
  sc_dataPeek (operand, &b, 0);
  return sc_accumulatorSet (ROTR15 (b, 1));
}

static inline int
op_NEG (int operand)
{
  int b;
  sc_dataPeek (operand, &b, 0);
  invers (&b);
  b += 1;
  if (!(b >> 14))
    b |= 0x4000;
  return sc_accumulatorSet (b);
}

// USER FUNC END

static void
init_commands (void)
{
  commands[0x00] = op_NOP;
  commands[0x01] = op_CPUINFO;
  commands[0x0A] = op_READ;
  commands[0x0B] = op_WRITE;
  commands[0x14] = op_LOAD;
  commands[0x15] = op_STORE;

  commands[0x1E] = com_ADD;
  commands[0x1F] = com_SUB;
  commands[0x20] = com_DIV;
  commands[0x21] = com_MUL;

  commands[0x28] = op_JUMP;
  commands[0x29] = op_JNEG;
  commands[0x2A] = op_JZ;
  commands[0x2B] = op_HALT;

  commands[0x33] = op_NOT;
  commands[0x34] = op_AND;
  commands[0x3F] = op_RCR;
  commands[0x40] = op_NEG;
}

int
execute (int command, int operand)
{
  if (!initialized)
    {
      init_commands ();
      initialized = 1;
    }

  if (command >= 0 && command < 128 && commands[command])
    {
      return commands[command](operand);
    }
  else
    {
      sc_regSet (FLAG_INVALIDCMD, 1);
      return -1;
    }
  return 0;
}

void
CU (void)
{
  if (t_count > 0)
    {
      t_count -= 1;
      return;
    }
  int val, sign, cmd, op;
  sc_icounterGet (&val);
  temp = val;
  if (sc_dataPeek (val, &val, 0) != 0)
    {
      return;
    }

  if (sc_commandDecode (val, &sign, &cmd, &op) != 0)
    return;

  if (cmd < 0 || cmd > 0x7F)
    return;

  if (execute (cmd, op) != 0)
    return;
  sc_icounterSet ((temp + 1) % 128);
}

void
nextTick (void)
{
  int t;
  sc_regGet (FLAG_IGNORE, &t);
  t_count = 0;
  if (!t)
    CU ();
  t_count = 0;
}

void
IRC (int signum)
{
  int t, val;
  sc_icounterGet (&val);
  temp = val;
  sc_regGet (FLAG_IGNORE, &t);
  if (signum == SIGALRM)
    {
      if (!t)
        CU ();
    }
  else if (signum == SIGUSR1)
    {
      nextTick ();
      printInterface ();
    }
  else if (signum == SIGINT)
    {
      stopExecutionMode ();
      mt_clrscr ();
      mt_gotoXY (1, 1);
      printf ("*** Прерывание: выполнение остановлено пользователем ***\n");
    }
}

void
initInterrupts (void)
{
  struct sigaction sa;
  memset (&sa, 0, sizeof (sa));
  sa.sa_handler = IRC;
  sigemptyset (&sa.sa_mask);
  sa.sa_flags = 0;

  if (sigaction (SIGALRM, &sa, NULL) == -1)
    {
      perror ("sigaction SIGALRM");
      exit (1);
    }

  if (sigaction (SIGUSR1, &sa, NULL) == -1)
    {
      perror ("sigaction SIGUSR1");
      exit (1);
    }
  if (sigaction (SIGINT, &sa, NULL) == -1)
    {
      perror ("sigaction SIGINT");
      exit (1);
    }
}

void
startExecutionMode (void)
{
  sc_regSet (FLAG_IGNORE, 0);
  struct itimerval iv = { { 0, timeout }, { 0, timeout } };
  setitimer (ITIMER_REAL, &iv, NULL);
}

void
stopExecutionMode (void)
{
  sc_regSet (FLAG_IGNORE, 1);
  struct itimerval iv = { { 0, 0 }, { 0, 0 } };
  setitimer (ITIMER_REAL, &iv, NULL);
  sc_regSet (FLAG_IGNORE, 1);
}
