#include "mySimpleComputer.h"
#include "myTerm.h"
#include <stdio.h>
#include <string.h>

#define CACHE_LINES 5
#define CACHE_LEN 10

extern int memory[128];
extern int t_count;

struct cacheline
{
  int data[CACHE_LEN];
  int age;
  int valid;
  int dirty;
  int tag;
};

struct cacheline cache[CACHE_LINES];

void
sc_cacheInit (void)
{
  memset (cache, 0, sizeof (cache));
}

int
getmaxage ()
{
  int current_max = 0, id = 0;
  for (int i = 0; i < CACHE_LINES; ++i)
    {
      if (cache[i].age > current_max)
        {
          current_max = cache[i].age;
          id = i;
        }
    }
  return id;
}

void
updateAgesExcept (int index)
{
  for (int i = 0; i < CACHE_LINES; ++i)
    cache[i].age += 1;
  cache[index].age = 0;
}

void
print ()
{
  mt_gotoXY (29, 1);
  for (int i = 0; i < CACHE_LINES; ++i)
    {
      printf ("tag: %3d\tage: %3d\n", cache[i].tag, cache[i].age);
    }
}

int
addcacheline (int line[], int tag)
{
  for (int i = CACHE_LINES - 1; i > 0; --i)
    {
      cache[i] = cache[i - 1];
      cache[i].age++;
    }
  memcpy (cache[0].data, line, sizeof cache[0].data);
  cache[0].age = 0;
  cache[0].tag = tag;
  cache[0].valid = 1;
  return 0;
}

int
sc_readtocache (int tag)
{
  int tempdata[10];
  for (int i = tag; i < tag + 10; ++i)
    {
      tempdata[i - tag] = memory[i];
    }
  addcacheline (tempdata, tag);
  return 0;
}

int
checkLineIncache (int tag)
{
  for (int i = 0; i < CACHE_LINES; ++i)
    {
      if (tag == cache[i].tag)
        {
          for (int x = 0; x < CACHE_LEN; ++x)
            {
              if (memory[tag + x] != cache[i].data[x])
                {
                  cache[i].dirty = 1;
                  return 1;
                }
            }
          return 0;
        }
    }
  return -1;
}

int
sc_getcacheline (int line[], int *tag, int index)
{
  if (index < 0 || index > 4)
    return -1;
  memcpy (line, cache[index].data, sizeof (cache[index].data));
  *tag = cache[index].tag;
  return 0;
}

int
sc_cacheRead (int address, int *value)
{
  int line = address / CACHE_LEN;
  int item = address % CACHE_LEN;

  for (int i = 0; i < CACHE_LINES; ++i)
    {
      if (cache[i].valid && cache[i].tag == line)
        {
          updateAgesExcept (i);
          *value = cache[i].data[item];
          t_count = 2;
          return 0;
        }
    }

  int index_max = getmaxage ();
  sc_cacheFlush (index_max);
  for (int i = 0; i < CACHE_LEN; ++i)
    cache[index_max].data[i] = memory[line * 10 + i];
  cache[index_max].valid = 1;
  cache[index_max].dirty = 0;
  cache[index_max].tag = line;
  updateAgesExcept (index_max);
  *value = cache[index_max].data[item];
  t_count = 10;
  return -1;
}

int
sc_cacheWrite (int address, int value)
{
  int line = address / CACHE_LEN;
  int item = address % CACHE_LEN;

  for (int i = 0; i < CACHE_LINES; ++i)
    {
      if (cache[i].valid && cache[i].tag == line)
        {
          updateAgesExcept (i);
          checkLineIncache (line * 10);
          cache[i].data[item] = value;
          t_count = 2;
          return 0;
        }
    }

  int index_max = getmaxage ();
  updateAgesExcept (index_max);
  sc_cacheFlush (index_max);
  for (int i = 0; i < CACHE_LEN; ++i)
    cache[index_max].data[i] = memory[line * 10 + i];
  cache[index_max].valid = 1;
  cache[index_max].dirty = 1;
  cache[index_max].tag = line;
  cache[index_max].data[item] = value;
  t_count = 10;
  return 0;
}

int
sc_cacheFlush (int line)
{
  int it = (line == -1) ? CACHE_LINES : 1;
  line = (line == -1) ? 0 : line;
  for (int i = 0; i < it; ++i)
    {
      if (cache[i + line].valid)
        {
          int tag = cache[i + line].tag;
          for (int j = 0; j < (10 * (tag < 12) + ((tag == 12) * 8)); ++j)
            {
              memory[tag * 10 + j] = cache[i + line].data[j];
            }
          cache[i + line].dirty = 0;
        }
    }
  return 0;
}

int
sc_updateCacheCell (int address, int value)
{
  int tag = address / 10;
  int item = address % 10;
  for (int i = 0; i < CACHE_LINES; ++i)
    {
      if (cache[i].tag == tag)
        {
          cache[i].data[item] = value;
          return 0;
        }
    }
  return -1;
}