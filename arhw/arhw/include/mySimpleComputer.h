#ifndef MYSIMPLECOMPUTER_H
#define MYSIMPLECOMPUTER_H

#define FLAG_INVALIDCMD 0x1
#define FLAG_IGNORE 0x2
#define FLAG_OVERFLOW 0x4
#define FLAG_DIVZERO 0x8
#define FLAG_OUTOFRANGE 0x10

#define INOUT_SIZE 5

/* Функции памяти */
int sc_memoryInit (void);
int sc_memorySet (int address, int value);
int sc_memoryGet (int address, int *value, int inout);
int sc_memorySave (const char *filename);
int sc_memoryLoad (const char *filename);
void sc_editcurrentcell (int address, int inout);
int sc_dataSet (int address, int value);
int sc_dataPeek (int address, int *value, int inout);

/* Функции регистров */
int sc_regInit (void);
int sc_regSet (int flag, int value);
int sc_regGet (int flag, int *value);
int sc_accumulatorInit (void);
int sc_accumulatorSet (int value);
int sc_accumulatorGet (int *value);
int sc_icounterInit (void);
int sc_icounterSet (int value);
int sc_icounterGet (int *value);

/* Команды */
int sc_commandEncode (int sign, int command, int operand, int *value);
int sc_commandDecode (int value, int *sign, int *command, int *operand);
void inoutAdd (int address, char type, int value);
void sc_editaccumulator (void);
void sc_editicounter (int *cell);
void invers (int *value);
int isvalidcommands ();

/*Функции процессора*/
void initInterrupts (void);
void startExecutionMode (void);
void stopExecutionMode (void);
void sc_cacheInit (void);
int sc_readtocache (int addressOfLine);
int sc_getcacheline (int *line, int *tag, int index);
int sc_cacheWrite (int address, int value);
int sc_cacheRead (int address, int *value);
int sc_cacheFlush (int line);
int sc_updateCacheCell (int address, int value);

typedef struct
{
  int address;
  char type;
  int value;
} InOutEntry;

#endif