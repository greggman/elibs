/*=======================================================================*
 |   file name : mlang.cpp
 |-----------------------------------------------------------------------*
 |   function  : 
 |-----------------------------------------------------------------------*
 |   author    : Gregg Tavares
 *=======================================================================*/
 
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

using std::string;
using std::vector;
using std::map;

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

typedef map<string, string, NoCaseStringCmp> VariableMap;

/************************** p r o t o t y p e s **************************/


/***************************** g l o b a l s *****************************/

VariableMap Variables;

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

// funcs
//    fnop (...)
//    fset (var,value)
//    fadd (value,value,...)
//    fsubtract,fsub (value,value,...)
//    fdivide,fdiv (value,value,...)
//    fmultiply,fmul,fmult (value,value,...)
//    fmodulo,fmod (value,value,...)
//    fconcatenate,fconcate,fcat (value,value,...)
//    fsubstr (str,index,len) (str,index)
//	  fleft(str,count)
//	  fright(str,count)
//    fstrlen (value,...)
//    fif (value,truecase,falsecase)
//    fequal,feq (value,value)
//    fgreaterthan,fgt (value,value)
//    fgreaterthanequal,fge (value,value)
//    flessthan,flt (value,value)
//    flessthanequal,fle (value,value)
//    fnotequal,fne, (value,value)
//    for (value,value,...)
//    fand (value,value,...)
//    fnot(value)
//    frand(range) ... (min,max)
//    fstrcmp(value,value)
//    fstricmp(value,value)
//	  fquote(value)
//	  fascii(value)
//	  feval(value)
//    fsearch(string1,string2)    : find string2 in string1
//    fformat(string,arg)
//    fint(arg)
//    fexists(arg)
//    ffilelength(arg)

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

const char* hack_sprintf (const char* fmt, ...)
{
    static char buf[100];
    
	va_list ap;	/* points to each unnamed arg in turn */
	va_start (ap, fmt);
    vsprintf (buf, fmt, ap);
	va_end (ap);	/* clean up when done */
    
    return buf;
}

static void DoMath (string& localSub, mlangArgList& argList, intfunc ifunc, floatfunc ffunc)
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
bool MLANG_SubVariables (string& str, const char* inputPath)
{
    // do a quick check, is there any possibility we might sub somthing
    if (str.find ('%') == string::npos)
    {
        return true;
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
						
					if (!strcmp(idstr.c_str(), "fquote"))
					{
						fquote = true;
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
									MLANG_SubVariables (arg, inputPath);
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
							MLANG_SubVariables (arg, inputPath);
						}
						argList.push_back (arg);

					}
					
					string arg0(*(argList.begin()));
					int numArgs = argList.size();
					
					// advance past last ')'
					s++;
					
					// yes I know this should be put into some kind of structure but I'm
					// too lazy
					if (!stricmp (idstr.c_str(), "fnop"))
					{
						localSub = "";
					}
					else if (!stricmp (idstr.c_str(), "fquote"))
					{
						for (mlangArgList::const_iterator it = argList.begin(); it != argList.end(); ++it)
						{
							localSub += *it;
						}
					}
					else if (!stricmp (idstr.c_str(), "fset"))
					{
						if (numArgs == 1)
						{
							MLANG_AddVariable (arg0.c_str(), "");
						}
						else if (numArgs > 1)
						{
							MLANG_AddVariable (arg0.c_str(), argList[1].c_str());
						}
						else
						{
							SetGlobalErr (ERR_GENERIC);
                            GEcatf ("wrong number of args for fset");
                            return false;
						}
						localSub  = "";
					}
					else if (!stricmp (idstr.c_str(), "fadd"))
					{
                        DoMath (localSub, argList, AddInts, AddFloats);
					}
					else if (!stricmp (idstr.c_str(), "fsubtract") || !stricmp (idstr.c_str(), "fsub"))
					{
                        DoMath (localSub, argList, SubInts, SubFloats);
					}
					else if (!stricmp (idstr.c_str(), "fdivide") || !stricmp (idstr.c_str(), "fdiv"))
					{
                        DoMath (localSub, argList, DivInts, DivFloats);
					}
					else if (!stricmp (idstr.c_str(), "fmultiply") || !stricmp (idstr.c_str(), "fmult") || !stricmp (idstr.c_str(), "fmul"))
					{
                        DoMath (localSub, argList, MultInts, MultFloats);
					}
					else if (!stricmp (idstr.c_str(), "fmodule") || !stricmp (idstr.c_str(), "fmod"))
					{
                        DoMath (localSub, argList, ModInts, ModFloats);
					}
					else if (!stricmp (idstr.c_str(), "fconcatenate") || !stricmp (idstr.c_str(), "fconcate") || !stricmp (idstr.c_str(), "fcat"))
					{
						for (mlangArgList::const_iterator it = argList.begin (); it != argList.end(); ++it)
						{
							localSub += *it;
						}
					}
					else if (!stricmp (idstr.c_str(), "fsubstr"))
					{
						if (numArgs == 2)
						{
							int index = mlang_atol (argList[1].c_str());
							
							localSub = arg0.substr(index);
						}
						if (numArgs == 3)
						{
							int index = mlang_atol (argList[1].c_str());
							int len   = mlang_atol (argList[2].c_str());
							
							localSub = arg0.substr(index,len);
						}
						else
						{
							// err
						}
					}
					else if (!stricmp (idstr.c_str(), "fright"))
					{
						if (numArgs == 2)
						{
							int len = mlang_atol (argList[1].c_str());
							
							localSub = arg0.substr(arg0.size() - len);
						}
						else
						{
							// err
						}
					}
					else if (!stricmp (idstr.c_str(), "fleft"))
					{
						if (numArgs == 2)
						{
							int len = mlang_atol (argList[1].c_str());
							
							localSub = arg0.substr(0,len);
						}
						else
						{
							// err
						}
					}
					else if (!stricmp (idstr.c_str(), "fstrlen"))
					{
						string::size_type fvalue = 0;
						
                        for (mlangArgList::const_iterator it = argList.begin(); it != argList.end(); ++it)
						{
							fvalue += (*it).size();
						}
						localSub = hack_sprintf ("%d", fvalue);
					}
					else if (!stricmp (idstr.c_str(), "fif"))
					{
						if (numArgs > 1)
						{
							if (mlang_isfloat(arg0.c_str()) ? mlang_atof(arg0.c_str()) : mlang_atol (arg0.c_str()))
							{
								localSub = argList[1];
							}
							else
							{
								if (numArgs == 3)
								{
									localSub = argList[2];
								}
								else if (numArgs > 3)
								{
									// err
								}
							}
						}
						else
						{
							// err
						}
					}
					else if (!stricmp (idstr.c_str(), "fequal") || !stricmp (idstr.c_str(), "feq"))
					{
						if (numArgs == 2)
						{
							double v1 = mlang_atof (arg0.c_str());
							double v2 = mlang_atof (argList[1].c_str());
                        
							localSub = hack_sprintf("%d", v1 == v2);
						}
						else
						{
							// err
						}
					}
					else if (!stricmp (idstr.c_str(), "fgreaterthan") || !stricmp (idstr.c_str(), "fgt"))
					{
						if (numArgs == 2)
						{
							double v1 = mlang_atof (arg0.c_str());
							double v2 = mlang_atof (argList[1].c_str());
							
							localSub = hack_sprintf("%d", v1 > v2);
						}
						else
						{
							// err
						}
					}
					else if (!stricmp (idstr.c_str(), "fgreaterthanequal") || !stricmp (idstr.c_str(), "fge"))
					{
						if (numArgs == 2)
						{
							double v1 = mlang_atof (arg0.c_str());
							double v2 = mlang_atof (argList[1].c_str());
							
							localSub = hack_sprintf("%d", v1 >= v2);
						}
						else
						{
							// err
						}
					}
					else if (!stricmp (idstr.c_str(), "flessthan") || !stricmp (idstr.c_str(), "flt"))
					{
						if (numArgs == 2)
						{
							double v1 = mlang_atof (arg0.c_str());
							double v2 = mlang_atof (argList[1].c_str());
							
							localSub = hack_sprintf("%d", v1 < v2);
						}
						else
						{
							// err
						}
					}
					else if (!stricmp (idstr.c_str(), "flessthanequal") || !stricmp (idstr.c_str(), "fle"))
					{
						if (numArgs == 2)
						{
							double v1 = mlang_atof (arg0.c_str());
							double v2 = mlang_atof (argList[1].c_str());
							
							localSub = hack_sprintf("%d", v1 <= v2);
						}
						else
						{
							// err
						}
					}
					else if (!stricmp (idstr.c_str(), "fnotequal") || !stricmp (idstr.c_str(), "fne"))
					{
						if (numArgs == 2)
						{
							double v1 = mlang_atof (arg0.c_str());
							double v2 = mlang_atof (argList[1].c_str());
							
							localSub = hack_sprintf("%d", v1 != v2);
						}
						else
						{
							// err
						}
					}
					else if (!stricmp (idstr.c_str(), "fnot"))
					{
						if (numArgs == 1)
						{
							double v1 = mlang_atof (arg0.c_str());
							
							localSub = hack_sprintf("%d", v1 != 0.0);
						}
						else
						{
							// err
						}
					}
					else if (!stricmp (idstr.c_str(), "fand"))
					{
						int fvalue = true;
						for (mlangArgList::const_iterator it = argList.begin(); it != argList.end() && fvalue; ++it)
						{
							fvalue = fvalue && (mlang_atof(it->c_str()) != 0.0);
						}
						
						localSub = hack_sprintf("%d", fvalue);
					}
					else if (!stricmp (idstr.c_str(), "for"))
					{
						int fvalue = false;
						for (mlangArgList::const_iterator it = argList.begin(); it != argList.end() && !fvalue; ++it)
						{
							fvalue = fvalue || (mlang_atof(it->c_str()) != 0.0);
						}
						
						localSub = hack_sprintf("%d", fvalue);
					}
					else if (!stricmp (idstr.c_str(), "frandom") || !stricmp (idstr.c_str(), "frand"))
					{
						int rmin = 0;
						int rmax = 0;
						
						if (numArgs == 1)
						{
							int rmax = mlang_atol (arg0.c_str());
						}
						else if (numArgs == 2)
						{
							int rmin = mlang_atol (arg0.c_str());
							int rmax = mlang_atol (argList[1].c_str());
						}
						else
						{
							// err
						}
						
						int range  = rmax - rmin;
						int fvalue = (rand () * range) / RAND_MAX;
						
						localSub = hack_sprintf("%d", fvalue);
					}
					else if (!stricmp (idstr.c_str(), "fstrcmp"))
					{
						if (numArgs == 2)
						{
							int fvalue = strcmp (arg0.c_str(), argList[1].c_str());
							
							localSub = hack_sprintf("%d", fvalue);
						}
						else
						{
							// err
						}
					}
					else if (!stricmp (idstr.c_str(), "fstricmp"))
					{
						if (numArgs == 2)
						{
							int fvalue = stricmp (arg0.c_str(), argList[1].c_str());
							
							localSub = hack_sprintf("%d", fvalue);
						}
						else
						{
							// err
						}
					}
					else if (!stricmp (idstr.c_str(), "fascii"))
					{
						if (numArgs == 1)
						{
							int fvalue = arg0[0];
							
							localSub = hack_sprintf("%d", fvalue);
						}
						else
						{
							// err
						}
					}
					else if (!stricmp (idstr.c_str(), "feval"))
					{
						for (mlangArgList::const_iterator it = argList.begin(); it != argList.end(); ++it)
						{
							string etemp = *it;
							MLANG_SubVariables (etemp, inputPath);
							localSub += etemp;
						}
					}
                    else if (!stricmp (idstr.c_str(), "fsearch"))
                    {
						if (numArgs == 2)
						{
							char* find = strstr (arg0.c_str(), argList[1].c_str());

							int fvalue = (find != NULL) ? find - (const char*)arg0.c_str() : -1;
							
							localSub = hack_sprintf("%d", fvalue);
						}
						else
						{
							SetGlobalErr (ERR_GENERIC);
                            GEcatf ("wrong number of args for fsearch");
                            return false;
						}
                    }
                    else if (!stricmp (idstr.c_str(), "fformat"))
                    {
						if (numArgs == 2)
						{
                            // int    : c,d,i,d,o,u,x,C,X,
                            // double : e,E,f,g,G,
                            // string : s,S
                            // other  : n,p
                            string fmt(arg0);
							if (arg0[0] != '%')
							{
								fmt = string("%") + arg0;
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
						}
						else
						{
							SetGlobalErr (ERR_GENERIC);
                            GEcatf ("wrong number of args for fformat");
                            return false;
						}
                    }
                    else if (!stricmp (idstr.c_str(), "fint"))
                    {
						if (numArgs == 1)
						{
							int fvalue = mlang_atol(arg0.c_str());
							
							localSub = hack_sprintf("%d", fvalue);
						}
						else
						{
							SetGlobalErr (ERR_GENERIC);
                            GEcatf ("wrong number of args for fint");
                            return false;
						}
                    }
					else if (!stricmp (idstr.c_str(), "fexists"))
					{
						if (numArgs == 1)
						{
    						char filename[EIO_MAXPATH];
                            int fValue;
                            
    						EIO_fnmerge (filename, EIO_Path(inputPath), arg0.c_str(), NULL);
                            fValue = EIO_FileExists (filename);
							if (!fValue)
                            {
            					EIO_fnmerge (filename, NULL, arg0.c_str(), NULL);
                                fValue = EIO_FileExists (filename);
                            }
    						localSub = hack_sprintf("%d", fValue);
						}
						else
						{
							SetGlobalErr (ERR_GENERIC);
                            GEcatf ("wrong number of args for fexists");
                            return false;
						}
					}
					else if (!stricmp (idstr.c_str(), "ffilelength"))
					{
						if (numArgs == 1)
						{
    						char filename[EIO_MAXPATH];
                            bool fFound = FALSE;
                            
    						EIO_fnmerge (filename, EIO_Path(inputPath), arg0.c_str(), NULL);
                            fFound = EIO_FileExists (filename) != 0;
                            if (!fFound)
                            {
        						EIO_fnmerge (filename, NULL, arg0.c_str(), NULL);
                                if (!EIO_FileExists (filename))
                                {
        							SetGlobalErr (ERR_GENERIC);
                                    GEcatf1 ("couldn't find file (%s) for ffilelength", arg0.c_str());
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
						}
						else
						{
							SetGlobalErr (ERR_GENERIC);
                            GEcatf ("wrong number of args for ffilelength");
                            return false;
						}
					}
                    else
                    {
                        SetGlobalErr (ERR_GENERIC);
                        GEcatf1 ("unknown macro command (%s)", idstr.c_str());
                        return false;
                    }
                }
				else // check for vars
                {
                    if (!MLANG_GetVariable (idstr, localSub))
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

