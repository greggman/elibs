/*=======================================================================*
 |   file name : mlang.h
 |-----------------------------------------------------------------------*
 |   function  : 
 |-----------------------------------------------------------------------*
 |   author    : Gregg Tavares
 *=======================================================================*/
 
#ifndef ECHIDNA_MLANG_H
#define ECHIDNA_MLANG_H
/**************************** i n c l u d e s ****************************/

#include "platform.h"
#include "switches.h"
#include "echidna\ensure.h"

/*************************** c o n s t a n t s ***************************/


/******************************* t y p e s *******************************/


/***************************** g l o b a l s *****************************/


/****************************** m a c r o s ******************************/


/************************** p r o t o t y p e s **************************/

#ifdef __cplusplus

extern "C" {
#endif


void MLANG_AddVariable (const char* name, const char* value);
void MLANG_RemoveVariable (const char* name);
char* MLANG_SubVariables (const char* str, const char* inputPath);

#ifdef __cplusplus
}

#include <string>
using std::string;

bool MLANG_GetVariable (const string& var, string& value);
bool MLANG_SubVariables (string& str, const char* inputPath);


#endif

#endif /* ECHIDNA_MLANG_H */




