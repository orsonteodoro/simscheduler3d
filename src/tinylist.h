/*****************************************************************************
   Tiny List - An ultra cheap implementation of a list for the C language.
   Copyright (c) 2012 Orson Teodoro <oteodoro@ics.uci.edu>

   Dual licensed under the MIT or GPL version 2 or above  
  
 *****************************************************************************/

#ifndef TINYLIST_H
#define TINYLIST_H

typedef int (*PTINYLISTEVALFN) (void *key, void *value, void *evaldata);
typedef int (*PTINYLISTCMPFN) (void *left, void *right);
typedef struct tinylist TINYLIST, *PTINYLIST;

PTINYLIST tl_new (PTINYLISTCMPFN cmpfn);
void tl_free (PTINYLIST tl);
void tl_clear (PTINYLIST tl);
int tl_empty (PTINYLIST tl);
int tl_notempty (PTINYLIST tl);
void* tl_get (PTINYLIST tl, void *key);
void tl_put (PTINYLIST tl, void *k, void *v);
void* tl_del (PTINYLIST tl, void *k);
void tl_foreach (PTINYLIST tl, PTINYLISTEVALFN evalfn, void *evaldata);
int tl_length (PTINYLIST tl);

#endif
