#ifndef _STACK_H_
#define _STACK_H_

#include <stdio.h>

#ifndef byte 
#define byte char
#endif

void stack_free ();

void movstack_init ();
int movstack_get_num_moves ();
void movstack_push (byte *, byte *);
byte *movstack_pop ();
void movstack_trunc ();
byte * movstack_forw ();
byte * movstack_back ();
void movstack_free ();

void statestack_push (void *state);
void *statestack_peek ();
void *statestack_pop ();
void statestack_trunc ();
void *statestack_forw ();
void *statestack_back ();
void statestack_free ();
#endif
