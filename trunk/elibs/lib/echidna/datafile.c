/*************************************************************************
 *                                                                       *
 *                              DATAFILE.C                               *
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
		Code to write 'data' files like ascii hex files...

   FUNCTIONS

   TABS : 5 9

   HISTORY
		07/11/96 : Created.

 *************************************************************************/

/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"

#include <stdio.h>

#include "echidna/eerrors.h"
#include "echidna/eio.h"
#include "echidna/listapi.h"
#include "echidna/utils.h"
#include "echidna/datafile.h"

/**************************** C O N S T A N T S ***************************/

#define OFF_NUM_BYTES	0
#define OFF_ADDRESS		1
#define OFF_END_FLAG	3
#define OFF_DATA		4

#define HEADER_SIZE		(1 + 2 + 1)
#define MAX_BYTES		(0x18)

/******************************** T Y P E S *******************************/

typedef struct {
	LST_NODE	 Node;
	uint32		 LenPos;
	uint32		 Length;
} CPEPosType;

/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/***************************** R O U T I N E S ****************************/

/*------------------------------------------------------------------------*/
/**# MODULE:DATATEFILE_IntelHex                                           */
/*------------------------------------------------------------------------*/

static uint8  HexBuffer[HEADER_SIZE + MAX_BYTES + 2];
static uint8 *HexOutPtr;
static uint16  ByteCount;
static uint16  DstAddress;
static uint8  StartFlag = FALSE;
static char   HexChrs[] = "0123456789ABCDEF";

/*********************************************************************
 *
 * WriteHexLine
 *
 * SYNOPSIS
 *		static short WriteHexLine (FILE *fp)
 *
 * PURPOSE
 *		Write Hex Line to File.
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
static short WriteHexLine (FILE *fp)
{
	short	 len;
	uint8	*data;
	uint8	 checksum;
	char	 line[80];
	char	*out;
	short	 i;

	HexBuffer[OFF_NUM_BYTES] = (uint8)ByteCount;
	HexBuffer[OFF_ADDRESS]   = DstAddress >> 8;
	HexBuffer[OFF_ADDRESS+1] = DstAddress & 0xFF;
	HexBuffer[OFF_END_FLAG]  = 0;

	len  = HEADER_SIZE + ByteCount;
	data = HexBuffer;
	checksum = 0;
	for (i = 0; i < len; i++) {
		checksum += *data++;
	}
	*data = 256 - checksum;
	len++;

	out  = line;
	data = HexBuffer;
	*out++ = ':';
	for (i = 0; i < len; i++) {
		*out++ = HexChrs[*data >> 4];
		*out++ = HexChrs[*data++ & 0x0F];
	}
	*out++ = '\n';
	*out++ = '\0';
	HexOutPtr   = &HexBuffer[OFF_DATA];
	DstAddress += ByteCount;
	ByteCount   = 0;

	return (fputs (line, fp) != EOF);

} /* WriteHexLine */

/*********************************************************************
 *
 * StartIntelHexSection
 *
 * SYNOPSIS
 *		short StartIntelHexSection (FILE *fp, uint16 Addr)
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
short StartIntelHexSection (FILE *fp, uint16 Addr)
{
	fp = fp;
	if (!StartFlag) {
		DstAddress = Addr;
		ByteCount  = 0;
		HexOutPtr  = &HexBuffer[OFF_DATA];
		StartFlag  = TRUE;
		return TRUE;
	} else {
		SetGlobalErr (ERR_GENERIC);
		GEprintf ("StartIntelHexSection: Started a new Section before Ending a previous section");
		return FALSE;
	}

} /* StartIntelHexSection */

/*********************************************************************
 *
 * WriteIntelHex
 *
 * SYNOPSIS
 *		short WriteIntelHex (FILE *fp, uint8 *data, size_t size)
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
short WriteIntelHex (FILE *fp, uint8 *data, size_t size)
{
	while (size) {
		*HexOutPtr++ = *data++;
		ByteCount++;
		if (ByteCount == MAX_BYTES) {
			if (!WriteHexLine (fp)) {
				return FALSE;
			}
		}
		size--;
	}

	return TRUE;

} /* WriteIntelHex */

/*********************************************************************
 *
 * EndIntelHexSection
 *
 * SYNOPSIS
 *		short EndIntelHexSection (FILE *fp)
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
short EndIntelHexSection (FILE *fp)
{

	StartFlag = FALSE;
	return WriteHexLine (fp);

} /* EndIntelHexSection */

/*********************************************************************
 *
 * EndIntelHexFile
 *
 * SYNOPSIS
 *		short EndIntelHexFile (FILE *fp)
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
short EndIntelHexFile (FILE *fp)
{
	return (fputs (":00000001FF\n", fp) != EOF);

} /* EndIntelHexFile */


/************************************  ************************************/
/************************************  ************************************/
/************************************  ************************************/

/*------------------------------------------------------------------------*/
/**# MODULE:DATATEFILE_DataStatements                                     */
/*------------------------------------------------------------------------*/

#define BUFFERSIZE	BufferSize
#define MAXBUFSIZE	80

static char		*Header;
static char		*Format;
static char		*Delim;
static uint8	 Buffer[MAXBUFSIZE];
static uint8	*b;
static short	 BufferSize;
static short	 Count;
static short	 Line;
static uint16	 ByteCount;
static uint16	 Flags;

/*********************************************************************
 *
 * WriteDataLine
 *
 * SYNOPSIS
 *		short WriteDataLine (FILE *fp)
 *
 * PURPOSE
 *		Write one line of data.
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
static short WriteDataLine (FILE *fp)
{
	short	i;

	b = Buffer;
	if (!(Flags & DATAST_NOLINEINFO) && !(Line & 0x0F)) {
		fprintf (fp, ";L%-4d: ", Line);
		fprintf (fp, Format, ByteCount);
		fprintf (fp, "\n");
	}
	fprintf (fp, Header);
	for (i = 0; i < Count - 1 ; i++) {
		fprintf (fp, Format, *b++);
		fprintf (fp, Delim);
	}
	fprintf (fp, Format, *b++);
	if (Flags & DATAST_ASMLINENO) {
		fprintf (fp, "\t;%d\n", Line);
	} else {
		fprintf (fp, "\n");
	}
	b          = Buffer;
	ByteCount += Count;
	Count      = 0;
	Line++;
	return TRUE;

} /* WriteDataLine */

/*********************************************************************
 *
 * StartDataStatements
 *
 * SYNOPSIS
 *		void StartDataStatements (
 *			char	*header,
 *			char	*format,
 *			char	*delimiter,
 *			short	 bytesperline,
 *			short	 flags)
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
void StartDataStatements (char *header, char *format, char *delimiter, short bytesperline, uint16 flags)
{
	Header     = header;
	Format     = format;
	Delim      = delimiter;
	Count      = 0;
	BufferSize = UTL_MIN(bytesperline,MAXBUFSIZE);
	b          = Buffer;
	Line       = 0;
	ByteCount  = 0;
	Flags      = flags;

} /* StartDataStatements */

/*********************************************************************
 *
 * WriteDataStatements
 *
 * SYNOPSIS
 *		short WriteDataStatements (FILE *fp, uint8 *data, size_t size)
 *
 * PURPOSE
 *		Write Data Statements
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
short WriteDataStatements (FILE *fp, uint8 *data, size_t size)
{
	while (size) {
		*b++ = *data++;
		size--;
		Count++;
		if (Count == BUFFERSIZE) {
			if (!WriteDataLine (fp)) {
				return FALSE;
			}
		}
	}
	return TRUE;

} /* WriteDataStatements */

/*********************************************************************
 *
 * EndDataStatements
 *
 * SYNOPSIS
 *		short EndDataStatements (FILE *fp)
 *
 * PURPOSE
 *		Write Last Data Line if any.
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
short EndDataStatements (FILE *fp)
{

	if (Count) {
		return WriteDataLine (fp);
	}
	return TRUE;

} /* EndDataStatements */

/*------------------------------------------------------------------------*/
/**# MODULE:DATATEFILE_StartDBHEXStatements                               */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * StartDBHEXStatements
 *
 * SYNOPSIS
 *		void StartDBHEXStatements (void)
 *
 * PURPOSE
 *		Setup WriteDataStatements for 'standard' DB statements with hex data
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
void StartDBHEXStatements (void)
{
	StartDataStatements ("\tDB ", "$%02x", ",", 16, 0);

} /* StartDBHEXStatements */

/*------------------------------------------------------------------------*/
/**# MODULE:DATATEFILE_StartDBDECStatements                               */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * StartDBDECStatements
 *
 * SYNOPSIS
 *		void StartDBDECStatements (void)
 *
 * PURPOSE
 *		Setup WriteDataStatements for 'standard' DB statements with DEC data
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
void StartDBDECStatements (void)
{

	StartDataStatements ("\tDB ", "$%3d", ",", 16, 0);

} /* StartDBDECStatements */

/************************************  ************************************/
/************************************  ************************************/
/************************************  ************************************/

/*------------------------------------------------------------------------*/
/**# MODULE:DATATEFILE_WordStatements                                     */
/*------------------------------------------------------------------------*/

#define BUFFERSIZE	BufferSize
#define MAXBUFSIZE	80

static char		*Header;
static char		*Format;
static char		*Delim;
static uint16	 wBuffer[MAXBUFSIZE];
static uint16	*w;
static short	 BufferSize;
static short	 Count;
static short	 Line;
static uint16	 WordCount;
static uint16	 Flags;

/*********************************************************************
 *
 * WriteWordLine
 *
 * SYNOPSIS
 *		short WriteWordLine (FILE *fp)
 *
 * PURPOSE
 *		Write one line of data.
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
static short WriteWordLine (FILE *fp)
{
	short	i;

	w = wBuffer;
	if (!(Flags & DATAST_NOLINEINFO) && !(Line & 0x0F)) {
		fprintf (fp, ";L%-4d: ", Line);
		fprintf (fp, Format, WordCount);
		fprintf (fp, "\n");
	}
	fprintf (fp, Header);
	for (i = 0; i < Count - 1 ; i++) {
		fprintf (fp, Format, *w++);
		fprintf (fp, Delim);
	}
	fprintf (fp, Format, *w++);
	if (Flags & DATAST_ASMLINENO) {
		fprintf (fp, "\t;%d\n", Line);
	} else {
		fprintf (fp, "\n");
	}
	w          = wBuffer;
	WordCount += Count;
	Count      = 0;
	Line++;
	return TRUE;

} /* WriteWordLine */

/*********************************************************************
 *
 * StartWordStatements
 *
 * SYNOPSIS
 *		void StartWordStatements (
 *			char	*header,
 *			char	*format,
 *			char	*delimiter
 *			short	 wordsperline
 *			short	 flags)
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
void StartWordStatements (
	char	*header,
	char	*format,
	char	*delimiter,
	short	 bytesperline,
	uint16	 flags
) {

	Header     = header;
	Format     = format;
	Delim      = delimiter;
	Count      = 0;
	BufferSize = UTL_MIN (bytesperline, MAXBUFSIZE);
	w          = wBuffer;
	Line       = 0;
	WordCount  = 0;
	Flags      = flags;

} /* StartWordStatements */

/*********************************************************************
 *
 * WriteWordStatements
 *
 * SYNOPSIS
 *		short WriteWordStatements (FILE *fp, uint16 *data, size_t size)
 *
 * PURPOSE
 *		Write Data Statements
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
short WriteWordStatements (FILE *fp, uint16 *data, size_t wordcount)
{
	while (wordcount) {
		*w++ = *data++;
		wordcount--;
		Count++;
		if (Count == BUFFERSIZE) {
			if (!WriteWordLine (fp)) {
				return FALSE;
			}
		}
	}
	return TRUE;

} /* WriteWordStatements */

/*********************************************************************
 *
 * EndWordStatements
 *
 * SYNOPSIS
 *		short EndWordStatements (FILE *fp)
 *
 * PURPOSE
 *		Write Last Data Line if any.
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
short EndWordStatements (FILE *fp)
{
	if (Count) {
		return WriteWordLine (fp);
	}
	return TRUE;

} /* EndWordStatements */

/************************************  ************************************/
/************************************  ************************************/
/************************************  ************************************/

/*------------------------------------------------------------------------*/
/**# MODULE:DATATEFILE_CPEFiles                                           */
/*------------------------------------------------------------------------*/

static uint32	 CPEBytesWritten;
static uint32	 CPELenPos = 0;
static LST_LIST	 CPEListX;
static LST_LIST	*CPEList = &CPEListX;
static char		*CPEName;

/*********************************************************************
 *
 * OpenCPEFileWrite
 *
 * SYNOPSIS
 *		int OpenCPEFileWrite (char *filename)
 *
 * PURPOSE
 *		Open a file and write the CPE format header into it.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *		file handle, (-1) if error
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
int OpenCPEFileWrite (char *filename)
{
	static	uint8	Header[] = { 'C', 'P', 'E', 1 };

	int		fh;

	fh = EIO_WriteOpen (filename);
	if (fh == (-1)) {
		SetGlobalErr (ERR_GENERIC);
		GEcatf1 ("\nOpenCPEFileWrite: Couldn't open '%s'", filename);
		return (-1);
	}

	EIO_Write (fh, Header, sizeof (Header));

	LST_InitList (CPEList);
	CPELenPos = 0;
	CPEName   = filename;

	return fh;

} /* OpenCPEFileWrite */

/*********************************************************************
 *
 * OpenCPEMemSection
 *
 * SYNOPSIS
 *		short OpenCPEMemSection (int fh, uint32 address)
 *
 * PURPOSE
 *		Write CPE memory section header to file and track bytes written.
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
short OpenCPEMemSection (int fh, uint32 address)
{
	uint8	Data[9];

	if (CPELenPos != 0) {
		SetGlobalErr (ERR_GENERIC);
		GEcatf ("\nOpenCPEMemSection:Section already open");
		return FALSE;
	}

	Data[0] = 1;
	Data[1] = (uint8)((address & 0x000000FFUL) >>  0);
	Data[2] = (uint8)((address & 0x0000FF00UL) >>  8);
	Data[3] = (uint8)((address & 0x00FF0000UL) >> 16);
	Data[4] = (uint8)((address & 0xFF000000UL) >> 24);
	
	CPEBytesWritten = 0;
	CPELenPos       = EIO_Seek (fh, 0, EIO_SEEK_CURRENT) + 5;

	if (EIO_Write (fh, Data, sizeof (Data)) != sizeof (Data)) {
		SetGlobalErr (ERR_GENERIC);
		GEcatf ("\nOpenCPEMemSection:Error writing header");
		return FALSE;
	}
	
	return TRUE;
} /* OpenCPEMemSection */

/*********************************************************************
 *
 * WriteCPEMemSection
 *
 * SYNOPSIS
 *		short WriteCPEMemSection (int fh, void *buffer, uint32 size)
 *
 * PURPOSE
 *		Write bytes to an CPE file and track bytes written.
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
short WriteCPEMemSection (int fh, void *buffer, uint32 size)
{
	if (EIO_Write (fh, buffer, size) != (int32)size) {
		SetGlobalErr (ERR_GENERIC);
		GEcatf ("\nWriteCPEMemSection:Error writing buffer");
		return FALSE;
	}

	CPEBytesWritten += size;
	return TRUE;

} /* WriteCPEMemSection */

/*********************************************************************
 *
 * CloseCPEMemSection
 *
 * SYNOPSIS
 *		short CloseCPEMemSection (void)
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
short CloseCPEMemSection (void)
{
	CPEPosType	*cpt;

	cpt = (CPEPosType *)LST_CreateNode (sizeof (CPEPosType), NULL);
	if (!cpt) {
		SetGlobalErr (ERR_OUT_OF_MEMORY);
		GEcatf ("CloseCPEMemSection:OOM CPEPosType");
		return FALSE;
	}

	cpt->LenPos = CPELenPos;
	cpt->Length = CPEBytesWritten;
	LST_AddTail (CPEList, cpt);

	CPELenPos = 0;

	return TRUE;

} /* CloseCPEMemSection */

/*********************************************************************
 *
 * WriteEntireCPEMemSection
 *
 * SYNOPSIS
 *		short WriteEntireCPEMemSection (
 *			int		 fh,
 *			uint32	 address,
 *			void	*buffer,
 *			uint32	 size
 *		)
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
short WriteEntireCPEMemSection (
	int		 fh,
	uint32	 address,
	void	*buffer,
	uint32	 size
) {
	if (OpenCPEMemSection (fh, address)) {
		if (WriteCPEMemSection (fh, buffer, size)) {
			return CloseCPEMemSection ();
		}
	}
	return FALSE;

} /* WriteEntireCPEMemSection */

/*********************************************************************
 *
 * CloseCPEFileWrite
 *
 * SYNOPSIS
 *		short CloseCPEFileWrite (int fh)
 *
 * PURPOSE
 *		Close CPE file and update all length headers.
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
short CloseCPEFileWrite (int fh)
{

	CPEPosType	*cpt;
	uint8		 Data[4];
	uint8		 End = 0;

	if (CPELenPos != 0) {
		SetGlobalErr (ERR_GENERIC);
		GEcatf ("\nCloseCPEFileWrite:MemSection still open");
		return FALSE;
	}

	if (EIO_Write (fh, &End, sizeof (End)) != sizeof (End)) {
		SetGlobalErr (ERR_GENERIC);
		GEcatf ("\nOpenCPEMemSection:Error writing CPE EOF");
		return FALSE;
	}

	EIO_Close (fh);

	fh = EIO_UpdateOpen (CPEName);
	if (fh == (-1)) {
		SetGlobalErr (ERR_GENERIC);
		GEcatf1 ("\nCloseCPEFileWrite:Couldn't reopen '%s'", CPEName);
		return FALSE;
	}

	while (cpt = (CPEPosType *)LST_RemHead (CPEList)) {
		Data[0] = (uint8)((cpt->Length & 0x000000FFUL) >>  0);
		Data[1] = (uint8)((cpt->Length & 0x0000FF00UL) >>  8);
		Data[2] = (uint8)((cpt->Length & 0x00FF0000UL) >> 16);
		Data[3] = (uint8)((cpt->Length & 0xFF000000UL) >> 24);

		EIO_Seek (fh, cpt->LenPos, EIO_SEEK_BEGINNING);

		EIO_Write (fh, Data, sizeof (Data));

		LST_DeleteNode (cpt);
	}

	EIO_Close (fh);
	return TRUE;

} /* CloseCPEFileWrite */

