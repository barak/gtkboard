#ifndef _ENGINE_H_
#define _ENGINE_H_

//! A struct representing a command that the engine understands.
typedef struct
{
	//! The string
	char * proto_str;
	//! Has this command been implemented
	int isimpl;
	//! Function which will execute this command
	void (*impl_func) (char *);
} Command;

extern Command  commands[];

float engine_eval (Pos *, Player);

//! Functions that do the actual thinking must periodically call this function.
/** It checks if new commands have arrived. */
void engine_poll ();


#endif
