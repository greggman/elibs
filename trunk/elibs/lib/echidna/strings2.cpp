/*************************************************************************
 *                                                                       *
 *                             STRINGS2.CPP                              *
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
		C++ String Class

   PROGRAMMERS


   FUNCTIONS

   TABS : 5 9

   HISTORY
		07/29/96 : Created.

 *************************************************************************/

/**************************** I N C L U D E S ****************************/

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"

#include "echidna/strings.h"

/*************************** C O N S T A N T S ***************************/


/******************************* T Y P E S *******************************/


/************************** P R O T O T Y P E S **************************/


/***************************** G L O B A L S *****************************/


/****************************** M A C R O S ******************************/


/**************************** R O U T I N E S ****************************/

#if (_EL_PLAT_WIN32__ && defined (_AFXDLL))

void ELString::EscapeString (void)
{
	int			length;
	ELString	temp;
	LPCTSTR		dest;

	length = ::EscapeString (NULL, *this);

	dest = temp.GetBuffer (length+1);

	::EscapeString ((char *)dest, *this);

	temp.ReleaseBuffer ();

	*this = temp;
}

void ELString::StripComments (void)
{
	int	 q1 = 0;
	int	 q2 = 0;
	LPCTSTR s = *this;
	int  count = 0;

	/*
	** Remove Comment.
	*/
	while (*s) {
		if (*s == '\'') {
			if (q1) { q1--; } else { q1++; }
		} else if (*s == '\"') {
			if (q2) { q2--; } else { q2++; }
		} else if ((*s == ';') && (!q1) && (!q2)) {
			break;
		}
		count++;
		s++;
	}

	ELString temp;

	temp  = Left(count);
	*this = temp;
}

void ELString::TrimWhitespace (void)
{
	int	 len;
	int	 pos = 0;

	len = GetLength ();
	LPCTSTR	str;
	
	str = *this;
	
	while (len && isspace (str[len - 1]))
	{
		len--;
	}
	while (isspace (str[pos]))
	{
		pos++;
	}
	
	ELString temp;

	temp  = Mid(pos, len - pos);
	*this = temp;
}

void ELString::UnEscapeString (void)
{
	int			length;
	ELString	temp;
	LPCTSTR		dest;

	length = ::UnEscapeString (NULL, *this);

	dest = temp.GetBuffer (length+1);

	::UnEscapeString ((char *)dest, *this);

	temp.ReleaseBuffer ();

	*this = temp;
}

BOOL ELString::GetFirstArg (ELString& arg)
{
	int	 c;
	BOOL quote = FALSE;
	BOOL esc   = FALSE;
	BOOL start = FALSE;
	int  pos   = 0;
	int  first = -1;
	int  last  = -1;
	LPCTSTR	line;

	line = *this;

	while ((c = *line) != '\0') {
		if (quote && !start) {
			start = TRUE;
			first = pos;
		}
		if (!start && isspace(c)) {
		} else if ((!quote && (isspace (c) || c == ';')) || (quote && !esc && c == '"')) {
			last = pos;
			break;
		} else if (start && quote && !esc && c == '\\') {
			esc = TRUE;
		} else if (!quote && c == '"') {
			quote = TRUE;
		} else {
			if (!start) {
				first = pos;
				start = TRUE;
			}
			esc = FALSE;
		}
		line++;
		pos++;
	}
	if (last < 0)
	{
		last = pos;
	}

	if (start)
	{
		arg = Mid(first, last - first);
		ELString temp;

		if (pos < GetLength())
		{
			pos++;
		}
		temp  = Mid(pos);
		*this = temp;

	}

	return start;
}

/*************************************************************************
                           ELString::GetLine                            
 *************************************************************************

   SYNOPSIS
		int ELString::GetLine (ELString& line)

   PURPOSE
  		Get one line from the current string and place it in 'line'.
		Remove that line from the string.  Skip blank lines (strip comments
		and whitespace) and return the number of lines used
		(ie. if we skip 3 lines and return the 4th line then return 4).
		If we are out of lines return 0.
		
		Concat lines that end in '\' just like C's preprocessor
  
   INPUT
		g :
		e :
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   RETURNS
  
  
   SEE ALSO
  
  
   HISTORY
		10/31/96 : Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int ELString::GetLine (ELString& line)
{
	int	lines = 0;
	
	line = "";
	
	if (GetLength () > 0)
	{
		LPCTSTR	s;
		int		len = 0;
		int	c;
		
		s = *this;
		do
		{
			c = *s++;
			if (c == '\0' || c == '\n')
			{
				ELString	temp;
				
				lines++;
				
				// copy data to line
				line += Left(len - (c == '\n'));
				
				// remove data from string
				if (len < GetLength())
				{
					len++;
				}
				temp = Mid(len);
				*this = temp;
				s = *this;
				
				// check if line has anything in it
				len = line.GetLength();
				if (len > 0)
				{
					if (line.GetAt(len - 1) == '\\')
					{
						line = line.Left(len - 1);
					}
					else
					{
						line.StripComments ();
						line.TrimWhitespace ();
						if (line.GetLength() > 0)
						{
							return lines;
						}
					}
				}
				len = 0;
				
			}
			else
			{
				len++;
			}
		}
		while (c != 0);
	}

	line.StripComments ();
	line.TrimWhitespace ();
	if (line.GetLength())
	{
		return lines;
	}
	
	return 0;
}

int ELString::NumArgs (void)
{
	int	 c;
	int	 count   = 0;
	BOOL quote   = FALSE;
	BOOL esc     = FALSE;
	BOOL start   = FALSE;
	LPCTSTR	line = *this;

	while ((c = *line) != '\0') {
		if (quote && !start) {
			start = TRUE;
			count++;
		}
		if ((!quote && (isspace (c) || c == ';')) || (quote && !esc && c == '"')) {
			if (c == ';') {
				break;
			}
			quote = FALSE;
			start = FALSE;
		} else if (start && quote && !esc && c == '\\') {
			esc = TRUE;
		} else if (!quote && c == '"') {
			quote = TRUE;
		} else {
			if (!start) {
				count++;
				start = TRUE;
			}
			esc = FALSE;
		}
		line++;
	}
	return count;
}

void ELString::ReplaceChar (TCHAR from, TCHAR to)
{
	int	i;

	for (i = 0; i < GetLength(); i++)
	{
		if (GetAt (i) == from)
		{
			SetAt (i, to);
		}
	}
}

BOOL ELString::GetInt (int& value)
{
	ELString	arg;

	value = 0;

	if (!GetArg(arg))
	{
		return FALSE;
	}
	value = EL_atol(arg);

	return TRUE;
}

BOOL ELString::GetLong (long& value)
{
	ELString	arg;

	value = 0;

	if (!GetArg(arg))
	{
		return FALSE;
	}
	value = EL_atol(arg);

	return TRUE;
}

typedef struct {
	char	*s;
	BOOL	 val;
} BoolEntry;

static BoolEntry BoolTable[] = {
{	"0",		FALSE,	},
{	"FALSE",	FALSE,	},
{	"No",		FALSE,	},
{	"N",		FALSE,	},
{	"F",		FALSE,	},
{	"Off",		FALSE,	},
{	"1",		TRUE,	},
{	"TRUE",		TRUE,	},
{	"Yes",		TRUE,	},
{	"T",		TRUE,	},
{	"Y",		TRUE,	},
{	"On",		TRUE,	},
{	NULL,		FALSE,  },
};


BOOL ELString::GetBool (BOOL& value)
{
	ELString	arg;

	value = FALSE;

	if (!GetArg(arg))
	{
		return FALSE;
	}

	{
		BoolEntry	*be;

		be = BoolTable;
		while (be->s)
		{
			if (!arg.CompareNoCase (be->s))
			{
				value = be->val;
				return TRUE;
			}
			be++;
		}
	}

	value = EL_atol(arg);

	return TRUE;
}

BOOL ELString::GetUInt32 (uint32& value)
{
	ELString	arg;

	value = 0;

	if (!GetArg(arg))
	{
		return FALSE;
	}
	value = EL_atol(arg);

	return TRUE;
}

#endif
