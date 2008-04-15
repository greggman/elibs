/*
 * maclang.cpp
 *
 * PROGRAMMER : Gregg A. Tavares
 *    VERSION : 00.000
 *    CREATED : 01/15/01
 *       TABS : 05 09
 *
 *	     \|///-_
 *	     \oo///_
 *	-----w/-w------
 *	 E C H I D N A
 *	---------------
 *
 *		Copyright (c) 2001-2008, Echidna
 *
 *		All rights reserved.
 *
 *		Redistribution and use in source and binary forms, with or
 *		without modification, are permitted provided that the following
 *		conditions are met:
 *
 *		* Redistributions of source code must retain the above copyright
 *		  notice, this list of conditions and the following disclaimer. 
 *		* Redistributions in binary form must reproduce the above copyright
 *		  notice, this list of conditions and the following disclaimer
 *		  in the documentation and/or other materials provided with the
 *		  distribution. 
 *
 *		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 *		CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
 *		INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *		MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *		DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 *		BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *		EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *		TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *		DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 *		ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *		OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *		OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *		POSSIBILITY OF SUCH DAMAGE.
 *
 * DESCRIPTION
 *		
 *
 * HISTORY
 *
*/

/**************************** i n c l u d e s ****************************/

#pragma warning(disable : 4786) // identifier was truncated to '255' characters in the browser information

#include "platform.h"
#include "switches.h"
#include "echidna/ensure.h"
#include "echidna/strings.h"
#include "echidna/maclang.h"
#include "echidna/eerrors.h"
#include "echidna/eio.h"
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>
#include <map>
#include <list>

#include <boost/shared_ptr.hpp>

using std::string;
using std::vector;
using std::map;
using std::list;

/*************************** c o n s t a n t s ***************************/


/******************************* t y p e s *******************************/

class NoCaseStringCmp
{
public:
    bool operator() (const string& s1, const string& s2) const
    {
        return stricmp(s1.c_str(), s2.c_str()) < 0;
    }
};

typedef list<string> StringList;

class MLangFunc
{
public:
	const	string	m_name;
	const	string	m_body;
	StringList	 	m_namedArgList;
	int				m_minArgs;
	int				m_maxArgs; // (-1) = unlimited
	
	MLangFunc(const string& name, const string& body) :
		m_name(name),
		m_body(body),
		m_minArgs(0),
		m_maxArgs(0)
		{ };
};

typedef map<string, string, NoCaseStringCmp> VariableMap;
typedef boost::shared_ptr<MLangFunc> SPMF;
typedef map<string, SPMF, NoCaseStringCmp> FunctionMap;

/************************** p r o t o t y p e s **************************/


/***************************** g l o b a l s *****************************/

VariableMap Variables;
FunctionMap Functions;

/****************************** m a c r o s ******************************/


/**************************** r o u t i n e s ****************************/

extern "C" void MLANG_AddVariable (const char* name, const char* value)
{
    Variables[name] = value;
}

extern "C" void MLANG_RemoveVariable (const char* name)
{
    Variables.erase(name);
}

// skip leading whitespace
static long mlang_atol (const char* str)
{
	while (*str && isspace (*str))
	{
		str++;
	}
	
	return EL_atol(str);
}

static double mlang_atof (const char* str)
{
	while (*str && isspace (*str))
	{
		str++;
	}
	
	return atof(str);
}

static bool mlang_isfloat (const char* str)
{
    int before = 0;
	while (*str && isspace (*str))
	{
		str++;
	}
	
	if (*str == '-' || *str == '+')
	{
		str++;
	}

    while (*str && isdigit (*str))
    {
        before++;
        str++;
    }

    if (!*str || *str != '.' || !before)
    {
        return false;
    }

    str++;

    if (*str && isdigit(*str))
    {
        return true;
    }

    return false;
}

static double AddFloats  (double a, double b) { return a + b; }
static int    AddInts    (int a, int b)       { return a + b; }
static double SubFloats  (double a, double b) { return a - b; }
static int    SubInts    (int a, int b)       { return a - b; }
static double MultFloats (double a, double b) { return a * b; }
static int    MultInts   (int a, int b)       { return a * b; }
static double DivFloats  (double a, double b) { return a / b; }
static int    DivInts    (int a, int b)       { return a / b; }
static double ModFloats  (double a, double b) { return fmod(a, b); }
static int    ModInts    (int a, int b)       { return a % b; }

typedef int (*intfunc)(int a, int b);
typedef double (*floatfunc)(double a, double b);
typedef vector<string> mlangArgList;

class MLangContext
{
	MLangContext();
public:
	const string	m_name;
	const char*	  	m_inputPath;
	VariableMap   	m_localVariables;
	StringList		m_funcArgs;
	bool			m_fReturn;
	bool			m_fBreak;
	
	MLangContext(const char* name, const char* inputPath) :
		m_name(name),
		m_inputPath(m_inputPath),
		m_fReturn(false),
		m_fBreak(false)
		{ };
};
bool MLANG_SubVariablesLocal (MLangContext& context, string& str);

const char* hack_sprintf (const char* fmt, ...)
{
    static char buf[100];

	va_list ap;	/* points to each unnamed arg in turn */
	va_start (ap, fmt);
    vsprintf (buf, fmt, ap);
	va_end (ap);	/* clean up when done */

    return buf;
}

static bool DoMath (string& localSub, const mlangArgList& argList, intfunc ifunc, floatfunc ffunc)
{
    // this should probably be some fancy C++ thing but I'm hacking so poo!
    bool   fFloat  = false;
    int    fIvalue = 0;
    double fFvalue = 0.0;

    if (argList.size() == 0)
    {
        localSub = "0";
    }
    else
    {
        mlangArgList::const_iterator it = argList.begin();

        {
    		string value = *it;
            if (mlang_isfloat (value.c_str()))
            {
                fFloat  = true;
        		fFvalue = mlang_atof (value.c_str());
            }
            else
            {
        		fIvalue = mlang_atol (value.c_str());
            }
        }

        ++it;
        while (it != argList.end())
        {
            string value = *it;

            if (mlang_isfloat (value.c_str()))
            {
                if (fFloat)
                {
                    fFvalue = ffunc(fFvalue, mlang_atof (value.c_str()));
                }
                else
                {
                    fFloat   = true;
                    fFvalue  = ffunc((double)fIvalue, mlang_atof (value.c_str()));
                }
            }
            else
            {
                if (fFloat)
                {
                    fFvalue  = ffunc(fFvalue, (double)mlang_atol (value.c_str()));
                }
                else
                {
                    fIvalue  = ifunc(fIvalue, mlang_atol (value.c_str()));
                }
            }
            ++it;
        }

        if (fFloat)
        {
            localSub = hack_sprintf("%f", fFvalue);
        }
        else
        {
            localSub = hack_sprintf ("%d", fIvalue);
        }
    }
	return true;
}

/********************************* fnop **********************************/
bool mc_fnop (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	localSub = "";
	return true;
}

/******************************** fquote *********************************/
bool mc_fquote (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	for (mlangArgList::const_iterator it = argList.begin(); it != argList.end(); ++it)
	{
		localSub += *it;
	}
	return true;
}

/********************************* fset **********************************/
bool mc_fset (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	if (argList.size() == 1)
	{
		MLANG_AddVariable (argList[0].c_str(), "");
	}
	else if (argList.size() > 1)
	{
		MLANG_AddVariable (argList[0].c_str(), argList[1].c_str());
	}
	localSub  = "";
	return true;
}

/********************************* fadd **********************************/
bool mc_fadd (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	return DoMath (localSub, argList, AddInts, AddFloats);
}

/******************************* fsubtract *******************************/
bool mc_fsubtract (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	return DoMath (localSub, argList, SubInts, SubFloats);
}

/******************************** fdivide ********************************/
bool mc_fdivide (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	return DoMath (localSub, argList, DivInts, DivFloats);
}

/******************************* fmultiply *******************************/
bool mc_fmultiply (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	return DoMath (localSub, argList, MultInts, MultFloats);
}

/******************************** fmodulo ********************************/
bool mc_fmodulo (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	return DoMath (localSub, argList, ModInts, ModFloats);
}

/***************************** fconcatenate ******************************/
bool mc_fconcatenate  (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	for (mlangArgList::const_iterator it = argList.begin (); it != argList.end(); ++it)
	{
		localSub += *it;
	}
	return true;
}

/******************************** fsubstr ********************************/
bool mc_fsubstr (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	if (argList.size() == 2)
	{
		int index = mlang_atol (argList[1].c_str());
		
		localSub = argList[0].substr(index);
	}
	if (argList.size() == 3)
	{
		int index = mlang_atol (argList[1].c_str());
		int len   = mlang_atol (argList[2].c_str());
		
		localSub = argList[0].substr(index,len);
	}
	return true;
}
					
/******************************** fright *********************************/
bool mc_fright (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	int len = mlang_atol (argList[1].c_str());
	
	localSub = argList[0].substr(argList[0].size() - len);
	
	return true;
}
					
/********************************* fleft *********************************/
bool mc_fleft (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	int len = mlang_atol (argList[1].c_str());
	
	localSub = argList[0].substr(0,len);
	
	return true;
}

/******************************** fstrlen ********************************/
bool mc_fstrlen (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	string::size_type fvalue = 0;
	
	for (mlangArgList::const_iterator it = argList.begin(); it != argList.end(); ++it)
	{
		fvalue += (*it).size();
	}
	localSub = hack_sprintf ("%d", fvalue);
	return true;
}

/********************************** fif **********************************/
bool mc_fif (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	string arg(argList[0]);
	MLANG_SubVariablesLocal (context, arg);

	if (mlang_isfloat(arg.c_str()) ? mlang_atof(arg.c_str()) : mlang_atol (arg.c_str()))
	{
		localSub = argList[1];
		MLANG_SubVariablesLocal (context, localSub);
	}
	else
	{
		if (argList.size() == 3)
		{
			localSub = argList[2];
			MLANG_SubVariablesLocal (context, localSub);
		}
	}
	return true;
}
					
/******************************** fequal *********************************/
bool mc_fequal (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	double v1 = mlang_atof (argList[0].c_str());
	double v2 = mlang_atof (argList[1].c_str());

	localSub = hack_sprintf("%d", v1 == v2);
	
	return true;
}

/***************************** fgreaterthan ******************************/
bool mc_fgreaterthan (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	double v1 = mlang_atof (argList[0].c_str());
	double v2 = mlang_atof (argList[1].c_str());
	
	localSub = hack_sprintf("%d", v1 > v2);
	return true;
}

/*************************** fgreaterthanequal ***************************/
bool mc_fgreaterthanequal (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	double v1 = mlang_atof (argList[0].c_str());
	double v2 = mlang_atof (argList[1].c_str());
	
	localSub = hack_sprintf("%d", v1 >= v2);
	return true;
}

/******************************* flessthan *******************************/
bool mc_flessthan (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	double v1 = mlang_atof (argList[0].c_str());
	double v2 = mlang_atof (argList[1].c_str());
	
	localSub = hack_sprintf("%d", v1 < v2);
	return true;
}

/**************************** flessthanequal *****************************/
bool mc_flessthanequal (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	double v1 = mlang_atof (argList[0].c_str());
	double v2 = mlang_atof (argList[1].c_str());
	
	localSub = hack_sprintf("%d", v1 <= v2);
	return true;
}

/******************************* fnotequal *******************************/
bool mc_fnotequal (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	double v1 = mlang_atof (argList[0].c_str());
	double v2 = mlang_atof (argList[1].c_str());
	
	localSub = hack_sprintf("%d", v1 != v2);
	return true;
}

/********************************* fnot **********************************/
bool mc_fnot (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	double v1 = mlang_atof (argList[0].c_str());
	
	localSub = hack_sprintf("%d", v1 == 0.0);
	
	return true;
}
					
/********************************* fand **********************************/
bool mc_fand (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	int fvalue = true;
	for (mlangArgList::const_iterator it = argList.begin(); it != argList.end() && fvalue; ++it)
	{
		string arg(*it);
		MLANG_SubVariablesLocal (context, arg);
		fvalue = fvalue && (mlang_atof(arg.c_str()) != 0.0);
	}
	
	localSub = hack_sprintf("%d", fvalue);
	return true;
}

/********************************** for **********************************/
bool mc_for (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	int fvalue = false;
	for (mlangArgList::const_iterator it = argList.begin(); it != argList.end() && !fvalue; ++it)
	{
		string arg(*it);
		MLANG_SubVariablesLocal (context, arg);
		fvalue = fvalue || (mlang_atof(arg.c_str()) != 0.0);
	}
	
	localSub = hack_sprintf("%d", fvalue);
	return true;
}

/******************************** frandom ********************************/
bool mc_frandom (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	int rmin = 0;
	int rmax = 0;
	
	if (argList.size() == 1)
	{
		int rmax = mlang_atol (argList[0].c_str());
	}
	else // if (argList.size() == 2)
	{
		int rmin = mlang_atol (argList[0].c_str());
		int rmax = mlang_atol (argList[1].c_str());
	}
	
	int range  = rmax - rmin;
	int fvalue = (rand () * range) / RAND_MAX;
	
	localSub = hack_sprintf("%d", fvalue);
	
	return true;
}

/******************************** fstrcmp ********************************/
bool mc_fstrcmp (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	int fvalue = strcmp (argList[0].c_str(), argList[1].c_str());
	
	localSub = hack_sprintf("%d", fvalue);
	return true;
}

/******************************* fstricmp ********************************/
bool mc_fstricmp (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	int fvalue = stricmp (argList[0].c_str(), argList[1].c_str());
	
	localSub = hack_sprintf("%d", fvalue);
	return true;
}

/******************************** fascii *********************************/
bool mc_fascii (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	int fvalue = argList[0][0];
	
	localSub = hack_sprintf("%d", fvalue);
	return true;
}
					
/********************************* feval *********************************/
bool mc_feval (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	for (mlangArgList::const_iterator it = argList.begin(); it != argList.end(); ++it)
	{
		string etemp = *it;
		MLANG_SubVariablesLocal (context, etemp);
		localSub += etemp;
	}
	return true;
}
					
/******************************** fsearch ********************************/
bool mc_fsearch (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	const char* find = strstr (argList[0].c_str(), argList[1].c_str());

	int fvalue = (find != NULL) ? find - (const char*)argList[0].c_str() : -1;
	
	localSub = hack_sprintf("%d", fvalue);
	return true;
}

/******************************** fformat ********************************/
bool mc_fformat (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	// int    : c,d,i,d,o,u,x,C,X,
	// double : e,E,f,g,G,
	// string : s,S
	// other  : n,p
	string fmt(argList[0]);
	if (argList[0][0] != '%')
	{
		fmt = string("%") + argList[0];
	}
	int typespec = fmt[(fmt.size() - 1)];

	if (strchr ("cCdidouxX", typespec))
	{
		localSub = hack_sprintf(fmt.c_str(), mlang_atol(argList[1].c_str()));
	}
	else if (strchr ("eEfgG", typespec))
	{
		localSub = hack_sprintf(fmt.c_str(), mlang_atof(argList[1].c_str()));
	}
	else
	{
		localSub = hack_sprintf(fmt.c_str(), argList[1].c_str());
	}
	return true;
}

/********************************* fint **********************************/
bool mc_fint (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	int fvalue = mlang_atol(argList[0].c_str());
	
	localSub = hack_sprintf("%d", fvalue);
	return true;
}

/******************************** fexists ********************************/
bool mc_fexists (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	char filename[EIO_MAXPATH];
	int fValue;

	EIO_fnmerge (filename, EIO_Path(context.m_inputPath), argList[0].c_str(), NULL);
	fValue = EIO_FileExists (filename);
	if (!fValue)
	{
		EIO_fnmerge (filename, NULL, argList[0].c_str(), NULL);
		fValue = EIO_FileExists (filename);
	}
	localSub = hack_sprintf("%d", fValue);
	return true;
}

/****************************** ffilelength ******************************/
bool mc_ffilelength (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	char filename[EIO_MAXPATH];
	bool fFound = FALSE;

	EIO_fnmerge (filename, EIO_Path(context.m_inputPath), argList[0].c_str(), NULL);
	fFound = EIO_FileExists (filename) != 0;
	if (!fFound)
	{
		EIO_fnmerge (filename, NULL, argList[0].c_str(), NULL);
		if (!EIO_FileExists (filename))
		{
			SetGlobalErr (ERR_GENERIC);
			GEcatf1 ("couldn't find file (%s) for ffilelength", argList[0].c_str());
			return false;
		}
	}

	int fh = EIO_ReadOpen (filename);
	long len = -1;
	if (fh < 0)
	{
		SetGlobalErr (ERR_GENERIC);
		GEcatf1 ("couldn't open file (%s) for ffilelength", filename);
		return false;
	}
	len = EIO_FileLength (fh);
	EIO_Close (fh);
	localSub = hack_sprintf("%d", len);
	return true;
}

/******************************** fdeffunc *******************************/
bool mc_fdeffunc (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	mlangArgList::const_iterator it = argList.begin();
	string name = *it;
	++it;

	MLangFunc* pFunc = new MLangFunc(name, *argList.rbegin());
	Functions[name] = SPMF(pFunc);

	for (; it != argList.end(); ++it)
	{
		mlangArgList::const_iterator next = it;
		++next;
		
		string arg = *it;
		// check for last arg
		if (next == argList.end())
		{
//			pFunc->m_body = arg;
		}
		// check for ...
		else if (!arg.compare("..."))
		{
			pFunc->m_maxArgs = -1;
		}
		// check for optional argument
		else if (arg.size() > 0 && arg[0] == '[' && arg[arg.size() - 1] == ']')
		{
			pFunc->m_namedArgList.push_back(arg.substr(1, arg.size() - 2));
			pFunc->m_maxArgs++;
		}
		// nornal argument
		else
		{
			pFunc->m_namedArgList.push_back(arg);
			pFunc->m_minArgs++;
			pFunc->m_maxArgs++;
		}
	}
	
	return true;
}

/******************************* fsetlocal *******************************/
bool mc_fsetlocal (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	if (argList.size() == 1)
	{
		context.m_localVariables[argList[0]] = "";
	}
	else if (argList.size() > 1)
	{
		context.m_localVariables[argList[0]] = argList[1];
	}
	localSub  = "";
	return true;
}

/******************************** return *********************************/
bool mc_freturn (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	context.m_fReturn = true;
	localSub = argList[0];
	return true;
}

/******************************** return *********************************/
bool mc_fshift (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	if (!context.m_funcArgs.empty())
	{
		localSub = *context.m_funcArgs.begin();
		context.m_funcArgs.pop_front();
	}
	else
	{
		SetGlobalErr (ERR_GENERIC);
		GEcatf1 ("error shifted one too many arguments in (%s)", context.m_name.c_str());
		return false;
	}
	return true;
}

/******************************** return *********************************/
bool mc_fnumargs (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	localSub = hack_sprintf("%d", context.m_funcArgs.size());
	return true;
}

/********************************* ffor **********************************/
bool mc_ffor (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	string arg(argList[0]);
	MLANG_SubVariablesLocal (context, arg);
	
	for (;;)
	{
		arg = argList[1];
		MLANG_SubVariablesLocal (context, arg);
		
		if (mlang_isfloat(arg.c_str()) ? mlang_atof(arg.c_str()) : mlang_atol (arg.c_str()))
		{
			arg = argList[3];
			MLANG_SubVariablesLocal (context, arg);
		}
		else
		{
			break;
		}
		
		if (context.m_fBreak)
		{
			break;
		}
		
		arg = argList[2];
		MLANG_SubVariablesLocal (context, arg);
	}
	
	context.m_fBreak = false;
	
	return true;
}

/********************************* while *********************************/
bool mc_fwhile (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	for (;;)
	{
		string arg(argList[0]);
		MLANG_SubVariablesLocal (context, arg);
	
		if (mlang_isfloat(arg.c_str()) ? mlang_atof(arg.c_str()) : mlang_atol (arg.c_str()))
		{
			arg = argList[1];
			MLANG_SubVariablesLocal (context, arg);
		}
		else
		{
			break;
		}
		
		if (context.m_fBreak)
		{
			break;
		}
	}
	context.m_fBreak = false;
	return true;
}

/********************************* break *********************************/
bool mc_fbreak (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	context.m_fBreak = true;
	return true;
}

/********************************* break *********************************/
bool mc_fprint (MLangContext& context, string& localSub, const mlangArgList &argList)
{
	for (mlangArgList::const_iterator it = argList.begin(); it != argList.end(); ++it)
	{
		EL_printf ((*it).c_str());
	}
	EL_printf ("\n");
	return true;
}

/***********************************  ************************************/
/***************************** command table *****************************/
/***********************************  ************************************/

typedef bool (*mc_command_func)(MLangContext& context, string& localSub, const mlangArgList &argList);

struct mc_command_info
{
	const char*		m_id;
	bool			m_fPostExpand;
	int				m_minargs;
	int				m_maxargs;
	mc_command_func	m_func;
};

mc_command_info mc_command_table[] =
{
{ "fnop",				true,  0, -1, mc_fnop,               }, // fnop (...)
{ "fset",				false, 1,  2, mc_fset,               }, // fset (var,value) ... (var)
{ "fadd",				false, 2, -1, mc_fadd,               }, // fadd (value,value,...)
{ "fsubtract",			false, 2, -1, mc_fsubtract,          }, // fsubtract (value,value,...)
{ "fsub",				false, 2, -1, mc_fsubtract,          }, // fsub (value,value,...)
{ "fdivide",			false, 2, -1, mc_fdivide,            }, // fdivide (value,value,...)
{ "fdiv",				false, 2, -1, mc_fdivide,            }, // fdiv (value,value,...)
{ "fmultiply",			false, 2, -1, mc_fmultiply,          }, // fmultiply (value,value,...)
{ "fmul",				false, 2, -1, mc_fmultiply,          }, // fmul (value,value,...)
{ "fmult",				false, 2, -1, mc_fmultiply,          }, // fmult (value,value,...)
{ "fmodulo",			false, 2, -1, mc_fmodulo,            }, // fmodulo (value,value,...)
{ "fmod",				false, 2, -1, mc_fmodulo,            }, // fmod (value,value,...)
{ "fconcatenate",		false, 2, -1, mc_fconcatenate,       }, // fconcatenate (...)
{ "fconcate",			false, 2, -1, mc_fconcatenate,       }, // fconcate (...)
{ "fcat",				false, 2, -1, mc_fconcatenate,       }, // fcat (value,value,...)
{ "fsubstr",			false, 2,  3, mc_fsubstr,            }, // fsubstr (str,index,len) (str,index)
{ "fleft",				false, 2,  2, mc_fleft,              }, // fleft (str,count)
{ "fright",				false, 2,  2, mc_fright,             }, // fright (str,count)
{ "fstrlen",			false, 0, -1, mc_fstrlen,            }, // fstrlen (value,...)
{ "fif",				true,  2,  3, mc_fif,                }, // fif (value,truecase[,falsecase])
{ "fequal",				false, 2,  2, mc_fequal,             }, // fequal (value,value)
{ "feq",				false, 2,  2, mc_fequal,             }, // feq (value,value)
{ "fgreaterthan",		false, 2,  2, mc_fgreaterthan,       }, // fgreaterthan (value,value)
{ "fgt",				false, 2,  2, mc_fgreaterthan,       }, // fgt (value,value)
{ "fgreaterthanequal",	false, 2,  2, mc_fgreaterthanequal,  }, // fgreaterthanequal (value,value)
{ "fge",				false, 2,  2, mc_fgreaterthanequal,  }, // fge (value,value)
{ "flessthan",			false, 2,  2, mc_flessthan,          }, // flessthan (value,value)
{ "flt",				false, 2,  2, mc_flessthan,          }, // flt (value,value)
{ "flessthanequal",		false, 2,  2, mc_flessthanequal,     }, // flessthanequal (value,value)
{ "fle",				false, 2,  2, mc_flessthanequal,     }, // fle (value,value)
{ "fnotequal",			false, 2,  2, mc_fnotequal,          }, // fnotequal (value,value)
{ "fne",				false, 2,  2, mc_fnotequal,          }, // fne (value,value)
{ "for",				true,  1, -1, mc_for,                }, // for (value,value,...)
{ "fand",				true,  1, -1, mc_fand,               }, // fand (value,value,...)
{ "fnot",				false, 1,  1, mc_fnot,               }, // fnot (value)
{ "frnd",				false, 1,  2, mc_frandom,            }, // frand (range) ... (min,max)
{ "frand",				false, 1,  2, mc_frandom,            }, // frand (range) ... (min,max)
{ "frandom",			false, 1,  2, mc_frandom,            }, // frand (range) ... (min,max)
{ "fstrcmp",			false, 2,  2, mc_fstrcmp,            }, // fstrcmp (value,value)
{ "fstricmp",			false, 2,  2, mc_fstricmp,           }, // fstricmp (value,value)
{ "fquote",				true,  0, -1, mc_fquote,             }, // fquote (value)
{ "fascii",				false, 1,  1, mc_fascii,             }, // fascii (value)
{ "feval",				false, 0, -1, mc_feval,              }, // feval (value)
{ "fsearch",			false, 2,  2, mc_fsearch,            }, // fsearch (string1,string2)    : find string2 in string1
{ "fformat",			false, 2,  2, mc_fformat,            }, // fformat (string,arg)
{ "fint",				false, 1,  1, mc_fint,               }, // fint (arg)
{ "fexists",			false, 1,  1, mc_fexists,            }, // fexists (arg)
{ "ffilelength",		false, 1,  1, mc_ffilelength,        }, // ffilelength (arg)
{ "fdeffunc",			true,  1, -1, mc_fdeffunc,           }, // fdeffunc (name, arg1, ..., body)
{ "fsetlocal",			false, 1,  2, mc_fsetlocal,          }, // fsetlocal (var, value)
{ "freturn",			false, 1,  1, mc_freturn,            }, // freturn (value)
{ "fshift",				false, 0,  1, mc_fshift,             }, // fshift (num)
{ "fnumargs",			false, 0,  0, mc_fnumargs,           }, // fnumargs ()
{ "ffor",               true,  4,  4, mc_ffor,               }, // ffor(init,condition,inc,code)
{ "fwhile",             true,  2,  2, mc_fwhile,             }, // fwhile(condition,loop)
{ "fbreak",             false, 0,  0, mc_fbreak,             }, // fbreak()
{ "fprint",             false, 0, -1, mc_fprint,             }, // fprint(...)
{ NULL, },
};

typedef map<string, mc_command_info*, NoCaseStringCmp> CommandMap;

CommandMap g_CommandTable;

void MLANG_InitCommands (void)
{
	mc_command_info* pCInfo = mc_command_table;
	
	while (pCInfo->m_id)
	{
		g_CommandTable[pCInfo->m_id] = pCInfo;		
		++pCInfo;
	}
}

bool MLANG_GetVariable (const string& var, string& value)
{
    VariableMap::const_iterator it = Variables.find(var);
    if (it != Variables.end())
    {
        value = it->second;
        return true;
    }
    return false;
}

// %var() followed by optional %
// substitue all vars
bool MLANG_SubVariablesLocal (MLangContext& context, string& str)
{
    // do a quick check, is there any possibility we might sub somthing
    if (str.find ('%') == string::npos)
    {
        return true;
    }
	
	{
		static bool fInit = true;
		
		if (fInit)
		{
			fInit = false;
			MLANG_InitCommands ();
		}
	}

	string	work;	// we'll build the new string here
	
	// walk through the old string
	const char *s = str.c_str();
	const char *last = s;
	
	if (s)
	{
		while (*s)
		{
			if (*s == '%')
			{
				bool fMath = false;
				int value = 0;
				const char* id;
				string localSub;
				
				// copy from the last place upto current
				if (last != s)
				{
					string temp(last, s - last);
					work += temp;
					last = s;
				}
				
				// find end of this var
				s++;	// skip %
				
				id = s;
				
				while (*s && (isalnum(*s) || *s == '_'))
				{
					s++;
				}
				
				string idstr(id, s - id);

                // skip whitespace
                while (*s && isspace(*s))
                {
                    s++;
                }
				
				// is there an arg?
				// is there some math to do
				if (*s == '(')
				{
					mlangArgList argList;
					
					const char *argStart;
					bool fquote = false;

					mc_command_info* pCInfo = NULL;
					CommandMap::const_iterator it = g_CommandTable.find(idstr);
					if (it != g_CommandTable.end())
					{
						pCInfo = it->second;
						
						if (pCInfo->m_fPostExpand)
						{
							fquote = true;
						}
					}
					
					s++;
					argStart = s;
					
					// search for closing ")" but skip all escaped / enclosed stuff
					{
						int parenLevel = 0;
						
						while (*s && !(*s == ')' && !parenLevel))
						{
							int c = *s;
							
							if (c == '(')
							{
								parenLevel++;
							}
							else if (c == ')')
							{
								parenLevel--;
							}
							else if (c == ',' && parenLevel == 0)
							{
								// start new argument
								string arg(argStart, s - argStart);
								
								// recurse the args
								if (!fquote)
								{
									MLANG_SubVariablesLocal (context, arg);
								}
								
								argList.push_back (arg);
								
								argStart = s + 1;
							}
							s++;
						}
					}
					
					// add last arg
					{
						string arg(argStart, s - argStart);

						if (!fquote)
						{
							MLANG_SubVariablesLocal (context, arg);
						}
						argList.push_back (arg);

					}
					
					int numArgs = argList.size();
					
					// advance past last ')'
					s++;
					
					// check builtin commands
					if (pCInfo)
					{
						if (numArgs < pCInfo->m_minargs)
						{
							SetGlobalErr (ERR_GENERIC);
							GEcatf3 ("not enough arguments for macro command (%s). %d given %d need", idstr.c_str(), numArgs, pCInfo->m_minargs);
							return false;
						}
						else if (pCInfo->m_maxargs >= 0 && numArgs > pCInfo->m_maxargs)
						{
							SetGlobalErr (ERR_GENERIC);
							GEcatf3 ("too many arguments for macro command (%s). %d given %d need", idstr.c_str(), numArgs, pCInfo->m_maxargs);
							return false;
						}
						else
						{
							if (!pCInfo->m_func (context, localSub, argList))
							{
								return false;
							}
							
							// should we return?
							if (context.m_fReturn || context.m_fBreak)
							{
								str = localSub;
								return true;
							}
						}
					}
                    else
                    {
						// check functions
						FunctionMap::const_iterator fit = Functions.find(idstr);
						if (fit != Functions.end())
						{
							MLangFunc* pFunc = &(*fit->second);
							
							if (numArgs < pFunc->m_minArgs)
							{
								SetGlobalErr (ERR_GENERIC);
								GEcatf3 ("not enough arguments for function (%s). %d given %d need", idstr.c_str(), numArgs, pFunc->m_minArgs);
								return false;
							}
							else if (pFunc->m_maxArgs >= 0 && numArgs > pFunc->m_maxArgs)
							{
								SetGlobalErr (ERR_GENERIC);
								GEcatf3 ("too many arguments for function (%s). %d given %d need", idstr.c_str(), numArgs, pFunc->m_maxArgs);
								return false;
							}
							else
							{
								MLangContext funcContext(idstr.c_str(), context.m_inputPath);
								
								// add args to new context;
								StringList::const_iterator paramit = pFunc->m_namedArgList.begin();
								mlangArgList::const_iterator argit;
								for (argit = argList.begin();
									 argit != argList.end() && paramit != pFunc->m_namedArgList.end();
									 ++argit, ++paramit)
								{
									funcContext.m_localVariables[*paramit] = *argit;
								}
								
								// add remain args to func arglist
								for (; argit != argList.end(); ++argit)
								{
									funcContext.m_funcArgs.push_back(*argit);
								}
								
								// "" def remaining params (so we don't get undef errors)
								for (; paramit != pFunc->m_namedArgList.end(); ++paramit)
								{
									funcContext.m_localVariables[*paramit] = "";
								}
								
								localSub = pFunc->m_body;
								
								MLANG_SubVariablesLocal(funcContext, localSub);
							}
						}
						else
						{
							SetGlobalErr (ERR_GENERIC);
							GEcatf1 ("unknown macro command (%s)", idstr.c_str());
							return false;
						}
                    }
                }
				else // check for vars
                {
					// check for local var
					VariableMap::const_iterator it = context.m_localVariables.find(idstr);
					if (it != context.m_localVariables.end())
					{
						localSub = it->second;
					}
					// check global vars
					else if (!MLANG_GetVariable (idstr, localSub))
                    {
                        // if it's not an lang var check environment vars
                        char *sub = getenv (idstr.c_str());
                        if (sub)
                        {
                            localSub = sub;
                        }
                        else
                        {
							SetGlobalErr (ERR_GENERIC);
                            GEcatf1 ("variable (%s) undefined", idstr.c_str());
                            return false;
                        }
                    }
                }

				// skip last % if there is one
				if (*s == '%')
				{
					s++;
				}
				
				work += localSub;
				last = s;
			}
			else
			{
				s++;
			}
		}
		if (last != s)
		{
			string temp(last, s - last);
			work += temp;
		}
	}
	
	// send the string back
	str = work;
	
	return true;
}

bool MLANG_SubVariables (string& str, const char* inputPath)
{
	MLangContext context("**top level**", inputPath);
	
	return MLANG_SubVariablesLocal (context, str);
}

extern "C" char* MLANG_SubVariables (const char* str, const char* inputPath)
{
    string localstr (str);

    if (MLANG_SubVariables(localstr, inputPath))
    {
		char* newstr;
        newstr = dupstr (localstr.c_str());

        return newstr;
    }
    return NULL;
}


