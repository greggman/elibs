#
$tempfile="$ENV{tmp}\\tmp" . time;
foreach $name (@ARGV)
{
	print "fixing $name\n";
	system "eggsfilt $name $tempfile";
	if ($ != 0) { goto error; }
	system "copy $tempfile $name";
	if ($ != 0) { goto error; }
	system "del $tempfile";
	if ($ != 0) { goto error; }
}
exit 0;

error:
{
	print "***ERROR:Could not fix $name\n";
}
exit 1;



