/*
 * BIN2LSIT.C
 *
 *  COPYRIGHT : 1993 Echidna.
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 05/09/93
 *   MODIFIED : 01/02/94
 *       TABS : 05 09
 *
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 * DESCRIPTION
 *		Takes a bunch of binary files and concatinates them, checks for
 *		duplicates and creates a long word relative offset table for each
 *
 * HISTORY
 *	
 * TODO
 *		* Support files larger than 64K (use lists of XTRAPntrs)
 *		* Support unlimited size by using files directly instead of
 *		  loading them into memory.
 *
*/

#include <echidna/platform.h>
#include "switches.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <echidna/utils.h>
#include <echidna/eerrors.h>
#include <echidna/argparse.h>
#include <echidna/datafile.h>
#include <echidna/eio.h>
#include <echidna/strings.h>
#include <echidna/listapi.h>
#include <echidna/checkglu.h>


/**************************** C O N S T A N T S ***************************/

/******************************** T Y P E S *******************************/

typedef struct {
	LST_NODE	 Node;
	UINT8*		 Data;
	UINT32		 Size;
	UINT32		 Offset;
	UINT32		 PadSize;
} FileContents;

/****************************** G L O B A L S *****************************/

int  			 Verbose      = FALSE;
int  			 Pack         = TRUE;
int  			 OutMode	  = 0;
UINT32			 PadSize	  = 1;
int  			 LittleEndian = TRUE;
int  			 BinOffsets   = TRUE;
int  			 HeaderNdxs   = FALSE;

LST_LIST		 FileListX;
LST_LIST		*FileList = &FileListX;

char			*hdformat = "%s\tBINARY_OFFSET\t$%08lx";

UINT8			*buf;

UINT32			 CurrentAddress;
UINT32			 TotalFiles;

/******************************* M A C R O S ******************************/

/******************************* T A B L E S ******************************/

UINT8			 ZeroData[256];

/***************************** R O U T I N E S ****************************/

/******************************** TEMPLATE ********************************/

#define ARG_OUTFILE		(newargs[ 0])
#define ARG_INFILES		(newargs[ 1])
#define ARG_VERBOSE		(newargs[ 2])
#define ARG_PACK		(newargs[ 3])
#define ARG_BIGENDIAN	(newargs[ 4])
#define ARG_BYTES		(newargs[ 5])
#define ARG_HEADER		(newargs[ 6])
#define ARG_CONST		(newargs[ 7])
#define ARG_HDFORMAT	(newargs[ 8])

char Usage[] = "Usage: BIN2LIST OUTFILE INFILES [switches...]\n";

ArgSpec Template[] = {
{STANDARD_ARG|REQUIRED_ARG,						"OUTFILE",	"\tOUTFILE   = Output filename\n", },
{STANDARD_ARG|REQUIRED_ARG|MULTI_ARG|LIST_ARG,	"INFILES",	"\tINFILES   = List of files to concate\n", },
{CHRSWITCH_ARG,									"V",		"\t-V        = Verbose (show errors)\n", },
{CHRSWITCH_ARG,									"P",		"\t-P        = Don't Pack\n", },
{CHRSWITCH_ARG,									"B",		"\t-B        = Big Endian\n", },
{CHRKEYWORD_ARG,								"S",		"\t-S<bytes> = Sector Size (Def. 1)\n", },
{CHRKEYWORD_ARG,								"H",		"\t-H<file>  = Write position info to <file> instead of binary\n", },
{CHRSWITCH_ARG,									"C",		"\t-C        = Header file is offset index and offset are still in binary\n", },
{CHRKEYWORD_ARG,								"F",		"\t-F<fmt>   = Header Output Format String\n"
															"\t            eg. \"%s BINARY_OFFSET $%08lx\"\n"
															"\t            or  \"#define %s 0x%08lx\"\n"
															, },
{0, NULL, NULL, },
};

/********************************** MAIN **********************************/

int main(int argc, char **argv)
{
	char			**newargs;

	LST_InitList (FileList);

	newargs = argparse (argc, argv, Template);

	if (!newargs) {
		EL_printf ("%s\n", GlobalErrMsg);
		printarghelp (Usage, Template);
		return EXIT_FAILURE;
	} else {

		Verbose      =  SWITCH_VALUE(ARG_VERBOSE);
		Pack         = !SWITCH_VALUE(ARG_PACK);
		LittleEndian = !SWITCH_VALUE(ARG_BIGENDIAN);
		HeaderNdxs   =  SWITCH_VALUE(ARG_CONST);

		if (ARG_HDFORMAT)	hdformat   = ARG_HDFORMAT;
		if (ARG_BYTES)		PadSize    = EL_atol (ARG_BYTES);

		if (ARG_HEADER && !HeaderNdxs)
		{
			BinOffsets = FALSE;
		}

		//
		// load all requested files
		//
		{
			LST_LIST	*nameList;
			LST_NODE	*nd;

			nameList = MULTI_ARGLINKEDLIST(ARG_INFILES);
			nd       = LST_Head (nameList);
			
			while (!LST_EndOfList (nd))
			{

				int				 fh;
				long			 size;
				char			*filename;
				
				filename = LST_NodeName (nd);

				fh = CHK_ReadOpen (filename);
				{
					size = CHK_FileLength (fh);
			
					if (size == 0)
					{
						WarnMess ("File '%s' is zero bytes long\n", filename);
					}

					if (Pack)
					{
						if (Verbose)
						{
							EL_printf ("Reading %14s : size %6ld", filename, size);
						}

						if (size)
						{
							buf = CHK_AllocateMemory (size, filename);
							CHK_Read (fh, buf, size);
						}
					}

					//
					// see if it's the same as a previous file
					//
					{
						FileContents	*fc;
						FileContents	*newfc;

						newfc = (FileContents*)CHK_CreateNode (sizeof (FileContents), filename, "FileContents");

						if (Pack)
						{
							fc = (FileContents*)LST_Head (FileList);
							while (!LST_EndOfList (fc))
							{
								if (fc->Size)
								{
									if (!memcmp (fc->Data, buf, size))
									{
										break;
									}
								}
								fc = (FileContents*)LST_Next (fc);
							}
						}

						if (Pack && !LST_EndOfList (fc))
						{
							//
							// it was the same
							//
							newfc->Offset = fc->Offset;
							if (Verbose)
							{
								EL_printf (" : Same as %s", LST_NodeName(fc));
							}
							CHK_DeallocateMemory (buf, "FileContents");
						}
						else
						{
							//
							// it was different
							//
							newfc->Size   = size;
							newfc->Offset = CurrentAddress;

							size  = (size + PadSize - 1) / PadSize;
							size *= PadSize;

							newfc->PadSize = size;

							CurrentAddress += size;

							if (Pack)
							{
								newfc->Data = buf;
							}
						}

						CHK_Close (fh);
				
						if (Pack && Verbose)
						{
							EL_printf ("\n");
						}
						LST_AddTail (FileList, newfc);
						TotalFiles++;
					}
				}

				nd = LST_Next (nd);
			}
		}

/******************************* Write Files ******************************/
		if (!ErrorCount)
		{
			FILE			*fp;
			int				 fh;
			FileContents	*fc;
			UINT32			 offsetTableSize;
			UINT32			 ndxCount = 0;

			fh = CHK_WriteOpen (ARG_OUTFILE);

			if (ARG_HEADER)
			{
				fp = CHK_fopen (ARG_HEADER, "w");
			}

			if (!BinOffsets)
			{
				offsetTableSize = 0;
			}
			else
			{
				offsetTableSize = (TotalFiles + 1) * sizeof (UINT32);
			}

			if (Verbose)
			{
				EL_printf ("Writing %ld bytes to file %s\n", CurrentAddress + offsetTableSize, ARG_OUTFILE);
			}

			fc = (FileContents*)LST_Head (FileList);
			while (!LST_EndOfList (fc))
			{
				UINT32	offset;

				offset = fc->Offset + offsetTableSize;

				if (ARG_HEADER)
				{
					char	work[EIO_MAXPATH];
					UINT32	hoffset;

					hoffset = (HeaderNdxs) ? ndxCount : offset;

					strcpy (work, EIO_Filename(LST_NodeName(fc)));
					strupr (work);

					{
						char	*s;

						s = work;
						while (*s)
						{
							if (*s == '.' || *s == '-')
							{
								*s = '_';
							}
							s++;
						}
					}

					CHK_fprintf (fp,
						hdformat,
						work,
						hoffset
						);
					CHK_fprintf (fp, "\n");
				}

				if (BinOffsets)
				{
					if (LittleEndian)
					{
						MakeLilLong (offset);
					}
					else
					{
						MakeBigLong (offset);
					}
					CHK_Write (fh, &offset, sizeof (UINT32));
				}

				ndxCount++;
			
				fc = (FileContents*)LST_Next (fc);
			}

			if (BinOffsets)
			{
				UINT32	zero = 0;

				CHK_Write (fh, &zero, sizeof (UINT32));
			}

			fc = (FileContents*)LST_Head (FileList);
			while (!LST_EndOfList (fc))
			{
				if (fc->Size)
				{
					if (Verbose)
					{
						EL_printf ("Writing %7ld bytes from %s\n", fc->Size, LST_NodeName (fc));
					}

					if (Pack)
					{
						if (fc->Size)
						{
							CHK_Write (fh, fc->Data, fc->Size);
						}
					}
					else
					{
						int		infh;
						long	bytes;
						
						#define	BUF_SIZE	(1024*1024)
						
						buf  = CHK_AllocateMemory (BUF_SIZE, "copy buffer");
						infh = CHK_ReadOpen (LST_NodeName(fc));
						
						bytes = fc->Size;
						while (bytes)
						{
							long	len;

							len = min (bytes, BUF_SIZE);
							CHK_Read (infh, buf, len);
							CHK_Write (fh, buf, len);
							
							bytes -= len;
						}
						
						CHK_Close (infh);
						
						CHK_DeallocateMemory (buf, "copy buffer");
					}
				}
				
				{
					UINT32	pad;

					pad = fc->PadSize - fc->Size;
					while (pad)
					{
						UINT32	part;

						part = min (pad, 256);
						CHK_Write (fh, ZeroData, part);
						pad -= part;
					}
				}

				fc = (FileContents*)LST_Next (fc);
			}

			CHK_Close (fh);

		}
/************************************  ************************************/
	}

	if (GlobalErr) {
		EL_printf ("%s\n", GlobalErrMsg);
		return EXIT_FAILURE;
	}

	if (ErrorCount)
	{
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

