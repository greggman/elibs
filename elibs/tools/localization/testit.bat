xlextractcells.pl -format c_escape -encoding utf8     excelin.txt excelout-utf8.htm -define COLUMN=B -define FILENAME=testphrases.xls
xlextractcells.pl -format c_escape -encoding shiftjis excelin.txt excelout-sjis.htm -define COLUMN=C -define FILENAME=testphrases.xls
xlextractcells.pl -format c_escape -encoding euc-kr   excelin.txt excelout-kr.htm   -define COLUMN=D -define FILENAME=testphrases.xls

