/*************************************************************************
 *                                                                       *
 *                                CHECK.C                                *
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


   PROGRAMMERS


   FUNCTIONS

   TABS : 5 9

   HISTORY
		07/15/96 GAT: Created from Echidna code

 *************************************************************************/

/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"

#include <stdlib.h>

#include "echidna/memsafe.h"
#include "echidna/listapi.h"
#include "echidna/checkglu.h"
#include "echidna/eerrors.h"
#include "echidna/eio.h"

char *pvCHK_dupstr (const char *str, const char *file, int line)
{
	if (str)
	{
		char	*s;
	
		if (!MEM_fAllocMemTypeNamedFileLine (s, strlen(str) + 1, mst_Any, "string", file, line))
		{
			OutOfMemErr ("Couldn't duplicate string '%s'\n", str);
		}
	
		strcpy (s, str);

		return s;
    }
	else
	{
		return NULL;
	}

}

void pvCHK_freestr (char *str)
{
	if (str)
	{
		MEM_FreeMemNamed (str, "string");
	}
}


/*------------------------------------------------------------------------*/
/**# MODULE:CHK_CreateList                                                */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * CHK_CreateNode
 *
 * SYNOPSIS
 *		void * CHK_CreateList (char *name, char *id)
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
LST_LIST *pvCHK_CreateList2 (const char *name, const char *id, const char *file, int line)
{
	LST_LIST	*list;

	if (!MEM_fAllocMemTypeNamedFileLine (list, sizeof (LST_LIST), mst_Any, id, file, line))
	{
		EL_printf ("Out of memory: couldn't allocate list %s\n", id);
		exit (EXIT_FAILURE);
	}

	memset (list, 0, sizeof (LST_LIST));

	LST_InitList (list);

	if (name)
	{
		list->name = CHK_dupstr (name);
	}

	return (LST_LIST *)list;

}

/*************************************************************************
                            pvCHK_DeleteList2
 *************************************************************************

   SYNOPSIS
		void pvCHK_DeleteList2 (LST_LIST *ptr, char *id)

   PURPOSE


   INPUT
		ptr :
		id  :

   OUTPUT
		None

   EFFECTS
		None

   SEE ALSO


   HISTORY
		11/14/95 GAT: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void pvCHK_DeleteList2 (LST_LIST *list, const char *id)
{
	if (list->name)
	{
		CHK_freestr (list->name);
	}
	MEM_FreeMemNamed (list, id);

}

/*------------------------------------------------------------------------*/
/**# MODULE:CHK_CreateNode                                                */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * CHK_CreateNode
 *
 * SYNOPSIS
 *		void * CHK_CreateNode (size_t size, char *name, char *id)
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
void *pvCHK_CreateNode2 (size_t size, const char *name, const char *id, const char *file, int line)
{
	LST_NODE	*nd;

	if (!MEM_fAllocMemTypeNamedFileLine (nd, size, mst_Any, id, file, line))
	{
		EL_printf ("Out of memory: couldn't allocate node %s\n", id);
		exit (EXIT_FAILURE);
	}

	memset (nd, 0, size);

	if (name)
	{
		LST_NodeName (nd) = CHK_dupstr (name);
	}

	return (void *)nd;

}

/*------------------------------------------------------------------------*/
/**# MODULE:CHK_DeleteNode                                                */
/*------------------------------------------------------------------------*/

/*************************************************************************
                            pvCHK_DeleteNode2
 *************************************************************************

   SYNOPSIS
		void pvCHK_DeleteNode2 (void *nd, char *id, char *file, int line)

   PURPOSE


   INPUT
		nd   :
		id   :
		file :
		line :

   OUTPUT
		None

   EFFECTS
		None

   SEE ALSO


   HISTORY
		11/14/95 GAT: Created.

 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void pvCHK_DeleteNode2 (void *ptr, const char *id)
{
	if (LST_NodeName (ptr))
	{
		CHK_freestr (LST_NodeName (ptr));
	}
	MEM_FreeMemNamed (ptr, id);

}

/*------------------------------------------------------------------------*/
/**# MODULE:CHK_AllocateMemory                                            */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * CHK_AllocateMemory
 *
 * SYNOPSIS
 *		void * CHK_AllocateMemory (long size, char *id)
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
void *pvCHK_AllocateMemory2 (long size, const char *id, const char *file, int line)
{
	void	*buf;

	if (!MEM_fAllocMemTypeNamedFileLine (buf, size, mst_Any, id, file, line))
	{
		EL_printf ("Out of memory: Couldn't allocate %s\n", id);
		exit (EXIT_FAILURE);
	}

	return buf;
}

/*------------------------------------------------------------------------*/
/**# MODULE:CHK_CallocateMemory                                           */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * CHK_CallocateMemory
 *
 * SYNOPSIS
 *		void * CHK_CallocateMemory (long size, char *id)
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
void *pvCHK_CallocateMemory2 (long size, const char *id, const char *file, int line)
{
	void	*buf;

	if (!MEM_fAllocMemTypeNamedFileLine (buf, size, mst_Any, id, file, line))
	{
		EL_printf ("Out of memory: Couldn't callocate %s\n", id);
		exit (EXIT_FAILURE);
	}

	memset (buf, 0, size);

	return buf;
}

/*------------------------------------------------------------------------*/
/**# MODULE:CHK_DeallocateMemory                                          */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * CHK_DeallocateMemory
 *
 * SYNOPSIS
 *		void  CHK_DeallocateMemory (void *ptr, char *id)
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
void pvCHK_DeallocateMemory2 (void *ptr, const char *id)
{
	MEM_FreeMemNamed (ptr, id);
}

/*------------------------------------------------------------------------*/
/**# MODULE:CHK_Open                                                      */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * CHK_Open
 *
 * SYNOPSIS
 *		int  CHK_Open (char *name, int access)
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
int CHK_Open (const char *name, int access)
{
	int	fh;

	fh = EIO_Open (name, access);
	if (fh == (-1))
	{
		EL_printf ("Couldn't open %s\n", name);
		exit (EXIT_FAILURE);
	}

	return fh;
}

/*------------------------------------------------------------------------*/
/**# MODULE:CHK_Read                                                      */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * CHK_Read
 *
 * SYNOPSIS
 *		long  CHK_Read (int fh, void *buf, long size)
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
long CHK_Read (int fh, void *buf, long size)
{
	long	len;

	len = EIO_Read (fh, buf, size);
	if (len == (-1))
	{
		EL_printf ("Error reading from file\n");
		exit (EXIT_FAILURE);
	}

	return len;

}

/*------------------------------------------------------------------------*/
/**# MODULE:CHK_Write                                                     */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * CHK_Write
 *
 * SYNOPSIS
 *		long  CHK_Write (int fh, void *buf, long size)
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
long CHK_Write (int fh, void *buf, long size)
{

	long	len;

	len = EIO_Write (fh, buf, size);
	if (len == (-1))
	{
		EL_printf ("Error writing to file\n");
		exit (EXIT_FAILURE);
	}

	return len;


}

/*------------------------------------------------------------------------*/
/**# MODULE:CHK_Close                                                     */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * CHK_Close
 *
 * SYNOPSIS
 *		void  CHK_Close (int fh)
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
void CHK_Close (int fh)
{

	if (EIO_Close (fh) == (-1))
	{
		EL_printf ("Error closing file\n");
		exit (EXIT_FAILURE);
	}

}

/*------------------------------------------------------------------------*/
/**# MODULE:CHK_Seek                                                      */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * CHK_Seek
 *
 * SYNOPSIS
 *		long  CHK_Seek (int fh, long off, int orig)
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
long CHK_Seek (int fh, long off, int orig)
{
	long	pos;

	pos = EIO_Seek (fh, off, orig);
	if (pos == (-1))
	{
		EL_printf ("Error seeking in file\n");
		exit (EXIT_FAILURE);
	}

	return pos;
}

/*------------------------------------------------------------------------*/
/**# MODULE:CHK_Tell                                                      */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * CHK_Tell
 *
 * SYNOPSIS
 *		long  CHK_Tell (int fh)
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
long CHK_Tell (int fh)
{
	long	pos;

	pos = EIO_Tell (fh);
	if (pos == (-1))
	{
		EL_printf ("Error getting current position in file\n");
		exit (EXIT_FAILURE);
	}

	return pos;
}

/*------------------------------------------------------------------------*/
/**# MODULE:CHK_FileLength                                                */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * CHK_FileLength
 *
 * SYNOPSIS
 *		long  CHK_FileLength (int fh)
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
long CHK_FileLength (int fh)
{

	long	pos;

	pos = EIO_FileLength (fh);
	if (pos == (-1))
	{
		EL_printf ("Error getting file length\n");
		exit (EXIT_FAILURE);
	}

	return pos;

}

/*------------------------------------------------------------------------*/
/**# MODULE:CHK_FindFile                                                  */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * CHK_FindFile
 *
 * SYNOPSIS
 *		void  CHK_FindFile (char *filename, char *guidename, char *pathbuff)
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
void CHK_FindFile (const char *filename, const char *guidename, char *pathbuff)
{

	if (!EIO_FindFile (filename, guidename, pathbuff))
	{
		EL_printf ("Couldn't find file %s in path\n", filename);
		exit (EXIT_FAILURE);
	}

}

/*------------------------------------------------------------------------*/
/**# MODULE:CHK_fopen                                                     */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * CHK_fopen
 *
 * SYNOPSIS
 *		FILE * CHK_fopen (char *filename, char *mode)
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
FILE* CHK_fopen (const char *filename, const char *mode)
{

	FILE	*fp;

	fp = fopen (filename, mode);
	if (!fp)
	{
		EL_printf ("Couldn't open file %s\n", filename);
		exit (EXIT_FAILURE);
	}

	return fp;

}

/*------------------------------------------------------------------------*/
/**# MODULE:CHK_fclose                                                    */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * CHK_fclose
 *
 * SYNOPSIS
 *		void  CHK_fclose (FILE *fp)
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
void CHK_fclose (FILE *fp)
{

	fclose (fp);

	#if 0
	if (!fclose (fp))
	{
		EL_printf ("Error closing file\n");
		exit (EXIT_FAILURE);
	}
	#endif

}

/*------------------------------------------------------------------------*/
/**# MODULE:CHK_fread                                                     */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * CHK_fread
 *
 * SYNOPSIS
 *		size_t  CHK_fread (void *buf, size_t size, size_t n, FILE *fp)
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
size_t CHK_fread (void *buf, size_t size, size_t n, FILE *fp)
{
	size_t	res;

	res = fread (buf, size, n, fp);
	if (!res)
	{
		EL_printf ("Error reading from file\n");
		exit (EXIT_FAILURE);
	}

	return res;
}

/*------------------------------------------------------------------------*/
/**# MODULE:CHK_fwrite                                                    */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * CHK_fwrite
 *
 * SYNOPSIS
 *		size_t  CHK_fwrite (void *buf, size_t size, size_t n, FILE *fp)
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
size_t CHK_fwrite (void *buf, size_t size, size_t n, FILE *fp)
{
	size_t	res;

	res = fwrite (buf, size, n, fp);
	if (!res)
	{
		EL_printf ("Error writing to file\n");
		exit (EXIT_FAILURE);
	}

	return res;
}

/*------------------------------------------------------------------------*/
/**# MODULE:CHK_fprintf                                                   */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * CHK_fprintf
 *
 * SYNOPSIS
 *		int  CHK_fprintf (FILE *fp, char *fmt, ...)
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
int CHK_fprintf (FILE *fp, const char *fmt, ...)
{
	int	res;
	va_list ap;	/* points to each unnamed arg in turn */

	va_start (ap,fmt); /* make ap point to 1st unnamed arg */
	res = vfprintf (fp, fmt, ap);
	va_end (ap);	/* clean up when done */

	if (res < 0)
	{
		EL_printf ("Error fprintf to file\n");
		exit (EXIT_FAILURE);
	}

	return res;
}

/*------------------------------------------------------------------------*/
/**# MODULE:CHK_fputc                                                     */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * CHK_fputc
 *
 * SYNOPSIS
 *		void  CHK_fputc (int c, FILE *fp)
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
void CHK_fputc (int c, FILE *fp)
{
	if (fputc (c, fp) == EOF)
	{
		EL_printf ("Error in CHK_fputc writing to file\n");
		exit (EXIT_FAILURE);
	}
}

/*------------------------------------------------------------------------*/
/**# MODULE:CHK_fputs                                                     */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * CHK_fputs
 *
 * SYNOPSIS
 *		void  CHK_fputs (FILE *fp, char *s)
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
void CHK_fputs (FILE *fp, const char *s)
{
	if (fputs (s, fp))
	{
		EL_printf ("Error in CHK_fputs writing to file\n");
		exit (EXIT_FAILURE);
	}
}

/*************************************************************************
                                CHK_fseek                                
 *************************************************************************

   SYNOPSIS
		void CHK_fseek (FILE *fp, long offset, int whence)

   PURPOSE
  		
  
   INPUT
		fp     :
		offset :
		whence :
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   SEE ALSO
  
  
   HISTORY
		06/12/98 GAT: Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void CHK_fseek (FILE *fp, long offset, int whence)
{
	if (fseek (fp, offset, whence))
	{
		EL_printf ("Error in CHK_fseek seeking in file\n");
		exit (EXIT_FAILURE);
	}
}

/*------------------------------------------------------------------------*/
/**# MODULE:CHK_CreateList                                                */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * CHK_CreateList
 *
 * SYNOPSIS
 *		LST_LIST * CHK_CreateList (char *name, char *id)
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
LST_LIST* CHK_CreateList (const char *name, const char *id)
{

	LST_LIST	*list;

	list = LST_CreateList (name);
	if (!list)
	{
		EL_printf ("Out of Memory: Could allocate list %s\n", id);
		exit (EXIT_FAILURE);
	}

	return list;
}

/*------------------------------------------------------------------------*/
/**# MODULE:CHK_CreateNode                                                */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * CHK_CreateNode
 *
 * SYNOPSIS
 *		void * CHK_CreateNode (size_t size, char *name, char *id)
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
void* CHK_CreateNode (size_t size, const char *name, const char *id)
{
	LST_NODE	*nd;

	nd = LST_CreateNode (size, name);
	if (!nd)
	{
		EL_printf ("Out of memory: couldn't allocate node %s\n", id);
		exit (EXIT_FAILURE);
	}

	return (void *)nd;

}

/*------------------------------------------------------------------------*/
/**# MODULE:CHK_AllocateMemory                                            */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * CHK_AllocateMemory
 *
 * SYNOPSIS
 *		void * CHK_AllocateMemory (long size, char *id)
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
void* CHK_AllocateMemory (long size, const char *id)
{
	void	*buf;

	buf = malloc (size);
	if (!buf)
	{
		EL_printf ("Out of memory: Couldn't allocate %s\n", id);
		exit (EXIT_FAILURE);
	}

	return buf;
}

/*------------------------------------------------------------------------*/
/**# MODULE:CHK_CallocateMemory                                           */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * CHK_CallocateMemory
 *
 * SYNOPSIS
 *		void * CHK_CallocateMemory (long size, char *id)
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
void* CHK_CallocateMemory (long size, const char *id)
{
	void	*buf;

	buf = CHK_AllocateMemory (size, id);
	memset (buf, 0, size);

	return buf;
}

/*------------------------------------------------------------------------*/
/**# MODULE:CHK_DeallocateMemory                                          */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * CHK_DeallocateMemory
 *
 * SYNOPSIS
 *		void  CHK_DeallocateMemory (void *ptr, char *id)
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
void CHK_DeallocateMemory (void *ptr, const char *id)
{
	id = id;
	free (ptr);

}

