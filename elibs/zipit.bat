rem this is a test
del elibs.zip eutils.zip ewinbin.zip
cd ..
"c:\program files\winzip\wzzip" -yb -ex -P -r -x*.bak -x*_index -xdebug\* -x*\ -x*.old -x*.obj -x*.ilk -x*.pch -x*.pdb -x*.idb -x*.exe -x*.ncb -x*.index -x*.user elibs\elibs.zip elibs\inc\*.* elibs\lib\*.* elibs\docs\*.* elibs\zipit.bat
"c:\program files\winzip\wzzip" -yb -ex -P -r elibs\ewinbin.zip elibs\bin\*.*
"c:\program files\winzip\wzzip" -yb -ex -P -r -x*.bak -x*_index -x*\debug\* -x*\ -x*.old -x*.obj -x*.ilk -x*.pch -x*.pdb -x*.idb -x*.exe -x*.ncb -x*.index -x*.user elibs\eutils.zip elibs\tools\*.*
cd elibs

