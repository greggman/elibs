/*************************************************************************
 *                                                                       *
 *                              DATAFILE.H                               *
 *                                                                       *
 *************************************************************************

                          Copyright 1996 Echidna

   DESCRIPTION


   PROGRAMMERS


   FUNCTIONS

   TABS : 5 9

   HISTORY
		07/11/96 : Created.

 *************************************************************************/

#ifndef EL_DATAFILE_H
#define EL_DATAFILE_H
/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**************************** C O N S T A N T S ***************************/

#define	DATAST_NOLINEINFO	0x0001
#define DATAST_ASMLINENO	0x0002

/******************************** T Y P E S *******************************/


/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/


/****************** F U N C T I O N   P R O T O T Y P E S *****************/

extern short StartIntelHexSection (FILE *fp, uint16 Addr);
extern short WriteIntelHex (FILE *fp, uint8 *data, size_t size);
extern short EndIntelHexSection (FILE *fp);
extern short EndIntelHexFile (FILE *fp);

extern void	 StartDataStatements (char *header, char *format, char *delimiter, short bytesperline, uint16 flags);
extern short WriteDataStatements (FILE *fp, uint8 *data, size_t size);
extern short EndDataStatements (FILE *fp);
extern void	 StartDBHEXStatements (void);
extern void	 StartDBDECStatements (void);
extern void	 StartWordStatements (char *header, char *format, char *delimiter, short wordsperlibe, uint16 flags);
extern short WriteWordStatements (FILE *fp, uint16 *data, size_t wordcount);
extern short EndWordStatements (FILE *fp);

extern int	 OpenCPEFileWrite (char *filename);
extern short OpenCPEMemSection (int fh, uint32 address);
extern short WriteCPEMemSection (int fh, void *buffer, uint32 size);
extern short CloseCPEMemSection (void);
extern short WriteEntireCPEMemSection (int fh, uint32 address, void *buffer, uint32 size);
extern short CloseCPEFileWrite (int fh);

#ifdef __cplusplus
}
#endif

#endif /* EL_DATAFILE_H */
