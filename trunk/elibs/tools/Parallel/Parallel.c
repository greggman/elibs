/*
 * PARALLEL.C
 *
 *  COPYRIGHT : 1991 Echidna.
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 05/03/91
 *   MODIFIED : 03/30/94
 *       TABS : 05 09
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 * DESCRIPTION
 *		Program to create parallel tables for games.
 *
 * HISTORY
 *
*/

#include <echidna/platform.h>
#include "switches.h"

#include <ctype.h>
#include <echidna/listapi.h>
#include <echidna/utils.h>
#include <echidna/eerrors.h>
#include <echidna/argparse.h>
#include <echidna/ezparse.h>
#include <echidna/strings.h>
#include <echidna/eio.h>


/**************************** C O N S T A N T S ***************************/

#define MODE_ASM	0
#define MODE_C		1

#define MAX_LINE_SIZE	1024
#define MAX_TABLES		80

#define GET_TABLES	1
#define GET_DATA	2
#define GET_COPY	3

#define TYPE_TABLES	0
#define TYPE_COPY	1

/******************************** T Y P E S *******************************/

typedef struct {
	LST_NODE	 Node;
	char		*Suffix;
	char		*Format;
	char		*Prefix;
} Format;

typedef	struct {
	LST_NODE	 Node;
	LST_LIST	 ListX;
	LST_LIST	*DataList;
	LST_LIST	 List2X;
	LST_LIST	*Formats;
} FormatType;

typedef struct {
	LST_NODE	 Node;
	short		 Type;
	LST_LIST	 ListX;
	LST_LIST	*FormatList;
} TableType;

/****************************** G L O B A L S *****************************/

static char	 Line[MAX_LINE_SIZE];

LST_LIST	 TableListX;
LST_LIST	*TableList = &TableListX;
TableType	*CurrentTable;

/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/



/******************************** TEMPLATE ********************************/

#define ARG_INFILE		(newargs[0])
#define ARG_OUTFILE		(newargs[1])
#define ARG_MODE		(newargs[2])

char Usage[] = "Usage: Parallel INFILE OUTFILE [switches...]\n";

ArgSpec Template[] = {
{	STANDARD_ARG | REQUIRED_ARG,	"INFILE",	"\tINFILE  = Table File\n", },
{	STANDARD_ARG | REQUIRED_ARG,	"OUTFILE",	"\tOUTFILE = File to write parallel tables into\n", },
{	CHRKEYWORD_ARG,					"M",		"\tMODE    = 0 : Asm (default)\n"
												"\t          1 : C\n"
												"\t          2 : C with prefix/suffix\n"
												, },
{	0, NULL, NULL, },
};

/********************************** MAIN **********************************/

int main(int argc, char **argv)
{
	char			**newargs;
	FILE			*fp;
	LST_NODE		*data;
	FormatType		*ft;
	Format			*f;
	char			*s1;
	char			*s2;
	char			*s3;
	short			 mode = MODE_ASM;
	short			 count;
	short			 i;
	short			 status = TRUE;
	short			 Mode = FALSE;
	char			*defprefix;
	char			*defsuffix;
	char			*defformat;
	char			*defending;
	char			*args[MAX_TABLES+1];

	newargs = argparse (argc, argv, Template);

	if (!newargs) {
		EL_printf ("%s\n", GlobalErrMsg);
		printarghelp (Usage, Template);
		return EXIT_FAILURE;
	} else {
		LST_InitList (TableList);

		if (ARG_MODE) {
			mode = atoi (ARG_MODE);
		}

		switch (mode) {
		case MODE_C:
			defprefix = "[] = {";
			defsuffix = "";
			defformat = "\t%s,";
			defending = "};\n\n";
			break;
		case MODE_ASM:
		default:
			defprefix = "";
			defsuffix = "";
			defformat = "\tdb\t%s";
			defending = "\n";
			break;
		}

		fp = fopen (ARG_INFILE, "r");
		if (!fp) {
			EL_printf ("Couldn't open '%s'\n", ARG_INFILE);
			return EXIT_FAILURE;
		}

		EZResetParseFilename (ARG_INFILE);

		while (EZParseGetLine (fp, Line, (short)MAX_LINE_SIZE, (short)(Mode != GET_COPY))) {
			if (!stricmp (Line, "[TABLES]")) {
				if (Mode == GET_TABLES) {
					EZParseError1 ("Missplaced [TABLES] Statement", "");
					status = FALSE;
				}
				Mode = GET_TABLES;
				CurrentTable = (TableType*)LST_CreateNode (sizeof (TableType), NULL);
				if (!CurrentTable) {
					EZParseError1 ("OOM: TableType", "");
					return EXIT_FAILURE;
				}
				CurrentTable->FormatList = &CurrentTable->ListX;
				CurrentTable->Type = TYPE_TABLES;
				LST_InitList (CurrentTable->FormatList);
				LST_AddTail (TableList, CurrentTable);
			} else if (!stricmp (Line, "[DATA]")) {
				if (Mode == GET_DATA) {
					EZParseError1 ("Missplaced [DATA] Statement", "");
					status = FALSE;
				}
				Mode = GET_DATA;
			} else if (!stricmp (Line, "[COPY]")) {
				if (Mode == GET_TABLES) {
					EZParseError1 ("Missplaced [COPY] Statement", "");
					status = FALSE;
				}
				Mode = GET_COPY;
				CurrentTable = (TableType*)LST_CreateNode (sizeof (TableType), NULL);
				if (!CurrentTable) {
					EZParseError1 ("OOM: TableType (2)", "");
					return EXIT_FAILURE;
				}
				CurrentTable->FormatList = &CurrentTable->ListX;
				CurrentTable->Type = TYPE_COPY;
				LST_InitList (CurrentTable->FormatList);
				LST_AddTail (TableList, CurrentTable);
			} else if (Mode == GET_COPY) {
				data = LST_CreateNode (sizeof (LST_NODE), Line);
				if (!data) {
					EZParseError1 ("OOM: CopyLine", "");
					return EXIT_FAILURE;
				}
				LST_AddTail (CurrentTable->FormatList, data);
			} else if (Mode == GET_TABLES) {
				count = argify (Line, MAX_TABLES, args);
				ft = (FormatType*)LST_CreateNode (sizeof (FormatType), args[0]);
				if (!ft) {
					EZParseError1 ("OOM: FormatType", "");
					return EXIT_FAILURE;
				}
				ft->DataList = &ft->ListX;
				ft->Formats  = &ft->List2X;
				LST_InitList (ft->DataList);
				LST_InitList (ft->Formats);
				LST_AddTail (CurrentTable->FormatList, ft);
				if (count == 1) {
					s1 = strdup (defsuffix);
					s2 = strdup (defformat);
					f  = (Format*)LST_CreateNode (sizeof (Format), NULL);
					if (!f || !s1 || !s2) {
						EZParseError1 ("OOM: Format (1)", "");
						return EXIT_FAILURE;
					}
					LST_AddTail (ft->Formats, f);
					f->Suffix = s1;
					f->Format = s2;
				} else {
					count--;
					i = 1;
					while (count > 0) {
						s1 = strdup(args[i]);
						count--;
						i++;
						if (count) {
							s2 = strdup(args[i]);
							count--;
							i++;
						} else {
							s2 = strdup(defformat);
						}
						if (mode == 2 && count)
						{
							s3 = strdup(args[i]);
							count--;
							i++;
						}
						else
						{
							s3 = strdup ("");
						}
						f  = (Format*)LST_CreateNode (sizeof (Format), NULL);
						if (!f || !s1 || !s2) {
							EZParseError1 ("OOM: Format (2)", "");
							return EXIT_FAILURE;
						}
						LST_AddTail (ft->Formats, f);
						if (mode != 2)
						{
							f->Suffix = s1;
							f->Format = s2;
						}
						else
						{
							f->Prefix = s1;
							f->Format = s2;
							f->Suffix = s3;
						}
					}
				}
			} else if (Mode == GET_DATA) {
				count = argify (Line, MAX_TABLES, args);
				ft = (FormatType*)LST_Head (CurrentTable->FormatList);
				i  = 0;
				while (count) {
					if (LST_EndOfList (ft)) {
						EZParseError1 ("More entries than tables", "");
						status = FALSE;
						break;
					}
					data = LST_CreateNode (sizeof (LST_NODE), args[i]);
					if (!data) {
						EZParseError1 ("OOM: Data", "");
						return EXIT_FAILURE;
					}
					LST_AddTail (ft->DataList, data);
					ft = (FormatType*)LST_Next (ft);
					count--;
					i++;
				}
				if (!LST_EndOfList (ft)) {
					EZParseError1 ("Not enough data", "");
					status = FALSE;
				}
			} else {
				EZParseError1 ("Bad Statement '%s'", Line);
				status = FALSE;
			}
		}

		fclose (fp);

		if (status) {
			fp = fopen (ARG_OUTFILE, "w");
			if (!fp) {
				EL_printf ("Couldn't open '%s'\n", ARG_OUTFILE);
			}

			CurrentTable = (TableType*)LST_Head (TableList);
			while (!LST_EndOfList (CurrentTable)) {
				switch (CurrentTable->Type) {
				case TYPE_TABLES:
					ft = (FormatType*)LST_Head (CurrentTable->FormatList);
					while (!LST_EndOfList (ft)) {
						f = (Format*)LST_Head (ft->Formats);
						while (!LST_EndOfList (f)) {
							if (mode != 2)
							{
								fprintf (fp,
									"%s%s%s\n",
									LST_NodeName (ft),
									f->Suffix,
									defprefix);
							}
							else
							{
								EIO_PrintEscString (fp, f->Prefix);
							}
							count = 0;
							data  = LST_Head (ft->DataList);
							while (!LST_EndOfList (data)) {
								/*** Substitute Name for %s in Format ***/
								{
									char	*s;
									char	*d;
									size_t	 len;

									s   = f->Format;
									d   = Line;
									len = strlen (LST_NodeName(data));
									while (*s) {
										if (s[0] == '%' && tolower(s[1]) == 's') {
											s += 2;
											strcpy (d, LST_NodeName(data));
											d += len;
										} else if (s[0] == '%' && tolower(s[1]) == 'u') {
											s += 2;
											strcpy (d, LST_NodeName(data));
											strupr (d);
											d += len;
										} else if (s[0] == '%' && tolower(s[1]) == 'l') {
											s += 2;
											strcpy (d, LST_NodeName(data));
											strlwr (d);
											d += len;
										} else if (s[0] == '%' && tolower(s[1]) == 'n') {
											s += 2;
											itoa (count, d, 10);
											d += strlen (d);
										} else {
											*d++ = *s++;
										}
									}
									*d = '\0';
									EIO_PrintEscString (fp, Line);
								}
								fprintf (fp,
									"\n");
								count++;
								data = LST_Next (data);
							}
							if (mode != 2)
							{
								fprintf (fp,
									defending);
							}
							else
							{
								EIO_PrintEscString (fp, f->Suffix);
							}
							f = (Format*)LST_Next (f);
						}
						ft = (FormatType*)LST_Next (ft);
					}
					break;
				case TYPE_COPY:
					ft = (FormatType*)LST_Head (CurrentTable->FormatList);
					while (!LST_EndOfList (ft)) {
						if (mode != 2)
						{
							EIO_PrintEscString (fp, LST_NodeName (ft));
							fprintf (fp,
								"\n");
						}
						else
						{
							fprintf (fp, "%s\n", LST_NodeName (ft));
						}
						ft = (FormatType*)LST_Next (ft);
					}
					break;
				}
				CurrentTable = (TableType*)LST_Next (CurrentTable);
			}
		}

	}

	if (GlobalErr) {
		EL_printf ("%s\n", GlobalErrMsg);
		return EXIT_FAILURE;
	}

	return (status) ? EXIT_SUCCESS : EXIT_FAILURE;
}
