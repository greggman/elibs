/*************************************************************************
 *                                                                       *
 *                               READINI.H                               *
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

#ifndef EL_READINI_H
#define EL_READINI_H
/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"


#include "echidna/listapi.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************** C O N S T A N T S ***************************/


/******************************** T Y P E S *******************************/

typedef LST_LIST IniList;

typedef struct
{
	LST_NODE	Node;
	LST_LIST	LineList;
} Section;

typedef struct
{
	LST_NODE	Node;
	int 		LineNo;
} ConfigLine;

typedef struct
{
	LST_LIST *ltSection;
	ConfigLine *clCurrentLine;
} SectionTracker;

/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/

#define szSectionName LST_NodeName
#define szConfigLine LST_NodeName

#define GetConfigLineNo(configline)	((configline)->LineNo)
#define	GetCurrentSectionLine(section)	GetConfigLineNo(((section)->clCurrentLine))
#define ReadINI(filename)				AppendINI(NULL,(filename))

#define INI_IsSectionUsed(sectionTracker)  ((sectionTracker)->ltSection->type != 0)
#define INI_markSectionAsUsed(sectionTracker)  ((sectionTracker)->ltSection->type = 1)
#define INI_markSectionAsUnused(sectionTracker)  ((sectionTracker)->ltSection->type = 0)

/****************** F U N C T I O N   P R O T O T Y P E S *****************/

extern void SetINICaseSensitive(BOOL f);	/* case sensitive section merge, FindSection & FindNextINILine */
extern void SetINISaveBlankLines(BOOL f);	/* Set TRUE to save blank lines as well */
extern void SetINITrimWhiteSpace(BOOL f);	/* Set TRUE to strip leading & trailing WS */
extern void SetINIStripComments(BOOL f);	/* Set TRUE to strip comments starting w/ <szComment> */
extern void SetINICommentAnywhere(BOOL f);/* Set TRUE to strip from <szComment> to end of line */
extern void SetINIMergeSections(BOOL f);	/* Set TRUE to merge lines with same section heading */
extern void SetINIUndefEnvVarIsError(BOOL f); /* Set TRUE to generate errors if referenced envvars are undefined */

extern void SetINIComment(const char *sz);		/* string to match for a comment. Eg., szComment = "//" */
extern void SetINISectionMarker(const char *sz);/* lines beginning w/ <szSectionMarker> start a section */

extern int GetINIWarnings(void);	/* returns number of warnings in parsing INI file */

extern void FreeINI(IniList *pIni);
extern void PrintINI(IniList *pIniList);
extern IniList *AppendINI(IniList *pIniList, const char *filename);

extern IniList *CreateINI(const char *pszName);
extern Section *AddINISection (IniList *pIniList, const char *pszSectionName);
extern ConfigLine *AddINILine (Section *pst, const char *pszIniLine, int lineNo);

extern void ResetSection(SectionTracker *pst);
extern SectionTracker *FindSection(SectionTracker *pst, IniList *pIniList, const char *sz);
extern ConfigLine *GetNextINILine(SectionTracker *pst);
extern int CountINILines(SectionTracker *pst);
extern char *FindNextINILine(SectionTracker *pst, const char *sz);
extern char *FindFirstINILine (SectionTracker *pst, const char *sz);

extern BOOL GetFirstBool (SectionTracker *pst, const char *sz, BOOL *dst);
extern BOOL GetFirstUINT32 (SectionTracker *pst, const char *sz, uint32 *dst);
extern BOOL GetFirstINT32 (SectionTracker *pst, const char *sz, int32 *dst);
extern BOOL GetFirstUINT16 (SectionTracker *pst, const char *sz, uint16 *dst);
extern BOOL GetFirstINT16 (SectionTracker *pst, const char *sz, int16 *dst);
extern BOOL GetFirstString (SectionTracker *pst, const char *sz, char *dst, size_t maxlen);
extern BOOL GetNextBool (SectionTracker *pst, const char *sz, BOOL *dst);
extern BOOL GetNextUINT32 (SectionTracker *pst, const char *sz, uint32 *dst);
extern BOOL GetNextINT32 (SectionTracker *pst, const char *sz, int32 *dst);
extern BOOL GetNextUINT16 (SectionTracker *pst, const char *sz, uint16 *dst);
extern BOOL GetNextINT16 (SectionTracker *pst, const char *sz, int16 *dst);
extern BOOL GetNextString (SectionTracker *pst, const char *sz, char *dst, size_t maxlen);


#ifdef __cplusplus
}
#endif

#endif /* EL_READINI_H */
