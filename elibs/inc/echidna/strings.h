/*************************************************************************
 *                                                                       *
 *                               STRINGS.H                               *
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
		07/09/96 : Created.

 *************************************************************************/

#ifndef EL_STRINGS_H
#define EL_STRINGS_H
/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdlib.h>
#include <stddef.h>

/******************************* M A C R O S ******************************/
#if (__TURBOC__ || (__ZTC__ && __MSDOS__) || __MACSC__ || __WATCOMC__ || _EL_PLAT_SGI__)
#define dupstr(val)		strdup (val)
#endif

#define freestr(val)	free (val)

/***************************** T Y P E D E F S ****************************/

#ifdef __cplusplus
}

#if ( _EL_PLAT_WIN32__ && defined(_AFXDLL))

class ELString : public CString {
public:
	// Constructors
	ELString()                           { }
	ELString(LPCTSTR str) : CString(str)    { }
	ELString(CString s): CString(s)      { }
	// Assignment
	const ELString& operator=(LPCTSTR str)
		{ CString::operator=(str); return *this; }
	const ELString& operator=(CString s)
		{ CString::operator=(s); return *this; }
	// Useful stuff
	
	void FormatV(LPCTSTR lpszFormat, va_list argList) { CString::FormatV(lpszFormat, argList); };
	void EscapeString (void);
	void TrimWhitespace (void);
	void StripComments (void);
	void UnEscapeString (void);
	void ReplaceChar (TCHAR from, TCHAR to);
	BOOL GetFirstArg (ELString& arg);
	int GetLine (ELString& line);
	int NumArgs (void);

	BOOL GetArg (ELString& arg) { return GetFirstArg (arg); }
	BOOL GetInt (int& value);
	BOOL GetLong (long& value);
	BOOL GetBool (BOOL& value);
	BOOL GetUInt32 (uint32& value);
};

#endif

extern "C" {
#endif

/****************************** G L O B A L S *****************************/


/****************** F U N C T I O N   P R O T O T Y P E S *****************/

extern const char *stristr (const char *data, const char *key);

extern int		 lchinstr (
							char	 ch,
							const char	*str
							);
extern int		 rchinstr (
							char	 ch,
							const char	*str
							);
extern int		 rstrchinstr (
							const char	*chset,
							const char	*str
							);
extern char		*TrimWhiteSpace (char *str);
extern char		*TrimWhiteSpaceAndQuotes (char *str);
extern void		 StripComment (char	*s);
extern int		 EscapeString (char* dst, const char* src);
extern int		 UnEscapeString (char* dst, const char* src);
extern char		*UnEscStr (char *orig);
extern short	 repstr (char **dest, const char *src);
extern short	fstrpcmp (
							const char *fpat,
							const char *fstr
						);

extern void		*memcpysrc (void *dst, void *src, size_t len);
extern void		*memcpydst (void *dst, void *src, size_t len);
extern short	 EL_atos (const char *s);
extern long		 EL_atol (const char *s);
extern BOOL		 EL_isnum (int c);
extern const char* EL_SafeStr (const char* str);

extern void		 pCharAppend(unsigned char p1, unsigned char *p2);
extern void		 pStrAppend(const unsigned char * src,unsigned char * dest);
extern void		 pStrCopy(const unsigned char *src,unsigned char *dest);


#if _EL_CC_VC__
extern char		*dupstr (const char *s);
#endif

#if _EL_PLAT_SGI__
	#define	stricmp		strcasecmp
	#define	strnicmp	strncasecmp
#endif

#if AZTEC_C
extern char		*dupstr (const char *s);
extern short	 stricmp (const char *s1, const char *s2);
extern short	 strnicmp (const char *s1, const char *s2, size_t len);
extern char		*strupr (char *s);
extern char		*itoa (int value, char *string, int radix);
extern char		*ltoa (long value, char *string, int radix);
#endif

#if __MACSC__
extern int		 stricmp (const char *s1, const char *s2);
extern int		 strnicmp (const char *s1, const char *s2, size_t len);
#endif

#if _EL_PLAT_SONY__
extern int		 stricmp (const char *s1, const char *s2);
extern int		 strnicmp (const char *s1, const char *s2, size_t len);
#endif

#ifdef __cplusplus
}
#endif

#endif /* EL_STRINGS_H */
