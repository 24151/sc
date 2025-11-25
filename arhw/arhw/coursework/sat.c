#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_SIZE 128

typedef struct
{
  const char *mnemonic;
  int opcode;
} MnemonicMap;

static const MnemonicMap opcode_table[]
    = { { "NOP", 0x00 },   { "CPUINFO", 0x01 }, { "READ", 0x0A },
        { "WRITE", 0x0B }, { "LOAD", 0x14 },    { "STORE", 0x15 },
        { "ADD", 0x1E },   { "SUB", 0x1F },     { "DIV", 0x20 },
        { "MUL", 0x21 },   { "JUMP", 0x28 },    { "JNEG", 0x29 },
        { "JZ", 0x2A },    { "HALT", 0x2B },    { "NOT", 0x33 },
        { "AND", 0x34 },   { "RCR", 0x3F },     { "NEG", 0x40 } };

#define OPCODE_COUNT (sizeof (opcode_table) / sizeof (opcode_table[0]))

int
opcode_of (const char *mnemonic)
{
  for (size_t i = 0; i < OPCODE_COUNT; ++i)
    if (strcmp (opcode_table[i].mnemonic, mnemonic) == 0)
      return opcode_table[i].opcode;
  return -1;
}

int
main (int argc, char *argv[])
{
  if (argc != 3)
    {
      fprintf (stderr, "Usage: %s source.sa output.o\n", argv[0]);
      return 1;
    }

  FILE *src = fopen (argv[1], "r");
  if (!src)
    {
      perror ("source file");
      return 1;
    }

  FILE *out = fopen (argv[2], "wb");
  if (!out)
    {
      perror ("output file");
      fclose (src);
      return 1;
    }

  int memory[MEMORY_SIZE] = { 0 };
  char line[256];
  while (fgets (line, sizeof (line), src))
    {
      char *comment = strchr (line, ';');
      if (comment)
        *comment = '\0';

      char addr_str[8], cmd[16], op_str[16];
      if (sscanf (line, "%s %s %s", addr_str, cmd, op_str) < 2)
        continue;

      int addr = atoi (addr_str);
      if (addr < 0 || addr >= MEMORY_SIZE)
        {
          fprintf (stderr, "Invalid address: %d\n", addr);
          continue;
        }

      if (strcmp (cmd, "=") == 0)
        {
          int val = atoi (op_str);
          memory[addr] = val;
        }
      else
        {
          int opcode = opcode_of (cmd);
          if (opcode == -1)
            {
              fprintf (stderr, "Unknown command: %s\n", cmd);
              continue;
            }
          int operand = atoi (op_str);
          if (operand < 0 || operand >= MEMORY_SIZE)
            {
              fprintf (stderr, "Invalid operand at address %d\n", addr);
              continue;
            }
          memory[addr] = (opcode << 7) | (operand & 0x7F);
        }
    }

  fwrite (memory, sizeof (int), MEMORY_SIZE, out);
  fclose (src);
  fclose (out);
  return 0;
}
