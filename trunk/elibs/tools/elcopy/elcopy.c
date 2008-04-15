/*************************************************************************
 *                                                                       *
 *                       ELCOPY.CPP                                      *
 *                                                                       *
 *************************************************************************

		Copyright (c) 1996-2008, Echidna

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
        Copy files with more useful options than copy or xcopy
		although not yet.

   PROGRAMMERS
        Gregg A. Tavares

   FUNCTIONS

   TABS : 5 9

   HISTORY
        07/15/96 : Created.

 *************************************************************************/

/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include "echidna\ensure.h"

#include "echidna\argparse.h"
#include "echidna\eerrors.h"
#include "echidna\eio.h"
#include "echidna\checkglu.h"
#include "echidna\strings.h"

/*************************** C O N S T A N T S ***************************/


/******************************* T Y P E S *******************************/


/************************** P R O T O T Y P E S **************************/


/***************************** G L O B A L S *****************************/


/****************************** M A C R O S ******************************/


/**************************** R O U T I N E S ****************************/

#define ARG_VERBOSE       (newargs[ 0])
#define ARG_DONTCOPY      (newargs[ 1])
#define ARG_RECURSE       (newargs[ 2])
#define ARG_UPDATE        (newargs[ 3])
#define ARG_DELETE        (newargs[ 4])
#define ARG_RESETREADONLY (newargs[ 5])
#define ARG_FROM          (newargs[ 6])
#define ARG_TO            (newargs[ 7])

char Usage[] =  "Usage: ELCOPY FROM TO [options]\n"
                ;

ArgSpec Template[] = {
{CHRKEYWORD_ARG,                        "V",        "\t-V<lvl> = Level of verbosines\n", },
{SWITCH_ARG,                            "-L",       "\t-L      = Don't copy just print what would be copied\n", },
{SWITCH_ARG,                            "-S",       "\t-S      = copy subdirectories\n", },
{SWITCH_ARG,                            "-U",       "\t-U      = copy only files that area newer or not in dest\n", },
{SWITCH_ARG,                            "-D",       "\t-D      = delete files/dirs on dest that are not on soruce\n", },
{SWITCH_ARG,                            "-R",       "\t-R      = reset (clear) read-only flag on dest of copied\n", },
{STANDARD_ARG|REQUIRED_ARG,             "FROM",     "\tFROM    = FROM\n", },
{STANDARD_ARG|REQUIRED_ARG,             "TO",       "\tTO      = TO\n", },
{0, NULL, NULL, },
};

/********************************** MAIN **********************************/

int main(int argc, char **argv)
BEGINFUNCMAIN(main)
{
    char            **newargs;
    int               fVerbose = FALSE;
    int               success = FALSE;
	int				  verboseLevel;

    newargs = argparse (argc, argv, Template);

    if (!newargs)
    {
        EL_printf ("%s\n", GlobalErrMsg);
        printarghelp (Usage, Template);
        RETURN EXIT_FAILURE;
    }
    else
    {
        BOOL    fRecurseDirs;
        BOOL    fDelete;
        BOOL    fUpdate;
        BOOL    fDontCopy;
		BOOL	fResetReadOnly;
        BOOL    fMakeDest = FALSE;
        LST_LIST*   fileList;

		if (ARG_VERBOSE)
		{
			verboseLevel = EL_atol (ARG_VERBOSE);
			fVerbose = verboseLevel > 0;
		}

        fRecurseDirs   = SWITCH_VALUE(ARG_RECURSE);
        fDelete        = SWITCH_VALUE(ARG_DELETE);
        fUpdate        = SWITCH_VALUE(ARG_UPDATE);
        fDontCopy      = SWITCH_VALUE(ARG_DONTCOPY);
        fResetReadOnly = SWITCH_VALUE(ARG_RESETREADONLY);

        if (verboseLevel > 1)
        {
            EL_printf ("Getting file list for '%s'\n", ARG_FROM);
        }

        switch (EIO_FileType (ARG_FROM))
        {
        case EIO_TYPE_FILE:
            break;
        case EIO_TYPE_DIRECTORY:
            fMakeDest = TRUE;
            break;
        default:
            {
				char temp[EIO_MAXPATH];

				strcpy (temp, EIO_Path(ARG_FROM));
				if (strlen(temp) && temp[strlen(temp) - 1] == DIRSEP)
				{
					temp[strlen(temp) - 1] = '\0';
				}

                switch (EIO_FileType (temp))
                {
                case EIO_TYPE_FILE:
                    break;
                case EIO_TYPE_DIRECTORY:
                    fMakeDest = TRUE;
                    break;
                default:
                    FailMess ("Can't read '%s'\n", ARG_FROM);
                    break;
                }
            }
            break;
        }

        if (fMakeDest)
        {
            switch (EIO_FileType (ARG_TO))
            {
            case EIO_TYPE_FILE:
                FailMess ("Destination '%s' is a file\n", ARG_TO);
                break;
            case EIO_TYPE_DIRECTORY:
                break;
            default:
                if ((verboseLevel > 1) || fDontCopy)
                {
                    EL_printf ("MkDir '%s'\n", ARG_TO);
                }

                if (!fDontCopy)
                {
                    if (!EIO_MakeDir (ARG_TO))
                    {
                        FailMess (GlobalErrMsg);
                    }
                }
                break;
            }
        }

        fileList = EIO_GetFileList (ARG_FROM, fRecurseDirs, TRUE);
        if (!fileList)
        {
            FailMess ("Couldn't get filelist for '%s'\n", ARG_FROM);
        }
        else
        {
            LST_NODE*   nd;

            nd = LST_Head (fileList);
            while (!LST_EndOfList (nd))
            {
                BOOL    fCopy = TRUE;
                char*   src;
                char    dst[EIO_MAXPATH];

                src = LST_NodeName(nd);

                {
                    char*   orig;
                    char*   s;

                    orig = ARG_FROM;
                    s    = src;
                    while (*s && *orig && *orig == *s)
                    {
                        s++;
                        orig++;
                    }

                    EIO_fnmerge (dst, ARG_TO, s, NULL);
                }

                if (verboseLevel > 1)
                {
                    EL_printf ("Working on '%s' -> '%s'\n", src, dst);
                }

                switch (EIO_FileListType (nd))
                {
                case EIO_TYPE_DIRECTORY:
                    fCopy = FALSE;
                    switch (EIO_FileType (dst))
                    {
                    case EIO_TYPE_FILE:
                        ErrMess ("Src '%s' is a diretory, Dst '%s' is a file\n");
                        break;
                    case EIO_TYPE_DIRECTORY:
                        break;
                    default:
                        if (fDontCopy || (verboseLevel > 1))
                        {
                            EL_printf ("Mkdir '%s'\n", dst);
                        }
                        if (!fDontCopy)
                        {
                            if (!EIO_MakeDir (dst))
                            {
                                ErrMess (GlobalErrMsg);
                                ClearGlobalError ();
                            }
                        }
                        break;
                    }
                    break;
                case EIO_TYPE_FILE:
					EL_printf ("dst[%d](%s)\n", strlen(dst), dst);
                    switch (EIO_FileType (dst))
                    {
                    case EIO_TYPE_FILE:
                        if (fUpdate)
                        {
                            int newer;

                            newer = EIO_FileNewer (src, dst);
                            if (newer < 0)
                            {
                                ErrMess (GlobalErrMsg);
                                ClearGlobalError ();
                                fCopy = FALSE;
                            }
                            else if (!newer)
                            {
                                fCopy = FALSE;
                            }
                        }
                        break;
                    case EIO_TYPE_DIRECTORY:
                        ErrMess ("Src '%s' is a file, Dst '%s' is a directory\n");
                        fCopy = FALSE;
                        break;
                    default:
                        break;
                    }

                    if (fCopy)
                    {
                        if (fDontCopy || fVerbose)
                        {
                            EL_printf ("Copying '%s' -> '%s'\n", src, dst);
                        }

                        if (!fDontCopy)
                        {
                            if (!EIO_CopyFile (src, dst))
                            {
                                ErrMess (GlobalErrMsg);
                                ClearGlobalError ();
                            }
							else
							{
								if (fResetReadOnly)
								{
									FileAttribType	fat;
							
									if (!EIO_GetFileAttrib (dst, &fat))
									{
										ErrMess (GlobalErrMsg);
										ClearGlobalError ();
									}
									else
									{
										// clear the read only flag
										//EIO_ModifyFileAttribBit (&fat, EIO_ATTR_WRITE, FALSE);
										if (!EIO_SetFileAttrib (dst, &fat))
										{
											ErrMess (GlobalErrMsg);
											ClearGlobalError ();
										}
									}
								}
							}
                        }
                    }

                    break;
                }

                nd = LST_Next (nd);
            }
        }
    }

    if (ErrorCount)
    {
        EL_printf ("Errors: %d\n", ErrorCount);
        success = FALSE;
    }

    if (WarnCount)
    {
        EL_printf ("Warnings: %d\n", WarnCount);
    }

    if (GlobalErr) {
        EL_printf ("%s\n", GlobalErrMsg);
        return EXIT_FAILURE;
    }

    return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}
ENDFUNCMAIN(main)


