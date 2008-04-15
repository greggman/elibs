/*************************************************************************
 *                                                                       *
 *                              RITEARGS.C                               *
 *                                                                       *
 *************************************************************************

		Copyright (c) 1997-2008, Echidna

		All rights reserved.

		Redistribution and use in source and binary forms, with or
		without modification, are permitted provided that the following
		conditions are met:

		* Redistributions of source code must retain the above copyright
		  notice, this list of conditions and the following disclaimer. 
		* Redistributions in binary form must reproduce the above copyright
		  notice, this list of conditions and the following disclaimer
		  in the documentation and/or other materials provided with the
		  distribution. 

		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
		CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
		INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
		MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
		DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
		BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
		EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
		TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
		DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
		ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
		OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
		OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
		POSSIBILITY OF SUCH DAMAGE.

 
   DESCRIPTION
		Write Args 
 
   PROGRAMMERS
		Gregg A. Tavares
 
   FUNCTIONS
 
   TABS : 5
 
   HISTORY
		03/25/97 GAT: Created.

 *************************************************************************/

/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include <echidna\ensure.h>

#include <stdlib.h>

#include <echidna\argparse.h>
#include <echidna\eerrors.h>
#include <echidna\eio.h>

/*************************** C O N S T A N T S ***************************/


/******************************* T Y P E S *******************************/


/************************** P R O T O T Y P E S **************************/


/***************************** G L O B A L S *****************************/


/****************************** M A C R O S ******************************/


/**************************** R O U T I N E S ****************************/





/**************************** T e m p l a t e ****************************/

#define	ARG_ALLARGS		(newargs[ 0])

char Usage[] = "Usage: riteargs ALLARGS\n";

ArgSpec Template[] = {
{STANDARD_ARG|MULTI_ARG,	"ALLARGS",	"", },
{0, NULL, NULL, },
};

/********************************** MAIN **********************************/

int main(int argc, char **argv)
BEGINFUNCMAIN(main)
{
	int		result = TRUE;
	char**	newargs;

	newargs = argparse (argc, argv, Template);

	if (!newargs)
	{
		EL_printf ("%s\n", GlobalErrMsg);
		printarghelp (Usage, Template);
		RETURN EXIT_FAILURE;
	}
	else
	{
		char	**args;
		
		args = MULTI_ARGLIST (ARG_ALLARGS);
		if (args)
		{
			while (*args)
			{
				EIO_PrintEscString (stdout, *args);
				args++;
			}
		}
	}
	RETURN (result ? EXIT_SUCCESS : EXIT_FAILURE);
}
ENDFUNCMAIN(main)



