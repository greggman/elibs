/*************************************************************************
 *                                                                       *
 *                                 EIO.C                                 *
 *                                                                       *
 *************************************************************************

                          Copyright 1996 Echidna

   DESCRIPTION
		Echidna I/O Routines.  Various useful routines for I/O that can
		be used with any machine.  You must link with arp stuff if on
		Amiga.

   PROGRAMMERS


   FUNCTIONS

   TABS : 5 9

   HISTORY
		07/09/96 : Created.

 *************************************************************************/

/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "echidna/eio.h"
#include "echidna/eerrors.h"
#include "echidna/strings.h"
#include "echidna/readini.h"
#include "echidna/utils.h"

#if _EL_CC_VC__
	#include <sys/utime.h>
	#include <time.h>
	#include <errno.h>
	#include <direct.h>
	#include <process.h>
#endif

#if _EL_CC_TURBOC__
	#include <errno.h>
	#include <io.h>
	#include <dir.h>
	#include <dos.h>
	#include <process.h>
#endif

#if _EL_CC_WATCOMC__
	#include <sys/types.h>
	#include <direct.h>
	#include <errno.h>
	#include <process.h>
#endif

#if _EL_OS_AMIGAOS__
	#include <exec/memory.h>
	#include <libraries/dos.h>
	#include <libraries/dosextens.h>
	#include <libraries/arpbase.h>
	#if AZTEC_C
		#include <functions.h>
	#endif
	#if LATTICE
		#include <proto/exec.h>
		#include <proto/dos.h>
		#include <proto/arp.h>
		#include <dos.h>
		extern struct   ArpBase *ArpBase;
	#endif
#endif

#if _EL_OS_MACOS__
	#include <Types.h>
	#include <Files.h>
	#include <ToolUtils.h>
	#include <AppleTalk.h>
	#include <ErrMgr.h>
	#include <unix.h>
	#include <Errors.h>
	#include <Memory.h>
	#include <Time.h>
#endif

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_FileExists                                                */
/*------------------------------------------------------------------------*/

/**************************************************************************
 *
 * EIO_FileExists
 *
 * SYNOPSIS
 *		int	 EIO_FileExists (const char* filename)
 *
 * PURPOSE
 *		test if a file exists.
 *
 * INPUT
 *		pointer to a NULL terminated string specifying a filename or
 *		filespec.
 *
 *
 * EXAMPLE
 *		#include <echidna/platform.h>
 *		#include "switches.h"
 *	
 *		#include <echidna/eio.h>
 *		#include <stdio.h>
 *		#include <stdlib.h>
 *
 *		int main (void)
 *		{
 *			char	*filename = "c:\myfile.txt";
 *			char	 string[80];
 *			FILE	*fp;
 *
 *			if (EIO_FileExists (filename)) {
 *				printf ("Are you sure you want to overwrite '%s'\n", filename);
 *				printf ("Y/N:");
 *				fflush (stdout);
 *				gets (string);
 *				if (string[0] == 'N') {
 *					return EXIT_FAILURE;
 *				}
 *			}
 *			fp = fopen (filename, "w");
 *			fprintf (fp, "Hmmm...");
 *			fclose (fp);
 *			return EXIT_SUCCESS;
 *		}	
 *
 *		
 *
 *
 * RETURN VALUE
 *		TRUE  = file does exists
 *		FALSE = file does NOT exists.
 *
 * BUGS
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *		EIO_FileType	
 *	
 * TOPICS
 *		Files, Fileexits
 *
*/
int	 EIO_FileExists (const char* filename)
{
#if _EL_OS_AMIGAOS__

	BPTR		lock;

	lock = Lock (filename, (uint32)ACCESS_READ);
	if (lock) {
		UnLock (lock);
		return TRUE;
	}
	return FALSE;

#else

	int		 fd;

	fd = open (filename, O_RDONLY);
	if (fd >= 0) {
		close (fd);
		return TRUE;
	}
	return FALSE;

#endif


} /* EIO_FileExists */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_ExpandEVars                                               */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * EIO_ExpandEVars
 *
 * SYNOPSIS
 *		char *EIO_ExpandEVarsWithErrors (char *new, const char* old, size_t maxlen, bool fCheckForErrors)
 *		char *EIO_ExpandEVars (char *new, const char* old, size_t maxlen)
 *
 * PURPOSE
 *		Expand all the environment vars in string old and put
 *		result in string 'new' but don't put more than maxlen
 *		characters.
 *
 * EXAMPLE
 *		#include <echidna/platform.h>
 *		#include "switches.h"
 *	
 *		#include <echidna/eio.h>
 *		#include <echidna/eerrors.h>
 *		#include <stdio.h>
 *	
 *		int main (void)
 *		{
 *			char	original[EIO_MAXPATH] = "%TMP%\MYFILE.TXT {TMP}\MYOTHER.TXT";
 *			char	new[EIO_MAXPATH];
 *
 *			EIO_ExpandEVars (new, original, EIO_MAXPATH);
 *			printf ("Expanded Environment Vars '%s'\n", new);
 *
 *			if (!EIO_ExpandEVarsWithErrors (new, original, EIO_MAXPATH, TRUE))
 *          {
 *         		printf ("%s\n", GlobalErrMsg);
 *  			ClearGlobalError();
 *          }
 *          else
 *          {
 *  			printf ("Expanded Environment Vars '%s'\n", new);
 *          }
 *
 *			return 0;
 *		}
 *
 * INPUTS
 *		new:	            string pointer to dest
 *		old:	            string pointer to src
 *		maxlen:             max number of character to put in dest.
 *      fCheckForErrors:    true = return an error (NULL) if a referenced
 *                          evinronment variable is undefined
 *
 * RESULTS
 *		returns new
 *
 * BUGS
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *		getenv
 *	
 * TOPICS
 *		Environment
*/
char *EIO_ExpandEVarsWithErrors (char *newstr, const char* old, size_t maxlen, bool fCheckForErrors)
{
	const char	*evar = NULL;
	char	*out = newstr;

	maxlen--;
	while (maxlen && *old)
	{
		if (!evar)
		{
			if (lchinstr (*old, "{%%") >= 0)
			{
				if (lchinstr (old[1], "{%%") >= 0)
				{
					*newstr++ = old[1];
					old++;
					maxlen--;
				}
				else
				{
					evar = old + 1;
				}
			}
			else
			{
				*newstr++ = *old;
				maxlen--;
			}
		}
		else
		{
			if (lchinstr (*old, "}%%") >= 0)
			{
				char	*s;
				char	 save;

				save = *old;
				*((char *)old) = '\0';

				s = getenv (evar);
				if (s)
				{
					while (*s && maxlen)
					{
						*newstr++ = *s++;
						maxlen--;
					}
				}
                else if (fCheckForErrors)
                {
        			SetGlobalErr (ERR_GENERIC);
        			GEcatf1 ("no env variable (%s)", evar);
        			return NULL;
                }

				*((char *)old) = save;
						
				evar = NULL;
			}
		}
		old++;
	}
	*newstr = '\0';

	return out;
}

char *EIO_ExpandEVars (char *newstr, const char* old, size_t maxlen)
{
    return EIO_ExpandEVarsWithErrors (newstr, old, maxlen, FALSE);
}


/*------------------------------------------------------------------------*/
/**# MODULE:EIO_FixDirSeps                                                */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * EIO_FixDirSeps
 *
 * SYNOPSIS
 *		char *EIO_FixDirSeps (char *str);	
 *
 * PURPOSE
 *		Convert directory seperator character to native system
 *
 * EXAMPLE
 *		#include <echidna/platform.h>
 *		#include "switches.h"
 *	
 *		#include <echidna/eio.h>
 *		#include <stdio.h>
 *	
 *		int main (void)
 *		{
 *			char	tmpdir[EIO_MAXPATH] = "D:\MYDIR\MYSUBDIR\MYFILE.TXT";
 *	
 *			EIO_FixDirSeps (tmpdir);
 *			printf ("Native file directory is '%s'\n", tmpdir);
 *			return 0;
 *		}
 *
 * INPUTS
 *		str:	string pointer to path to modify
 *
 * RESULTS
 *		returns str
 *
 * BUGS
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *		EIO_fnmerge, EIO_fnsplit, EIO_Filename, EIO_Name, EIO_Path, EIO_Ext
 *	
 * TOPICS
 *		Filenames, Filespecs
*/
char *EIO_FixDirSeps (
	char	*str
) {
	char	*s;

	//
	// skip first colon
	//
	s = strstr (str, ":");
	if (!s || *str == ':')
	{
		s = str;
	}
	else
	{
		s++;
	}

	while ((s = strpbrk (s, DIRSEPS)) != NULL)
	{
		*s++ = DIRSEP;
	}

	return str;
}

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_InsureEndSlash                                            */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * EIO_InsureEndSlash
 *
 * SYNOPSIS
 *		int	 EIO_InsureEndSlash (char *str);	
 *
 * PURPOSE
 *		To make sure a directory path has a backslash on the end.
 *		Only appends one if one isn't already there.
 *
 * EXAMPLE
 *		#include <echidna/platform.h>
 *		#include "switches.h"
 *	
 *		#include <echidna/eio.h>
 *		#include <stdio.h>
 *	
 *		int main (void)
 *		{
 *			char	tmpdir[EIO_MAXPATH];
 *	
 *			strcpy(tmpdir, getenv ("TMP"));
 *			EIO_InsureEndSlash (tmpdir);
 *			printf ("Temporary file directory is '%s'\n", tmpdir);
 *			return 0;
 *		}
 *
 * INPUTS
 *		str:	string buffer must have enough space to add another 
 *				character if required (May be NULL in which case nothing
 *				is appended and function returns 0).
 *
 * RESULTS
 *		Returns non-zero if appended a backslash. Else returns 0.
 *
 * BUGS
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *		EIO_fnmerge, EIO_fnsplit, EIO_Filename, EIO_Name, EIO_Path, EIO_Ext
 *	
 * TOPICS
 *		Filenames, Filespecs
*/
int	 EIO_InsureEndSlash (
	char	*str
) {
	size_t  len;
	char	c;

	if (!str) {
		return 0;
	}

	len = strlen (str);

	if (len) {
		c = str[len - 1];
		if (c != DIRSEP && c != ':') {
			str[len] = DIRSEP;
			str[len + 1] = '\0';
			return 1;
		}
	}
 	return 0;

} /* EIO_InsureEndSlash */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_GetTempFileName                                           */
/*------------------------------------------------------------------------*/

/**************************************************************************
 *
 * EIO_GetTempFileName
 *
 * SYNOPSIS
 *		void EIO_GetTempFileName (char *temp)
 *
 * PURPOSE
 *		Create a temp filename (including path to temp dir if available.)
 *		@b@NOTE@p@: on an amiga T: MUST be assigned to your temp dir.
 *
 * INPUT
 *		pointer to a buffer of characters at least EIO_MAXPATH big.
 *
 * EFFECTS
 *		buffer pointed to by temp will be filled with temporary filename.
 *
 * RETURN VALUE
 *		NONE
 *
 * BUGS
 *		Won't work correctly under Windows yet.  Will only create 9999
 *		filename and then it repeats.
 *
 * EXAMPLE
 *		#include <echidna/platform.h>
 *		#include "switches.h"
 *	
 *		#include <echidna/eio.h>
 *		#include <stdio.h>
 *	
 *		int main (void)
 *		{
 *			char	 tmpname[EIO_MAXPATH];
 *			FILE	*fp;
 *	
 *			EIO_GetTempFileName (tmpname);
 *			fp = fopen (tmpname, "w");
 *			fprintf (fp, "writing data to temporary file %s", tmpname);
 *			fclose (fp);
 *			return 0;
 *		}
 *	
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
void EIO_GetTempFileName (char *temp)
{
#if (_EL_CC_TURBOC__ || _EL_CC_WATCOMC__)
	static int	 tmpcnt = 0;
	char	*ptr;

	ptr = getenv ("TMP");
	if (ptr) {
		strcpy (temp, ptr);
		EIO_InsureEndSlash (temp);
	} else {
		*temp = '\0';
	}
	sprintf (&temp[strlen(temp)], "temp%d", tmpcnt++ % 9999);
#elif _EL_OS_AMIGAOS__
	static int	 tmpcnt = 0;

	sprintf (temp, "T:tmp-%02d-%05d", (int	)((struct Process *)FindTask(0L))->pr_TaskNum, tmpcnt++);

#elif _EL_OS_MACOS__	

{
	int save,tim;
    unsigned int	 net;
    OSErr err;
    char buf2[10];
	Str255 tempstr;
    static unsigned tmpnum = 1;
    int	 myNode,myNet;
    struct tm *t;
    time_t ltime;
    FInfo infoblk;

    strcpy(temp,"TMP");
    save = errno;
    err = GetNodeAddress(&myNode,&myNet);
    if(err != noMPPErr && myNet != 0)
    	{
        net = myNode << 8 | myNet;
	strcat(temp,itoa(net,buf2,16));
	}
    time(&ltime);
    t = localtime(&ltime);
    tim = (t->tm_hour << 16) | (t->tm_min << 8) | t->tm_sec;
    strcat(temp,itoa(tim,buf2,16));	/* append the hour */
    do
    {   
		strcat(temp,itoa(tmpnum++,buf2,10));
		strcpy((char *)&tempstr[1], temp);
		tempstr[0] = strlen(temp);	/* make a pascal string */
		err = GetFInfo((unsigned char *)tempstr,0,&infoblk);
    } while (err != fnfErr);
    errno = save;
}

#elif (_EL_OS_IRIX53__ || _EL_OS_WIN32__)
	static int	 tmpcnt = 0;
	char work[EIO_MAXPATH];

	printf ("EIO_GetTempFileName needs to put process&machine id in name\n");
	sprintf (work, "tmp-%02d-%05d", (int	)1, tmpcnt++);
	EIO_fnmerge (temp, getenv("TMP"), work, NULL);

#else
#error Need 'EIO_GetTempFileName'
#endif

} /* EIO_GetTempFileName */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_fnmerge                                                   */
/*------------------------------------------------------------------------*/

/**************************************************************************
 *
 * EIO_fnmerge
 *
 * SYNOPSIS
 *		void EIO_fnmerge (char *path, const char* dir, const char* name, const char* ext)
 *
 * PURPOSE
 *		Builds a path from component parts.
 *	
 *		Concatinates dir, name and ext into path.  If dir doesn't
 *		end with ':', '\' or '/' then the correct directory separator
 *		will be appened.
 *
 * INPUT
 *		path =	pointer to string to be filled in.  Must be EIO_MAXPATH
 *				in size.
 *		dir =	path of file as in 'X:' or 'X:\DIR\SUBDIR' or 'DF0:\DIR\'.
 *				May be NULL.
 *		name =	name of file as in 'MYFILE' or 'MYFILE.TXT'.  May be NULL.
 *		ext =	extention of file as in '.EXE' or '.COM' or '.TXT'
 *
 * EFFECTS
 *		string pointed to by path will contain newly created filespec.
 *
 * RETURN VALUE
 *		None.
 *
 * BUGS
 *
 * EXAMPLE
 *		#include <echidna/platform.h>
 *		#include "switches.h"
 *	
 *		#include <echidna/eio.h>
 *		#include <stdio.h>
 *	
 *		int main (void)
 *		{
 *			char	 newname[EIO_MAXPATH];
 *			FILE	*fp;
 *	
 *			EIO_fnmerge (newname, "C:\TESTDIR", "TESTFILE", ".TST");
 *			fp = fopen (newname, "w");
 *			fprintf (fp, "writing data to new file %s", newname);
 *			fclose (fp);
 *			return 0;
 *		}
 *		
 * HISTORY
 *
 *
 * SEE ALSO
 *		EIO_fnsplit, EIO_Filename, EIO_Name, EIO_Ext, EIO_Path
 *
*/
void EIO_fnmerge (char *path, const char* dir, const char* name, const char* ext)
{
	size_t	len   = 0;

	if (dir) {
		strcpy (path, dir);
		len   = strlen (path);
		if (len && !strchr(DIRSEPS, path[len - 1])) {
			path[len] = DIRSEP;
			len++;
			path[len] = 0;
		}
	}

	if (name && strlen(name)) {
		const char	*s;

		s = name;
		if (len && strchr(DIRSEPS, *s))
		{
			if (strchr(DIRSEPS, path[len - 1]))
			{
				s++;
			}
		}
			
		strcpy (&path[len], s);
		len = strlen (path);
	}

	if (ext) {
		strcpy (&path[len], ext);
	}

} /* EIO_fnmerge */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_fnsplit                                                   */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * EIO_fnsplit
 *
 * SYNOPSIS
 *		uint16 EIO_fnsplit (
 *				char	*path,
 *				char	*dir,
 *				char	*name,
 *				char	*ext)
 *
 * PURPOSE
 *		To extract the component parts of a file specification and put them
 *		in corresponding strings.
 *
 *		For example, if path = "df1:\mydir\subdir\myfile.test.4"
 *		then we get:
 *
 *			dir		= "df1:\mydir\subdir\"
 *			name	= "myfile.test"
 *			ext		= ".4"
 *
 * INPUTS
 *		path	: path string to extract component parts from.
 *		dir		: pointer to string buffer (may be NULL).
 *		name	: pointer to string buffer (may be NULL).
 *		ext		: pointer to string buffer (may be NULL).
 *
 * RESULTS
 *		Returns an uint16 (composed of five flags defined in echidna/eio.h)
 *		indicating which of the full path name components were present in
 *		'path'; these flags and the components they represent are
 *	
 *		EIO_DIRECTORY		A directory (and possibly subdirectories)
 *		EIO_FILENAME		A file name
 *		EIO_EXTENSION		An extension
 *
 *		The string buffers must be large enough to contain their extracted 
 *		component.  The maximum sizes that will be copied into them by
 *		EIO_fnsplit are given by the constants 'EIO_MAXDIR', 'EIO_MAXFILE'
 *		and 'EIO_MAXEXT' (defined in echidna/eio.h) and each size
 *		includes space for the null-terminator.
 *
 * EXAMPLE
 *		#include <echidna/platform.h>
 *		#include "switches.h"
 *	
 *		#include <echidna/eio.h>
 *		#include <stdio.h>
 *	
 *		int main (void)
 *		{
 *			char	 dir[EIO_MAXDIR];
 *			char	 name[EIO_MAXFILE];
 *			char	 ext[EIO_MAXEXT];
 *	
 *			EIO_fnsplit ("C:\TESTDIR\TESTFILE.TST", dir, name, ext);
 *			printf ("dir  = '%s'\n", dir);
 *			printf ("name = '%s'\n", name);
 *			printf ("ext  = '%s'\n", ext);
 *			return 0;
 *		}
 *		
 * BUGS
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *		EIO_fnmerge, EIO_Filename, EIO_Name, EIO_Ext, EIO_Path
 *
 *
*/
uint16 EIO_fnsplit (
	const char	*path,
	char	*dir,
	char	*name,
	char	*ext
) {
	uint16		flags = 0;
	int			rslashpos	= -1;
	int			rdotpos		= -1;

	rslashpos = rstrchinstr (DIRSEPS, path);
 	if (rslashpos >= 0) {
		if (dir) {
			strncpy (dir, path, (size_t)UTL_MIN (EIO_MAXDIR, rslashpos + 1));
			dir [UTL_MIN (EIO_MAXDIR, rslashpos + 1)] = '\0';
		}
		flags |= EIO_DIRECTORY;
		path += rslashpos + 1;
	} else if (dir)
		dir[0] = '\0';

	rdotpos = rchinstr ((char)'.', path);
	if (rdotpos < 0)
		rdotpos = strlen (path);
	if (rdotpos > 0) {
		if (name) {
			strncpy (name, path, (size_t)UTL_MIN (EIO_MAXFILE, rdotpos));
			name [UTL_MIN (EIO_MAXFILE, rdotpos)] = '\0';
		}
		flags |= EIO_FILENAME;
		path += rdotpos;
	} else if (name)
		name[0] = '\0';

	if (strlen (path)) {
		if (ext) {
			strncpy (ext, path, (size_t)UTL_MIN (EIO_MAXEXT, strlen (path)));
			ext [UTL_MIN (EIO_MAXEXT, strlen(path))] = '\0';
		}
		flags |= EIO_EXTENSION;
	} else if (ext)
		ext[0] = '\0';

	return flags;
} /* EIO_fnsplit */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_Path                                                      */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * EIO_Path
 *
 * SYNOPSIS
 *		char *EIO_Path (const char* filespec)
 *
 * PURPOSE
 *		return a pointer to a string that contains just the path
 *		of the passed filespec.  @b@NOTE@p@: the string is only valid
 *		until the LST_Next call to EIO_Path.
 *		
 *		Example:
 *			printf ("'%s'", EIO_Path ("Work:Gregg\TUME.txt"));
 *
 *		Prints:
 *			'Work:Gregg'
 *
 * INPUT
 *		pointer to a valid filespec.
 *
 *
 * RETURN VALUE
 *		pointer to a copy of the path of the filespec.
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *		EIO_fnsplit, EIO_fnmerge, EIO_Filename, EIO_Name, EIO_Ext
 *
*/
char *EIO_Path (const char* filespec)
{
	static char	 path[EIO_MAXPATH];

	EIO_fnsplit (filespec, path, NULL, NULL);
	return path;

} /* EIO_Path */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_Name                                                      */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * EIO_Name
 *
 * SYNOPSIS
 *		char *EIO_Name (const char* filespec)
 *
 * PURPOSE
 *		return a pointer to a string that contains just the name
 *		of the passed filespec.  @b@NOTE@p@: the string is only valid
 *		until the LST_Next call to EIO_Name
 *		
 *		Example:
 *			printf ("'%s'", EIO_Name ("Work:Gregg\TUME.txt"));
 *
 *		Prints:
 *			'TUME'
 *
 * INPUT
 *		pointer to a valid filespec.
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		pointer to a copy of the name of the filespec.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *		EIO_fnsplit, EIO_fnmerge, EIO_Filename, EIO_Name, EIO_Name
 *
*/
char *EIO_Name (const char* filespec)
{
	static char	 name[EIO_MAXPATH];

	EIO_fnsplit (filespec, NULL, name, NULL);
	return name;

} /* EIO_Name */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_Ext                                                       */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * EIO_Ext
 *
 * SYNOPSIS
 *		char *EIO_Ext (const char* filespec)
 *
 * PURPOSE
 *		return a pointer to a string that contains just the extension
 *		of the passed filespec.  @b@NOTE@p@: the string is only valid
 *		until the LST_Next call to EIO_Ext
 *		
 *		Example:
 *			printf ("'%s'", EIO_Ext ("Work:Gregg\TUME.txt"));
 *
 *		Prints:
 *			'.txt'
 *
 * INPUT
 *		pointer to a valid filespec.
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		pointer to a copy of the extension of the filespec.
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *		EIO_fnsplit, EIO_fnmerge, EIO_Filename, EIO_Name, EIO_Path
 *
*/
char *EIO_Ext (const char* filespec)
{
	static char	 ext[EIO_MAXPATH];

	EIO_fnsplit (filespec, NULL, NULL, ext);
	return ext;

} /* EIO_Ext */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_Filename                                                      */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * EIO_Filename
 *
 * SYNOPSIS
 *		char *EIO_Filename (const char* filespec)
 *
 * PURPOSE
 *		return a pointer to a string that contains just the FileName
 *		of the passed filespec.  @b@NOTE@p@: the string is only valid
 *		until the LST_Next call to EIO_Filename
 *		
 *		Example:
 *			printf ("'%s'", EIO_Filename ("Work:Gregg\TUME.txt"));
 *
 *		Prints:
 *			'TUME.txt'
 *
 * INPUT
 *		pointer to a valid filespec.
 *
 *
 * RETURN VALUE
 *		pointer to a copy of the filename of the filespec.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *		EIO_fnsplit, EIO_fnmerge, EIO_Path, EIO_Name, EIO_Ext
 *
*/
char *EIO_Filename (const char* filespec)
{
	static char	 filename[EIO_MAXPATH];
	char		 name[EIO_MAXPATH];
	char		 ext[EIO_MAXPATH];

	EIO_fnsplit (filespec, NULL, name, ext);
	EIO_fnmerge (filename, NULL, name, ext);
	return filename;

} /* EIO_Filename */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_CurrentDir                                                */
/*------------------------------------------------------------------------*/

/**************************************************************************
 *
 * EIO_CurrentDir
 *
 * SYNOPSIS
 *		char *EIO_CurrentDir (char *path)
 *
 * PURPOSE
 *		Store the path for the currect directory in 'path'.  (ie. if
 *		the current directory is 'VD0:work' then 'VD0:work\0' will be
 *		stored in the string pointed to by 'path'.  path must point to
 *		at least EIO_MAXPATH chars.
 *
 * INPUT
 *		pointer to buffer for path.
 *
 * RETURN VALUE
 *		path, if no errors.
 *		NULL, if some kind of error occured (see GlobalErr)
 *
 * EXAMPLE
 *		#include <echidna/platform.h>
 *		#include "switches.h"
 *	
 *		#include <echidna/eio.h>
 *		#include <stdio.h>
 *	
 *		int main (void)
 *		{
 *			char	 curdir[EIO_MAXDIR];
 *	
 *			if (EIO_CurrentDir (curdir)) {
 *				printf ("Current Directory is '%s'\n", curdir);
 *			}
 *			return 0;
 *		}
 *		
 *
 * BUGS
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
char *EIO_CurrentDir (char *path)
{

#if _EL_OS_MSDOS__
	#if _EL_CC_TURBOC__
		strcpy (path, "X:\\");
		path[0] = 'A' + getdisk ();
		getcurdir (0, path+3);
	#elif _EL_CC_WATCOMC__
	
		getcwd(path, EIO_MAXPATH);
	
	#endif
#elif _EL_OS_AMIGAOS__

	if (!(PathName (((struct Process *)(FindTask(0L)))->pr_CurrentDir, path, EIO_MAXPATH - 1))) {
		SetGlobalErr (ERR_GENERIC);
		GEprintf ("Couldn't get Current Directory Path");
		return NULL;
	}

#elif _EL_OS_MACOS__

	{
		OSErr		 io;
		Str255		 str;
		Str255		 start_path;
		Str255		 full_path;
		CInfoPBRec	 MyCIPB;
		int			 workVRef;
		long		 workDirID;
		
		full_path[0] = 0;
		str[0] = 0;
		
		memset (&MyCIPB, 0, sizeof (MyCIPB));
		
		io = HGetVol (NULL, &workVRef, &workDirID);
		if (io != noErr)
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf1 ("EIO_CurrentDir:Error calling HGetVol: %s\n", MacSystemErrMsg (io));
			return NULL;
		}
		
		MyCIPB.dirInfo.ioCompletion = 0L;
		MyCIPB.dirInfo.ioNamePtr    = str;
		MyCIPB.dirInfo.ioVRefNum    = workVRef;
		MyCIPB.dirInfo.ioDrDirID    = workDirID;
		MyCIPB.dirInfo.ioFDirIndex  = -1;
		
		while ( (io = PBGetCatInfo(&MyCIPB,FALSE)) == noErr)
		{
			pStrCopy(full_path,start_path);
			pStrCopy(MyCIPB.dirInfo.ioNamePtr,full_path);
			pCharAppend( ':',full_path);
			pStrAppend( start_path,full_path);
			MyCIPB.dirInfo.ioDrDirID = MyCIPB.dirInfo.ioDrParID;
		}
		
		full_path[full_path[0] + 1] = '\0';
		strcpy ((char *)path, (char *)&full_path[1]);
	}
	
#elif (_EL_OS_IRIX53__ || _EL_OS_WIN32__)

	getcwd(path, EIO_MAXPATH);

#else
#error Need 'EIO_CurrentDir'
#endif
	return path;

} /* EIO_CurrentDir */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_FreeDirTracker                                            */
/*------------------------------------------------------------------------*/

/**************************************************************************
 *
 * EIO_FreeDirTracker
 *
 * SYNOPSIS
 *		void EIO_FreeDirTracker (DirTracker *dt)
 *
 * PURPOSE
 *		Frees the DirTracker pointed to by dt.
 *
 * INPUT
 *		pointer to a DirTracker.
 *
 * RETURN VALUE
 *		None
 *
 * EXAMPLE
@DESC:
 *		See EIO_GetFirstFile, EIO_GetFirstPath or EIO_GetFirstEnv
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *		EIO_GetFirstFile, EIO_GetNextFile
 *		EIO_GetFirstPath, EIO_GetNextPath
 *		EIO_GetFirstEnv, EIO_GetNextEnv
 *	
*/
void EIO_FreeDirTracker (DirTracker *dt)
{

	if (dt) {
#if _EL_OS_AMIGAOS__
		if (dt->SubDir) {
			EIO_FreeDirTracker (dt->SubDir);
		}
		if (dt->Anchor) {
		  	FreeAnchorChain(dt->Anchor);
			FreeMem (dt->Anchor, sizeof (struct AnchorPath) + EIO_MAXPATH);
		}
		if (dt->EnvString) {
			free (dt->EnvString);
		}
#endif
#if _EL_OS_IRIX53__
		if (dt->dirp)
		{
			closedir (dt->dirp);
		}
#endif
		free (dt);
	}

} /* EIO_FreeDirTracker */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_GetNextPath                                               */
/*------------------------------------------------------------------------*/

/**************************************************************************
 *
 * EIO_GetNextPath
 *
 * SYNOPSIS
 *		int	 EIO_GetNextPath (DirTracker *dt)
 *
 * PURPOSE
 *		Get LST_Next path after call to EIO_GetFirstPath.  Call to get
 *		LST_Next path until dt->Status is FALSE;  Path is stored
 *		in dt->Path;
 *
 * INPUT
 *		dt = DirTracker as obtained from EIO_GetFirstPath;
 *
 * RETURN VALUE
 *		FALSE = error
 *
 * EXAMPLE
@DESC:
 *		See EIO_GetFirstPath
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *		EIO_GetFirstPath, EIO_FreeDirTracker
 *
*/
int	 EIO_GetNextPath (DirTracker *dt)
{
#if _EL_OS_MSDOS__ || _EL_OS_WIN32__

	char far32	*s   = dt->PathPtr;
	int		 pos;

	if (!*s) {
		dt->Status = FALSE;
	} else {
		pos = strcspn (dt->PathPtr, "!;");
		strncpy (dt->Path, dt->PathPtr, pos);
		dt->Path[pos] = '\0';
		dt->PathPtr  += pos;
		if (*dt->PathPtr == '!' || *dt->PathPtr == ';') {
			dt->PathPtr++;
		}
		dt->Status    = TRUE;
	}

	return TRUE;

#elif _EL_OS_AMIGAOS__

	if (!dt->APath) {
		dt->Status = FALSE;
	} else {
		if (!(PathName (dt->APath->path_Lock, dt->Path, EIO_MAXPATH - 1))) {
			SetGlobalErr (ERR_GENERIC);
			GEprintf ("Error from PathName 'EIO_GetNextPath()'");
			return FALSE;
		}
		dt->Status = TRUE;
		dt->APath  = (struct Path *)BADDR (dt->APath->path_Next);
	}

	return TRUE;

#elif _EL_OS_MACOS__

	char	*s   = dt->PathPtr;
	int		 pos;

	if (!*s) {
		dt->Status = FALSE;
	} else {
		pos = strcspn (dt->PathPtr, ",");
		strncpy (dt->Path, dt->PathPtr, pos);
		dt->Path[pos] = '\0';
		dt->PathPtr  += pos;
		if (*dt->PathPtr == ',') {
			dt->PathPtr++;
		}
		dt->Status    = TRUE;
	}

	return TRUE;

#elif _EL_OS_IRIX53__

	char	*s   = dt->PathPtr;
	int		 pos;

	if (!*s) {
		dt->Status = FALSE;
	} else {
		pos = strcspn (dt->PathPtr, ":");
		strncpy (dt->Path, dt->PathPtr, pos);
		dt->Path[pos] = '\0';
		dt->PathPtr  += pos;
		if (*dt->PathPtr == ':') {
			dt->PathPtr++;
		}
		dt->Status    = TRUE;
	}

	return TRUE;

#else
#error Need 'EIO_GetNextPath'
#endif

} /* EIO_GetNextPath */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_GetFirstPath                                              */
/*------------------------------------------------------------------------*/

/**************************************************************************
 *
 * EIO_GetFirstPath
 *
 * SYNOPSIS
 *		DirTracker *EIO_GetFirstPath (void)
 *
 * PURPOSE
 *		get path to first entry in command search path.
 *		Call EIO_GetNextPath for LST_Next entry in path and EIO_FreeDirTracker
 *		and free DirTracker allocated by EIO_GetFirstPath.
 *
 * INPUT
 *		None
 *
 * RETURN VALUE
 *		Pointer to a DirTracker filled out with the first path entry.
 *		or NULL if error.
 *
 * EXAMPLE
 *		#include <echidna/platform.h>
 *		#include "switches.h"
 *	
 *		#include <echidna/eio.h>
 *		#include <stdio.h>
 *	
 *		int main (void)
 *		{
 *			DirTracker	*dt;
 *			int			 aokay = TRUE;
 *	
 *			dt = EIO_GetFirstPath ();
 *			if (dt) {
 *				while (aokay && dt->Status) {
 *					printf ("Path Dir '%s'\n", dt->Path);
 *					aokay = EIO_GetNextPath (dt);
 *				}
 *				EIO_FreeDirTracker (dt);
 *			}
 *			if (!aokay || !dt) {
 *				printf ("Error printing path\n");
 *			}
 *			return 0;
 *		}
 *
 * BUGS
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *		EIO_GetNextPath, EIO_FreeDirTracker
 *	
*/
DirTracker *EIO_GetFirstPath (void)
{
#if _EL_OS_MSDOS__ || _EL_OS_WIN32__

	DirTracker	*dt;
	char		*ptr;

	if ((dt = calloc ((size_t)1, sizeof (DirTracker))) == NULL) {
		SetGlobalErr (ERR_OUT_OF_MEMORY);
		GEprintf ("Couldn't allocate DirTracker EIO_GetFirstPath()");
		return NULL;
	}
	ptr = getenv ("PATH");
	if (ptr) {
		strcpy (dt->TotalPath, ptr);
		dt->PathPtr = dt->TotalPath;
	}

	if (!EIO_GetNextPath (dt)) {
		SetGlobalErr (ERR_GENERIC);
		GEprintf ("Err in call to EIO_GetNextPath 'EIO_GetFirstPath()'");
		EIO_FreeDirTracker (dt);
		return NULL;
	}

	return dt;

#elif _EL_OS_AMIGAOS__

	DirTracker		*dt;
	struct Process	*proc;
	struct CommandLineInterface	*cli;

	if (!(dt = calloc ((size_t)1, sizeof (DirTracker)))) {
		SetGlobalErr (ERR_OUT_OF_MEMORY);
		GEprintf ("Couldn't allocate DirTracker EIO_GetFirstPath()");
		return NULL;
	}

	proc = (struct Process *)FindTask (0L);
	cli  = (struct CommandLineInterface *)(proc->pr_CLI << 2);
	dt->APath = (struct Path *)BADDR (cli->cli_CommandDir);

	if (!EIO_GetNextPath (dt)) {
		SetGlobalErr (ERR_GENERIC);
		GEprintf ("Err in call to EIO_GetNextPath EIO_GetFirstPath()");
		EIO_FreeDirTracker (dt);
		return NULL;
	}

	return dt;

#elif _EL_OS_MACOS__

	DirTracker	*dt;
	char		*ptr;

	if ((dt = calloc ((size_t)1, sizeof (DirTracker))) == NULL) {
		SetGlobalErr (ERR_OUT_OF_MEMORY);
		GEprintf ("Couldn't allocate DirTracker EIO_GetFirstPath()");
		return NULL;
	}
	ptr = getenv ("Commands");
	if (ptr) {
		dt->PathPtr = ptr;
	}

	if (!EIO_GetNextPath (dt)) {
		SetGlobalErr (ERR_GENERIC);
		GEprintf ("Err in call to EIO_GetNextPath 'EIO_GetFirstPath()'");
		EIO_FreeDirTracker (dt);
		return NULL;
	}

	return dt;

#elif _EL_OS_IRIX53__

	DirTracker	*dt;
	char		*ptr;

	if ((dt = calloc ((size_t)1, sizeof (DirTracker))) == NULL) {
		SetGlobalErr (ERR_OUT_OF_MEMORY);
		GEprintf ("Couldn't allocate DirTracker EIO_GetFirstPath()");
		return NULL;
	}
	ptr = getenv ("PATH");
	if (ptr) {
		dt->PathPtr = ptr;
	}

	if (!EIO_GetNextPath (dt)) {
		SetGlobalErr (ERR_GENERIC);
		GEprintf ("Err in call to EIO_GetNextPath 'EIO_GetFirstPath()'");
		EIO_FreeDirTracker (dt);
		return NULL;
	}

	return dt;

#else
#error Need 'EIO_GetFirstPath'
#endif

} /* EIO_GetFirstPath */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_SpawnProgram                                              */
/*------------------------------------------------------------------------*/
 
/**************************************************************************
 *
 * EIO_SpawnProgram
 *
 * SYNOPSIS
 *		int	 EIO_SpawnProgram (const char* program, const char* *argv)
 *
 * PURPOSE
 *		Start another program (suspending the calling program until
 *		the 'spawned' program terminates.
 *
 * INPUT
 *		program		= program name as in 'GREP.COM'
 *		argv		= arguments to program
 *
 * RETURN VALUE
 *		return value from previous program.  0 = success
 *
 * BUGS
 *	
 *	
 * EXAMPLE
 *		#include <echidna/platform.h>
 *		#include "switches.h"
 *	
 *		#include <echidna/eio.h>
 *		#include <stdio.h>
 *	
 *		int main (void)
 *		{
 *			char		*args[5];
 *			int			 aokay = TRUE;
 *	
 *			args[0] = "GREP.COM";
 *			args[1] = "-i+";
 *			args[2] = "echidna";
 *			args[3] = "*.h";
 *			args[4] = NULL;
 *			EIO_SpawnProgram ("GREP.COM", args);
 *			return 0;
 *		}
 *
 * TODO
 *		For MS-DOS, put program in EMS to free up memory.
 *	
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
int	 EIO_SpawnProgram (const char* program, const char* *argv)
{
#if _EL_OS_MSDOS__
	#if (_EL_CC_TURBOC__ || _EL_CC_WATCOMC__)
		return spawnvp (P_WAIT, program, argv);
	#else
	#error Need 'EIO_SpawnProgram'
	#endif
#elif _EL_OS_WIN32__

	return spawnvp (P_WAIT, program, argv);
	
#elif _EL_OS_AMIGAOS__
	#if AZTEC_C
		int		 result;
	
		result = fexecv (program, argv);
		if (result != -1) {
			result = wait ();
		}
	
		return result;
	
	#elif LATTICE
	
		int				result;
		struct	ProcID	child;
	
		result	= forkv (program, argv, NULL, &child);
		if (result	!= -1) {
			result	= wait (&child);
		}
	
		return	((int	) result);
	
	#else
	#error Need 'EIO_SpawnProgram'
	#endif

#elif _EL_OS_MACOS__
	
	{
		static	 int		started;
		FILE	*fp;
		
		if (!started)
		{
			started = TRUE;
			remove ("EioMacSpawnScript");
		}
		
		fp = fopen ("EioMacSpawnScript", "a");
		fprintf (fp, "%s", program);
		if (argv && *argv)
		{
			argv++;
			while (*argv)
			{
				fprintf (fp, " %s", *argv);
				argv++;
			}
		}
		fprintf (fp, "\n");
		fclose (fp);
	}
	return 0;
	
#elif _EL_OS_IRIX53__
	
	{
		static	 int		started;
		FILE	*fp;
		
		if (!started)
		{
			started = TRUE;
			remove ("unixspawnscript");
		}
		
		fp = fopen ("unixspawnscript", "a");
		fprintf (fp, "%s", program);
		if (argv && *argv)
		{
			argv++;
			while (*argv)
			{
				fprintf (fp, " %s", *argv);
				argv++;
			}
		}
		fprintf (fp, "\n");
		fclose (fp);
	}
	return 0;
	
#else
#error Need 'EIO_SpawnProgram'
#endif

} /* EIO_SpawnProgram */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_FileLength                                                */
/*------------------------------------------------------------------------*/

/**************************************************************************
 *
 * EIO_FileLength
 *
 * SYNOPSIS
 *		long EIO_FileLength (int fd)
 *
 * PURPOSE
 *		Return Size of file in bytes.  'fd' is a file descriptor as
 *		returned by EIO_Open.
 *
 * INPUT
 *		valid open file descriptor.
 *
 * RETURN VALUE
 *		-1 = error
 *
 * EXAMPLE
 *		#include <echidna/platform.h>
 *		#include "switches.h"
 *	
 *		#include <echidna/eio.h>
 *		#include <stdio.h>
 *	
 *		int main (void)
 *		{
 *			int			 fd;
 *	
 *			fd = EIO_ReadOpen ("C:\COMMAND.COM");
 *			printf ("Length of C:\\COMMAND.COM is %ld\n", EIO_FileLength (fd));
 *			EIO_Close (fd);
 *			return 0;
 *		}
 *
 * BUGS
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *		EIO_Open
 *	
*/
long EIO_FileLength (int fd)
{
#if _EL_OS_MSDOS__
	#if (_EL_CC_TURBOC__ || __ZTC__ || _EL_CC_WATCOMC__)
		return filelength (fd);
	#else
	#error Need 'EIO_FileLength'
	#endif
#elif _EL_OS_WIN32__
	return filelength (fd);
#elif _EL_OS_AMIGAOS__
	#if (AZTEC_C || LATTICE)
		long	 oldpos;
		long	 filelength;
	
		if ((oldpos = lseek (fd, 0L, 1)) == -1)	return -1L;
		if ((filelength = lseek (fd, 0L, 2)) == -1) return -1L;
		if ((oldpos = lseek (fd, oldpos, 0)) == -1) return -1L;
	
		return filelength;
		
	#else
	#error Need 'EIO_FileLength'
	#endif

#elif _EL_OS_MACOS__
	long	 oldpos;
	long	 filelength;

	if ((oldpos = lseek (fd, 0L, 1)) == -1)	return -1L;
	if ((filelength = lseek (fd, 0L, 2)) == -1) return -1L;
	if ((oldpos = lseek (fd, oldpos, 0)) == -1) return -1L;

	return filelength;
	
#elif _EL_OS_IRIX53__
	long	 oldpos;
	long	 filelength;

	if ((oldpos = lseek (fd, 0L, 1)) == -1)	return -1L;
	if ((filelength = lseek (fd, 0L, 2)) == -1) return -1L;
	if ((oldpos = lseek (fd, oldpos, 0)) == -1) return -1L;

	return filelength;
	
#else
#error Need 'EIO_FileLength'
#endif
} /* EIO_FileLength */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_DirTools                                                  */
/*------------------------------------------------------------------------*/

#if _EL_OS_MACOS__

#define MATCHANY	'Å'
#define MATCHZERO	255
#define MATCHONE	255
#define MATCHCHAR	'?'

#elif _EL_OS_IRIX53__

#define MATCHANY	'*'
#define MATCHZERO	255
#define MATCHONE	255
#define MATCHCHAR	'?'

#endif

#if (_EL_OS_IRIX53__ || _EL_OS_MACOS__)

static int file_str_chk_wild (const char* a1, const char* a0)
{
	char	d0;
	char	d1;
	char	d2;
	char	d7;

	const char	*a2;
	const char	*a3;

	if (!(*a0))
		return TRUE;

fscw_loop:
	d0 = *a0;
	if (d0 != MATCHANY) goto fscw_notast;

	d0 = MATCHCHAR;
	d1 = TRUE;
	goto fscw_inner;

fscw_notast:
	if (d0 != MATCHCHAR) goto fscw_notskip;

	d1 = FALSE;
	goto fscw_inner;

fscw_notskip:
	if (d0 != MATCHZERO) goto fscw_notcount;

	a0++;
	d0 = *a0;
	if (!d0) return TRUE;
	d1 = TRUE;
	goto fscw_inner;

fscw_notcount:
	d1 = FALSE;

fscw_inner:
	d2 = d0;
	a2 = a0;
	a3 = a1;
	a1++;

	d7 = 0;

fscw_testloop:
	if (d2 == MATCHCHAR) goto fscw_next;

	if (d2 != (*a3)) goto fscw_testfin;
fscw_next:
	a2++;
	a3++;
	d2 = *a2;
	goto fscw_check;

fscw_testfin:
	d7 = TRUE;
	if (!(*a3)) return FALSE;
	goto fscw_notyet;

fscw_check:
	if (!(*a3)) goto fscw_tlfin;
	if (!(*a2)) goto fscw_notyet;
	if ((*a2) == MATCHZERO) goto fscw_notyet;
	if ((*a2) != MATCHANY) goto fscw_testloop;
	goto fscw_notyet;

fscw_tlfin:
	if (!(*a2)) return TRUE;
	return FALSE;

fscw_notyet:
	if (d1) goto fscw_check2;
	if (!(*a2)) return FALSE;
	if (d7) return FALSE;
	goto fscw_innerfin;

fscw_check2:
	if ((*a2) == MATCHZERO) goto fscw_innerfin;
	if ((*a2) != MATCHANY) goto fscw_inner;

fscw_innerfin:
	a0 = a2;
	a1 = a3;

	goto fscw_loop;
}
#endif

/**************************************************************************
 *
 * NextFile
 *
 * SYNOPSIS
 *		int	 NextFile (DirTracker *dt)
 *
 * PURPOSE
 *		
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		FALSE = Error
 *
 * BUGS
 *
 *
 * HISTORY
 *		05/13/93 Thursday (dcc) - _EL_CC_TURBOC__ - check _doserrno vs errno
 *
 * SEE ALSO
 *
*/
static int	 NextFile (DirTracker *dt, int	 done)
{

#if _EL_CC_VC__

	while (!done && !dt->DirFlag && (dt->finddata.attrib & _A_SUBDIR) == _A_SUBDIR ||
            (!done && !strcmp (dt->finddata.name, ".")) ||
            (!done && !strcmp (dt->finddata.name, ".."))) {
//      EL_printf ("-->'%s'\n",dt->finddata.name);
		done = _findnext (dt->findHandle, &dt->finddata);
	}


//  EL_printf ("--->'%s' %s\n", dt->finddata.name, ((done == TRUE) ? "**DONE**" : ""));

	if (done) {
		if (errno == ENOENT) {
			dt->Status = FALSE;
			if (dt->findHandle != (-1)) _findclose (dt->findHandle);
//          EL_printf ("DONE 1\n");
			return TRUE;
		} else {
			if (dt->findHandle != (-1)) _findclose (dt->findHandle);
//          EL_printf ("DONE 2\n");
			return FALSE;
		}
	}

	dt->IsDir  = (dt->finddata.attrib & _A_SUBDIR) == _A_SUBDIR;
	dt->Status = TRUE;

	strcpy (dt->Path, dt->finddata.name);
	return TRUE;

#elif _EL_CC_TURBOC__

	while (!done && !dt->DirFlag && (dt->FFBlk.ff_attrib & FA_DIREC) ||
			!strcmp (dt->FFBlk.ff_name, ".") ||
			!strcmp (dt->FFBlk.ff_name, "..")) {
		done = findnext (&dt->FFBlk);
	}

	if (done) {
		if (_doserrno == ENMFILE) {
			dt->Status = FALSE;
			return TRUE;
		} else {
			return FALSE;
		}
	}

	dt->IsDir  = dt->FFBlk.ff_attrib & FA_DIREC;
	dt->Status = TRUE;

	strcpy (dt->Path, dt->FFBlk.ff_name);
	return TRUE;

#elif (_EL_CC_WATCOMC__ && _EL_OS_MSDOS__)

	while (!done && !dt->DirFlag && (dt->FFBlk.attrib & _A_SUBDIR))
	{
		done = _dos_findnext (&dt->FFBlk);
	}

	if (done)
	{
		if (errno == ENOENT)
		{
			dt->Status = FALSE;
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	dt->IsDir  = dt->FFBlk.attrib & _A_SUBDIR;
	dt->Status = TRUE;

	strcpy (dt->Path, dt->FFBlk.name);
	return TRUE;

#elif _EL_OS_AMIGAOS__

	while (!done && !dt->DirFlag && dt->Anchor->ap_Info.fib_DirEntryType >= 0) {
		done = FindNext (dt->Anchor);
	}

	if (done) {
		if (done == ERROR_NO_MORE_ENTRIES) {
			dt->Status = FALSE;
			return TRUE;
		} else {
			return FALSE;
		}
	}

	dt->IsDir  = dt->Anchor->ap_Info.fib_DirEntryType >= 0;
	dt->Status = TRUE;
	EIO_fnsplit (dt->Anchor->ap_Buf, NULL, dt->Path, NULL);
	EIO_fnsplit (dt->Anchor->ap_Buf, NULL, NULL, dt->Path + strlen(dt->Path));
	return TRUE;

#elif _EL_OS_MACOS__

	{
		CInfoPBRec	 myCPB;
		OSErr		 io;
		Str255		 filename;
		int			 done = FALSE;
		
		dt->Status = TRUE;

		myCPB.hFileInfo.ioFDirIndex	 = dt->fileNdx++;     		/* set up the index */
		filename[0] = 0;
		myCPB.hFileInfo.ioCompletion = 0L;
		myCPB.hFileInfo.ioNamePtr 	 = filename;				/* Where to stick the filename. */
		myCPB.hFileInfo.ioVRefNum 	 = dt->parentfss.vRefNum;			
		myCPB.hFileInfo.ioDirID		 = dt->parentfss.parID; 
		
		do
		{
			if ( (io = PBGetCatInfo(&myCPB,false)) == noErr)
			{
				filename[filename[0]+1] = '\0';
				
				strcpy (dt->Path, (char *)&filename[1]);
				dt->IsDir = (BitTst(&(myCPB.hFileInfo.ioFlAttrib),3)); 	/* Is it a folder? */
				if (!(dt->IsDir && !dt->DirFlag))
				{
					done = TRUE;
				}
			}
			else
			{
				if (io == fnfErr)
				{
					dt->Status = FALSE;
					done = TRUE;
				}
				else
				{
					SetGlobalErr (ERR_GENERIC);
					GEcatf1 ("\nEIO_NextFile:PBGetCatInfo() - %s", MacSystemErrMsg (io));
					return FALSE;
				}
			}
		}
		while (!done);
	}
	return TRUE;

#elif _EL_OS_IRIX53__

	{
		struct direct *dp;
		int done = FALSE;
		
		dt->Status = TRUE;
		
		do
		{
			dp = readdir (dt->dirp);
			if (dp)
			{
					//
					// get file type
					//
					char fullname[EIO_MAXPATH];
				
					strcpy (dt->Path, dp->d_name);
					EIO_fnmerge (fullname, dt->FullPath, dp->d_name, NULL);
					dt->IsDir = (EIO_FileType (fullname) == EIO_TYPE_DIRECTORY);
					if (strcmp (dt->Path, ".") && strcmp (dt->Path, ".."))
					{
						done = TRUE;
					}
			}
			else
			{
				dt->Status = FALSE;
				done = TRUE;
			}
		}
		while (!done);
	}
	return TRUE;
	
#else
#error Need 'NextFile'
#endif

} /* NextFile */

/**************************************************************************
 *
 * EIO_GetFirstFile
 *
 * SYNOPSIS
 *		DirTracker *EIO_GetFirstFile (const char* path, int	 dirflag)
 *
 * PURPOSE
 *		To get a directory of files.  EIO_GetFirstFile gets first
 *		filename that matches specification in 'path'.
 *
 * INPUT
 *		path	: Valid filespec filter like "*.*" or "*.TXT"
 *		dirflag	: TRUE if it should include directories ?!?!?
 *
 * RETURN VALUE
 *		NULL = error
 *		DirTracker->Status = TRUE (more files left)
 *		DirTracker->Status = FALSE (no more files.)
 *
 * EXAMPLE
 *		#include <echidna/platform.h>
 *		#include "switches.h"
 *	
 *		#include <echidna/eio.h>
 *		#include <stdio.h>
 *	
 *		int main (void)
 *		{
 *			DirTracker	*dt;
 *			int			 aokay = TRUE;
 *	
 *			dt = EIO_GetFirstFile ("*.*", FALSE);
 *			if (dt) {
 *				while (aokay && dt->Status) {
 *					printf ("'%s'\n", dt->Path);
 *					aokay = EIO_GetNextFile (dt);
 *				}
 *				EIO_FreeDirTracker (dt);
 *			}
 *			if (!aokay || !dt) {
 *				printf ("Error reading directory\n");
 *			}
 *			return 0;
 *		}
 *
 * BUGS
 *		Reading directory in MS-DOS is not re-entrant therefore if you
 *		are getting a directory do not do any other kinds of I/O until
 *		you are done.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
DirTracker *EIO_GetFirstFile (const char* path, int	 dirflag)
{
#if _EL_CC_VC__

	DirTracker	*dt;

	if ((dt = calloc ((size_t)1, sizeof (DirTracker))) == NULL) {
		SetGlobalErr (ERR_OUT_OF_MEMORY);
		GEprintf ("Couldn't allocate DirTracker EIO_GetFirstFile()");
		return NULL;
	}
	strcpy (dt->TotalPath, path);
	dt->DirFlag = dirflag;

	dt->findHandle = _findfirst (path, &dt->finddata);

#if 0
	if (dt->findHandle == (-1))
	{
		SetGlobalErr (ERR_GENERIC);
		GEcatf ("\nError in NextFile 'EIO_GetFirstFile()'");
		EIO_FreeDirTracker (dt);
		return NULL;
	}
#endif

	if (!NextFile (dt, dt->findHandle == (-1))) {
		SetGlobalErr (ERR_GENERIC);
		GEcatf ("\nError in NextFile 'EIO_GetFirstFile()'");
		EIO_FreeDirTracker (dt);
		return NULL;
	}

	return dt;

#elif _EL_CC_TURBOC__
	DirTracker	*dt;

	if ((dt = calloc ((size_t)1, sizeof (DirTracker))) == NULL) {
		SetGlobalErr (ERR_OUT_OF_MEMORY);
		GEprintf ("Couldn't allocate DirTracker EIO_GetFirstFile()");
		return NULL;
	}
	strcpy (dt->TotalPath, path);
	dt->DirFlag = dirflag;

	if (!NextFile (dt, findfirst (path, &dt->FFBlk, (dirflag ? FA_DIREC : 0)))) {
		SetGlobalErr (ERR_GENERIC);
		GEcatf ("\nError in NextFile 'EIO_GetFirstFile()'");
		EIO_FreeDirTracker (dt);
		return NULL;
	}

	return dt;

#elif (_EL_CC_WATCOMC__ && _EL_OS_MSDOS__)
	DirTracker	*dt;

	if ((dt = calloc ((size_t)1, sizeof (DirTracker))) == NULL)
	{
		SetGlobalErr (ERR_OUT_OF_MEMORY);
		GEprintf ("Couldn't allocate DirTracker EIO_GetFirstFile()");
		return NULL;
	}
	strcpy (dt->TotalPath, path);
	dt->DirFlag = dirflag;

	if (!NextFile (dt, _dos_findfirst (path, (dirflag ? _A_SUBDIR : 0), &dt->FFBlk)))
	{
		SetGlobalErr (ERR_GENERIC);
		GEprintf ("Error in NextFile 'EIO_GetFirstFile()'");
		EIO_FreeDirTracker (dt);
		return NULL;
	}

	return dt;

#elif _EL_OS_AMIGAOS__

	DirTracker	*dt;

	if (!(dt = calloc ((size_t)1, sizeof (DirTracker)))) {
		SetGlobalErr (ERR_OUT_OF_MEMORY);
		GEprintf ("Couldn't allocate DirTracker 'EIO_GetFirstFile()'");
		return NULL;
	}
	strcpy (dt->TotalPath, path);
	dt->DirFlag = dirflag;

	if (!(dt->Anchor = (struct AnchorPath *)AllocMem (sizeof(struct AnchorPath) + EIO_MAXPATH, NULL))) {
		SetGlobalErr (ERR_OUT_OF_MEMORY);
		GEprintf ("Couldn't allocate AnchorPath 'EIO_GetFirstFile()'");
		EIO_FreeDirTracker (dt);
		return NULL;
	}

	dt->Anchor->ap_StrLen = EIO_MAXPATH;

	if (!NextFile (dt, (int	)FindFirst (path, dt->Anchor))) {
		SetGlobalErr (ERR_GENERIC);
		GEprintf ("Error in NextFile 'EIO_GetFirstFile()'");
		EIO_FreeDirTracker (dt);
		return NULL;
	} 

	return dt;

#elif _EL_OS_MACOS__

	DirTracker	*dt;
	Str255		 pathname;
	char		 dir[EIO_MAXPATH];
	
	dt = calloc ((size_t)1, sizeof (DirTracker));
	if (!dt)
	{
		SetGlobalErr (ERR_OUT_OF_MEMORY);
		GEprintf ("Couldn't allocate DirTracker 'EIO_GetFirstFile()'");
		return NULL;
	}
	
	dt->DirFlag = dirflag;
	dt->fileNdx = 1;
	strcpy (dir, EIO_Path (path));
	strcpy (dt->Pattern, EIO_Filename (path));
	
	if (!strlen (dir))
	{
		strcpy (dir, ":");
	}
	
	pathname[0] = strlen (dir);
	strcpy (((char *)pathname) + 1, dir);
	
	{
		CInfoPBRec MyCIPB;
		OSErr io;
		
		MyCIPB.dirInfo.ioCompletion = 0L;
		MyCIPB.dirInfo.ioNamePtr    = (StringPtr) pathname;	
		MyCIPB.dirInfo.ioVRefNum    = 0;
		MyCIPB.dirInfo.ioDrDirID    = 0L;
		MyCIPB.dirInfo.ioFDirIndex  = 0;
		if ((io = PBGetCatInfo(&MyCIPB,FALSE)) != noErr)
		{
			Str255	temp;
			
			temp[0] = 0;
			pCharAppend (':', temp);
			pStrAppend (pathname, temp);
			pStrCopy (temp, pathname);
			
			MyCIPB.dirInfo.ioCompletion = 0L;
			MyCIPB.dirInfo.ioNamePtr    = (StringPtr) pathname;	
			MyCIPB.dirInfo.ioVRefNum    = 0;
			MyCIPB.dirInfo.ioDrDirID    = 0L;
			MyCIPB.dirInfo.ioFDirIndex  = 0;
			if ((io = PBGetCatInfo(&MyCIPB,FALSE)) != noErr)
			{
				SetGlobalErr (ERR_GENERIC);
				GEcatf1 ("EIO_GetFirstFile: PBGetCatInfo %s\n", MacSystemErrMsg(io));
				EIO_FreeDirTracker (dt);
				return NULL;
			}
		}
		
		{
			dt->parentfss.parID = MyCIPB.dirInfo.ioDrDirID;
			
			{
				HParamBlockRec	 volCPB;
				Str255			 volumeName;
				
				{
					int	 i;
					for (i = 1; i <= pathname[0]; i++)
					{
						volumeName[i] = pathname[i];
						if (pathname[i] == ':')
						{
							volumeName[0] = i;
							break;
						}
					}
					if (volumeName[volumeName[0]] != ':')
					{
						pCharAppend(':',volumeName);
					}
				}
				
				volCPB.volumeParam.ioCompletion = 0L;
				volCPB.volumeParam.ioNamePtr = volumeName;
				volCPB.volumeParam.ioVRefNum = 0;
				volCPB.volumeParam.ioVolIndex = -1;
				if ( (io = PBGetVInfo ( (ParmBlkPtr) &volCPB,FALSE)) == noErr)
				{
					dt->parentfss.vRefNum = volCPB.volumeParam.ioVRefNum;
				}
			}
		}
	}
	
	if (!EIO_GetNextFile (dt))
	{
		SetGlobalErr (ERR_GENERIC);
		GEprintf ("Error in NextFile 'EIO_GetFirstFile()'");
		EIO_FreeDirTracker (dt);
		return NULL;
	}
	
	return dt;

#elif _EL_OS_IRIX53__

	DirTracker	*dt;
	char		 dir[EIO_MAXPATH];
	
	dt = calloc ((size_t)1, sizeof (DirTracker));
	if (!dt)
	{
		SetGlobalErr (ERR_OUT_OF_MEMORY);
		GEprintf ("Couldn't allocate DirTracker 'EIO_GetFirstFile()'");
		return NULL;
	}
	
	dt->DirFlag = dirflag;
	EIO_fnmerge (dir, EIO_Path (path), ".", NULL);
	strcpy (dt->FullPath, EIO_Path (path));
	strcpy (dt->Pattern, EIO_Filename (path));

	dt->dirp = opendir (dir);
	if (!dt->dirp)
	{
		SetGlobalErr (ERR_GENERIC);
		GEcatf1 ("\nCouldn't read directory '%s'", path);
		EIO_FreeDirTracker (dt);
		return NULL;
	}
	
	if (!EIO_GetNextFile (dt))
	{
		SetGlobalErr (ERR_GENERIC);
		GEprintf ("Error in NextFile 'EIO_GetFirstFile()'");
		EIO_FreeDirTracker (dt);
		return NULL;
	}
	
	return dt;

#else
#error Need 'EIO_GetFirstFile'
#endif

} /* EIO_GetFirstFile */

/**************************************************************************
 *
 * EIO_GetNextFile
 *
 * SYNOPSIS
 *		int	 EIO_GetNextFile (DirTracker *dt)
 *
 * PURPOSE
 *		Get LST_Next filename that matches spec in 'path' that was specfied
 *		when calling EIO_GetFirstFile;
 *
 * INPUT
 *		dt	: pointer to a DirTracker obtained from EIO_GetFirstFile.
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		FALSE = error
 *		dt->Status = FALSE = done, no more filenames
 *
 * EXAMPLE
@DESC:
 *		See EIO_GetNextFile.
 *	
 * HISTORY
 *
 *
 * SEE ALSO
 *		EIO_GetNextFile
 *
*/
int	 EIO_GetNextFile (DirTracker *dt)
{
#if _EL_CC_VC__

	return NextFile (dt, _findnext (dt->findHandle, &dt->finddata));

#elif _EL_CC_TURBOC__

	return NextFile (dt, findnext (&dt->FFBlk));

#elif (_EL_CC_WATCOMC__ && _EL_OS_MSDOS__)

	return NextFile (dt, _dos_findnext (&dt->FFBlk));

#elif _EL_OS_AMIGAOS__

	return NextFile (dt, (int	)FindNext (dt->Anchor));

#elif _EL_OS_MACOS__

	{
		int		result;
		int		done;
		
		do
		{
			done = TRUE;
			result = NextFile (dt, FALSE);
			if (result && dt->Status)
			{
				if (dt->IsDir && !dt->DirFlag)
				{
					done = FALSE;
				}
				else
				{
					char	pattern[EIO_MAXPATH];
					char	path[EIO_MAXPATH];
					
				//	printf ("EIO--'%s', Pattern = %s\n", dt->Path, dt->Pattern);

					strcpy (path, dt->Path);
					strcpy (pattern, dt->Pattern);
					strlwr (path);
					strlwr (pattern);
					
					if (!file_str_chk_wild(path, pattern))
					{
						done = FALSE;
					}
				}
			}
		} while (!done);
		
		return result;
	}
	
#elif _EL_OS_IRIX53__

	{
		int		result;
		int		done;
		
		do
		{
			done = TRUE;
			result = NextFile (dt, FALSE);
			if (result && dt->Status)
			{
				if (dt->IsDir && !dt->DirFlag)
				{
					done = FALSE;
				}
				else
				{
					char	pattern[EIO_MAXPATH];
					char	path[EIO_MAXPATH];
					
				//	printf ("EIO--'%s', Pattern = %s\n", dt->Path, dt->Pattern);

					strcpy (path, dt->Path);
					strcpy (pattern, dt->Pattern);
				//	strlwr (path);
				//	strlwr (pattern);
					
					if (!file_str_chk_wild(path, pattern))
					{
						done = FALSE;
					}
				}
			}
		} while (!done);
		
		return result;
	}

#else
#error Need 'EIO_GetNextFile'
#endif
} /* EIO_GetNextFile */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_GetFileList                                               */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * EIO_GetLocalFileList
 *
 * SYNOPSIS
 *		static int	 EIO_GetLocalFileList (
 *							LST_LIST	*list,
 *							char		*path,
 *							int			 type)
 *
 * PURPOSE
 *		Get a directory and add a node of each file OR directory on the
 *		specified list. 
 *	
 *		values for type
 *	
 *			0 = get dirs for recursive director search
 *			1 = get files
 *			2 = get files and dirs
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
static int	 EIO_GetLocalFileList (
					LST_LIST	*list,
					char		*path,
					int			 type)
{
	DirTracker	*dt;
	LST_NODE	*nd;
	int			 aokay = TRUE;
	int			 doit;
	char		 dirspec[EIO_MAXPATH];

	strcpy (dirspec, path);
	if (!type)
	{
		EIO_fnmerge (dirspec, EIO_Path (path), DIRMATCHALL, NULL);
	}

//		printf ("EIO_GetLocalFileList ('%s',%d)\n", path, type);

	dt = EIO_GetFirstFile (dirspec, TRUE);
	if (dt)
	{
		while (aokay && dt->Status)
		{
			char	newspec[EIO_MAXPATH];
			char	workspec[EIO_MAXPATH];

			switch (type)
			{
			case 0:
				doit = dt->IsDir;
				EIO_fnmerge (
						workspec,
						EIO_Path (path),
						EIO_Name (dt->Path),
						EIO_Ext (dt->Path));
				EIO_fnmerge (newspec,
						workspec,
						EIO_Name (path),
						EIO_Ext (path));
						
				break;
			case 1:
				doit = !dt->IsDir; 
				EIO_fnmerge (
						newspec,
						EIO_Path (path),
						EIO_Name (dt->Path),
						EIO_Ext (dt->Path));
				break;
			case 2:
				doit = TRUE;
				EIO_fnmerge (
						newspec,
						EIO_Path (path),
						EIO_Name (dt->Path),
						EIO_Ext (dt->Path));
				break;
			}

			if (doit)
			{
				nd = LST_CreateNode (sizeof (LST_NODE), newspec);
				if (!nd)
				{
					SetGlobalErr (ERR_OUT_OF_MEMORY);
					GEcatf ("\nCouldn't create filelist node");
					return FALSE;
				}
				LST_AddTail (list, nd);
				nd->type = ((dt->IsDir) ?
							EIO_TYPE_DIRECTORY :
							EIO_TYPE_FILE );
			}
			aokay = EIO_GetNextFile (dt);
		}
		EIO_FreeDirTracker (dt);
	}
	if (!aokay || !dt)
	{
		SetGlobalErr (ERR_GENERIC);
		GEcatf ("\nTrouble reading directory");
		return FALSE;
	}

	return TRUE;
} /* EIO_GetLocalFileList */

/*********************************************************************
 *
 * EIO_GetFileList
 *
 * SYNOPSIS
 *		LST_LIST *EIO_GetFileList (
 *					char	*filespec,
 *					int		 recursive,
 *					int		 directories
 *				)
 *
 * PURPOSE
 *		Gets a directory of files specified by 'filespec' and puts
 *		it in a linked list.  If 'recursive' is TRUE, will desend into
 *		all subdirectories also.  If the 'directories' flag is set to TRUE,
 *		then directoies will be in the list.
 *
 *      List is guarenteed to be, first a dir, then all files in that dir, then a sub
 *
 *      Note: if you get recurisive list for *.jpg you will get all *.jpg files
 *      even in directories that don't end with '*.jpg'  If you set directories to TRUE
 *      you will still get all *.jpg files but you will only get directories in the list
 *      that end in .jpg
 *
 *      Example:
 *
 *      dir structure
 *
 *      d:\temp\foo.jpg				a file
 *      d:\temp\foo2.jpg			a file
 *      d:\temp\dira				a dir
 *      d:\temp\dira\goo.jpg	    a file
 *		d:\temp\dirb.jpg        	a dir
 *		d:\temp\dirb.jpg\moo.jpj	a file
 *          
 *      EIO_GetFileList ("d:\tmp\*.jpg", TRUE, TRUE);
 *
 *      List is:
 *
 *      d:\temp\foo.jpg				a file
 *      d:\temp\foo2.jpg			a file
 *      d:\temp\dira\goo.jpg	    a file
 *		d:\temp\dirb.jpg        	a dir
 *		d:\temp\dirb.jpg\moo.jpj	a file
 *      
 *      Notice "dira" is missing but "dira\goo.jpg" is not
 *
 * INPUT
 *		filespec    = directory specification as in '*.*', '*.lbm' or 'mydir\*.lbm'
 *		recursive   = TRUE = go into subdirectories
 *		directories = put directories in list
 *
 * RETURN VALUE
 *		NULL if error, otherwise a standard linked list of files
 *
 * EXAMPLE
 *		#include <echidna/platform.h>
 *		#include "switches.h"
 *	
 *		#include <echidna/eio.h>
 *		#include <echidna/listfunc.h>
 *		#include <stdio.h>
 *	
 *		int main (void)
 *		{
 *			LST_LIST	*fileList;
 *	
 *			fileList = EIO_GetFileList ("*.*", TRUE, TRUE);
 *			if (fileList)
 *			{
 *				LST_NODE	*nd;
 *	
 *				nd = LST_Head (fileList);
 *				while (!LST_EndOfList (nd))
 *				{
 *					switch (EIO_FileListType(nd))
 *					{
 *					case EIO_TYPE_FILE:
 *						printf ("     File : %s\n", LST_NodeName (nd));
 *						break;
 *					case EIO_TYPE_DIRECTORY:
 *						printf ("Directory : %s\n", LST_NodeName (nd));
 *						break;
 *					}
 *	
 *					nd = LST_Next (nd);
 *				}
 *	
 *				DeleteList (fileList);
 *			}
 *			return 0;
 *		}
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
LST_LIST *EIO_GetFileList (
			const char	*filespec,
			int		 recursive,
			int		 directories
		)
{
	LST_LIST	*fileList;
	LST_LIST	 dirListX;
	LST_LIST	*dirList;
	LST_NODE	*nd;

	fileList = LST_CreateList (NULL);
	if (!fileList)
	{
		SetGlobalErr (ERR_OUT_OF_MEMORY);
		GEcatf ("\nCouldn't create filelist");
		return NULL;
	}

	dirList = &dirListX;
	LST_InitList (dirList);

	nd = LST_CreateNode (sizeof (LST_NODE), filespec);
	if (!nd)
	{
		SetGlobalErr (ERR_OUT_OF_MEMORY);
		GEcatf ("\nCouldn't create initial filelist node");
		return NULL;
	}
	LST_AddTail (dirList, nd);

	while ((nd = LST_RemTail (dirList)) != 0)
	{
		if (recursive)
		{
			if (!EIO_GetLocalFileList (dirList, LST_NodeName (nd), 0))
			{
				return NULL;
			}
		}

		if (!EIO_GetLocalFileList (fileList, LST_NodeName (nd), 1 + directories))
		{
			return NULL;
		}
		LST_DeleteNode (nd);
	}

	return fileList;
} /* EIO_GetFileList */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_FindFile                                                  */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * EIO_FindFile
 *
 * SYNOPSIS
 *		int	 EIO_FindFile (
 *			char	*filename,
 *			char	*guidname,
 *			char	*pathbuff
 *		)
 *
 * PURPOSE
 *		To find 'filename' by searching the current directory.  If not in
 *		current directory then search for it in any directory in the DOS
 *		environment path that contains the file 'guidename'.  
 *		The full path of the found file is copied into
 *		'path' which must be big enough to hold it. 
 *	
 *		If no 'guidename' is given then 'filename' can exist anywhere
 *		in path.
 *
 * INPUT
 *		filename:	ptr to filename you want to find in path
 *		guidname:	ptr to Filename of file that must be in the same
 *					directory for find to succeed.
 *		pathbuff:	ptr to char buffer of size EIO_MAXPATH
 *		
 *
 * EFFECTS
 *		pathbuff:	Gets filled in with path of found file.
 *					Not touched on failure.
 *
 * EXAMPLE
 *		#include <echidna/platform.h>
 *		#include "switches.h"
 *	
 *		#include <echidna/eio.h>
 *		#include <stdio.h>
 *	
 *		int main (void)
 *		{
 *			char		 mydatapath[EIO_MAXPATH];
 *	
 *			if (EIO_FindFile ("tdconfig.td","td.exe",mydatapath)) {
 *				printf ("Found it at '%s'\n", mydatapath);
 *			}
 *			return 0;
 *		}
 *
 *		
 * RETURN VALUE
 *		Returns non-zero if succesful, else returns 0.
 *
 * HISTORY
 *	01/29/90 Monday - Now just tries to open file in each directory. (GAT)
 *					  instead of scanning the directory.  Also uses new
 *					  'generic' Echidna I/O Routines.
 *
 *
 * SEE ALSO
 *
*/
int	 EIO_FindFile (
	const char	*filename,
	const char	*guidename,
	char	*pathbuff
) {
	DirTracker	*dt;
	int			 success = FALSE;
	int			 aokay   = TRUE;

	if (EIO_FileExists (filename)) {
		strcpy (pathbuff, filename);
		return TRUE;
	}

	dt = EIO_GetFirstPath ();
	if (dt) {
		while (aokay && !success && dt->Status) {
			EIO_fnmerge (pathbuff, dt->Path, guidename, NULL);
			if (!guidename || EIO_FileExists (pathbuff)) {
				EIO_fnmerge (pathbuff, dt->Path, filename, NULL);
				if (EIO_FileExists (pathbuff)) {
					success = TRUE;
				}
			}
			if (!success) {
				aokay = EIO_GetNextPath (dt);
			}
		}
		EIO_FreeDirTracker (dt);
	}

	return success;

} /* EIO_FindFile */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_PrintEscString                                            */
/*------------------------------------------------------------------------*/

/**************************************************************************
 *
 * EIO_PrintEscString 
 *
 * SYNOPSIS
 *		void EIO_PrintEscString (FILE *fp, const char* str)
 *
 * PURPOSE
 *		Print a string with 'C' style escape sequences (ie '\n' '\t'...);
 *
 * INPUT
 *		fp	: a standard C style FILE pointer.
 *		str	: string to be 'printed' into file.
 *
 * EFFECTS
 *		str is printed into file fp with standard 'C' style escape
 *		sequences converted into their respective values.  Respective
 *		values are compiler dependant.
 *	
 *		Codes handled are:
@SEPARATOR:
 *		@b@Code	Translation@p@
@SEPARATOR:
 *		\a		Audible Bell
 *		\b		Backspace
 *		\f		Formfeed
 *		\n		LineFeed
 *		\r		Carriage Return
 *		\t		Horizontal Tab
 *		\v		Vertical Tab
 *		\\		BackSlash
 *		\xH		character specified by H string of HEX digits. (8-bit only)
 *		\XH		character specified by H string of HEX digits. (8-bit only)
 *		\O		character specified by O string of OCTAL digits. (8-bit only)
@SEPARATOR:
 *
 *		For any code not covered above the '\' is just stripped meaning
 *		'\g' becomes 'g' and '\$' becomes '$'.
 *
 * RETURN VALUE
 *		None.
 *
 * EXAMPLE
 *		#include <echidna/platform.h>
 *		#include "switches.h"
 *	
 *		#include <echidna/eio.h>
 *		#include <stdio.h>
 *	
 *		int main (int argc, char **argv)
 *		{
 *			int		 ndx;
 *	
 *			for (ndx = 1; ndx < argc; ndx++) {
 *				EIO_PrintEscString (stdout, argv[ndx]);
 *			}
 *			return 0;
 *		}
 *	
 * BUGS
 *		There is no return value.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
int	 EIO_PrintEscString (FILE *fp, const char* s)
{
	int		 c;
	char	 digit[10];
	int		 digits = 0;
	int		 esc = FALSE;
	int		 hex = FALSE;
	int		 oct = FALSE;
	int		 count = 0;
	int		 valid;

	while (*s) {
		if (oct) {
			valid = (*s >= '0' && *s <= '7');
			if (valid || digits < 3) {
				if (valid) {
					digit[digits++] = *s++;
				}
				if (!(*s >= '0' && *s <= '7') || digits == 3) {
					oct = FALSE;
					digit[digits] = '\0';
					sscanf (digit, "%o", &c);
					if (putc (c, fp) == EOF) {
						return EOF;
					}
					count++;
				} else {
					continue;
				}
			}
		} else if (hex) {
			valid = isxdigit (*s);
			if (valid || digits < 2) {
				if (valid) {
					digit[digits++] = *s++;
				}
				if (!isxdigit(*s) || digits == 2) {
					hex = FALSE;
					digit[digits] = '\0';
					sscanf (digit, "%x", &c);
					if (putc (c, fp) == EOF) {
						return EOF;
					}
					count++;
				} else {
					continue;
				}
			}
		}
		if (!esc && *s == '\\') {
			esc = TRUE;
			s++;
		} else if (esc) {
			esc = FALSE;
			  c = -1;
			switch (*s) {
			case 'a': c = '\a'; break;
			case 'b': c = '\b'; break;
			case 'f': c = '\f'; break;
			case 'n': c = '\n'; break;
			case 'r': c = '\r'; break;
			case 't': c = '\t'; break;
			case 'v': c = '\v'; break;
			case 'X':
			case 'x': hex = TRUE; digits = 0; break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
					  oct = TRUE; digits = 1; digit[0] = *s; break;
			default:  c = *s; break;
			}
			if (c >= 0) {
				if (putc (c, fp) == EOF) {
					return EOF;
				}
				count++;
			}
			s++;
		} else {
			if (putc (*s, fp) == EOF) {
				return EOF;
			}
			count++;
			s++;
		}
	}

	return count;

} /* EIO_PrintEscString  */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_GetNextEnv                                                */
/*------------------------------------------------------------------------*/

/**************************************************************************
 *
 * EIO_GetNextEnv
 *
 * SYNOPSIS
 *		int	 EIO_GetNextEnv (DirTracker *dt)
 *
 * PURPOSE
 *		Get LST_Next environment variable after call to EIO_GetFirstEnv.
 *		Call to get LST_Next environment variable until
 *		dt->Status = FALSE;  Env is stored in dt->Path;
 *
 * INPUT
 *		dt = DirTracker as obtained from EIO_GetFirstEnv;
 *
 * RETURN VALUE
 *		FALSE = error
 *
 * EXAMPLE
@DESC:
 *		See EIO_GetFirstEnv
 *	
 * BUGS
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *	EIO_GetFirstEnv
 *	
*/
int	 EIO_GetNextEnv (DirTracker *dt)
{
#if (_EL_OS_MSDOS__)
	char far32	*s   = dt->PathPtr;
	char far32	*d;
	size_t	 len;

	if (!*s) {
		dt->Status = FALSE;
	} else {
		d   = strchr (s, '=');
		len = (size_t)(d - s);
		if (d) {
			strncpy (dt->Path, s, len);
			dt->Path[len] = '\0';
			dt->EnvString = d + 1;
		} else {
			strcpy (dt->Path, s);
		}
		dt->PathPtr += strlen (s) + 1;
		dt->Status   = TRUE;
	}

	return TRUE;

#elif _EL_OS_AMIGAOS__

	DirTracker	*pdt;
	DirTracker	*newdt;

	pdt = dt->PrivateDir;

	/** find lowest subdirectory **/
	while (pdt->SubDir) {
		pdt = pdt->SubDir;
	}

	for (;;) {

		/** remove finished subdirs **/
		while (pdt && !pdt->Status) {
			newdt = pdt->ParentDir;
			EIO_FreeDirTracker (pdt);
			pdt = newdt;
			if (pdt) {
				pdt->SubDir = NULL;
			}
		}

		/** If none left we're done! **/
		if (!pdt) {
			dt->Status = FALSE;
			return TRUE;
		}
	
		/** if subdir go into it **/
		if (pdt->IsDir) {
			char	newpath[EIO_MAXPATH];

			EIO_fnsplit (pdt->TotalPath, newpath, NULL, NULL);
			EIO_InsureEndSlash (newpath);
			strcat (newpath, pdt->Path);
			EIO_InsureEndSlash (newpath);
			strcat (newpath, "#?");

			newdt = EIO_GetFirstFile (newpath, TRUE);
			if (!newdt) {
				return FALSE;
			}
			pdt->SubDir      = newdt;
			newdt->ParentDir = pdt;

			if (!EIO_GetNextFile (pdt)) {
				return FALSE;
			}
			pdt = newdt;
		} else {
			char	newpath[EIO_MAXPATH];

			EIO_fnsplit (pdt->TotalPath + 4, newpath, NULL, NULL);
			EIO_fnmerge (dt->Path, newpath, pdt->Path, NULL);
			dt->Status = TRUE;

			if (!EIO_GetNextFile (pdt)) {
				return FALSE;
			}

			{
				int		fh;
				long	len;

				EIO_fnmerge (newpath, "ENV:", dt->Path, NULL);
				fh = EIO_ReadOpen (newpath);
				if (!fh) {
					SetGlobalErr (ERR_GENERIC);
					GEprintf1 ("(1) Couldn't read environment var '%s'", dt->Path);
					return FALSE;
				}

				len = EIO_FileLength (fh);
				if (len == (-1)) {
					SetGlobalErr (ERR_GENERIC);
					GEprintf1 ("(2) Couldn't read environment var '%s'", dt->Path);
					return FALSE;
				}

				if (dt->EnvString) {
					free (dt->EnvString);
					dt->EnvString = NULL;
				}

				dt->EnvString = malloc (len + 1);
				if (!dt->EnvString) {
					SetGlobalErr (ERR_OUT_OF_MEMORY);
					GEprintf1 ("(2) OOM reading environment var '%s'", dt->Path);
					return FALSE;
				}

				if (len != EIO_Read (fh, dt->EnvString, len)) {
					SetGlobalErr (ERR_GENERIC);
					GEprintf1 ("(3) Couldn't read environment var '%s'", dt->Path);
					return FALSE;
				}

				EIO_Close (fh);
				dt->EnvString[len] = '\0';
			}

			return TRUE;
		}
	}

#elif _EL_OS_MACOS__

	char **envp   = (char **)dt->PathPtr;

	if (!*envp) {
		dt->Status = FALSE;
	} else {
		strcpy (dt->Path, *envp);
		dt->EnvString = *envp + strlen (*envp) + 1;
		dt->PathPtr   = (char *)(envp + 1);
		dt->Status    = TRUE;
	}

	return TRUE;

#elif (_EL_OS_IRIX53__ || _EL_OS_WIN32__)

	char **envp   = (char **)dt->PathPtr;

	if (!*envp) {
		dt->Status = FALSE;
	} else {
		char	*eq;
		int	len;

		eq = strchr (*envp, '=');
		len = (eq) ? (eq - *envp) : strlen (*envp);
		strncpy (dt->Path, *envp, (len));
		dt->EnvString = *envp + len + 1;
		dt->PathPtr   = (char *)(envp + 1);
		dt->Status    = TRUE;
	}

	return TRUE;

#else
#error Need 'EIO_GetNextEnv'
#endif

} /* EIO_GetNextEnv */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_GetFirstEnv                                               */
/*------------------------------------------------------------------------*/

/**************************************************************************
 *
 * EIO_GetFirstEnv
 *
 * SYNOPSIS
 *		DirTracker *EIO_GetFirstEnv (void)
 *
 * PURPOSE
 *		get value of first environment variable in environment.
 *		Call EIO_GetNextEnv for LST_Next variable.
 *
 * INPUT
 *		None		
 *
 * EFFECTS
 *		dt->Path	: environment string as in 'PROMPT=$p$g'
 *		dt->Status	: FALSE if no more environment variables.
 *
 *
 * RETURN VALUE
 *		NULL = error.
 *
 *
 * EXAMPLE
 *	
 *		#include <echidna/platform.h>
 *		#include "switches.h"
 *	
 *		#include <echidna/eio.h>
 *		#include <stdio.h>
 *	
 *		int main (void)
 *		{
 *			DirTracker	*dt;
 *			int			 aokay = TRUE;
 *	
 *			dt = EIO_GetFirstEnv ();
 *			if (dt) {
 *				while (aokay && dt->Status) {
 *					printf ("'%s'='%s'\n", dt->Path, dt->EnvString);
 *					aokay = EIO_GetNextEnv (dt);
 *				}
 *				EIO_FreeDirTracker (dt);
 *			}
 *			if (!aokay || !dt) {
 *				printf ("Error reading environment variables\n");
 *			}
 *			return 0;
 *		}
 *	
 * BUGS
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *		EIO_GetNextEnv
 *
*/
DirTracker *EIO_GetFirstEnv (void)
{
#if (_EL_CC_WATCOMC__ && _EL_OS_MSDOS__)
	#if 0
	DirTracker	*dt;
	char far32	*ptr;

	if ((dt = calloc ((size_t)1, sizeof (DirTracker))) == NULL)
	{
		SetGlobalErr (ERR_OUT_OF_MEMORY);
		GEprintf ("Couldn't allocate DirTracker EIO_GetFirstPath()");
		return NULL;
	}

	/* Find Environment Block from PSP */

	ptr = MK_FP32 (_psp, 0);
	ptr = MK_FP32 (*((unsigned int	 *)(&ptr[0x2C])), 0);
	EL_printf("%s\n", ptr);
	dt->PathPtr = ptr;

	if (!EIO_GetNextEnv (dt))
	{
		SetGlobalErr (ERR_GENERIC);
		GEprintf ("Err in call to EIO_GetNextEnv 'EIO_GetFirstEnv()'");
		EIO_FreeDirTracker (dt);
		return NULL;
	}
	return dt;
	#else
	puts ("EIO_GetFirstEnv not implemented");
	return NULL;
	#endif

#elif _EL_OS_AMIGAOS__
	
	DirTracker	*dt;
	DirTracker	*pdt;

	if ((dt = calloc ((size_t)1, sizeof (DirTracker))) == NULL) {
		SetGlobalErr (ERR_OUT_OF_MEMORY);
		GEprintf ("Couldn't allocate DirTracker EIO_GetFirstPath()");
		return NULL;
	}

	pdt = EIO_GetFirstFile ("ENV:#?", TRUE);
	if (pdt) {
		dt->PrivateDir = pdt;
		if (EIO_GetNextEnv (dt)) {
			return dt;
		}
	}
	EIO_FreeDirTracker (dt);

	return NULL;

#elif _EL_OS_MACOS__

	DirTracker	*dt;
	char		*ptr;

	if ((dt = calloc ((size_t)1, sizeof (DirTracker))) == NULL) {
		SetGlobalErr (ERR_OUT_OF_MEMORY);
		GEprintf ("Couldn't allocate DirTracker EIO_GetFirstPath()");
		return NULL;
	}

	{
		extern char *_ArgC[];
		char	**envp;
		
		envp = (char **)_ArgC[2];
		dt->PathPtr = (char *)envp;
	}

	if (!EIO_GetNextEnv (dt)) {
		SetGlobalErr (ERR_GENERIC);
		GEprintf ("Err in call to EIO_GetNextEnv 'EIO_GetFirstEnv()'");
		EIO_FreeDirTracker (dt);
		return NULL;
	}
	return dt;

#elif (_EL_OS_IRIX53__ || (_EL_CC_TURBOC__ && __MSDOS32X__) || _EL_OS_WIN32__)

	DirTracker	*dt;

	if ((dt = calloc ((size_t)1, sizeof (DirTracker))) == NULL) {
		SetGlobalErr (ERR_OUT_OF_MEMORY);
		GEprintf ("Couldn't allocate DirTracker EIO_GetFirstPath()");
		return NULL;
	}

	{
		extern char **_environ;
		char	**envp;
		
		envp = (char **)_environ;
		dt->PathPtr = (char *)envp;
	}

	if (!EIO_GetNextEnv (dt)) {
		SetGlobalErr (ERR_GENERIC);
		GEprintf ("Err in call to EIO_GetNextEnv 'EIO_GetFirstEnv()'");
		EIO_FreeDirTracker (dt);
		return NULL;
	}
	return dt;

#else
#error Need 'EIO_GetFirstEnv'
#endif

} /* EIO_GetFirstEnv */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_FileType                                                  */
/*------------------------------------------------------------------------*/
 
/*********************************************************************
 *
 * EIO_FileType
 *
 * SYNOPSIS
 *		int	 EIO_FileType (const char* filename)
 *
 * PURPOSE
 *		Is specifed filename a directory or not
 *
 * INPUT
 *		filename	: pointer to filename or filespec.
 *
 * RETURN VALUE
 *		FALSE				= error (probably file does not exists)
 *		EIO_TYPE_FILE		= filename is a standard file
 *		EIO_TYPE_DIRECTORY	= filename is a directory.
 *
 * EXAMPLE
 *		#include <echidna/platform.h>
 *		#include "switches.h"
 *	
 *		#include <echidna/eio.h>
 *		#include <stdio.h>
 *	
 *		int main (int argc, char **argv)
 *		{
 *			int		 filetype;
 *	
 *			filetype = EIO_FileType ("C:\\COMMAND.COM");
 *			if (!filetype) {
 *				printf ("C:\\COMMAND.COM not found\n");
 *			} else if (filetype == EIO_TYPE_FILE) {
 *				printf ("C:\\COMMAND.COM is a file\n");
 *			} else if (filetype == EIO_TYPE_DIRECTORY) {
 *				printf ("C:\\COMMAND.COM is a directory\n");
 *			}
 *	
 *			filetype = EIO_FileType ("C:\\DOS");
 *			if (!filetype) {
 *				printf ("C:\\DOS not found\n");
 *			} else if (filetype == EIO_TYPE_FILE) {
 *				printf ("C:\\DOS is a file\n");
 *			} else if (filetype == EIO_TYPE_DIRECTORY) {
 *				printf ("C:\\DOS is a directory\n");
 *			}
 *			return 0;
 *		}
 *	
 *	
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
int	 EIO_FileType (const char* filename)
{

#if _EL_OS_WIN32__
{
	struct stat fileStat;

	if (!stat (filename, &fileStat))
	{
		if ((fileStat.st_mode & S_IFMT) == S_IFDIR)
		{
			return EIO_TYPE_DIRECTORY;
		}
		return EIO_TYPE_FILE;
	}

	return FALSE;
}


#elif _EL_OS_MSDOS__
	#if _EL_CC_TURBOC__
		{
			int	ac;
			int fd;
	
			ac = access (filename, 6);
			if (ac != (-1)) {
				fd = open (filename, O_RDONLY);
				if (fd != (-1)) {
					close (fd);
					return EIO_TYPE_FILE;
				}
				return EIO_TYPE_DIRECTORY;
			}
	
			return FALSE;
		}
	
	#elif _EL_CC_WATCOMC__
	
		{
			int	ac;
			int fd;
	
			ac = access (filename, W_OK | R_OK);
			if (ac != (-1)) {
				fd = open (filename, O_RDONLY);
				if (fd != (-1)) {
					close (fd);
					return EIO_TYPE_FILE;
				}
				return EIO_TYPE_DIRECTORY;
			}
	
			return FALSE;
		}
	
	#elif (__ZTC__ && _EL_OS_MSDOS__)
	
		{
			int	ac;
			int fd;
	
			ac = access (filename, W_OK | R_OK);
			if (ac != (-1)) {
				fd = open (filename, O_RDONLY);
				if (fd != (-1)) {
					close (fd);
					return EIO_TYPE_FILE;
				}
				return EIO_TYPE_DIRECTORY;
			}
	
			return FALSE;
		}
	
	#else
	#error NO Support for EIO_FileType
	#endif

#elif _EL_OS_AMIGAOS__

	{
		BPTR					 lock	= NULL;
		struct FileInfoBlock	*fib	= NULL;
		int	 					 type	= FALSE;

		lock = Lock (filename, (uint32)ACCESS_READ);
		if (lock) {
			fib = AllocMem (sizeof (struct FileInfoBlock), MEMF_CLEAR);
			if (fib) {
				if (!Examine (lock, fib)) {
					type = (fib->fib_DirEntryType >= 0) ? EIO_TYPE_DIRECTORY : EIO_TYPE_FILE;
				}
			}
		}

		if (fib)	FreeMem (fib, sizeof (struct FileInfoBlock));
		if (lock)	UnLock (lock);

		return type;
	}

#elif _EL_OS_MACOS__

	{
		OSErr	iErr;
		int		volRefNum;
		long	parentDirID;
		long	createdDirID;
		Str255	macName;
		
		macName[0] = strlen (filename);
		strcpy ((char *)&macName[1], filename);
		
		iErr = HGetVol (NULL, &volRefNum, &parentDirID);
		if (iErr != noErr)
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf2 ("\nEIO_FileType(%s): HGetVol() %s\n", filename, MacSystemErrMsg (iErr));
			return FALSE;
		}
		
		{
			CInfoPBRec myCPB;
	
			myCPB.hFileInfo.ioFDirIndex	 = 0;
			myCPB.hFileInfo.ioCompletion = 0L;
			myCPB.hFileInfo.ioNamePtr 	 = macName;
			myCPB.hFileInfo.ioVRefNum 	 = volRefNum;			
			myCPB.hFileInfo.ioDirID		 = parentDirID; 
			
			if ( (iErr = PBGetCatInfo(&myCPB,false)) != noErr)
			{
				SetGlobalErr (ERR_GENERIC);
				GEcatf2 ("\nEIO_FileType:PBGetCatInfo(%s) %s\n", filename, MacSystemErrMsg (iErr));
				return FALSE;
			}
			return ((BitTst(&(myCPB.hFileInfo.ioFlAttrib),3)) ? EIO_TYPE_DIRECTORY : EIO_TYPE_FILE);
		}
		
		return FALSE;
	}
#elif _EL_OS_IRIX53__	
	{
       		struct stat fileStat;

	       	if (!stat (filename, &fileStat))
		{
			if ((fileStat.st_mode & S_IFMT) == S_IFDIR)
			{
				return EIO_TYPE_DIRECTORY;
			}
			return EIO_TYPE_FILE;
		}

		return FALSE;
	}

#else
#error NO Support for EIO_FileType
#endif
	

} /* EIO_FileType */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_GetFileDate                                               */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * EIO_GetFileDate
 *
 * SYNOPSIS
 *		int	 EIO_GetFileDate (const char* filename, FileDateType *fdt)
 *
 * PURPOSE
 *		Read Last Modification Date of file for @b@COPYING OR COMPARING
 *		ONLY!  All fields in a FileDataType are private!@p@
 *
 * INPUT
 *		filename	: pointer to filename/filespec of file
 *		fdt			: pointer to FileDateType structure to be filled in with
 *					  date.
 *
 * RETURN VALUE
 *		FALSE = failure
 *
 * EXAMPLE
@DESC:
 *		See EIO_PutFileDate and EIO_CmpDates
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *		EIO_PutFileData, EIO_CmpDates
 *	
*/
int	 EIO_GetFileDate (const char* filename, FileDateType *fdt)
{
#if _EL_OS_MSDOS__
#if _EL_CC_TURBOC__

	{
		int	handle;

		handle = open (filename, O_RDONLY | O_BINARY);
		if (handle != (-1)) {
			if (getftime (handle, &fdt->FTime)) {
				close (handle);
				SetGlobalErr (ERR_GENERIC);
				GEprintf ("EIO_GetFileDate: Couldn't read date of file");
				return FALSE;
			}
			close (handle);
			return TRUE;
		}
		return FALSE;
	}

#elif _EL_CC_WATCOMC__

	{
		int	handle;

		handle = open (filename, O_RDONLY | O_BINARY);
		if (handle != (-1)) {
			if (_dos_getftime (handle, &fdt->Date, &fdt->Time))
			{
				close (handle);
				SetGlobalErr (ERR_GENERIC);
				GEprintf ("EIO_GetFileDate: Couldn't read date of file");
				return FALSE;
			}
			close (handle);
			return TRUE;
		}
		return FALSE;
	}

#elif (__ZTC__ && _EL_OS_MSDOS__)

	{
		int	handle;

		handle = open (filename, O_RDONLY | O_BINARY);
		if (handle != (-1)) {
			if (dos_getftime (handle, &fdt->Date, &fdt->Time)) {
				close (handle);
				SetGlobalErr (ERR_GENERIC);
				GEprintf ("EIO_GetFileDate: Couldn't read date of file");
				return FALSE;
			}
			close (handle);
			return TRUE;
		}
		return FALSE;
	}

#else
#error Need Support for 'EIO_GetFileDate'
#endif

#elif _EL_OS_AMIGAOS__

#if AZTEC_C

	{
		BPTR					 lock	= NULL;
		struct FileInfoBlock	*fib	= NULL;
		int						 status = FALSE;

		lock = Lock (filename, (uint32)ACCESS_READ);
		if (lock) {
			fib = AllocMem (sizeof (struct FileInfoBlock), MEMF_CLEAR);
			if (fib) {
				if (!Examine (lock, fib)) {
					fdt->DateStamp = fib->fib_Date;
					status = TRUE;
				}
			}
		}

		if (fib)	FreeMem (fib, sizeof (struct FileInfoBlock));
		if (lock)	UnLock (lock);

		return status;
	}

#else
#error Need Support for 'EIO_GetFileDate'
#endif

#elif _EL_OS_MACOS__

	{
		Str255		 tempname;
		OSErr		 io;
		int			 workVRef;
		long		 workDirID;

		io = HGetVol (NULL, &workVRef, &workDirID);
		if (io != noErr)
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf2 ("\nEIO_GetFileDate(%s):Error calling HGetVol: %s", filename, MacSystemErrMsg (io));
			return FALSE;
		}
		
		strcpy ((char *)&tempname[1], filename);
		tempname[0] = strlen (filename);
		
		fdt->cipb.hFileInfo.ioNamePtr   = tempname;
		fdt->cipb.hFileInfo.ioVRefNum   = workVRef;
		fdt->cipb.hFileInfo.ioDirID     = workDirID;
		fdt->cipb.hFileInfo.ioFDirIndex = 0;
		
		io = PBGetCatInfo (&fdt->cipb, FALSE);
		if (io)
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf2 ("\nEIO_GetFileDate(%s):%s", filename, MacSystemErrMsg (io));
			return FALSE;
		}
		return TRUE;
	}
	
#elif (_EL_OS_IRIX53__ || _EL_OS_WIN32__)

	{
		if (stat (filename, &fdt->fileStat))
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf1 ("\nEIO_GetFileDate(%s):Couldn't read date for file", filename);
			return FALSE;
		}
		return TRUE;
	}

#else
#error Need Support for 'EIO_GetFileDate'
#endif

} /* EIO_GetFileDate */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_CmpDates                                                  */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * EIO_CmpDates
 *
 * SYNOPSIS
 *		int	 EIO_CmpDates (FileDateType *fdt1, FileDateType *fdt2)
 *
 * PURPOSE
 *		Compare Two Dates gotten with EIO_GetFileDate.
 *
 * INPUT
 *		fdt1	: pointer to a valid FileDataType
 *		fdt2	: pointer to a valid FileDataType
 *
 * RETURN VALUE
 *		0	= dates are equal
 *		1	= fdt1 is newer than fdt2
 *		-1	= fdt1 is older than fdt2
 *
 * EXAMPLE
 *		#include <echidna/platform.h>
 *		#include "switches.h"
 *	
 *		#include <echidna/eio.h>
 *		#include <stdio.h>
 *	
 *		int main (void)
 *		{
 *			FileDateType	 fdt1;
 *			FileDateType	 fdt2;
 *			int				 diff;
 *	
 *			if (!EIO_GetFileDate ("C:\\AUTOEXEC.BAT", &fdt1)) {
 *	
 *				printf ("Error getting date of C:\\AUTOEXEC.BAT\n");
 *				return 0;
 *			}
 *	
 *			if (!EIO_GetFileDate ("C:\\CONFIG.SYS", &fdt2)) {
 *	
 *				printf ("Error getting date of C:\\CONFIG.SYS\n");
 *				return 0;
 *			}
 *	
 *			diff = EIO_CmpDates (&fdt1, &fdt2);
 *	
 *			if (!diff) {
 *	
 *				printf ("C:\\AUTOEXEC.BAT & C:\\CONFIG.SYS\n");
 *				printf ("were modified at exaclty the same time\n");
 *	
 *			} else if (diff > 0) {
 *	
 *				printf ("C:\\AUTOEXEC.BAT is newer than C:\\CONFIG.SYS\n");
 *	
 *			} else \\ if (diff < 0) {
 *	
 *				printf ("C:\\AUTOEXEC.BAT is older than C:\\CONFIG.SYS\n");
 *			}
 *	
 *			return 0;
 *		}
 *	
 *	
 * HISTORY
 *
 *
 * SEE ALSO
 *		EIO_GetFileDate
 *	
*/
int	 EIO_CmpDates (FileDateType *fdt1, FileDateType *fdt2)
{

#if _EL_OS_MSDOS__
#if _EL_CC_TURBOC__
	{
		int	 result;

		if (!(result = (fdt1->FTime.ft_year - fdt2->FTime.ft_year)))
			if (!(result = (fdt1->FTime.ft_month - fdt2->FTime.ft_month)))
				if (!(result = (fdt1->FTime.ft_day - fdt2->FTime.ft_day)))
					if (!(result = (fdt1->FTime.ft_hour - fdt2->FTime.ft_hour)))
						if (!(result = (fdt1->FTime.ft_min - fdt2->FTime.ft_min)))
							result = (fdt1->FTime.ft_tsec - fdt2->FTime.ft_tsec);

		return result;
	}
#elif _EL_CC_WATCOMC__
	{
		int	 result;

		if (!(result = (fdt1->Date - fdt2->Date)))
			result = (fdt1->Time - fdt2->Time);

		return result;
	}
#elif (__ZTC__ && _EL_OS_MSDOS__)
	{
		int	 result;

		if (!(result = (fdt1->Date - fdt2->Date)))
			result = (fdt1->Time - fdt2->Time);

		return result;
	}
#else
#error Need support for 'EIO_CmpDates'
#endif

#elif _EL_OS_AMIGAOS__

	{
		int	 result;

		if (!(result = (fdt1->DateStamp.ds_Days - fdt2->DateStamp.ds_Days)))
			if (!(result = (fdt1->DateStamp.ds_Minute - fdt2->DateStamp.ds_Minute)))
				result = (fdt1->DateStamp.ds_Tick - fdt2->DateStamp.ds_Tick);

		return result;
	}

#elif _EL_OS_MACOS__

	{
		long	result;

		result = (long)(fdt1->cipb.hFileInfo.ioFlMdDat - fdt2->cipb.hFileInfo.ioFlMdDat);
		if (result != 0)
		{
			result = (result > 0) ? 1 : (-1);
		}
		return (int	)result;
	}

#elif (_EL_OS_IRIX53__)
	
	{
		long	result;

		result = (long)difftime (fdt1->fileStat.st_mtim.tv_sec, fdt2->fileStat.st_mtim.tv_sec);

		if (result != 0)
		{
			result = (result > 0) ? (1) : (-1);
		}
		return (int	)result;
	}

#elif (_EL_OS_WIN32__)

	{
		long	result;

		result = (long)difftime (fdt1->fileStat.st_mtime, fdt2->fileStat.st_mtime);

		if (result != 0)
		{
			result = (result > 0) ? (1) : (-1);
		}
		return (int	)result;
	}
#else
#error Need support for 'EIO_CmpDates'
#endif

} /* EIO_CmpDates */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_SetFileDate                                               */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * EIO_SetFileDate
 *
 * SYNOPSIS
 *		int	 EIO_SetFileDate (const char* filename, FileDateType *fdt)
 *
 * PURPOSE
 *		Set Last Modification Date of file.  This is only used when
 *		copying the date from another file.
 *
 * INPUT
 *		filename	: pointer to filename/filespec of file
 *		fdt			: pointer to FileDateType structure to be filled in with
 *					  date.
 *
 * RETURN VALUE
 *		FALSE = failure
 *
 * EXAMPLE
 *		#include <echidna/platform.h>
 *		#include "switches.h"
 *	
 *		#include <echidna/eio.h>
 *		#include <stdio.h>
 *	
 *		int main (void)
 *		{
 *			FileDateType	 fdt;
 *	
 *			if (!EIO_GetFileDate ("C:\\AUTOEXEC.BAT", &fdt)) {
 *	
 *				printf ("Error getting date of C:\\AUTOEXEC.BAT\n");
 *				return 0;
 *			}
 *	
 *			if (!EIO_SetFileDate ("C:\\CONFIG.SYS", &fdt)) {
 *	
 *				printf ("Error setting date of C:\\CONFIG.SYS\n");
 *				return 0;
 *			}
 *	
 *			printf ("Date of C:\\CONFIG.SYS is now\n");
 *			printf ("the same as C:\\AUTOEXEC.BAT\n");
 *	
 *			return 0;
 *		}
 *	
 * HISTORY
 *
 *
 * SEE ALSO
 *		EIO_GetFileDate
 *	
*/
int	 EIO_SetFileDate (const char* filename, FileDateType *fdt)
{
#if _EL_OS_MSDOS__
#if _EL_CC_TURBOC__
	{
		int	handle;

		handle = open (filename, O_WRONLY | O_BINARY);
		if (handle != (-1)) {
			if (setftime (handle, &fdt->FTime)) {
				close (handle);
				SetGlobalErr (ERR_GENERIC);
				GEprintf ("EIO_SetFileDate: Couldn't set date of file");
				return FALSE;
			}
			close (handle);
			return TRUE;
		}
		return FALSE;
	}
#elif _EL_CC_WATCOMC__
	{
		int	handle;

		handle = open (filename, O_WRONLY | O_BINARY);
		if (handle != (-1))
		{
			if (_dos_setftime (handle, fdt->Date, fdt->Time))
			{
				close (handle);
				SetGlobalErr (ERR_GENERIC);
				GEprintf ("EIO_SetFileDate: Couldn't set date of file");
				return FALSE;
			}
			close (handle);
			return TRUE;
		}
		return FALSE;
	}
#elif (__ZTC__ && _EL_OS_MSDOS__)
	{
		int	handle;

		handle = open (filename, O_WRONLY | O_BINARY);
		if (handle != (-1)) {
			if (dos_setftime (handle, fdt->Date, fdt->Time)) {
				close (handle);
				SetGlobalErr (ERR_GENERIC);
				GEprintf ("EIO_SetFileDate: Couldn't set date of file");
				return FALSE;
			}
			close (handle);
			return TRUE;
		}
		return FALSE;
	}
#else
#error Need Support for 'EIO_SetFileDate'
#endif

#elif _EL_OS_AMIGAOS__

#define ACTION_SET_DATE	34L
	{
		uint32			 args[7] 	= { NULL, };
		int				 status		= FALSE;
		BPTR			 filelock;
		BPTR			 parentlock;
		struct MsgPort	*devicePort;
		FileDateType	*bfdt;
		void			*bstr;

		bfdt = AllocMem (sizeof (FileDateType), 0L);
		if (bfdt) {
			bfdt->DateStamp = fdt->DateStamp;
			bstr = AllocMem (EIO_MAXPATH + 1, 0L);
			if (!bstr) {
				CtoBStr (filename, (uint32)bstr >> 2, EIO_MAXPATH);
				filelock = Lock (filename, ACCESS_READ);
				if (filelock) {
					parentlock = ParentDir (filelock);
					if (parentlock) {
						devicePort = DeviceProc (filename);
						if (devicePort) {
							args[1] = (uint32)parentlock;
							args[2] = (uint32)bstr >> 2;
							args[3] = (uint32)bfdt;
							status = SendPacket (ACTION_SET_DATE, (void *)args, devicePort);
						}
						UnLock (parentlock);
					}
					UnLock (filelock);
				}
				FreeMem (bstr, EIO_MAXPATH + 1);
			}
			FreeMem (bfdt, sizeof (FileDateType));
		}
		return status;
	}

#elif _EL_OS_MACOS__

	{
		Str255		 tempname;
		OSErr		 io;
		int			 workVRef;
		long		 workDirID;
		
		io = HGetVol (NULL, &workVRef, &workDirID);
		if (io != noErr)
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf2 ("\nEIO_SetFileDate(%s):Error calling HGetVol: %s", filename, MacSystemErrMsg (io));
			return FALSE;
		}
		
		strcpy ((char *)&tempname[1], filename);
		tempname[0] = strlen (filename);
		
		fdt->cipb.hFileInfo.ioNamePtr   = tempname;
		fdt->cipb.hFileInfo.ioVRefNum   = workVRef;
		fdt->cipb.hFileInfo.ioDirID     = workDirID;
		fdt->cipb.hFileInfo.ioFDirIndex = 0;
		
		io = PBSetCatInfo (&fdt->cipb, FALSE);
		if (io)
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf2 ("\nEIO_GetFileDate(%s):%s", filename, MacSystemErrMsg (io));
			return FALSE;
		}
		
		return TRUE;
	}

#elif (_EL_OS_IRIX53__)
	
	{
		struct utimbuf utb;

		utb.actime  = fdt->fileStat.st_atim.tv_sec;
		utb.modtime = fdt->fileStat.st_mtim.tv_sec;

		if (utime (filename, &utb))
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf1 ("\nEIO_GetFileDate(%s):couldn't set date", filename);
			return FALSE;
		}
		return TRUE;
	}

#elif (_EL_OS_WIN32__)

	{
		struct utimbuf utb;

		utb.actime  = fdt->fileStat.st_atime;
		utb.modtime = fdt->fileStat.st_mtime;

		if (utime (filename, &utb))
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf1 ("\nEIO_GetFileDate(%s):couldn't set date", filename);
			return FALSE;
		}
		return TRUE;
	}

#else
#error Need Support for 'EIO_SetFileDate'
#endif

} /* EIO_SetFileDate */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_GetFileAttrib                                             */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * EIO_GetFileAttrib
 *
 * SYNOPSIS
 *		int	 EIO_GetFileAttrib (const char* filename, FileAttribType *fat)
 *
 * PURPOSE
 *		Get Read/Write... Permission of file.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
int	 EIO_GetFileAttrib (const char* filename, FileAttribType *fat)
{
#if _EL_OS_MSDOS__
#if _EL_CC_TURBOC__
	{
		int	attrib;

		attrib = _rtl_chmod (filename, 0);
		fat->Attrib = attrib;
		if (attrib == (-1)) {
			SetGlobalErr (ERR_GENERIC);
			GEprintf1 ("EIO_GetFileAttrib:Couldn't get file '%s' attributes", filename);
			return FALSE;
		}
		return TRUE;
	}
#elif _EL_CC_WATCOMC__
	{
		unsigned attrib = 0;

		if (_dos_getfileattr (filename, &attrib))
		{
			SetGlobalErr (ERR_GENERIC);
			GEprintf1 ("EIO_GetFileAttrib:Couldn't get file '%s' attributes", filename);
			return FALSE;
		}
		fat->Attrib = attrib;
		return TRUE;
	}
#elif (__ZTC__ && _EL_OS_MSDOS__)
	{
		unsigned attrib = 0;

		if (dos_getfileattr (filename, &attrib)) {
			SetGlobalErr (ERR_GENERIC);
			GEprintf1 ("EIO_GetFileAttrib:Couldn't get file '%s' attributes", filename);
			return FALSE;
		}
		fat->Attrib = attrib;
		return TRUE;
	}
#else
#error Need Support for 'EIO_GetFileAttrib'
#endif

#elif _EL_OS_AMIGAOS__

	{
		BPTR					 lock	= NULL;
		struct FileInfoBlock	*fib	= NULL;
		int						 status = FALSE;

		lock = Lock (filename, (uint32)ACCESS_READ);
		if (lock) {
			fib = AllocMem (sizeof (struct FileInfoBlock), MEMF_CLEAR);
			if (fib) {
				if (!Examine (lock, fib)) {
					fat->Attrib = fib->fib_Protection;
					status = TRUE;
				}
			}
		}

		if (fib)	FreeMem (fib, sizeof (struct FileInfoBlock));
		if (lock)	UnLock (lock);

		return status;
	}

#elif _EL_OS_MACOS__

	{
		if (!EIO_GetFileDate (filename, &fat->fdt))
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf1 ("\nEIO_GetFileAttrib(%s)", filename);
			return FALSE;
		}
		return TRUE;
	}

#elif (_EL_OS_IRIX53__ || _EL_OS_WIN32__)

	{
		if (stat (filename, &fat->fileStat))
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf1 ("\nEIO_GetFileAttrib(%s):Couldn't read attributes for file", filename);
			return FALSE;
		}
		return TRUE;
	}

#else
#error Need Support for 'EIO_GetFileAttrib
#endif

} /* EIO_GetFileAttrib */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_SetFileAttrib                                             */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * EIO_SetFileAttrib
 *
 * SYNOPSIS
 *		int	 EIO_SetFileAttrib (int handle, FileAttribType *fat)
 *
 * PURPOSE
 *		Set Read/Write Permission of file.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
int	 EIO_SetFileAttrib (const char* filename, FileAttribType *fat)
{
#if _EL_OS_MSDOS__
#if _EL_CC_TURBOC__
	if (_rtl_chmod (filename, 1, fat->Attrib) == (-1)) {
		SetGlobalErr (ERR_GENERIC);
		GEprintf1 ("EIO_SetFileAttrib:Couldn't set file '%s' attributes", filename);
		return FALSE;
	}
	return TRUE;
#elif _EL_CC_WATCOMC__
	if (_dos_setfileattr (filename, fat->Attrib))
	{
		SetGlobalErr (ERR_GENERIC);
		GEprintf1 ("EIO_SetFileAttrib:Couldn't set file '%s' attributes", filename);
		return FALSE;
	}
	return TRUE;
#elif (__ZTC__ && _EL_OS_MSDOS__)
	if (dos_setfileattr (filename, fat->Attrib)) {
		SetGlobalErr (ERR_GENERIC);
		GEprintf1 ("EIO_SetFileAttrib:Couldn't set file '%s' attributes", filename);
		return FALSE;
	}
	return TRUE;
#else
#error Need Support for 'EIO_SetFileAttrib
#endif

#elif _EL_OS_AMIGAOS__

	{
		return SetProtection (filename, fat->Attrib);
	}

#elif _EL_OS_MACOS__

	{
		if (!EIO_SetFileDate (filename, &fat->fdt))
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf1 ("\nEIO_SetFileAttrib(%s)", filename);
			return FALSE;
		}
		return TRUE;
	}

#elif (_EL_OS_IRIX53__ || _EL_OS_WIN32__)

	{
		if (chmod (filename, fat->fileStat.st_mode))
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf1 ("\nEIO_SetFileAttrib(%s)", filename);
			return FALSE;
		}
		return TRUE;
	}

#else
#error Need Support for 'EIO_SetFileAttrib
#endif

} /* EIO_SetFileAttrib */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_GetFileComment                                            */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * EIO_GetFileComment
 *
 * SYNOPSIS
 *		int	 EIO_GetFileComment (const char* filename, FileCommentType *fct)
 *
 * PURPOSE
 *		Get the comment of a file if this file system has one.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
int	 EIO_GetFileComment (const char* filename, FileCommentType *fct)
{
#if _EL_OS_MSDOS__

	filename = filename;
	fct->Comment[0] = '\0';
	return TRUE;

#elif _EL_OS_AMIGAOS__

	{
		BPTR					 lock	= NULL;
		struct FileInfoBlock	*fib	= NULL;
		int						 status = FALSE;

		lock = Lock (filename, (uint32)ACCESS_READ);
		if (lock) {
			fib = AllocMem (sizeof (struct FileInfoBlock), MEMF_CLEAR);
			if (fib) {
				if (!Examine (lock, fib)) {
					fct->Comment[80] = '\0';
					memcpy (fct, fib->fib_Comment, 80);
					status = TRUE;
				}
			}
		}

		if (fib)	FreeMem (fib, sizeof (struct FileInfoBlock));
		if (lock)	UnLock (lock);

		return status;
	}

#elif _EL_OS_MACOS__

	{
		if (!EIO_GetFileDate (filename, &fct->fdt))
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf1 ("\nEIO_GetFileComment(%s)", filename);
			return FALSE;
		}
		return TRUE;
	}

#elif (_EL_OS_IRIX53__ || _EL_OS_WIN32__)

	return TRUE;

#else
#error Need 'EIO_GetFileComment'
#endif

} /* EIO_GetFileComment */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_SetFileComment                                            */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * EIO_SetFileComment
 *
 * SYNOPSIS
 *		int	 EIO_SetFileComment (const char* filename, FileCommentType *fct)
 *
 * PURPOSE
 *		Set the comment of a file if the file system has one.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
int	 EIO_SetFileComment (const char* filename, FileCommentType *fct)
{
#if _EL_OS_MSDOS__

	filename = filename;
	fct      = fct;
	return TRUE;

#elif _EL_OS_AMIGAOS__

	return SetComment (filename, fct->Comment);

#elif _EL_OS_MACOS__

	{
		if (!EIO_SetFileDate (filename, &fct->fdt))
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf1 ("\nEIO_SetFileComment(%s)", filename);
			return FALSE;
		}
		return TRUE;
	}

#elif (_EL_OS_IRIX53__ || _EL_OS_WIN32__)

	return TRUE;

#else
#error Need 'EIO_SetFileComment'
#endif

} /* EIO_SetFileComment */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_CopyFile                                                  */
/*------------------------------------------------------------------------*/

#if _EL_OS_MACOS__
/*****************************************************************************/
/*	The deny-mode privileges to use when opening the source and destination files. */

enum
{
	dmNone			= 0x0000,
	dmNoneDenyRd	= 0x0010,
	dmNoneDenyWr	= 0x0020,
	dmNoneDenyRdWr	= 0x0030,
	dmRd			= 0x0001,	/* Single writer, multiple readers; the readers */
	dmRdDenyRd		= 0x0011,
	dmRdDenyWr		= 0x0021,	/* Browsing - equivalent to fsRdPerm */
	dmRdDenyRdWr	= 0x0031,
	dmWr			= 0x0002,
	dmWrDenyRd		= 0x0012,
	dmWrDenyWr		= 0x0022,
	dmWrDenyRdWr	= 0x0032,
	dmRdWr			= 0x0003,	/* Shared access - equivalent to fsRdWrShPerm */
	dmRdWrDenyRd	= 0x0013,
	dmRdWrDenyWr	= 0x0023,	/* Single writer, multiple readers; the writer */
	dmRdWrDenyRdWr	= 0x0033	/* Exclusive access - equivalent to fsRdWrPerm */
};

enum
{
	srcCopyMode = dmRdDenyWr,
	dstCopyMode = dmWrDenyRdWr
};

/*	The largest (16K) and smallest (.5K) copy buffer to use if the caller doesn't supply 
**	their own copy buffer. */

enum
{
	bigCopyBuffSize  = 0x00004000,
	minCopyBuffSize  = 0x00000200
};

/*
**	For those times where you need to use more than one kind of File Manager parameter
**	block but don't feel like wasting stack space, here's a parameter block you can reuse.
*/
union UniversalFMPB
{
	ParamBlockRec	PB;
	CInfoPBRec		ciPB;
	DTPBRec			dtPB;
	HParamBlockRec	hPB;
	CMovePBRec		cmPB;
	WDPBRec			wdPB;
	FCBPBRec		fcbPB;
};
typedef union UniversalFMPB UniversalFMPB;
typedef UniversalFMPB *UniversalFMPBPtr, **UniversalFMPBHandle;

#define hasCopyFile(volParms)		(((volParms).vMAttrib >> bHasCopyFile) & 1)

/*****************************************************************************/

/* Copy a pascal-string. */

static void	pcpy(StringPtr d, StringPtr s)
{
	int		i;

	i = *s;
	do {
		d[i] = s[i];
	} while (i--);
}



/*****************************************************************************/

static pascal	OSErr	GetDirID(int	 vRefNum,
						 long dirID,
						 StringPtr name,
						 long *theDirID,
						 Boolean *isDirectory)
{
	CInfoPBRec pb;
	OSErr error;

	pb.hFileInfo.ioNamePtr = name;
	pb.hFileInfo.ioVRefNum = vRefNum;
	pb.hFileInfo.ioDirID = dirID;
	pb.hFileInfo.ioFDirIndex = 0;	/* use ioNamePtr and ioDirID */
	error = PBGetCatInfoSync(&pb);
	*theDirID = pb.hFileInfo.ioDirID;
	*isDirectory = (pb.hFileInfo.ioFlAttrib & 0x10) != 0;
	return (error);
}

/*****************************************************************************/

static pascal	OSErr	HGetVolParms(StringPtr volName,
							 int	 vRefNum,
							 GetVolParmsInfoBuffer *volParmsInfo,
							 long *infoSize)
{
	HParamBlockRec pb;
	OSErr error;

	pb.ioParam.ioNamePtr = volName;
	pb.ioParam.ioVRefNum = vRefNum;
	pb.ioParam.ioBuffer = (Ptr)volParmsInfo;
	pb.ioParam.ioReqCount = *infoSize;
	error = PBHGetVolParmsSync(&pb);
	*infoSize = pb.ioParam.ioActCount;
	return (error);
}

/*****************************************************************************/

static pascal	OSErr	HCreateMinimum(int	 vRefNum,
							   long dirID,
							   const Str255 fileName)
{
	HParamBlockRec pb;

	pb.fileParam.ioNamePtr = (StringPtr)fileName;
	pb.fileParam.ioVRefNum = vRefNum;
	pb.ioParam.ioVersNum = 0;
	pb.fileParam.ioDirID = dirID;
	return (PBHCreateSync(&pb));
}
/* static prototypes */

/*****************************************************************************/
static	OSErr	PreflightFileCopySpace(int	 srcVRefNum,
									   long srcDirID,
									   const Str255 srcName,
									   StringPtr dstVolName,
									   int	 dstVRefNum,
									   Boolean *spaceOK);
/*	PreflightFileCopySpace determines if there's enough space on a
	volume to copy the specified file to that volume.
	Note: The results of this routine are not perfect. For example if the
	volume's catalog or extents overflow file grows when the new file is
	created, more allocation blocks may be needed beyond those needed for
	the file's data and resource forks.

	srcVRefNum		input:	Volume specification of the file's current
							location.
	srcDirID		input:	Directory ID of the file's current location.
	srcName			input:	The name of the file.
	dstVolName		input:	A pointer to the name of the volume where
							the file will be copied or nil.
	dstVRefNum		input:	Volume specification indicating the volume
							where the file will be copied.
	spaceOK			output:	true if there's enough space on the volume for
							the file's data and resource forks.
*/

/*****************************************************************************/

static pascal	OSErr	DetermineVRefNum(StringPtr pathname,
								 int	 vRefNum,
								 int	 *realVRefNum)
{
	HParamBlockRec pb;
	Str255 tempPathname;
	OSErr error;

	pb.volumeParam.ioVRefNum = vRefNum;
	if (pathname == nil) {
		pb.volumeParam.ioNamePtr = nil;
		pb.volumeParam.ioVolIndex = 0;		/* use ioVRefNum only */
	}
	else {
		pcpy((StringPtr)tempPathname, pathname);	/* make a copy of the string and */
		pb.volumeParam.ioNamePtr = (StringPtr)tempPathname;	/* use the copy so original isn't trashed */
		pb.volumeParam.ioVolIndex = -1;	/* use ioNamePtr/ioVRefNum combination */
	}
	error = PBHGetVInfoSync(&pb);
	*realVRefNum = pb.volumeParam.ioVRefNum;
	return (error);
}

/*****************************************************************************/

static pascal	OSErr	CopyFileMgrAttributes(int	 srcVRefNum,
									  long srcDirID,
									  StringPtr srcName,
									  int	 dstVRefNum,
									  long dstDirID,
									  StringPtr dstName,
									  Boolean copyLockBit)
{
	UniversalFMPB pb;
	OSErr error;
	Boolean objectIsDirectory;

	pb.ciPB.hFileInfo.ioVRefNum = srcVRefNum;
	pb.ciPB.hFileInfo.ioDirID = srcDirID;
	pb.ciPB.hFileInfo.ioNamePtr = srcName;
	pb.ciPB.hFileInfo.ioFDirIndex = 0;
	error = PBGetCatInfoSync(&pb.ciPB);
	if (error == noErr)
	{
		objectIsDirectory = (pb.ciPB.hFileInfo.ioFlAttrib & 0x10);
		pb.ciPB.hFileInfo.ioVRefNum = dstVRefNum;
		pb.ciPB.hFileInfo.ioDirID = dstDirID;
		pb.ciPB.hFileInfo.ioNamePtr = dstName;
		/* don't copy the hasBeenInited bit */
		pb.ciPB.hFileInfo.ioFlFndrInfo.fdFlags = (pb.ciPB.hFileInfo.ioFlFndrInfo.fdFlags & 0xfeff);
		error = PBSetCatInfoSync(&pb.ciPB);
		if ((error == noErr) && (copyLockBit) && (pb.ciPB.hFileInfo.ioFlAttrib & 0x01))
		{
			error = PBHSetFLockSync(&pb.hPB);
			if ((error != noErr) && (objectIsDirectory))
				error = noErr; /* ignore lock errors if destination is directory */
		}
	}
	return (error);
}

/*****************************************************************************/

static pascal	OSErr	HOpenAware(int	 vRefNum,
						   long dirID,
						   const Str255 fileName,
						   int	 denyModes,
						   int	 *refNum)
{
	HParamBlockRec pb;
	OSErr error;

	pb.ioParam.ioVersNum = 0;
	pb.ioParam.ioMisc = nil;
	pb.fileParam.ioNamePtr = (StringPtr)fileName;
	pb.fileParam.ioVRefNum = vRefNum;
	pb.accessParam.ioDenyModes = denyModes;
	pb.fileParam.ioDirID = dirID;
	error = PBHOpenDenySync(&pb);				/* Try OpenDeny first */
	if (error == paramErr)
	{
		/* OpenDeny not supported, so try File Manager Open functions */
		/* Set File Manager permissions to closest thing possible */
		pb.ioParam.ioPermssn = ((denyModes == dmWr) || (denyModes == dmRdWr)) ? (fsRdWrShPerm) : (denyModes % 4);

		error = PBHOpenDFSync(&pb);				/* Try OpenDF */
		if (error == paramErr)
			error = PBHOpenSync(&pb);			/* OpenDF not supported, so try Open */
	}
	*refNum = pb.ioParam.ioRefNum;
	return (error);
}

/*****************************************************************************/

static pascal	OSErr	GetFileLocation(int	 refNum,
								int	 *vRefNum,
								long *dirID,
								StringPtr fileName)
{
	FCBPBRec pb;
	OSErr error;

	pb.ioNamePtr = fileName;
	pb.ioVRefNum = 0;
	pb.ioRefNum = refNum;
	pb.ioFCBIndx = 0;
	error = PBGetFCBInfoSync(&pb);
	*vRefNum = pb.ioFCBVRefNum;
	*dirID = pb.ioFCBParID;
	return (error);
}

/*****************************************************************************/

static pascal	OSErr	HOpenRFAware(int	 vRefNum,
							 long dirID,
							 const Str255 fileName,
							 int	 denyModes,
							 int	 *refNum)
{
	HParamBlockRec pb;
	OSErr error;

	pb.fileParam.ioNamePtr = (StringPtr)fileName;
	pb.fileParam.ioVRefNum = vRefNum;
	pb.ioParam.ioVersNum = 0;
	pb.accessParam.ioDenyModes = denyModes;
	pb.ioParam.ioMisc = nil;
	pb.fileParam.ioDirID = dirID;
	error = PBHOpenRFDenySync(&pb);				/* Try OpenRFDeny first */
	if (error == paramErr)
	{
		/* OpenRFDeny not supported, so try File Manager OpenRF function */
		/* Set File Manager permissions to closest thing possible */
		pb.ioParam.ioPermssn = ((denyModes == dmWr) || (denyModes == dmRdWr)) ? (fsRdWrShPerm) : (denyModes % 4);

		error = PBHOpenRFSync(&pb);				/* Try OpenRF */
	}
	*refNum = pb.ioParam.ioRefNum;
	return (error);
}

/*****************************************************************************/

static pascal	OSErr	CopyFork(int	 srcRefNum,
						 int	 dstRefNum,
						 void *copyBufferPtr,
						 long copyBufferSize)
{
	ParamBlockRec srcPB;
	ParamBlockRec dstPB;
	OSErr srcError;
	OSErr dstError;

	if ((copyBufferPtr == nil) || (copyBufferSize == 0))
		return (paramErr);
	
	srcPB.ioParam.ioRefNum = srcRefNum;
	dstPB.ioParam.ioRefNum = dstRefNum;

	/* preallocate the destination fork and */
	/* ensure the destination fork's EOF is correct after the copy */
	srcError = PBGetEOFSync(&srcPB);
	if (srcError != noErr)
		return (srcError);
	dstPB.ioParam.ioMisc = srcPB.ioParam.ioMisc;
	dstError = PBSetEOFSync(&dstPB);
	if (dstError != noErr)
		return (dstError);

	/* reset source fork's mark */
	srcPB.ioParam.ioPosMode = fsFromStart;
	srcPB.ioParam.ioPosOffset = 0;
	srcError = PBSetFPosSync(&srcPB);
	if (srcError != noErr)
		return (srcError);

	/* reset destination fork's mark */
	dstPB.ioParam.ioPosMode = fsFromStart;
	dstPB.ioParam.ioPosOffset = 0;
	dstError = PBSetFPosSync(&srcPB);
	if (dstError != noErr)
		return (dstError);

	/* set up fields that won't change in the loop */
	srcPB.ioParam.ioBuffer = copyBufferPtr;
	srcPB.ioParam.ioPosMode = fsAtMark + 0x0020;/* fsAtMark + noCacheBit */
	srcPB.ioParam.ioReqCount = ((copyBufferSize >= 512) && (copyBufferSize % 512)) ? (copyBufferSize / 512) * 512 : (copyBufferSize);
	/* If copyBufferSize is greater than 512 bytes, make it a multiple of 512 bytes */
	/* This will make writes on local volumes faster */

	dstPB.ioParam.ioBuffer = copyBufferPtr;
	dstPB.ioParam.ioPosMode = fsAtMark + 0x0020;/* fsAtMark + noCacheBit */

	while ((srcError == noErr) && (dstError == noErr))
	{
		srcError = PBReadSync(&srcPB);
		dstPB.ioParam.ioReqCount = srcPB.ioParam.ioActCount;
		dstError = PBWriteSync(&dstPB);
	}

	/* make sure there were no errors at the destination */
	if (dstError != noErr)
		return (dstError);

	/* make sure the only error at the source was eofErr */
	if (srcError != eofErr)
		return (srcError);

	return (noErr);
}

/*****************************************************************************/

static pascal	OSErr	CopyComment(int	 srcVRefNum,
							long srcDirID,
							StringPtr srcName,
							int	 dstVRefNum,
							long dstDirID,
							StringPtr dstName)
/* Both volumes must support the Desktop Manager for this to work */
{
	DTPBRec pb;
	OSErr error;
	unsigned char commentBuffer[199];
	long commentLength;

	pb.ioNamePtr = srcName;
	pb.ioVRefNum = srcVRefNum;
	error = PBDTOpenInform(&pb);
	if (error == paramErr)
		error = PBDTGetPath(&pb);
	if (error == noErr)
	{
		pb.ioNamePtr = srcName;
		pb.ioDirID = srcDirID;
		pb.ioDTBuffer = (Ptr) & commentBuffer;
		error = PBDTGetCommentSync(&pb);
		if (error == noErr)
		{
			commentLength = pb.ioDTActCount;
			pb.ioNamePtr = dstName;
			pb.ioVRefNum = dstVRefNum;
			error = PBDTOpenInform(&pb);
			if (error == paramErr)
				error = PBDTGetPath(&pb);
			if (error == noErr)
			{
				pb.ioNamePtr = dstName;
				pb.ioDirID = dstDirID;
				pb.ioDTBuffer = (Ptr) & commentBuffer;
				pb.ioDTReqCount = commentLength;
				error = PBDTSetCommentSync(&pb);
			}
		}
	}
	return (error);
}

/*****************************************************************************/

static pascal	OSErr	HCopyFile(int	 srcVRefNum,
						  long srcDirID,
						  const Str255 srcName,
						  int	 dstVRefNum,
						  long dstDirID,
						  StringPtr dstPathname,
						  StringPtr copyName)
{
	HParamBlockRec pb;

	pb.copyParam.ioVRefNum = srcVRefNum;
	pb.copyParam.ioDirID = srcDirID;
	pb.copyParam.ioNamePtr = (StringPtr)srcName;
	pb.copyParam.ioDstVRefNum = dstVRefNum;
	pb.copyParam.ioNewDirID = dstDirID;
	pb.copyParam.ioNewName = dstPathname;
	pb.copyParam.ioCopyName = copyName;
	return (PBHCopyFileSync(&pb));
}

/*****************************************************************************/

static	OSErr	PreflightFileCopySpace(int	 srcVRefNum,
									   long srcDirID,
									   const Str255 srcName,
									   StringPtr dstVolName,
									   int	 dstVRefNum,
									   Boolean *spaceOK)
{
	HParamBlockRec pb;
	OSErr error;
	Str255 tempPathname;
	long dstFreeBlocks;
	long dstBlksPerAllocBlk;
	long srcDataBlks;
	long srcResourceBlks;
	
	/* Get the number of 512 byte blocks per allocation block and */
	/* number of free allocation blocks on the destination volume */
	pb.volumeParam.ioVRefNum = dstVRefNum;
	if (dstVolName == nil) {
		pb.volumeParam.ioNamePtr = nil;
		pb.volumeParam.ioVolIndex = 0;		/* use ioVRefNum only */
	}
	else {
		pcpy((StringPtr)tempPathname, dstVolName);	/* make a copy of the string and */
		pb.volumeParam.ioNamePtr = (StringPtr)tempPathname;	/* use the copy so original isn't trashed */
		pb.volumeParam.ioVolIndex = -1;	/* use ioNamePtr/ioVRefNum combination */
	}
	error = PBHGetVInfoSync(&pb);
	if (error == noErr)
	{
		/* get allocation block size (always multiple of 512) and divide by 512
		  to get number of 512-byte blocks per allocation block */
		dstBlksPerAllocBlk = (pb.volumeParam.ioVAlBlkSiz >> 9);
		dstFreeBlocks = pb.volumeParam.ioVFrBlk;
		
		/* Now, get the size of the file's data resource forks */
		pb.fileParam.ioNamePtr = (StringPtr)srcName;
		pb.fileParam.ioVRefNum = srcVRefNum;
		pb.fileParam.ioDirID = srcDirID;
		pb.fileParam.ioFDirIndex = 0;
		error = PBHGetFInfoSync(&pb);
		if (error == noErr)
		{
			/* get number of 512-byte blocks needed for data fork */
			srcDataBlks = (pb.fileParam.ioFlLgLen % 512) ? ((pb.fileParam.ioFlLgLen >> 9) + 1) : (pb.fileParam.ioFlLgLen >> 9);
			/* now, calculate number of new allocation blocks needed */
			srcDataBlks = (srcDataBlks % dstBlksPerAllocBlk) ? ((srcDataBlks / dstBlksPerAllocBlk) + 1) : (srcDataBlks / dstBlksPerAllocBlk);
		
			/* get number of 512-byte blocks needed for resource fork */
			srcResourceBlks = (pb.fileParam.ioFlRLgLen % 512) ? ((pb.fileParam.ioFlRLgLen >> 9) + 1) : (pb.fileParam.ioFlRLgLen >> 9);
			/* now, calculate number of new allocation blocks needed */
			srcResourceBlks = (srcResourceBlks % dstBlksPerAllocBlk) ? ((srcResourceBlks / dstBlksPerAllocBlk) + 1) : (srcResourceBlks / dstBlksPerAllocBlk);
			
			/* Is there enough room on the destination volume for the source file? */
			*spaceOK = ((srcDataBlks + srcResourceBlks) <= dstFreeBlocks);
		}
	}
	return (error);
}

/*****************************************************************************/

static pascal	OSErr	FileCopy(int	 srcVRefNum,
						 long srcDirID,
						 const Str255 srcName,
						 int	 dstVRefNum,
						 long dstDirID,
						 StringPtr dstPathname,
						 StringPtr copyName,
						 Ptr copyBufferPtr,
						 long copyBufferSize,
						 Boolean preflight)
{
	OSErr	err;

	int		srcRefNum = 0,			/* 0 when source data and resource fork are closed  */
			dstDataRefNum = 0,		/* 0 when destination data fork is closed */
			dstRsrcRefNum = 0;		/* 0 when destination resource fork is closed */
	
	Str63	dstName;				/* The filename of the destination. It might be the
									** source filename, it might be a new name... */
	
	GetVolParmsInfoBuffer infoBuffer; /* Where PBGetVolParms dumps its info */
	long	srcServerAdr;			/* AppleTalk server address of source (if any) */
	
	Boolean	dstCreated = false,		/* true when destination file has been created */
			ourCopyBuffer = false,	/* true if we had to allocate the copy buffer */
			isDirectory;			/* true if destination is really a directory */
	
	long	tempLong;
	int		tempInt;
	
	Boolean	spaceOK;				/* true if there's enough room to copy the file to the destination volume */

	/* Preflight for size */
	if (preflight) {
		err = PreflightFileCopySpace(srcVRefNum, srcDirID, srcName,
									 dstPathname, dstVRefNum, &spaceOK);
		if (err != noErr)
			return(err);
		if (!spaceOK)
			return(dskFulErr);
	}

	/* get the destination's real dirID and make sure it really is a directory */
	err = GetDirID(dstVRefNum, dstDirID, dstPathname, &dstDirID, &isDirectory);
	if (err != noErr)
		goto ErrorExit;
	if (!isDirectory)
		return (dirNFErr);

	/* get the destination's real vRefNum */
	err = DetermineVRefNum(dstPathname, dstVRefNum, &dstVRefNum);
	if (err != noErr)
		goto ErrorExit;
	
	/* See if PBHCopyFile can be used.  Using PBHCopyFile saves time by letting the file server
	** copy the file if the source and destination locations are on the same file server. */
	tempLong = sizeof(infoBuffer);
	err = HGetVolParms((StringPtr)srcName, srcVRefNum, &infoBuffer, &tempLong);
	if ((err != noErr) && (err != paramErr))
		return(err);

	if ((err != paramErr) && hasCopyFile(infoBuffer)) {
		/* The source volume supports PBHCopyFile. */
		srcServerAdr = infoBuffer.vMServerAdr;

		/* Now, see if the destination volume is on the same file server. */
		tempLong = sizeof(infoBuffer);
		err = HGetVolParms(nil, dstVRefNum, &infoBuffer, &tempLong);
		if ((err != noErr) && (err != paramErr))
			return(err);
		if ((err != paramErr) && (srcServerAdr == infoBuffer.vMServerAdr)) {
			/* Source and Dest are on same server and PBHCopyFile is supported. Copy with CopyFile. */
			err = HCopyFile(srcVRefNum, srcDirID, srcName, dstVRefNum, dstDirID, nil, copyName);
			if (err != noErr)
				goto ErrorExit;
			/* AppleShare's CopyFile clears the isAlias bit, so I still need to attempt to copy
			   the File's attributes to attempt to get things right. */
			return(CopyFileMgrAttributes(srcVRefNum, srcDirID, (StringPtr)srcName,
										 dstVRefNum, dstDirID, (StringPtr)&copyName, true));
		}
	}

	/* If we're here, then PBHCopyFile couldn't be used so we have to copy the file by hand. */

	/* Make sure a copy buffer is allocated. */
	if (copyBufferPtr == nil) {
		/* The caller didn't supply a copy buffer so grab one from the application heap.
		** Try to get a big copy buffer, if we can't, try for a 512-byte buffer.
		** If 512 bytes aren't available, we're in trouble. */
		copyBufferSize = bigCopyBuffSize;
		copyBufferPtr = NewPtr(copyBufferSize);
		if (copyBufferPtr == nil) {
			copyBufferSize = minCopyBuffSize;
			copyBufferPtr = NewPtr(copyBufferSize);
			if (copyBufferPtr == nil)
				return(MemError());
		}
		ourCopyBuffer = true;
	}

	/* Open the source data fork. */
	err = HOpenAware(srcVRefNum, srcDirID, srcName, srcCopyMode, &srcRefNum);
	if (err != noErr)
		goto ErrorExit;
	
	/* See if the copy will be renamed. */
	if (copyName != nil)				/* Did caller supply copy file name? */
		pcpy((StringPtr)&dstName, copyName);	/* Yes, use the caller supplied copy file name. */
	else {								/* They didn't, so get the source file name and use it. */
		err = GetFileLocation(srcRefNum, &tempInt, &tempLong, (StringPtr)&dstName);
		if (err != noErr)
			goto ErrorExit;
	}

	/* Create the destination file. */
	err = HCreateMinimum(dstVRefNum, dstDirID, dstName);
	if (err != noErr)
		goto ErrorExit;
	dstCreated = true;	/* After creating the destination file, any
						** error conditions should delete the destination file */

	/* An AppleShare dropbox folder is a folder for which the user has the Make Changes
	** privilege (write access), but not See Files (read access) and See Folders (search access).
	** Copying a file into an AppleShare dropbox presents some special problems. Here are the
	** rules we have to follow to copy a file into a dropbox:
	** ¥ File attributes can be changed only when both forks of a file are empty.
	** ¥ DeskTop Manager comments can be added to a file only when both forks of a file 
	**   are empty.
	** ¥ A fork can be opened for write access only when both forks of a file are empty.
	** So, with those rules to live with, we'll do those operations now while both forks
	** are empty. */

	/* Copy attributes but don't lock the destination. */
	err = CopyFileMgrAttributes(srcVRefNum, srcDirID, (StringPtr)srcName,
								dstVRefNum, dstDirID, (StringPtr)&dstName, false);
	if (err != noErr)
		goto ErrorExit;

	/* Attempt to copy the comments while both forks are empty.
	** Ignore the result because we really don't care if it worked or not. */
	CopyComment(srcVRefNum, srcDirID, (StringPtr)srcName, dstVRefNum, dstDirID, (StringPtr)&dstName);

	/* Open the destination data fork. */
	err = HOpenAware(dstVRefNum, dstDirID, dstName, dstCopyMode, &dstDataRefNum);
	if (err != noErr)
		goto ErrorExit;

	/* Open the destination resource fork. */
	err = HOpenRFAware(dstVRefNum, dstDirID, dstName, dstCopyMode, &dstRsrcRefNum);
	if (err != noErr)
		goto ErrorExit;

	/* Copy the data fork. */
	err = CopyFork(srcRefNum, dstDataRefNum, copyBufferPtr, copyBufferSize);
	if (err != noErr)
		goto ErrorExit;

	/* Close both data forks and clear reference numbers. */
	FSClose(srcRefNum);
	FSClose(dstDataRefNum);
	srcRefNum = dstDataRefNum = 0;

	/* Open the source resource fork. */
	err = HOpenRFAware(srcVRefNum, srcDirID, srcName, srcCopyMode, &srcRefNum);
	if (err != noErr)
		goto ErrorExit;

	/* Copy the resource fork. */
	err = CopyFork(srcRefNum, dstRsrcRefNum, copyBufferPtr, copyBufferSize);
	if (err != noErr)
		goto ErrorExit;

	/* Close both resource forks and clear reference numbers. */
	FSClose(srcRefNum);
	FSClose(dstRsrcRefNum);
	srcRefNum = dstRsrcRefNum = 0;

	/* Get rid of the copy buffer if we allocated it. */
	if (ourCopyBuffer)
		DisposPtr(copyBufferPtr);

	/* Attempt to copy attributes again to set mod date.  Copy lock condition this time
	** since we're done with the copy operation.  This operation will fail if we're copying
	** into an AppleShare dropbox, so we don't check for error conditions. */
	CopyFileMgrAttributes(srcVRefNum, srcDirID, (StringPtr)srcName,
							dstVRefNum, dstDirID, (StringPtr)&dstName, true);

	/* Hey, we did it! */
	return (noErr);
	
ErrorExit:
	if (srcRefNum)
		FSClose(srcRefNum);		/* Close the source file */
	if (dstDataRefNum)
		FSClose(dstDataRefNum);	/* Close the destination file data fork */
	if (dstRsrcRefNum)
		FSClose(dstRsrcRefNum);	/* Close the destination file resource fork */
	if (dstCreated)
		HDelete(dstVRefNum, dstDirID, dstName);	/* Delete dest file.  This may fail if the file 
												   is in a "drop folder" */
	if (ourCopyBuffer)			/* dispose of any memory we allocated */
		DisposPtr(copyBufferPtr);
	return (err);
}
#endif

/*********************************************************************
 *
 * EIO_CopyFile
 *
 * SYNOPSIS
 *		int	 EIO_CopyFile (const char* from, const char* to)
 *
 * PURPOSE
 *		Copies file from TO file to including creation date,
 *		file attributes and file comments.
 *
 * INPUT
 *		from	: pointer to source filename
 *		to		: pointer to dest filename
 *
 * RETURN VALUE
 *		FALSE = failure
 *
 * EXAMPLE
 *		#include <echidna/platform.h>
 *		#include "switches.h"
 *	
 *		#include <echidna/eio.h>
 *		#include <stdio.h>
 *	
 *		int main (void)
 *		{
 *			if (EIO_CopyFile ("C:\\AUTOEXEC.BAT", "D:\\AUTOEXEC.BAT")) {
 *	
 *				printf ("AUTOEXEC.BAT copyied from C:\\ to D:\\\n");
 *			}
 *	
 *			return 0;
 *		}
 *	
 *	
 * HISTORY
 *
 *
 * SEE ALSO
 *		EIO_GetFileDate, EIO_SetFileDate
 *		EIO_GetFileComment, EIO_SetFileComment
 *		EIO_GetFileAttrib, EIO_SetFileAttrib
 *	
*/
#define CBUF_SIZE	16384
int	 EIO_CopyFile (const char* from, const char* to)
{
#if (_EL_OS_MSDOS__ || _EL_OS_AMIGAOS__ || _EL_OS_IRIX53__ || _EL_OS_WIN32__)
	{
		uint8	*buf = NULL;
		int		 in  = -1;
		int		 out = -1;
		long	 bytes;
		int		 status = FALSE;
		
		buf = malloc (CBUF_SIZE);
		if (!buf) {
			SetGlobalErr (ERR_OUT_OF_MEMORY);
			GEprintf ("EIO_CopyFile:OOM copy buffer");
			goto cfcleanup;
		}
	
		in = EIO_ReadOpen (from);
		if (in == (-1)) {
			SetGlobalErr (ERR_GENERIC);
			GEprintf1 ("EIO_CopyFile:Couldn't open source file '%s'", from);
			goto cfcleanup;
		}
	
		out = EIO_WriteOpen (to);
		if (out == (-1)) {
			SetGlobalErr (ERR_GENERIC);
			GEprintf1 ("EIO_CopyFile:Couldn't open destination file '%s'", to);
			goto cfcleanup;
		}
	
		while ((bytes = EIO_Read (in, buf, CBUF_SIZE)) > 0) {
//			EL_printf ("copying %ld\n", bytes);
			if (EIO_Write (out, buf, bytes) == (-1)) {
				SetGlobalErr (ERR_GENERIC);
				GEprintf1 ("EIO_CopyFile:Error Writing to file '%s'", to);
				goto cfcleanup;
			}
		}
	
		if (bytes < 0) {
			SetGlobalErr (ERR_GENERIC);
			GEprintf1 ("EIO_CopyFile:Error Reading from file '%s'", from);
			goto cfcleanup;
		}
	
		close (out);	out = -1;
		close (in);		in  = -1;

		{
			FileDateType	fdt;
	
			if (!EIO_GetFileDate (from, &fdt)) {
				goto cfcleanup;
			}
	
			if (!EIO_SetFileDate (to, &fdt)) {
				goto cfcleanup;
			}
		}
	
		{
			FileAttribType	fat;
	
			if (!EIO_GetFileAttrib (from, &fat)) {
				goto cfcleanup;
			}
	
			if (!EIO_SetFileAttrib (to, &fat)) {
				goto cfcleanup;
			}
	
		}
	
		{
			FileCommentType	fct;
	
			if (!EIO_GetFileComment (from, &fct)) {
				goto cfcleanup;
			}
	
			if (!EIO_SetFileComment (to, &fct)) {
				goto cfcleanup;
			}
		}
	
		status = TRUE;
	
	cfcleanup:
		if (buf) free (buf);
		if (out != -1) close (out);
		if (in  != -1) close (in);
		return status;
	}
	
#elif _EL_OS_MACOS__

	{
		Str255		 srcName;
		Str255		 dstName;
		Str255		 dstPath;
		char		 dstPathTmp[EIO_MAXPATH];
		char		 dstNameTmp[EIO_MAXPATH];
		OSErr		 io;
		int			 workVRef;
		long		 workDirID;
		
		if (!EIO_FileExists (from))
		{
			SetGlobalErr (ERR_FILE_NOT_FOUND);
			GEcatf1 ("\nEIO_Copyfile:Source file '%s' doesn't exist", from);
			return FALSE;
		}
		
		if (EIO_FileExists (to))
		{
			if (remove (to))
			{
				SetGlobalErr (ERR_FILE_NOT_FOUND);
				GEcatf1 ("\nEIO_Copyfile:Couldn't delete dest file '%s'", to);
				return FALSE;
			}
		}
		
		io = HGetVol (NULL, &workVRef, &workDirID);
		if (io != noErr)
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf1 ("\nEIO_CopyFile:Error calling HGetVol: %s", MacSystemErrMsg (io));
			return FALSE;
		}
		
		strcpy ((char *)&srcName[1], from);
		srcName[0] = strlen (from);
		
		strcpy (dstPathTmp, EIO_Path (to));
		strcpy (dstNameTmp, EIO_Filename (to));
		
		strcpy ((char *)&dstName[1], dstNameTmp);
		dstName[0] = strlen (dstNameTmp);
		
		strcpy ((char *)&dstPath[1], dstPathTmp);
		dstPath[0] = strlen (dstPathTmp);
		
		io = FileCopy (
				workVRef, workDirID, srcName,
				workVRef, workDirID, &dstPath[0], &dstName[0],
				NULL, NULL, true);
		if (io != noErr)
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf1 ("\nEIO_CopyFile: %s", MacSystemErrMsg (io));
			return FALSE;
		}
	}

#else
#error Need Code
#endif

} /* EIO_CopyFile */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_MakeDir                                                   */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * EIO_MakeDir
 *
 * SYNOPSIS
 *		int	 EIO_MakeDir (const char* filename)
 *
 * PURPOSE
 *		Create a Directory.
 *
 * INPUT
 *		filename	: Name/Path of directory to create
 *
 * EFFECTS
 *		Directory filename is created.
 *
 * RETURN VALUE
 *		FALSE = failure.
 *
 * EXAMPLE
 *		#include <echidna/platform.h>
 *		#include "switches.h"
 *	
 *		#include <echidna/eio.h>
 *		#include <stdio.h>
 *	
 *		int main (void)
 *		{
 *			if (EIO_MakeDir ("C:\\MYDIR")) {
 *				printf ("Directory "C:\\MYDIR Created\n");
 *			}
 *	
 *			return 0;
 *		}
 *	
 *		
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
int	 EIO_MakeDir (const char* filename)
{
#if _EL_OS_MSDOS__
#if (_EL_CC_TURBOC__ || __ZTC__ || _EL_CC_WATCOMC__)
	if (mkdir (filename)) {
		SetGlobalErr (ERR_GENERIC);
		GEprintf1 ("EIO_MakeDir:Couldn't make directory '%s'", filename);
		return FALSE;
	}
	return TRUE;
#else
#error NO Support for 'EIO_MakeDir'
#endif

#elif _EL_OS_AMIGAOS__

	{
		BPTR	lock;

		lock = CreateDir (filename);
		if (lock) {
			UnLock (lock);
		}

		return (lock != NULL);
	}

#elif _EL_OS_MACOS__

	{
		OSErr	iErr;
		int		volRefNum;
		long	parentDirID;
		long	createdDirID;
		Str255	dirName;
		
		dirName[0] = strlen (filename);
		strcpy ((char *)&dirName[1], filename);
		
		iErr = HGetVol (NULL, &volRefNum, &parentDirID);
		if (iErr != noErr)
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf1 ("\nEIO_MakeDir: HGetVol() %s\n", MacSystemErrMsg (iErr));
			return FALSE;
		}
		
		iErr = DirCreate(volRefNum,parentDirID,dirName,&createdDirID);
		if (iErr != noErr)
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf1 ("\nEIO_MakeDir: DirCreate() %s\n", MacSystemErrMsg (iErr));
			return FALSE;
		}
		
		return TRUE;
	}

#elif _EL_OS_IRIX53__

	{
		if (mkdir (filename, S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH))
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf1 ("\nEIO_MakeDir: couldn't make dir '%s'\n", filename);
			return FALSE;
		}
		return TRUE;
	}

#elif _EL_OS_WIN32__

	{
		if (mkdir (filename))
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf1 ("\nEIO_MakeDir: couldn't make dir '%s'\n", filename);
			return FALSE;
		}
		return TRUE;
	}

#else
#error NO Support for 'EIO_MakeDir'
#endif

} /* EIO_MakeDir */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_Read                                                      */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * EIO_Read
 *
 * SYNOPSIS
 *		long EIO_Read (int fh, void *buf, long size)
 *
 * PURPOSE
 *		
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
#if (_EL_CC_TURBOC__ || _EL_OS_MSDOS__)
long EIO_Read (int fh, void *buf, long size)
{
	void huge32	*ptr;
	long		 count;
	int			 bytesread;
	int			 bytestoread;

	ptr   = buf;
	count = 0;
	while (size) {
		bytestoread = (int)UTL_MIN (size, 32767);
		bytesread   = read (fh, (void far32 *) ptr, bytestoread);
		if (bytesread == (-1)) {
			return (-1);
		}
		size  -= bytesread;
		count += bytesread;
		ptr    = ((char huge32 *)ptr) + bytesread;
		if (bytesread != bytestoread) {
			break;
		}
	}
	return count;

} /* EIO_Read */
#endif

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_Write                                                     */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * EIO_Write
 *
 * SYNOPSIS
 *		long EIO_Write (int fh, void *buf, long size)
 *
 * PURPOSE
 *		
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
#if (_EL_CC_TURBOC__ || _EL_OS_MSDOS__)
long EIO_Write (int fh, void *buf, long size)
{
	void huge32	*ptr;
	long		 count;
	int			 byteswritten;
	int			 bytestowrite;

	ptr   = buf;
	count = 0;
	while (size) {
		bytestowrite = (int)UTL_MIN (size, 32767);
		byteswritten = write (fh, (void far32 *)ptr, (int)bytestowrite);
		if (byteswritten == (-1)) {
			return (-1);
		}
		size  -= byteswritten;
		count += byteswritten;
		ptr    = ((char huge32 *)ptr) + byteswritten;
		if (byteswritten != bytestowrite) {
			break;
		}
	}
	return count;

} /* EIO_Write */
#endif

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_FixupFilespec                                             */
/*------------------------------------------------------------------------*/

#if _EL_OS_MSDOS__
	#define	MACHINEFILESPECSECTION	"[DOSNameSubst]"
#elif _EL_OS_MACOS__
	#define	MACHINEFILESPECSECTION	"[MacNameSubst]"
#elif _EL_OS_IRIX53__
	#define	MACHINEFILESPECSECTION	"[SGINameSubst]"
#elif _EL_OS_WIN32__
	#define	MACHINEFILESPECSECTION	"[Win32NameSubst]"
#else
#error "need code"
#endif

static IniList *filespecIni;

/*********************************************************************
 *
 * EIO_InitFilespecFixup
 *
 * SYNOPSIS
 *		void  EIO_InitFilespecFixup (const char* spec)
 *
 * PURPOSE
 *		
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
void EIO_InitFilespecFixup (const char* spec)
{
	const char	*inifile;
	char	 realname[EIO_MAXPATH];

	inifile = (spec) ? (spec) : "filespec.ini";
	strcpy (realname, inifile);

	if (!EIO_FileExists (realname))
	{
		if (!EIO_FindFile (EIO_Filename(inifile),  EIO_Filename (inifile), realname))
		{
			FailMess ("couldn't find configuration file '%s'\n", inifile);
		}
	}

	filespecIni = ReadINI (realname);

	if (!filespecIni)
	{
		FailMess ("problem in configuration file '%s'\n", realname);
	}
} /* EIO_InitFilespecFixup */

/*********************************************************************
 *
 * EIO_FixupFilespec
 *
 * SYNOPSIS
 *		void  EIO_FixupFilespec (char *inname, char *outname)
 *
 * PURPOSE
 *		
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
void EIO_FixupFilespec (char *outname, char* inname)
{
	SectionTracker	 secx;
	SectionTracker	*sec = &secx;

	if (!filespecIni)
	{
		FailMess ("Didn't load filespec.ini");
	}

	strlwr (inname);
	
	sec = FindSection (sec, filespecIni, MACHINEFILESPECSECTION);
	if (sec)
	{
		char	*s;
		char	*d;

		s = inname;
		d = outname;

		while (*s)
		{
			int			 rep = FALSE;
			char		*r;

			ResetSection (sec);

			while (((r = FindNextINILine (sec, "Subst=")) != NULL) && !rep)
			{
				char	 delim;
				char	*c;

				c     = s;
				delim = *r++;

				if (*r)
				{
					while (*r && *r != delim && *c && toupper(*c) == toupper(*r))
					{
						r++;
						c++;
					}

					if (*r == delim)
					{
						s = c;
						r++;

						while (*r && *r != delim)
						{
							*d++ = *r++;
							rep  = TRUE;
						}
					}
				}
			}
			
			if (!rep)
			{
				*d++ = *s++;
			}
		}

		*d = '\0';
	}
	else
	{
		strcpy (outname, inname);
	}

	EIO_FixDirSeps (outname);

} /* EIO_FixupFilespec */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_CheckDates                                                */
/*------------------------------------------------------------------------*/

int	EIO_CheckDates = TRUE;

/*********************************************************************
 *
 * EIO_FileNewer
 *
 * SYNOPSIS
 *		int  EIO_FileNewer (const char* source, const char* target)
 *
 * PURPOSE
 *		
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		1  = It's newer or target doesn't exist
 *		0  = It's older
 *		-1 = Error
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
int EIO_FileNewer (const char* source, const char* target)
{
	if (EIO_CheckDates)
	{
		FileDateType	s_fdt;
		FileDateType	t_fdt;
		int				newer;

		if (!EIO_GetFileDate (source, &s_fdt))
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf2 ("\nError getting date of '%s'\n%s\n", source, GlobalErrMsg);
			return -1;
		}
		if (!EIO_FileExists (target))
		{
// EL_printf ("--->Make '%s' from '%s'\n", target, source);
			return 1;
		}
		if (!EIO_GetFileDate (target, &t_fdt))
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf2 ("Error getting date of '%s'\n%s\n", target, GlobalErrMsg);
			return -1;
		}
		newer = (EIO_CmpDates (&s_fdt, &t_fdt) > 0);

		if (newer)
		{
// EL_printf ("--->Make '%s' from '%s'\n", target, source);
		}
		else
		{
// EL_printf ("--->'%s' is newer than '%s'\n", target, source);
		}
		
		return newer;
	}
	else
	{
// EL_printf ("--->Make '%s' from '%s'\n", target, source);
		return 1;
	}
} /* EIO_FileNewer */

/*********************************************************************
 *
 * EIO_DirExists
 *
 * SYNOPSIS
 *		int  EIO_DirExists (const char* dirPath)
 *
 * PURPOSE
 *		
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		1  = Dir Exists
 *		0  = Dir Does Not exist
 *		-1 = Error
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
int EIO_DirExists (const char* dirPath)
{
	if (EIO_CheckDates)
	{
		int		fileType;

		fileType = EIO_FileType (dirPath);
		
		if (fileType == EIO_TYPE_FILE)
		{
			SetGlobalErr (ERR_GENERIC);
			GEprintf1 ("'%s' is an existing file, cannot use as a directory\n", dirPath);
			return -1;
		}
		else if (fileType == EIO_TYPE_DIRECTORY)
		{
			return 1;
		}
		else
		{
			ClearGlobalError ();
			return 0;
		}
		
	}
	else
	{
		return 0;
	}


} /* EIO_DirExists */

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_FindInclude                                               */
/*------------------------------------------------------------------------*/

/*************************************************************************
                           EIO_AddIncludePath                            
 *************************************************************************

   SYNOPSIS
		int EIO_AddIncludePath (int groupID, const char* path)

   PURPOSE
  		Adds an include path to a particular group of include paths
  
   INPUT
		groupID : ID for group to add path too, you can use any label
		          though for INCLUDES, LIBS etc you should use the
				  EIOINCPATH_ macros to be compatible across modules.
				  they are NOT case-sensitive.
		path    : filespec for path (with or without trailing slash)
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   RETURNS
		TRUE if added
  
   SEE ALSO
		EIO_FindInclude
  
   HISTORY
		05/08/02 GAT: Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static LST_LIST	EIO_IncludePathLists;
static int		EIO_fIncludePathInit;

typedef struct
{
	LST_NODE	 node;
	LST_LIST	 pathlist;
} EIO_PATHLIST;

static void EIO_InitIncludePaths (void)
{
	// init if not inited yet
	if (!EIO_fIncludePathInit)
	{
		LST_InitList (&EIO_IncludePathLists);
	}
}
	 
int EIO_AddIncludePath (const char* groupName, const char* path)
{
	EIO_PATHLIST*	pEPL;
	
	EIO_InitIncludePaths ();
	
	pEPL = (EIO_PATHLIST*)LST_FindIName (&EIO_IncludePathLists, groupName);
	if (!pEPL)
	{
		// create new pathlist
		pEPL = (EIO_PATHLIST*)LST_CreateNode (sizeof (EIO_PATHLIST), groupName);
		if (!pEPL)
		{
			SetGlobalErr (ERR_OUT_OF_MEMORY);
			GEprintf ("Couldn't allocate pathlist EIO_AddIncludePath()");
			return FALSE;
		}
		
		LST_InitList (&pEPL->pathlist);
		
		LST_AddTail (&EIO_IncludePathLists, pEPL);
	}
	
	// add the path
	{
		LST_NODE*	nd;
		
		nd = LST_CreateNode (sizeof (LST_NODE), path);
		if (!nd)
		{
			SetGlobalErr (ERR_OUT_OF_MEMORY);
			GEprintf ("Couldn't allocate pathlistnode EIO_AddIncludePath()");
			return FALSE;
		}
		
		LST_AddHead (&pEPL->pathlist, nd);
	}

	return TRUE;
}

/*************************************************************************
                             EIO_FindInclude                             
 *************************************************************************

   SYNOPSIS
		int EIO_FindInclude (char* fixedfilespec, const char* groupName, const char* newfilespec, const char*currentfilespec, int fUseCurrentDir)

   PURPOSE
  		given a current filespec and a newfilespec, tries to find an existing file
		similar to the way a C compiler searches for includes
		
		Paths are searched in the reverse order they were added by calling
		EIO_AddIncludePath()
  
   INPUT
		fixedfilespec   :	destination string EIO_MAXPATH
		groupName       :	ID for group to add path too, you can use any label
							though for INCLUDES, LIBS etc you should use the
							EIOINCPATH_ macros to be compatible across modules.
							they are NOT case-sensitive.
		newfilespec     :	file we are trying to find
		currentfilespec :	file we are starting from (the file that included this one),
							if NULL we will not sure this
		fUseCurrentDir	:	TRUE if we want to search the current directory.
  
   OUTPUT
		fixedfilespec will contain the filespec to use to open the file
  
   EFFECTS
		None  
  
   RETURNS
		!FALSE  = file was found
		FALSE/0 = file was not found
  
   EXAMPLE
 		#include <echidna/platform.h>
 		#include "switches.h"
 	
 		#include <echidna/eio.h>
 		#include <stdio.h>
 	
 		int main (void)
 		{
			char incspec[EIO_MAXPATH];
			
			EIO_AddIncludePath (EIO_INCPATH_INCLUDES, "c:\\mycompiler\\inc");
			EIO_AddIncludePath (EIO_INCPATH_INCLUDES, "c:\\mycompiler\\otherinc");
			
			if (EIO_FindInclude (incspec, EIO_INCPATH_INCLUDES, "sys\\stat.h", "myproject\\src\\myfile.c", TRUE))
			{
 				printf ("file exists at %s\n", incspec);
 			}
 	
 			return 0;
 		}
		
		in the example above the following places will be searched in the
		following order
		
			sys\stat.h
			myproject\src\sys\stat.h
			c:\mycompiler\otherinc\sys\stat.h
			c:\mycompiler\inc\sys\stat.h
 	
   SEE ALSO
		EIO_AddIncludePath
 
   HISTORY
		05/08/02 GAT: Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int EIO_FindInclude (char* fixedfilespec, const char *groupName, const char* newfilespec, const char*currentfilespec, int fUseCurrentDir)
{
	EIO_PATHLIST* pEPL;
	
	EIO_InitIncludePaths ();
	
	if (fUseCurrentDir)
	{
		if (EIO_FileExists (newfilespec))
		{
			strcpy (fixedfilespec, newfilespec);
			return TRUE;
		}
	}
	
	if (currentfilespec)
	{
		EIO_fnmerge (fixedfilespec, EIO_Path (currentfilespec), newfilespec, NULL);
		if (EIO_FileExists (fixedfilespec))
		{
			return TRUE;
		}
	}

	pEPL = (EIO_PATHLIST*)LST_FindIName (&EIO_IncludePathLists, groupName);
	if (pEPL)
	{
		LST_NODE*	nd;
		
		// go through list of include folders
		nd = LST_Head (&pEPL->pathlist);
		while (!LST_EndOfList (nd))
		{
			EIO_fnmerge (fixedfilespec, LST_NodeName(nd), newfilespec, NULL);
			if (EIO_FileExists (fixedfilespec))
			{
				return TRUE;
			}
		}
	}
	
	return FALSE;
}

/*------------------------------------------------------------------------*/
/**# MODULE:EIO_BatchFiles                                                */
/*------------------------------------------------------------------------*/

#if _EL_OS_MSDOS__
	char	EIO_BatchPrefix[]=	"";
	char	EIO_CallPrefix[]=	"call ";
	char	EIO_EnvPrefix[]=	"%";
	char	EIO_EnvSuffix[]=	"%";
#elif _EL_OS_MACOS__
	char	EIO_BatchPrefix[]=	"";
	char	EIO_CallPrefix[]=	"";
	char	EIO_EnvPrefix[]=	"{";
	char	EIO_EnvSuffix[]=	"}";
#elif _EL_OS_IRIX53__
	char	EIO_BatchPrefix[]=	"#! /bin/csh -e -f\n#\n";
	char	EIO_CallPrefix[]=	"";
	char	EIO_EnvPrefix[]=	"${";
	char	EIO_EnvSuffix[]=	"}";
#elif _EL_OS_WIN32__
	char	EIO_BatchPrefix[]=	"";
	char	EIO_CallPrefix[]=	"call ";
	char	EIO_EnvPrefix[]=	"%";
	char	EIO_EnvSuffix[]=	"%";
#else
#error "need code"
#endif


