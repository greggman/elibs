/*************************************************************************
 *                                                                       *
 *                               STRINGS.C                               *
 *                                                                       *
 *************************************************************************

                          Copyright 1996 Echidna

   DESCRIPTION


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
#include "echidna/strings.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#if !_EL_PLAT_SONY__
/*------------------------------------------------------------------------*/
/**# MODULE:STRINGS_stristr                                               */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * stristr
 *
 * SYNOPSIS
 *      char *stristr (char *data, char *key)
 *
 * PURPOSE
 *      Find key inside of data.  Similar to the ANSI routine strstr()
 *      but this version is case insensitive.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *      address of first character of key inside data or
 *      NULL if not found.
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *      strstr
 *  
*/
const char *stristr (const char *data, const char *key)
{
    if (*key)
    {
        while (*data)
        {
            const char  *d;
            const char  *k;

            d = data;
            k = key;
            while (toupper(*d) == toupper(*k))
            {
                d++;
                k++;
                if (!*k)
                {
                    return data;
                }
            }
            data++;
        }
    }
    return NULL;

} /* stristr */


/*------------------------------------------------------------------------*/
/**# MODULE:STRINGS_lchinstr                                              */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * lchinstr
 *
 * SYNOPSIS
 *      int lchinstr (char ch, char *str)
 *
 * USAGE
 *      FirstDotPosition = lchinstr ('.', filenamestr);
 *
 * PURPOSE
 *      To find the position of the first occurence from the left of the
 *      character 'ch' in string 'str'.
 *
 * INPUTS
 *      ch      : character to find.
 *      str     : string in which to find character. (may be NULL)
 *
 * RESULTS
 *      If character is in string then returns positon (0..strlen (str)) of
 *      leftmost occurence of character in string. Else returns < 0.
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
int lchinstr (
    char     ch,
    const char  *str
)
{
    int position = -1;
    int i;

    if (!str)
        return -1;
    for (i = 0; *str && (position < 0); i++, str++){
        if (ch == *str)
            position = i;
    }
    return position;
} /* lchinstr */

/*------------------------------------------------------------------------*/
/**# MODULE:STRINGS_rchinstr                                              */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * rchinstr
 *
 * SYNOPSIS
 *      int rchinstr (char ch, char *str)
 *
 * USAGE
 *      LastSlashPosition = rchinstr ('/', filenamestr);
 *
 * PURPOSE
 *      To find the position of the Last occurence from the left of the
 *      character 'ch' in string 'str'.
 *
 * INPUTS
 *      ch      : character to find.
 *      str     : string in which to find character. (may be NULL)
 *
 * RESULTS
 *      If character is in string then returns positon (0..strlen (str)) of
 *      rightmost occurence of character in string. Else returns < 0.
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
int rchinstr (
    char     ch,
    const char  *str
)
{
    int position = -1;
    int i;

    if (!str)
        return -1;
    for (i = 0; *str; i++, str++){
        if (ch == *str)
            position = i;
    }
    return position;
} /* rchinstr */

/*------------------------------------------------------------------------*/
/**# MODULE:STRINGS_rstrchinstr                                           */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * rstrchinstr
 *
 * SYNOPSIS
 *      int rstrchinstr (char *chset, char *str)
 *
 * USAGE
 *      LastPathDelimPosition = rstrchinstr ("\/", path);
 *
 * PURPOSE
 *      To find the position of the last occurence in string 'str' of
 *      any character from string 'chset' except NULL character.
 *
 * INPUTS
 *      chset   : string of characters to find.
 *      str     : string in which to find characters. (may be NULL)
 *
 * RESULTS
 *      If character from 'chset' is in string then returns positon
 *      (0..strlen (str)) of rightmost occurence of characters in string.
 *      Else returns < 0.
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
int rstrchinstr (
    const char  *chset,
    const char  *str
)
{
    int position = -1;
    int i;

    if (!str || !chset)
        return -1;
    for (i = 0; *str; i++, str++){
        if (lchinstr (*str,chset) >= 0)
            position = i;
    }
    return position;
} /* rstrchinstr */

/*------------------------------------------------------------------------*/
/**# MODULE:STRINGS_dupstr                                                */
/*------------------------------------------------------------------------*/

#if (AZTEC_C || LATTICE || _EL_CC_VC__)
char *dupstr (const char *s)
{
    char    *d;

    d = malloc (strlen (s) + 1);
    if (d) {
        strcpy (d, s);
    }
    return (d);
}
#endif

#endif // !_EL_PLAT_SONY__
/*------------------------------------------------------------------------*/
/**# MODULE:STRINGS_stricmp                                               */
/*------------------------------------------------------------------------*/

#if (AZTEC_C || __MACSC__ || _EL_PLAT_SONY__)
int stricmp (const char *s1, const char *s2)
{
    while (*s1 && *s2) {
        if (toupper (*s1) < toupper (*s2)) return -1;
        if (toupper (*s1) > toupper (*s2)) return  1;
        s1++;
        s2++;
    }

    if (*s1) {
        return  1;
    } else if (*s2) {
        return -1;
    }

    return 0;
}
#endif

/*------------------------------------------------------------------------*/
/**# MODULE:STRINGS_strnicmp                                              */
/*------------------------------------------------------------------------*/

#if (AZTEC_C || __MACSC__ || _EL_PLAT_SONY__)
int strnicmp (const char *s1, const char *s2, size_t len)
{
    while (*s1 && *s2 && len) {
        if (toupper (*s1) < toupper (*s2)) return -1;
        if (toupper (*s1) > toupper (*s2)) return  1;
        s1++;
        s2++;
        len--;
    }

    return 0;
}
#endif

/*------------------------------------------------------------------------*/
/**# MODULE:STRINGS_strupr                                                */
/*------------------------------------------------------------------------*/

#if AZTEC_C	|| _EL_CC_SGIC__
char *strupr (char *s)
{
    register char *h = s;

    while (*h) {
        *h = toupper (*h);
        h++;
    }
    return s;
}
#endif

/*------------------------------------------------------------------------*/
/**# MODULE:STRINGS_strlwr                                                */
/*------------------------------------------------------------------------*/

#if AZTEC_C	|| _EL_CC_SGIC__
char *strlwr (char *s)
{
    register char *h = s;

    while (*h) {
        *h = tolower (*h);
        h++;
    }
    return s;
}
#endif

#if !_EL_PLAT_SONY__

/*------------------------------------------------------------------------*/
/**# MODULE:STRINGS_TrimWhiteSpace                                        */
/*------------------------------------------------------------------------*/

/**************************************************************************
 *
 * TrimWhiteSpace 
 *
 * SYNOPSIS
 *      char *TrimWhiteSpace (char *str)
 *
 * PURPOSE
 *      Remove both leading and trailing white space from string. 
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
 * BUGS
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
char *TrimWhiteSpace (char *str)
{
    short    len;

    len = strlen (str);
    while (len && isspace (str[len - 1])) {
        len--;
    }
    str[len] = '\0';
    while (isspace (*str)) {
        str++;
    }
    return str;

} /* TrimWhiteSpace  */

/*------------------------------------------------------------------------*/
/**# MODULE:STRINGS_TrimWhiteSpaceAndQuotes                               */
/*------------------------------------------------------------------------*/

/**************************************************************************
 *
 * TrimWhiteSpaceAndQuotes
 *
 * SYNOPSIS
 *      char *TrimWhiteSpaceAndQuotes (char *str)
 *
 * PURPOSE
 *      Remove both leading and trailing white space from string then
 *      double or single quotes if they exit.
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
 * BUGS
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
char *TrimWhiteSpaceAndQuotes (char *str)
{
	str = TrimWhiteSpace (str);
	
	if (*str == '"' || *str == '\'')
	{
		char q = *str;
		char* s;
		
		str++;
		s = str;
		while (*s && *s != q) s++;
		*s = 0;
	}
    return str;

} /* TrimWhiteSpaceAndQuotes  */

/*------------------------------------------------------------------------*/
/**# MODULE:STRINGS_EscapeString                                          */
/*------------------------------------------------------------------------*/

int EscapeString (char* dst, const char* src)
{
    int count = 0;

    while (*src)
    {
        if (*src == '\r')
        {
            if (dst)    *dst++ = '\\';
            if (dst)    *dst++ = 'r';
            count += 2;
        }
        else if (*src == '\n')
        {
            if (dst)    *dst++ = '\\';
            if (dst)    *dst++ = 'n';
            count += 2;
        }
        else if (*src == '\t')
        {
            if (dst)    *dst++ = '\\';
            if (dst)    *dst++ = 't';
            count += 2;
        }
        else if (*src == '"')
        {
            if (dst)    *dst++ = '\\';
            if (dst)    *dst++ = '"';
            count += 2;
        }
        else if (*src == '\\')
        {
            if (dst)    *dst++ = '\\';
            if (dst)    *dst++ = '\\';
            count += 2;
        }
        else if (*src < 0x20 || *src > 0x7F)
        {
            if (dst)    *dst++ = '\\';
            if (dst)    *dst++ = 'x';
            if (dst)    *dst++ = *src >> 16;
            if (dst)    *dst++ = *src & 0x0F;
            count += 4;
        }
        else
        {
            if (dst)    *dst++ = *src;
            count += 1;
        }

        src++;
    }

    if (dst)    *dst++ = '\0';

    return count;
}

/*------------------------------------------------------------------------*/
/**# MODULE:STRINGS_UnEscStr                                              */
/*------------------------------------------------------------------------*/

/**************************************************************************
 *
 * UnEscStr
 *
 * SYNOPSIS
 *      char *UnEscStr (char *str)
 *
 * PURPOSE
 *      convert esc string to un-escaped string IN PLACE using
 *      'C' style escape sequences (ie '\n' '\t' '\o123'...);
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
 * BUGS
 *
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
char *UnEscStr (char *orig)
{
    UnEscapeString (orig, orig);

    return orig;

} /* UnEscStr */

/*------------------------------------------------------------------------*/
/**# MODULE:STRINGS_UnEscStr                                              */
/*------------------------------------------------------------------------*/

/**************************************************************************
 *
 * UnEscapeString
 *
 * SYNOPSIS
 *      int UnEscapeString (char *dst, const char *src)
 *
 * PURPOSE
 *      convert esc string to un-escaped string using
 *      'C' style escape sequences (ie '\n' '\t' '\o123'...);
 *
 *      NOTE: you can pass NULL for the destination in which
 *      case you'll get the length of the result but nothing
 *      will be stored.
 *
 * INPUT
 *
 *
 * EFFECTS
 *
 *
 * RETURN VALUE
 *      Length in chars of new string (not including '\0' at end)
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
int UnEscapeString (char *dst, const char* src)
{
    int     c;
    int     count = 0;
    char    digit[10];
    short   digits = 0;
    short   esc = FALSE;
    short   hex = FALSE;
    short   oct = FALSE;
    short   valid;

    while (*src) {
        if (oct) {
            valid = (*src >= '0' && *src <= '7');
            if (valid || digits < 3) {
                if (valid) {
                    digit[digits++] = *src++;
                }
                if (!(*src >= '0' && *src <= '7') || digits == 3) {
                    oct = FALSE;
                    digit[digits] = '\0';
                    sscanf (digit, "%o", &c);
                    if (dst) *dst++ = c;
                    count++;
                } else {
                    continue;
                }
            }
        } else if (hex) {
            valid = isxdigit (*src);
            if (valid || digits < 2) {
                if (valid) {
                    digit[digits++] = *src++;
                }
                if (!isxdigit(*src) || digits == 2) {
                    hex = FALSE;
                    digit[digits] = '\0';
                    sscanf (digit, "%x", &c);
                    if (dst) *dst++ = c;
                    count++;
                } else {
                    continue;
                }
            }
        }
        if (!esc && *src == '\\') {
            esc = TRUE;
            src++;
        } else if (esc) {
            esc = FALSE;
              c = -1;
            switch (*src) {
            case 'b': c = '\b'; break;
            case 'f': c = '\f'; break;
            case 'n': c = '\n'; break;
            case 'r': c = '\r'; break;
            case 't': c = '\t'; break;
            case 'v': c = '\v'; break;
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
                      oct = TRUE; digits = 1; digit[0] = *src; break;
            default:  c = *src; break;
            }
            if (c >= 0) {
                if (dst) *dst++ = c;
                count++;
            }
            src++;
        } else {
            if (dst) *dst++ = *src;
            src++;
            count++;
        }
    }
    if (dst) *dst = '\0';

    return count;

} /* UnEscapeString */

/*------------------------------------------------------------------------*/
/**# MODULE:STRINGS_itoa                                                  */
/*------------------------------------------------------------------------*/

#if AZTEC_C
char *itoa (int value, char *string, int radix)
{
    char    *s = string;
    uint16   d;
    uint16   v;
    uint16   w;

    if (radix == 10 && value < 0) {
        *s++ = '-';
        v    = -value;
    } else {
        v    = value;
    }

    d = 1;

    do {
        d = d * radix;
    } while (d <= v);

    d = d / radix;

    do {
        w    = v / d;
        *s++ = w + '0';
        v    = v - w * d;
        d    = d / radix;
    } while (d);

    *s++ = '\0';

    return string;
}
#endif

/*------------------------------------------------------------------------*/
/**# MODULE:STRINGS_ltoa                                                  */
/*------------------------------------------------------------------------*/

#if AZTEC_C
char *ltoa (long value, char *string, int radix)
{
    char    *s = string;
    uint32   d;
    uint32   v;
    uint32   w;

    if (radix == 10 && value < 0) {
        *s++ = '-';
        v    = -value;
    } else {
        v    = value;
    }

    d = 1;

    do {
        d = d * radix;
    } while (d <= v);

    d = d / radix;

    do {
        w    = v / d;
        *s++ = w + '0';
        v    = v - w * d;
        d    = d / radix;
    } while (d);

    *s++ = '\0';

    return string;
}
#endif


/*------------------------------------------------------------------------*/
/**# MODULE:STRINGS_StripComment                                          */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * StripComment
 *
 * SYNOPSIS
 *      void StripComment (char *line)
 *
 * PURPOSE
 *      Remove ';' type comment from a string.
 *      Places a '\0' at the last none quoted ';' in the string.
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
void StripComment (char *s)
{
    int  q1 = 0;
    int  q2 = 0;

    /*
    ** Remove Comment.
    */
    while (*s) {
        if (*s == '\'') {
            if (q1) { q1--; } else { q1++; }
        } else if (*s == '\"') {
            if (q2) { q2--; } else { q2++; }
        } else if ((*s == ';') && (!q1) && (!q2)) {
            *s = '\0';
            break;
        }
        s++;
    }
} /* StripComment */

/*------------------------------------------------------------------------*/
/**# MODULE:STRINGS_repstr                                                */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * repstr
 *
 * SYNOPSIS
 *      short repstr (char **dest, char *src)
 *
 * PURPOSE
 *      Replace String pointed to by the pointer in dest with the
 *      string pointed to by src.
 *  
 *      repstr first checks to see the strings are different.  If they
 *      are different then repstr calls dupstr on the string pointed to
 *      by src.  If the call to dupstr is successful then the string
 *      pointed to the the pointer in dest is freed using freestr
 *      and the dest pointer is pointed to the new string.
 *
 * INPUT
 *      dest    : pointer to pointer to NULL terminated string.
 *      src     : pointer to NULL terminated string.
 *
 * EXAMPLE
 *      #include <echidna/platform.h>
 *      #include "switches.h"
 *  
 *      #include <echidna/strings.h>
 *      #include <stdio.h>
 *  
 *      void main (void) {
 *          char    *s;
 *  
 *          s = dupstr ("gregg");
 *          if (repstr (&s, "richard")) {
 *              printf ("s = '%s'\n", s);
 *          }
 *          
 *      }
 *
 * RETURN VALUE
 *      TRUE = success
 *      FALSE = failure
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *      dupstr, freestr
 *  
*/
short repstr (char **dest, const char *src)
{
    char    *s;

    if (strcmp (*dest, src)) {
        s = dupstr (src);
        if (!s) {
            return FALSE;
        }
        freestr (*dest);
        *dest = s;
    }
    return TRUE;
} /* repstr */

/*------------------------------------------------------------------------*/
/**# MODULE:STRINGS_fstrpcmp                                              */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * fstrpcmp
 *
 * SYNOPSIS
 *      short   fstrpcmp (
 *          char *fpat
 *          char *fstr
 *      )
 *
 * PURPOSE
 *      To compare two file name strings, the first of which may contain
 *      wildcards:
 *          *   Match all characters up to period (.) or end of string.
 *          ?   Match any one character or nothing if at period (.) or
 *              end of string.
 *          .   Match period (.) or end of string.
 *      Using this routine will give you the same matches you get when
 *      using the DOS dir command.
 *
 * INPUT
 *      fpat:   filename string (non-NULL) that may contain wildcards.
 *      fstr:   filename string (non-NULL).
 *      
 * EFFECTS
 *      None.
 *
 * RETURN VALUE
 *      Returns 0 if matched. Returns non-zero if no match.
 *
 * HISTORY
 *
 *      Note: Does the '.' really need to be searched for?  Because
 *      *.* could mean search for anthing that starts with anything
 *      has a '.' and ends with anything which would match MS-DOS files
 *      but it would also allow searches of '*A*' and such.  Of course
 *      then '*' would match any file but I think it already does that
 *      because to list files with no extension you need '*.' at least
 *      for the MS-DOS dir command
 *
 * SEE ALSO
 *
*/
short   fstrpcmp (
    const char *fpat,
    const char *fstr
)
{
    short    str_spanned = 0;
    char    *periodptr;

    for (;;) {
        switch (*fpat) {
        case '*':
            if ((periodptr = strchr (fstr, '.')) != 0) 
                fstr = periodptr + 1;
            else
                fstr = "\0";
            if ((periodptr = strchr (fpat, '.')) != 0)
                fpat = periodptr + 1;
            else
                fpat = "\0";
            continue;
        case '.':
            if (*fstr == '.')
                break;
            if (!*fstr) {
                ++fpat;
                continue;
            } else
                return 2;
        case '?':
            if (*fstr == '.' || !*fstr) {
                ++fpat;
                continue;
            }
            break;
        case '\0':
            return (short)*fstr;
        default:
            if (toupper(*fpat) != toupper(*fstr)) {
                return 3;
            }
        }
        ++fpat;
        ++fstr;
        ++str_spanned;
    }

} /* fstrpcmp */

/*------------------------------------------------------------------------*/
/**# MODULE:STRINGS_memcpysrc                                             */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * memcpysrc
 *
 * SYNOPSIS
 *      void *memcpysrc (void *dst, void *src, size_t len)
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
 *      (void *)src + len
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
void *memcpysrc (void *dst, void *src, size_t len)
{
    memcpy (dst, src, len);
    return (void *)(((char *)src) + len);

} /* memcpysrc */

/*------------------------------------------------------------------------*/
/**# MODULE:STRINGS_memcpydst                                             */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * memcpydst
 *
 * SYNOPSIS
 *      void *memcpydst (void *dst, void *src, size_t len)
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
 *      (void *)dst + len
 *
 * HISTORY
 *
 *
 * SEE ALSO
 *
*/
void *memcpydst (void *dst, void *src, size_t len)
{

    memcpy (dst, src, len);
    return (void *)(((char *)dst) + len);

} /* memcpydst */

#endif // !_EL_PLAT_SONY__

/*------------------------------------------------------------------------*/
/**# MODULE:STRINGS_EL_atos                                             */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * EL_atos
 *
 * SYNOPSIS
 *      short EL_atos (char *s)
 *
 * PURPOSE
 *      returns a value of a string as a short (just like the ANSI
 *      function atoi() but this one handles number that start with
 *      "$", "0x" or end with "h" as hex
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
short EL_atos (const char *s)
{
    const char  *d;
    short    hex = 0;

    while (*s && isspace (*s))
    {
        s++;
    }
    d = s;

    while (*s && (isxdigit(*s) || (s == d && *s == '-')))
    {
        s++;
    }

    if (d[0] == '0' && tolower(d[1]) == 'x')
    {
        hex = 3;
    }
    else if (d[0] == '$')
    {
        hex = 2;
    }
    else if (tolower(*s) == 'h')
    {
        hex = 1;
    }

    if (hex)
    {
        unsigned short  v = 0;

        d += (hex - 1);
        while (isxdigit (*d))
        {
            v = v * 16 + ((*d <= '9') ? (*d - '0') : (toupper (*d) - 'A' + 10));
            d++;
        }
        return v;
    }
    else
    {
        return atoi (d);
    }
} /* EL_atos */

/*------------------------------------------------------------------------*/
/**# MODULE:STRINGS_EL_atol                                             */
/*------------------------------------------------------------------------*/

/*********************************************************************
 *
 * EL_atol
 *
 * SYNOPSIS
 *      long EL_atol (char *s)
 *
 * PURPOSE
 *      returns a value of a string as a long (just like the ANSI
 *      function atol() but this one handles number that start with
 *      "$", "0x" or end with "h" as hex and "!" as 16.16 fixed point
 *
 *      NOTE: if you add new types you must update EL_isnum()
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
long EL_atol (const char *s)
{
    const char  *d;
    int   hex  = 0;
	int	  frac = 0;

    while (*s && isspace (*s))
    {
        s++;
    }
    d = s;

    while (*s && (isxdigit(*s) || (s == d && *s == '-')))
    {
        s++;
    }

    if (d[0] == '0' && tolower(d[1]) == 'x')
    {
        hex = 3;
    }
    else if (d[0] == '$')
    {
        hex = 2;
    }
	else if (d[0] == '!')
	{
		frac = 2;
	}
    else if (tolower(*s) == 'h')
    {
        hex = 1;
    }

    if (hex)
    {
        unsigned long   v = 0;

        d += (hex - 1);
        while (isxdigit (*d))
        {
            v = v * 16 + ((*d <= '9') ? (*d - '0') : (toupper (*d) - 'A' + 10));
            d++;
        }
        return v;
    }
	else if (frac)
	{
		unsigned long v = 0;
		unsigned long wholePart = 0;
		unsigned long fracPart  = 0;
		BOOL neg = FALSE;
		
		d += (frac - 1);
		if (*d == '-')
		{
			neg = TRUE;
			d++;
		}
		wholePart = atoi(d);
		// find '.'
		while (isdigit(*d))
		{
			d++;
		}
		if (*d == '.')
		{
			char fracDigits[5];
			char *f = fracDigits;
			int	 i = 0;
			
			strcpy (fracDigits, "0000");
			
			d++;
			// count digits supplied
			while (i < 4 && isdigit(*d))
			{
				*f++ = *d++;
				i++;
			}
			
			fracPart = atol (fracDigits);
		}
		
		v = (wholePart << 16) + ((fracPart * 0x10000) / 10000);
		if (neg)
		{
			v = -((int32)v);
		}
		return v;
	}
    else
    {
        return atoi (d);
    }
} /* EL_atol */

/*************************************************************************
                                EL_isnum                                
 *************************************************************************

   SYNOPSIS
		BOOL EL_isnum (int c)

   PURPOSE
  		Return all the valid characters that EL_atol works with
  
   INPUT
		c :
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   RETURNS
  
  
   SEE ALSO
  
  
   HISTORY
		07/11/97 GAT: Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

BOOL EL_isnum (int c)
BEGINFUNC (EL_isnum)
{
	RETURN (isdigit(c) || c == '$' || c == '-' || c == '!');
	
} ENDFUNC (EL_isnum)


/*************************************************************************
                                EL_SafeStr                               
 *************************************************************************

   SYNOPSIS
		const char* EL_SafeStr (const char* str)

   PURPOSE
  		
  
   INPUT
		str :
  
   OUTPUT
		None  
  
   EFFECTS
		None  
  
   RETURNS
  
  
   SEE ALSO
  
  
   HISTORY
		06/12/98 GAT: Created.
  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

const char* EL_SafeStr (const char* str)
BEGINFUNC ( EL_SafeStr)
{
	if (!str)
	{
		RETURN "(NULL)";
	}
	RETURN str;
} ENDFUNC ( EL_SafeStr)

/*------------------------------------------------------------------------*/
/**# MODULE:STRINGS_pStrCopy                                              */
/*------------------------------------------------------------------------*/

void pStrCopy(const unsigned char *src,unsigned char *dest)
{
    memmove (dest,src,(long) *src + 1);
}

/*------------------------------------------------------------------------*/
/**# MODULE:STRINGS_pStrAppend                                            */
/*------------------------------------------------------------------------*/

void pStrAppend(const unsigned char * src,unsigned char * dest)
{
    unsigned short newlen;
    newlen = *dest + *src;
    memmove (dest + *dest + 1,src + 1,(int) *src);
    *dest = (unsigned char)newlen;
}

/*------------------------------------------------------------------------*/
/**# MODULE:STRINGS_pCharAppend                                           */
/*------------------------------------------------------------------------*/

void pCharAppend( unsigned char p1,unsigned char *p2)
{
    *p2 += 1;
    *(p2 + *p2) = p1;
}
