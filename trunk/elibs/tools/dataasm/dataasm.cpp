/*************************************************************************
 *                                                                       *
 *                               DATAASM.C                               *
 *                                                                       *
 *************************************************************************
 
                          Copyright 1998 Echidna                         
 
   DESCRIPTION
		 compile data using assembler like macros and stuff
 
   PROGRAMMERS
		
 
   FUNCTIONS
 
   TABS : 5
 
   HISTORY
		08/14/98 : Created.

 *************************************************************************/

/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include "echidna\ensure.h"

/*************************** C O N S T A N T S ***************************/

/*

-- psuedo ops --
  ORG <expr>
  MACRO arg1,arg2,arg3
  MEND/ENDM
  DC.B arg1,arg2...
  DC.W arg1,arg2...
  DC.L arg1,arg2...
  DC.F arg1,arg2...
  DC.D arg1,arg2...
  DS.B arg
  DS.W arg
  DS.L arg
  DS.F arg
  DS.D arg
  PUSH exp
  POP exp
  PUSHS exp,<list>
  POP exp <list>
  EQU exp
  EQUS text
  = exp
  INC/INCLUDE path
  BINC/BINCLUDE/BININC path
  INFORM lvl,msg,args
  RSRESET
  rb <count>
  rw <count>
  rl <count>
  rept <exp>
  shift
  endr
  if
  else
  elif/elseif
  endif
  
  CASE
=<exp>
=?
  ENDCASE

  -- operators--
  ==
  !=
  <=
  <
  >=
  >
  ||
  !
  &&
  ?:
  
  |
  &
  ~
  <<
  >>
  
  +
  -
  *
  /
  %
  
  ()
  
  -- functions --
  ENV()
  STRLEN()
  SUBSTR()
  
  -- number prefixes/suffixes
  $
  0x
  h
  b
  f
  
  -- new sutff --
  DC.FIX1.2
  
/*

/******************************* T Y P E S *******************************/

struct Expression;
struct Operator
{
	int				operator;
	Expression*		leftExp;
	Expression*		rightExp;
};

struct Expression;
{
	LST_NODE	node;
	int			type;
	union
	{
		long		iValue;
		double		fValue;
		Operator	oValue;
	}
};

struct Statement
{
	LST_NODE	node;
	long		numBytes;
	long		opSize;		// bytes/words/longs...
	LST_LIST	dataList;
};

struct Macro
{
	char*		name;
	LST_LIST	namedArgsList;
	LST_LIST	linesList;
};

struct Label
{
	char*		name;
	long		offset;
};


/************************** P R O T O T Y P E S **************************/


/***************************** G L O B A L S *****************************/

HashTable*	g_pLabelTable;
HashTable*	g_pMacroTable;
HashTable*	g_pConstantTable;
HashTable*	g_pStringStacksTable;


/****************************** M A C R O S ******************************/


/**************************** R O U T I N E S ****************************/


#define ARG_INFILE		(newargs[ 0])
#define ARG_OUTFILE		(newargs[ 1]) 
#define ARG_VERBOSE		(newargs[ 2])
#define ARG_DEFINE		(newargs[ 3])
#define ARG_INCLUDEDIR	(newargs[ 4])

char Usage[] = "Usage: DATAASM INFILE OUTFILE\n";

ArgSpec Template[] = {
{STANDARD_ARG|REQUIRED_ARG,				"INFILE",		"\tINFILE  = file to compile\n", },
{STANDARD_ARG|REQUIRED_ARG,				"OUTFILE",		"\tOUTFILE = binary output file\n", },
{SWITCH_ARG,							"-V",			"\t-V      = Verbose\n", },
{SWITCH_ARG,							"-LSBF",		"\t-LSBF   = Least Significant Byte First\n", },
{KEYWORD_ARG|MULTI_ARG,					"-D",			"\t-D<def> = Define a constant (ie -D DEBUG=1)\n", },
{KEYWORD_ARG|MULTI_ARG,					"-I",			"\t-I<dir> = Include path (ie -I d:\mylibs\inc)\n", },
{0, NULL, NULL, },
};

/********************************** MAIN **********************************/

int main(int argc, char **argv)
BEGINFUNCMAIN(main)
{
	char			**newargs;
	int				  fVerbose;
	int				  success = FALSE;

	newargs = argparse (argc, argv, Template);
	
	if (!newargs)
	{
		EL_printf ("%s\n", GlobalErrMsg);
		printarghelp (Usage, Template);
		return EXIT_FAILURE;
	}
	else
	{
		int		infh1;
		int		infh2;
		SetVerboseFlag (SWITCH_VALUE(ARG_VERBOSE));

	}
	
	if (GlobalErr)
	{
		EL_printf ("%s\n", GlobalErrMsg);
		return EXIT_FAILURE;
	}

	return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}
ENDFUNCMAIN(main)


