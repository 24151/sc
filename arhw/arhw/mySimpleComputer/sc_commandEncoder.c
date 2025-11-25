#include "mySimpleComputer.h"
#include <stdio.h>

static const int valid_cmd_tbl[128] = {
  /* IN-OUT */
  [0x00] = 1, /*  NOP     */
  [0x01] = 1, /*  CPUINFO */
  [0x0A] = 1, /*  READ    */
  [0x0B] = 1, /*  WRITE   */
  /* L/S Accumulator */
  [0x14] = 1, /*  LOAD    */
  [0x15] = 1, /*  STORE   */
  /* Аrithmetic functions */
  [0x1E] = 1, /*  ADD     */
  [0x1F] = 1, /*  SUB     */
  [0x20] = 1, /*  DIVIDE  */
  [0x21] = 1, /*  MUL     */
  /* Transfer control */
  [0x28] = 1, /*  JUMP    */
  [0x29] = 1, /*  JNEG    */
  [0x2A] = 1, /*  JZ      */
  [0x2B] = 1, /*  HALT    */
  /* User func */
  [0x33] = 1, /*  NOT     */
  [0x34] = 1, /*  AND     */
  [0x35] = 1, /*  OR      */
  [0x36] = 1, /*  XOR     */
  [0x37] = 1, /*  JNS     */
  [0x38] = 1, /*  JC      */
  [0x39] = 1, /*  JNC     */
  [0x3A] = 1, /*  JP      */
  [0x3B] = 1, /*  JNP     */
  [0x3C] = 1, /*  CHL     */
  [0x3D] = 1, /*  SHR     */
  [0x3E] = 1, /*  RCL     */
  [0x3F] = 1, /*  RCR     */
  [0x40] = 1, /*  NEG     */
  [0x41] = 1, /*  ADDC    */
  [0x42] = 1, /*  SUBC    */
  [0x43] = 1, /*  LOGLC   */
  [0x44] = 1, /*  LOGRC   */
  [0x45] = 1, /*  RCCL    */
  [0x46] = 1, /*  RCCR    */
  [0x47] = 1, /*  MOVA    */
  [0x48] = 1, /*  MOVR    */
  [0x49] = 1, /*  MOVCA   */
  [0x4A] = 1, /*  MOVCR   */
  [0x4B] = 1, /*  ADDC    */
  [0x4C] = 1  /*  SUBC    */
};

int
sc_commandEncode (int sign, int command, int operand, int *value)
{
  if (value == NULL)
    {
      fprintf (stderr, "Error: the value pointer is NULL.\n");
      return -1;
    }
  if (command < 0 || command > 127)
    {
      fprintf (
          stderr,
          "Warning: The command must be in the range 0-127. Received: %d\n",
          command);
      return -1;
    }
  if (operand < 0 || operand > 127)
    {
      fprintf (
          stderr,
          "Warning: The operand must be in the range 0-127. Received: %d\n",
          operand);
      return -1;
    }
  *value = (sign & 1) << 14;
  *value |= (command & 0x7F) << 7;
  *value |= (operand & 0x7F);

  return 0;
};

int
sc_commandDecode (int value, int *sign, int *command, int *operand)
{
  if (!sign || !command || !operand)
    {
      return -1;
    }
  *sign = value & 0x4000;
  *command = value >> 7 & 0x7f;
  *operand = value & 0x7F;
  return 0;
}

int
isvalidcommands (void)
{
  for (int i = 0; i < 128; ++i)
    {
      int raw, sign, cmd, op;
      if (sc_memoryGet (i, &raw, 0) != 0)
        continue;
      if (sc_commandDecode (raw, &sign, &cmd, &op) != 0)
        continue;
      if (sign == 0 && (cmd < 0 || cmd >= 128 || !valid_cmd_tbl[cmd]))
        {
          sc_regSet (FLAG_INVALIDCMD, 1);
          return 1;
        }
    }
  return 0;
}