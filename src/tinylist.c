/*****************************************************************************
   Tiny List - An ultra cheap implementation of a list for the C language.
   Copyright (c) 2012 Orson Teodoro <oteodoro@ics.uci.edu>

   Dual licensed under the MIT or GPL version 2 or above  
  
 *****************************************************************************/

//#define TEST

#ifdef TEST
#include <stdio.h>
#endif
#include <stdlib.h>
#include "tinylist.h"

typedef struct tinylistnode TINYLISTNODE, *PTINYLISTNODE;

struct tinylistnode
{
  void *key;
  void *value;
  PTINYLISTNODE next;
};

typedef struct tinylist
{
  PTINYLISTNODE head;
  PTINYLISTCMPFN cmpfn;
};

PTINYLIST
tl_new (PTINYLISTCMPFN cmpfn)
{
  PTINYLIST tl = calloc (1,sizeof (TINYLIST));
  tl->cmpfn = cmpfn;
  return tl;
}

void
tl_clear (PTINYLIST tl)
{
  PTINYLISTNODE x, t; 
  x = tl->head;
  while(x)
    {
      t = x, x = x->next;
      free (t);
    }
  tl->head = NULL;
}

void
tl_free (PTINYLIST tl)
{
  tl_clear (tl);
  free (tl);
}

int
tl_empty (PTINYLIST tl)
{
  return !tl->head;
}

int
tl_notempty (PTINYLIST tl)
{
  return tl->head != NULL;
}

int
tl_length (PTINYLIST tl)
{
  int i = 0;
  PTINYLISTNODE x = tl->head;
  while(x)
    {
      i++;
      x = x->next;
    }
  return i;
}

void*
tl_get (PTINYLIST tl, void *key)
{
  PTINYLISTNODE x = tl->head;
  while(x)
    {
      if (!tl->cmpfn (x->key, key))
        return x->value;
      x = x->next;
    }
  return NULL;
}

void
tl_put (PTINYLIST tl, void *k, void *v)
{
  PTINYLISTNODE x = tl->head;
  PTINYLISTNODE t = x;
  if (!x)
    {
      PTINYLISTNODE n = calloc (1, sizeof (TINYLISTNODE));
      n->key = k;
      n->value = v;
      tl->head = n;
    }
  else
    {
      while (x && tl->cmpfn (x->key, k) < 0)
        t = x, x = x->next;
      if (x && !tl->cmpfn (x->key, k))
        {
          x->value = v;
        }
      else
        {
          PTINYLISTNODE n = calloc (1, sizeof (TINYLISTNODE));
          n->key = k;
          n->value = v;
          if (x == t && t == tl->head)
            {
              n->next = tl->head;
              tl->head = n;
            }
          else
            {
              n->next = t->next;
              t->next = n;
            }
        }
    }
}

void*
tl_del (PTINYLIST tl, void *k)
{
  PTINYLISTNODE x = tl->head;
  PTINYLISTNODE t = x;
  if (!x)
      return NULL;
  else
    {
      while (x && tl->cmpfn (x->key, k) < 0)
        t = x, x = x->next;
      if (x && !tl->cmpfn (x->key, k))
        {
          void *d;
          d = x->value;
          if (tl->head == x)
              tl->head = t->next;
          else
              t->next = x->next;
          free (x);
          return d;
        }
    }
  return NULL;
}

void
tl_foreach (PTINYLIST tl, PTINYLISTEVALFN evalfn, void *evaldata)
{
  PTINYLISTNODE x = tl->head;
  while(x)
    {
      if (!evalfn (x->key, x->value, evaldata))
        break;
      x = x->next;
    }
}

#ifdef TEST

int peeker (void *_key, void *_value, void *data)
{
  int *key = _key;
  int *value = _value;
  printf ("key: %d value: %d \n", *key, *value);
}

int sorter (void *_left, void *_right)
{
  int *left = _left;
  int *right = _right;
  return *left - *right;
}

void
tl_test_put ()
{
  PTINYLIST tl = tl_new (sorter);
  
  int i;
  int v[100];

  srandom (time (NULL));

  for (i=0;i<100;i++)
    v[i] = random (), printf ("%d \n", v[i]);

  for (i=0;i<100;i++)
    tl_put (tl,v+i,v+i);

  tl_foreach (tl, peeker, NULL);

  tl_free (tl);
}

void
tl_test_put2 ()
{
  PTINYLIST tl = tl_new (sorter);
  
  int i;
  int v[100];

  for (i=0;i<100;i++)
    v[i] = i, printf ("%d \n", v[i]);

  for (i=0;i<100;i++)
    tl_put (tl,v+i,v+i);

  tl_foreach (tl, peeker, NULL);

  tl_free (tl);
}

#define SIZE 100

void
tl_test_del ()
{

  PTINYLIST tl = tl_new (sorter);
  
  int i;
  int v[SIZE];
  int *d;

  for (i=0;i<SIZE;i++)
    v[i] = i, printf ("%d \n", v[i]);

  for (i=0;i<SIZE;i++)
    tl_put (tl,v+i,v+i);

  tl_foreach (tl, peeker, NULL);
  d = tl_del (tl,v+0), printf ("%d \n", *d);
  tl_foreach (tl, peeker, NULL);

  tl_foreach (tl, peeker, NULL);
  d = tl_del (tl,v+SIZE-1), printf ("%d \n", *d);
  tl_foreach (tl, peeker, NULL);

  tl_foreach (tl, peeker, NULL);
  d = tl_del (tl,v+SIZE/2), printf ("%d \n", *d);
  tl_foreach (tl, peeker, NULL);

  tl_free (tl);
}

void
tl_test_get ()
{
  PTINYLIST tl = tl_new (sorter);
  
  int i;
  int v[SIZE];
  int *d;

  for (i=0;i<SIZE;i++)
    v[i] = i, printf ("%d \n", v[i]);

  for (i=0;i<SIZE;i++)
    tl_put (tl,v+i,v+i);

  d = tl_get (tl,v+0), printf ("%d \n", *d);

  d = tl_get (tl,v+SIZE-1), printf ("%d \n", *d);

  d = tl_get (tl,v+SIZE/2), printf ("%d \n", *d);

  tl_free (tl);
}

#undef SIZE
#define SIZE 3

void
tl_test_clear ()
{
  PTINYLIST tl = tl_new (sorter);

  int i;
  int v[SIZE];
  int u[SIZE];
  int *d;

  for (i=0;i<SIZE;i++)
    v[i] = i, printf ("%d \n", v[i]);

  for (i=0;i<SIZE;i++)
    u[i] = i+200, printf ("%d \n", u[i]);

  for (i=0;i<SIZE;i++)
    tl_put (tl,v+i,u+i);

  tl_foreach (tl, peeker, NULL);
  tl_clear (tl);

  tl_foreach (tl, peeker, NULL);

  printf("empty? %d", tl_empty(tl));

  tl_free (tl);
}

int
main (int argc, int argv)
{
  //tl_test_put ();
  //tl_test_put2 ();
  //tl_test_del ();  
  //tl_test_get ();
  tl_test_clear ();
}

#endif
