#!/usr/bin/perl
#  ************************************************************************
#
#                            XLEXTRACTCOLUMN.PL
#
#  ************************************************************************
#
#                          Copyright 2003 Echidna
#
#    DESCRIPTION
#  	Extract a column from an Excel file in various encodings
#
#    PROGRAMMERS
#  	Gregg Tavares
#
#    HISTORY
#	10/30/03 : Created.
#
#
#
use strict;
use warnings;

use File::Basename;        # for basename/dirname
use File::Spec;            # for rel2abs
use IO::Handle;            # for filehandles 5.004 or higher
use Getopt::Long;          # for commandline options
use Pod::Usage;            # for help
use Encode;                # for encoding unicode, etc...
use Win32::OLE;
use Win32::OLE::Variant;

#
# globals
#
my $g_numErrors  = 0;               # total number of errors
my $g_xl         = undef;           # handle for excel object

sub failMsg
{
   $g_numErrors++;
   print "ERROR: ", @_;
   exit 1;
}

sub errorMsg
{
   $g_numErrors++;
   print "ERROR: ", @_;
}

#
# change all "/" to "\" because Excel doesn't like "/"
#
sub fix_slashes
{
   my ($filename) = @_;

   $filename =~ s{/}{\\}sg;

   return $filename;
}

#
# ColumnNumber ($column)
#
# returns a column number,ie (A=1, B=2, AA=27, 1=1)
sub ColumnNumber
{
   my ($col) = @_;

   if ($col =~ /^\d+$/)
   {
      return int($col);
   }
   elsif ($col =~ /^[A-Z]+$/i)
   {
      my $len = length($col);
      my $val = 0;
      my $ii;

      for ($ii = 0; $ii < $len; $ii++)
      {
         $val = $val * 26 + ord(uc(substr($col,$ii,1))) - 64;
      }
      return $val;
   }
}

#
# NameNum
#
# returns a integer if number, string if not
#
sub NameNum
{
   my ($val) = @_;

   if ($val =~ /^\d+$/)
   {
      return int($val);
   }

   return $val;
}

#***********************************  ************************************
#********************************* main **********************************
#***********************************  ************************************
{
   my $man = 0;
   my $help = 0;
   my $showenc = 0;
   my $sheet      = "1";             # sheet to extract from
   my $column     = undef;           # column to extract from
   my $firstRow   = 3;               # first row to extract from
   my $lastRow    = undef;           # last row to extract from
   my $encoding   = 'utf8';          # encoding we want
   my $format     = 'c_fullescape';  # format we want

   GetOptions(
      'help|?'          => \$help,
      'man'             => \$man,
      'showencodings'   => \$showenc,
      'encoding=s'      => \$encoding,
      'format=s'        => \$format,
      'sheet=s'         => \$sheet,
      'column=s'        => \$column,
      'firstrow=i'      => \$firstRow,
      'lastrow=i'       => \$lastRow,
      ) or pod2usage(2);
   pod2usage(1) if $help;
   pod2usage(-exitstatus => 0, -verbose => 2) if $man;
   if ($showenc)
   {
      my @all_encodings = Encode->encodings(":all");

      print "--encodings--\n";
      map { print $_,"\n"; } @all_encodings;
      exit 1;
   }
   if (scalar(@ARGV) > 2)  { failMsg ("too many arguments\n"); }

   my ($xlfile, $outfile) = @ARGV;

   if (!defined($xlfile))  { failMsg ("no excel file specified\n");    }
   if (!defined($outfile)) { failMsg ("no output file specfified\n");  }

   # validate column
   if (!defined ($column) || !($column =~ /^(?:[a-z]+|\d+)$/i))
   {
      failMsg ("bad column specifier\n");
   }

   print "extracting column ($column) from ($xlfile) to ($outfile)\n";

   # open the output file
   my $outfh = IO::Handle->new();
   if (!open($outfh, ">" . $outfile))
   {
      errorMsg ("cannot write file (", $outfile, ")\n");
   }
   else
   {
      # set perl's OLE module to return Unicode
      Win32::OLE->Option(CP => Win32::OLE::CP_UTF8);

      # get the Excel Application
      $g_xl = Win32::OLE->new('Excel.Application', sub {$_[0]->Quit;});
      if (!defined ($g_xl))
      {
         errorMsg ("cannot start Excel\n");
      }
      else
      {
         # make sure we clean up
#        eval
         {
            # it's not been loaded already so load it
            my $absName = File::Spec->rel2abs(fix_slashes($xlfile));

            # load the excel file
            my $xlFileHandle = $g_xl->Workbooks->Open($absName);
            if (!defined ($xlFileHandle))
            {
               errorMsg ("couldn't open excel file (", $absName, ")\n");
               die;
            }
            else
            {
               # we have not accessed this sheet yet
               my $xlSheetHandle = $xlFileHandle->WorkSheets(NameNum($sheet));
               if (!defined ($xlSheetHandle))
               {
                  errorMsg ("no sheet (", $sheet, ") in excel file (", $absName, ")\n");
               }
               else
               {
                  # lookup last row if not specified
                  if (!defined($lastRow))
                  {
                     $lastRow = $xlSheetHandle->UsedRange->Rows($xlSheetHandle->UsedRange->Rows->{Count})->{Row};
                  }

                  my $row;

                  for ($row = $firstRow; $row <= $lastRow; $row++)
                  {
                     if (($row % 100 == 0))
                     {
                        print "extracted ", $row, "\r";
                     }
                     # get a handle to the cell
                     my $xlCellHandle = $xlSheetHandle->Cells(int($row), int(ColumnNumber($column)));
                     if (!defined($xlCellHandle))
                     {
                        errorMsg ("could not access cell(", $column, ",", $row, ") in sheet (", $sheet, ") file (", $absName, ")\n");
                     }
                     else
                     {
                        my $result;
                        # get the value of the cell in unicode
                        # so we can search for quote.  If we pulled it
                        # out in some other language the value 0x22 (quote)
                        # might be embedded in some wide characters and
                        # we could not search for it
                        my $uni = $xlCellHandle->{Value};
                        # there is some voodoo here.  I'm sure some perl guru can explain
                        # if I just use $uni directly things don't work!?!?
                        my $temp = $uni;
                        if (!defined ($temp))
                        {
                           $result = "";
                        }
                        else
                        {
                           if ($format =~ /^c_escape$/i)
                           {
                              # esacpe it for C. as in (He calls himself "The Boss")
                              # becomes (He calls himself \"The Boss\")
                              #
                              # note: we need to do this search while it's still unicode
                              # or else we'll find 0x22 (quote) embedded in wide characters
                              # etc..

                              $temp =~ s/\\/\\\\/sg;
                              $temp =~ s/"/\\"/sg;
                              $temp =~ s/\n/\\n/sg;
                              $temp =~ s/\t/\\t/sg;
                              $temp =~ s/\r/\\r/sg;

                              # convert to desired encoding
                              $result = encode ($encoding, $temp);
                           }
                           elsif ($format =~ /^c_fullescape$/i)
                           {
                              # esacpe it all for C.
                              #
                              # note, you might think regular escaping like above would
                              # work but unfortunately once you've converted to a locale
                              # it's unlikely your C compiler is aware of that and will
                              # barf if 0x22 or 0x0D is in your string in a wide character
                              #
                              # note that CodeWarrior claims to be multi-byte aware
                              # but as far as I can tell that means only in the current
                              # locale.  In other words if you have Japanese in string
                              # in your source code and you are compiling on a machine
                              # with Japanese set to the default language everything
                              # is fine but if you are compiling Korean or Chinese on
                              # a Japanese machine you are SOL!
                              #
                              # so, the safe bet is just to escape it all.
                              #

                              # convert to desired encoding
                              $result = encode ($encoding, $temp);
                              $result =~ s/(.)/
                                          sprintf ('\\x%02x', ord($1));
                                          /sge;
                           }
                           elsif ($format =~ /^c_bytes$/i)
                           {
                              # convert to desired encoding
                              $result = encode ($encoding, $temp);
                              $result =~ s/(.)/
                                          sprintf ('0x%02x,', ord($1));
                                          /sge;
                           }
                           else
                           {
                              failMsg ("unknown format (", $format, ")\n");
                           }
                        }

                        print $outfh $result, "\n";
                     }
                  }
                  print ("extracted to row ", $lastRow, "\n");
               }
            }
         };
         $g_xl->Quit;
      }
      close ($outfh);
   }

   print "finished\n";
}

__END__

=head1 xlextractcolumn

extract the contents of cells from Excel files and generate a new file based on a template

=head1 SYNOPSIS

xlextractcolumn [options] template outfile

 Options:
   -help           brief help message
   -man            full documentation
   -showencodings  show all available encodings
   -encoding <enc> output encoding (see full docs)
   -format         format for output (see full docs)
   -sheet          sheet to extract from, can be name or number (def: 1)
   -column         column to extract
   -firstrow       first row to start extracting from (def: 3)
   -lastrow        last row to stop extracting from (def: last used)

=head1 OPTIONS

=over 8

=item B<-help>

Print a brief help message and exits.

=item B<-man>

Prints the manual page and exits.

=item B<-encoding>

Sets the output encoding.  NOTE: CASE-SENSITIVE!!!!
Default is utf8.  Common encodings:

   ascii
   latin1   (iso-8859-1)
   utf8     (8bit unicode)
   shiftjis (Shift-JIS)
   euc-jp   (EUC)
   euc-kr   (Korean EUC)

See perl docs for the module "Encode" for all
available encodings. http://www.perldoc.com/perl5.8.0/lib/Encode.html

=back

=head1 DESCRIPTION

B<xlextractcolumn> will read the given template file
and open the excel file referenced their in, extract
referenced cells and generate an output file

=cut
