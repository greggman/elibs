/*************************************************************************
 *                                                                       *
 *                                 EIO.H                                 *
 *                                                                       *
 *************************************************************************

                          Copyright 1996 Echidna

   DESCRIPTION
		Types and Prototype for EIO (Echidna I/O) routines.

   PROGRAMMERS


   FUNCTIONS

   TABS : 5 9

   HISTORY
		07/09/96 : Created.

 *************************************************************************/

#ifndef EL_EIO_H
#define EL_EIO_H
/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#if _EL_CC_VC__
	#include <io.h>
	#include <sys/stat.h>
	#include <sys/types.h>
#endif

#if _EL_CC_TURBOC__
	#include <dos.h>
	#include <dir.h>
	#include <io.h>
	#include <sys/stat.h>
#endif

#if _EL_CC_WATCOMC__
	#include <dos.h>
	#include <io.h>
	#include <sys/types.h>
	#include <sys/stat.h>
#endif

#if _EL_OS_AMIGAOS__
	#include <libraries/dos.h>
	#include <libraries/arpbase.h>
#endif

#if _EL_OS_MACOS__
	#include <files.h>
#endif

#if _EL_OS_IRIX53__
	#include <sys/types.h>
	#include <sys/dir.h>
	#include <sys/stat.h>
	#include <utime.h>
	#include <unistd.h>
#endif

#include "echidna/listapi.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************** C O N S T A N T S ***************************/

#if _EL_OS_MSDOS__

	// #define EIO_MAXPATH		80
	// #define EIO_MAXDIR		66
	// #define EIO_MAXFILE		20
	// #define EIO_MAXEXT		20
	
	#define EIO_MAXPATH		256
	#define EIO_MAXDIR		256
	#define EIO_MAXFILE		256
	#define EIO_MAXEXT		256
	
	#define	DIRCURRENT		".\\"
	#define	DIRSEPS			":/\\"
	#define	DIRMATCHALL		"*.*"
	#define DIRSEP			'\\'
	#define OPENEXTRA		,S_IREAD|S_IWRITE

	#define	EIO_BATCHPREFIX		""
	#define	EIO_CALLPREFIX  	"call "
	#define	EIO_ENVPREFIX   	"%"
	#define	EIO_ENVSUFFIX   	"%"
		
#elif _EL_OS_WIN32__

	// the OS will fail if the name passed is longer than this
	#define EIO_MAXPATH		MAX_PATH
	#define EIO_MAXDIR		MAX_PATH
	#define EIO_MAXFILE		MAX_PATH
	#define EIO_MAXEXT		MAX_PATH
	
	#define	DIRCURRENT		".\\"
	#define	DIRSEPS			":/\\"
	#define	DIRMATCHALL		"*.*"
	#define DIRSEP			'\\'
	#define OPENEXTRA		,S_IREAD|S_IWRITE

	#define	EIO_BATCHPREFIX		""
	#define	EIO_CALLPREFIX  	"call "
	#define	EIO_ENVPREFIX   	"%"
	#define	EIO_ENVSUFFIX   	"%"
		
#elif _EL_OS_AMIGAOS__

	#define EIO_MAXPATH		(LONG_FSIZE * 2 + 2)
	#define EIO_MAXDIR		EIO_MAXPATH
	#define EIO_MAXFILE		EIO_MAXPATH
	#define EIO_MAXEXT		32
	
	#define	DIRCURRENT		""
	#define	DIRSEPS			":/\\"
	#define DIRMATCHALL		"#?"
	#define DIRSEP			'/'
	#define O_BINARY		0
	#define OPENEXTRA	

	#define	EIO_BATCHPREFIX		""
	#define	EIO_CALLPREFIX  	""
	#define	EIO_ENVPREFIX   	"{"
	#define	EIO_ENVSUFFIX   	"}"
	
#elif _EL_OS_MACOS__

	#define EIO_MAXPATH		256
	#define EIO_MAXDIR		EIO_MAXPATH
	#define EIO_MAXFILE		40
	#define EIO_MAXEXT		32
	
	#define	DIRCURRENT		":"
	#define	DIRSEPS			":/\\"
	#define DIRMATCHALL		"Å"
	#define DIRSEP			':'
	#define O_BINARY		0
	#define OPENEXTRA	

	#define	EIO_BATCHPREFIX		""
	#define	EIO_CALLPREFIX  	""
	#define	EIO_ENVPREFIX   	"{"
	#define	EIO_ENVSUFFIX   	"}"
	
#elif _EL_OS_IRIX53__

	#define EIO_MAXPATH		256
	#define EIO_MAXDIR		EIO_MAXPATH
	#define EIO_MAXFILE		256
	#define EIO_MAXEXT		256
	
	#define	DIRCURRENT		"./"
	#define	DIRSEPS			":/\\"
	#define DIRMATCHALL		"*"
	#define DIRSEP			'/'
	#define O_BINARY		0
	#define OPENEXTRA	

	#define	EIO_BATCHPREFIX		"#! /bin/csh -e -f\n#\n"
	#define	EIO_CALLPREFIX  	""
	#define	EIO_ENVPREFIX   	"${"
	#define	EIO_ENVSUFFIX   	"}"

#else
#error need some code
#endif

/*
 * Constants returned EIO_fnsplit()
 */
#define EIO_DIRECTORY	(1 << 0)
#define EIO_FILENAME	(1 << 1)
#define EIO_EXTENSION	(1 << 2)

/*
 * Constants returned EIO_FileType()
 */
#define EIO_TYPE_FILE		1
#define EIO_TYPE_DIRECTORY	2

/*
 * Constants used by EIO_Seek()
 */
#define EIO_SEEK_BEGINNING	0
#define EIO_SEEK_CURRENT	1
#define EIO_SEEK_END		2

/*
 * Constants used by EIO_ModifyFileAttribBit()
 */
#define EIO_ATTR_WRITE		0x0001	// read only
#define EIO_ATTR_ARCHIVE	0x0002
#define EIO_ATTR_EXECUTE	0x0004
#define EIO_ATTR_READ		0x0008

/*
 * Constants used by EIO_FindInclude()
 */
#define EIO_INCPATH_INCLUDES	"includes"
#define EIO_INCPATH_LIBS		"libs"
#define EIO_INCPATH_INIS		"inis"


/***************************** T Y P E D E F S ****************************/

#if _EL_CC_TURBOC__

typedef unsigned rwtype;

typedef struct {
	int				 DirFlag;
	struct ffblk	 FFBlk;
	char far32		*PathPtr;
	char			 TotalPath[256];
	
	int				 Status;		// public
	int				 IsDir;			// public
	char far32		*EnvString;		// public
	char			 Path[256];		// public
} DirTracker;

typedef struct {
	struct ftime	FTime;
} FileDateType;

typedef struct {
	int		Attrib;
} FileAttribType;

typedef struct {
	char	Comment[1];
} FileCommentType;

#elif (_EL_CC_WATCOMC__ && _EL_OS_MSDOS__)

typedef unsigned rwtype;

typedef struct {
	int				 DirFlag;
	int				 GotFirst;
	struct find_t 	 FFBlk;
	char far32		*PathPtr;
	char			 TotalPath[256];
	
	int				 Status;		// public
	int				 IsDir;			// public
	char far32		*EnvString;		// public
	char			 Path[256];		// public
} DirTracker;

typedef struct {
	unsigned int		 Date;
	unsigned int		 Time;
} FileDateType;

typedef struct {
	unsigned	 Attrib;
} FileAttribType;

typedef struct {
	char	Comment[1];
} FileCommentType;

#elif (_EL_CC_VC__)

typedef unsigned rwtype;

typedef struct {
	int					 DirFlag;
	int					 GotFirst;
	long				 findHandle;
	struct _finddata_t 	 finddata;
	char far32			*PathPtr;
	char				 TotalPath[256];
	
	int					 Status;		// public
	int					 IsDir;			// public
	char far32			*EnvString;		// public
	char				 Path[256];		// public
} DirTracker;

typedef struct {
	struct		stat		fileStat;
} FileDateType;

typedef FileDateType FileAttribType;

typedef FileDateType FileCommentType;

#elif _EL_OS_AMIGAOS__

typedef size_t rwtype;

struct Path {
	LONG	path_Next;
	LONG	path_Lock;
};

typedef struct DirTracker {
	int					 DirFlag;
	struct DirTracker	*SubDir;
	struct DirTracker	*ParentDir;
	struct DirTracker	*PrivateDir;
	struct Path			*APath;
	struct AnchorPath	*Anchor;
	char				 TotalPath[EIO_MAXPATH];
	
	int					 IsDir;				// public
	int					 Status;			// public
	char				*EnvString;			// public
	char				 Path[EIO_MAXPATH];	// public
} DirTracker;

typedef struct {
	struct DateStamp	DateStamp;
} FileDateType;

typedef struct {
	LONG		Attrib;
} FileAttribType;

typedef struct {
	char	Comment[81];
} FileCommentType;

#elif _EL_OS_MACOS__

typedef size_t rwtype;

typedef struct {
	char			*PathPtr;		// private
	char			 Pattern[256];	// private
	int				 fileNdx;		// private
	int				 DirFlag;		// private
	FSSpec			 parentfss;		// private
	
	int				 Status;		// public
	int				 IsDir;			// public
	char			*EnvString;		// public
	char			 Path[256];		// public
} DirTracker;

typedef struct {
	CInfoPBRec		cipb;
} FileDateType;

typedef struct {
	FileDateType	fdt;
} FileAttribType;

typedef struct {
	FileDateType	fdt;
} FileCommentType;

#elif _EL_OS_IRIX53__

typedef size_t rwtype;

typedef struct {
	char			*PathPtr;		// private
	char			 Pattern[256];	// private
	int				 fileNdx;		// private
	int				 DirFlag;		// private
	DIR				*dirp;			// private
	char			 FullPath[256];	// private
	int				 Status;		// public
	int				 IsDir;			// public
	char			*EnvString;		// public
	char			 Path[256];		// public
} DirTracker;

typedef struct {
	struct		stat		fileStat;
} FileDateType;

typedef FileDateType FileAttribType;

typedef FileDateType FileCommentType;

#else
#error Need Stuff
#endif

/******************************* M A C R O S ******************************/

#if (_EL_OS_WIN32__ || _EL_OS_MSDOS__)

#define EIO_Open(name,mode)		open((name),(mode),(S_IREAD|S_IWRITE))
#define EIO_Close(fh)			close(fh)
#define EIO_Seek(fh,off,org)	lseek((fh),(off),(org))
#define EIO_Tell(fh)			tell(fh)

#elif _EL_OS_MACOS__

#define EIO_Open(name,mode)		open((name),(mode))
#define EIO_Read(fh,buf,size)	read((fh),(buf),(size))
#define EIO_Write(fh,buf,size)	write((fh),(buf),(size))
#define EIO_Close(fh)			close(fh)
#define EIO_Seek(fh,off,org)	lseek((fh),(off),(org))
#define EIO_Tell(fh)			tell(fh)

#elif _EL_OS_IRIX53__

#define EIO_Open(name,mode)		open((name),(mode),(S_IREAD|S_IWRITE|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH))
#define EIO_Close(fh)			close(fh)
#define EIO_Read(fh,buf,size)	read((fh),(buf),(size))
#define EIO_Write(fh,buf,size)	write((fh),(buf),(size))
#define EIO_Seek(fh,off,org)	lseek((fh),(off),(org))
#define EIO_Tell(fh)			tell(fh)

#else
#error Need EIO_Open,EIO_Close...
#endif

#define EIO_ReadOpen(name)		EIO_Open((name), O_RDONLY | O_BINARY)
#define EIO_OpenRead(name)		EIO_Open((name), O_RDONLY | O_BINARY)
#define EIO_WriteOpen(name)		EIO_Open((name), O_WRONLY | O_BINARY | O_TRUNC | O_CREAT)
#define EIO_OpenWrite(name)		EIO_Open((name), O_WRONLY | O_BINARY | O_TRUNC | O_CREAT)
#define EIO_UpdateOpen(name)	EIO_Open((name), O_RDWR | O_BINARY | O_CREAT)
#define EIO_OpenUpdate(name)	EIO_Open((name), O_RDWR | O_BINARY | O_CREAT)
#define EIO_AppendOpen(name)	EIO_Open((name), O_RDWR | O_BINARY | O_CREAT | O_APPEND)
#define EIO_OpenAppend(name)	EIO_Open((name), O_RDWR | O_BINARY | O_CREAT | O_APPEND)

#define	EIO_FileListType(nd)	((nd)->type)

/****************************** G L O B A L S *****************************/

extern int	 EIO_CheckDates;
extern char	 EIO_BatchPrefix[];
extern char	 EIO_CallPrefix[];
extern char	 EIO_EnvPrefix[];
extern char	 EIO_EnvSuffix[];

/****************** F U N C T I O N   P R O T O T Y P E S *****************/

extern int			 EIO_FileExists (const char* filename);
extern char         *EIO_ExpandEVarsWithErrors (char *newstr, const char* old, size_t maxlen, bool fCheckForErrors);
extern char			*EIO_ExpandEVars (char *newstr, const char* old, size_t maxlen);
extern char			*EIO_FixDirSeps (char *str);
extern int			 EIO_InsureEndSlash (char *str);
extern void			 EIO_GetTempFileName (char *temp);
extern void			 EIO_fnmerge (char *path, const char* dir, const char* name, const char* ext);
extern uint16		 EIO_fnsplit (const char* path, char *dir, char *name, char *ext);
extern char			*EIO_Path (const char* filespec);
extern char			*EIO_Name (const char* filespec);
extern char			*EIO_Ext (const char* filespec);
extern char			*EIO_Filename (const char* filespec);
extern char			*EIO_CurrentDir (char *path);
extern int			 EIO_ChangeDir (char *path);
extern void			 EIO_FreeDirTracker (DirTracker *dt);
extern int			 EIO_GetNextPath (DirTracker *dt);
extern DirTracker	*EIO_GetFirstPath (void);
extern int			 EIO_SpawnProgram (const char* program, const char* *argv);
extern long			 EIO_FileLength (int fd);
extern DirTracker 	*EIO_GetFirstFile (const char* path, int	 dirflag);
extern int			 EIO_GetNextFile (DirTracker *dt);
extern LST_LIST		*EIO_GetFileList (const char* filespec, int	 recursive, int	 directories);
extern int			 EIO_FindFile (const char* filename, const char* guidename, char* pathbuff);
extern int			 EIO_PrintEscString (FILE *fp, const char* s);
extern DirTracker 	*EIO_GetFirstEnv (void);
extern int			 EIO_GetNextEnv (DirTracker *dt);
extern int 			 EIO_FileType (const char* filename);
extern int 			 EIO_GetFileDate (const char* filename, FileDateType *fdt);
extern int 			 EIO_SetFileDate (const char* filename, FileDateType *fdt);
extern int 			 EIO_GetFileAttrib (const char* filename, FileAttribType *fat);
extern int 			 EIO_SetFileAttrib (const char* filename, FileAttribType *fat);
extern void			 EIO_ModifyFileAttribBit (FileAttribType* fat, int bits, bool fSet);
extern int 			 EIO_GetFileComment (const char* filename, FileCommentType *fct);
extern int 			 EIO_SetFileComment (const char* filename, FileCommentType *fct);
extern int 			 EIO_CopyFile (const char* from, const char* to);
extern int 			 EIO_MakeDir (const char* filename);
extern int			 EIO_CmpDates (FileDateType *fdt1, FileDateType *fdt2);
extern int			 EIO_FileNewer (const char* source, const char* target);
extern int			 EIO_DirExists (const char* dirPath);
extern void			 EIO_InitFilespecFixup (const char* spec);
extern void			 EIO_FixupFilespec (char *outname, char* inname);
extern int			 EIO_AddIncludePath (const char* groupName, const char* path);
extern int			 EIO_FindInclude (char* fixedfilespec, const char *groupName, const char* newfilespec, const char*currentfilespec, int fUseCurrentDir);

#if (_EL_OS_MSDOS__)

	extern long		 EIO_Read (int fh, void *buf, long size);
	extern long		 EIO_Write (int fh, void *buf, long size);

#else

	#define EIO_Read(fh,buf,size)	read((fh),(buf),(size))
	#define EIO_Write(fh,buf,size)	write((fh),(buf),(size))

#endif

#ifdef __cplusplus
}
#endif

#endif /* EL_EIO_H */
