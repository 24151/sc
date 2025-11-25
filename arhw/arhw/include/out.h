#ifndef OUT_H
#define OUT_H

enum colors;

int printHints (void);
int printCache (void);
int printInterface (void);

void printCell (int address, enum colors fg, enum colors bg);
void printDecodedCommand (int value);
void printEditCell (int address);
void printAccumulator (void);
void printCounters (void);
void printBin (int value);
void printOct (int value);
void printHex (int value);
void printMem (int edit);
void printFlags (void);
void printInOut (void);
void printCMD (void);
#endif
