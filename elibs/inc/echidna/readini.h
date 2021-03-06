/*************************************************************************
 *                                                                       *
 *                               READINI.H                               *
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

#define USE_STL 1

/******************************** T Y P E S *******************************/

typedef struct
{
	#if USE_STL
	void* data;
	#else
    LST_LIST SectionList;
    LST_LIST FileList;
	#endif
}
IniList;

typedef struct
{
	LST_NODE	Node;
	LST_LIST	LineList;
	int 		LineNo;     // where this section started
    const char* Filename;   // file this section started (note: if you have MergeSections on then of course this might not tell you the whole story
    const char* Args;       // part after the ',' if fParseArgsInSection is true
} Section;

typedef struct
{
	LST_NODE	Node;
	int 		LineNo;
    const char* Filename;
} ConfigLine;

typedef struct
{
	Section    *ltSection;
	ConfigLine *clCurrentLine;
} SectionTracker;

/****************************** G L O B A L S *****************************/


/******************************* M A C R O S ******************************/

#define szSectionName LST_NodeName
#define szConfigLine LST_NodeName

#define GetConfigLineNo(configline)	((configline)->LineNo)
#define GetConfigFilename(configline)	((configline)->Filename)
#define	GetCurrentSectionLine(section)	GetConfigLineNo(((section)->clCurrentLine))
#define GetSectionArgs(sectiontracker)  ((sectiontracker)->ltSection->Args)
#define ReadINI(filename)				AppendINI(NULL,(filename))

#define INI_IsSectionUsed(sectionTracker)  ((sectionTracker)->ltSection->LineList.type != 0)
#define INI_markSectionAsUsed(sectionTracker)  ((sectionTracker)->ltSection->LineList.type = 1)
#define INI_markSectionAsUnused(sectionTracker)  ((sectionTracker)->ltSection->LineList.type = 0)

/****************** F U N C T I O N   P R O T O T Y P E S *****************/

extern void SetINICaseSensitive(BOOL f);	/* case sensitive section merge, FindSection & FindNextINILine */
extern void SetINISaveBlankLines(BOOL f);	/* Set TRUE to save blank lines as well */
extern void SetINITrimWhiteSpace(BOOL f);	/* Set TRUE to strip leading & trailing WS */
extern void SetINIStripComments(BOOL f);	/* Set TRUE to strip comments starting w/ <szComment> */
extern void SetINICommentAnywhere(BOOL f);/* Set TRUE to strip from <szComment> to end of line */
extern void SetINIMergeSections(BOOL f);	/* Set TRUE to merge lines with same section heading */
extern void SetINIUndefEnvVarIsError(BOOL f); /* Set TRUE to generate errors if referenced envvars are undefined */
extern void SetINIUseMacroLanguage(BOOL f); /* Set TRUE to allow macro language, see docs */
extern void SetINIStripCPlusPlusComments(BOOL f);  /* Set TRUE to allow CPlusPlus comments, added because I needed both C++ and ASM comments */
extern void SetINIErrorOnDuplicateSection(BOOL f); /* set TRUE to cause error if more than one section with same name */
extern void SetINIParseArgsInSection(BOOL f);   /* set to TRUE to make [sectionname,arg=foo] get parsed */
extern void SetINIChangeCurrentDir(BOOL f);   /* set to TRUE to have it change the current directory to the current file's path so that relative includes will work */

extern void SetINIComment(const char *sz);		/* string to match for a comment. Eg., szComment = "//" */
extern void SetINISectionMarker(const char *sz);/* lines beginning w/ <szSectionMarker> start a section */

extern int GetINIWarnings(void);	/* returns number of warnings in parsing INI file */

extern void FreeINI(IniList *pIni);
extern void PrintINI(IniList *pIniList);
extern IniList *AppendINI(IniList *pIniList, const char *filename);

extern IniList *CreateINI(const char *pszName);
extern Section *AddINISection (IniList *pIniList, const char *pszSectionName);
extern ConfigLine *AddINILine (Section *pst, const char *pszIniLine, int lineNo, const char* filename);
//extern ConfigLine *AddINILine (Section *pst, const char *pszIniLine, int lineNo);

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
