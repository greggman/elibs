#!/usr/bin/perl
#  ************************************************************************
#
#                             XLEXTRACTCELLS.PL
#
#  ************************************************************************
#
#		Copyright (c) 2001-2008, Echidna
#
#		All rights reserved.
#
#		Redistribution and use in source and binary forms, with or
#		without modification, are permitted provided that the following
#		conditions are met:
#
#		* Redistributions of source code must retain the above copyright
#		  notice, this list of conditions and the following disclaimer. 
#		* Redistributions in binary form must reproduce the above copyright
#		  notice, this list of conditions and the following disclaimer
#		  in the documentation and/or other materials provided with the
#		  distribution. 
#
#		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
#		CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
#		INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
#		MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#		DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
#		BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#		EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
#		TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#		DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
#		ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
#		OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
#		OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#		POSSIBILITY OF SUCH DAMAGE.
#
#    DESCRIPTION
#  	Extract Cells from an excel file and apply to a template
#
#    PROGRAMMERS
#  	Gregg Tavares
#
#    HISTORY
#	10/25/03 : Created.
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
my %g_defines    = ( );             # hash of defines
my $g_xlFiles    = { };             # hashref of excel files by filename
my $g_lineNo     = 0;               # line number of input file
my $g_numErrors  = 0;               # total number of errors
my $g_defFile    = undef;           # current default file
my $g_defSheet   = undef;           # current default sheet
my $g_defColumn  = undef;           # current default column
my $g_encoding   = 'utf8';          # encoding we want
my $g_format     = 'c_fullescape';  # format we want
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

sub errorLineMsg
{
   errorMsg ("ERROR: Line ", $g_lineNo, " : ", @_);
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
# read_file(filename)
#
# read an entire file into a string
#
sub read_file
{
   my ($filename) = $_;
   my $fh = IO::Handle->new();

   if (!open($fh, $filename))
   {
      errorMsg ("couldn't open file (", $filename, ")\n");
      die;
   }

   local($/) = undef;
   my $data = <$fh>;
   close($fh);

   return ($data);
}

#
# isDigits($str)
#
# return true if $str is all digits
#
sub isDigits
{
   return $_[0] =~ /^[0-9]+$/i;
}

#
# isColRow ($str)
#
# returns true if $str is a column and row spec (like B5)
sub isColRow
{
   return $_[0] =~ /^[a-z]+\d+$/i;
}

#
# GetColRow ($str, $column, $row)
#
# given a ColRow str like "B5" will set $column to "B" and $row to "5"
sub GetColRow
{
   $_[0] =~ /^([a-z]+)(\d+)$/i;
   $_[1] = $1;
   $_[2] = $2;
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
# DoCommand ($keyword, $argstr)
#
sub DoCommand
{
   my ($keyword, $args) = @_;
   my $result = "";

   # subsitute defines and env vars
   $args =~ s/%(\w*)%/
               my $result;
               if (defined($g_defines{$1}))
               {
                  $result = $g_defines{$1};
               }
               elsif (defined($ENV{$1}))
               {
                  $result = $ENV{$1};
               }
               elsif ($1 eq "")
               {
                  $result = "%";
               }
               else
               {
                  errorLineMsg ("no define or environment variable (", $1, ")\n");
                  $result = "";
               }
               $result;
            /sgie;

   # separate args by "," or "!"
   my @arg = split ("[,!]", $args);
   my $numArgs = scalar (@arg);

   # strip leading and trailing spaces from each argument
   @arg = map {
      s/^\s+//;
      s/\s+$//;
      $_;
      } @arg;

   {
      my $ii;

      print "---(", $keyword, ")---\n";
      for ($ii = 0; $ii < $numArgs; $ii++)
      {
         print "arg ", $ii, ": (", $arg[$ii], ")\n";
      }
   }

   # do the command
   if ($keyword =~ /^cell$/i)
   {
      my $file   = $g_defFile;
      my $sheet  = $g_defSheet;
      my $column = $g_defColumn;
      my $row;
      my $fProcess = 1;

      if ($numArgs == 1)
      {
         # 1 arg, could be col+row (eg, "B5") or just row (eg, "5")
         if (isColRow($arg[0]))
         {
            GetColRow ($arg[0], $column, $row);
         }
         else
         {
            $row = $arg[0];
         }
      }
      elsif ($numArgs == 2)
      {
         # 2 args, could be Sheet,col+row (eg, "Sheet1,B5") or col,row (eg, "B,5")
         if (isColRow($arg[1]))
         {
            $sheet  = $arg[0];
            GetColRow ($arg[1], $column, $row);
         }
         else
         {
            $column = $arg[0];
            $row    = $arg[1];
         }
      }
      elsif ($numArgs == 3)
      {
         # 3 args, could be
         #   File, Sheet, Col+Row (eg, file.xls,sheet1,B5)
         #   Sheet, Col, Row (eg, Sheet1,B,5)
         if (isColRow ($arg[2]))
         {
            $file = $arg[0];
            SetIfDef($sheet, $arg[1]);
            GetColRow ($arg[2], $column, $row);
         }
         else
         {
            $sheet  = $arg[0];
            $column = $arg[1];
            $row    = $arg[2];
         }
      }
      elsif ($numArgs == 4)
      {
         # 4 args, Must be, File, Sheet, Col, Row
         # but allow Col to be blank (ie, $CELL[file.xls,sheet1,,Col+Row]
         if ($arg[2] eq "" && isColRow ($arg[3]))
         {
            $file   = $arg[0];
            SetIfDef($sheet, $arg[1]);
            GetColRow ($arg[3], $column, $row);
         }
         else
         {
            $file   = $arg[0];
            SetIfDef($sheet, $arg[1]);
            $column = $arg[2];
            $row    = $arg[3];
         }
      }
      else
      {
         errorLineMsg ("wrong number of args (", $numArgs, ") for CELL\n");
         $fProcess = 0;
      }

      if ($fProcess)
      {
         # get the file
         if (!defined ($g_xlFiles->{$file}))
         {
            # it's not been loaded already so load it
            my $absName = File::Spec->rel2abs(fix_slashes($file));

            # load the excel file
            my $xlFileHandle = $g_xl->Workbooks->Open($absName);
            if (!defined ($xlFileHandle))
            {
               errorLineMsg ("couldn't open excel file (", $absName, ")\n");
               die;
            }
            else
            {
               # remember info about this file
               $g_xlFiles->{$file} = {
                     'name'     => $file,
                     'fullpath' => $absName,
                     'handle'   => $xlFileHandle,
                     'sheets'   => { },
                  };
            }
         }

         {
            my $xlFile = $g_xlFiles->{$file};

            # check if we have the sheet
            if (!defined ($xlFile->{'sheets'}->{$sheet}))
            {
               # we have not accessed this sheet yet
               my $xlSheetHandle = $xlFile->{'handle'}->WorkSheets($sheet);
               if (!defined ($xlSheetHandle))
               {
                  errorLineMsg ("no sheet (", $sheet, ") in excel file (", $xlFile->{'fullpath'}, ")\n");
               }
               else
               {
                  $xlFile->{'sheets'}->{$sheet} = $xlSheetHandle;
               }
            }

            # check if the sheet exists
            if (defined ($xlFile->{'sheets'}->{$sheet}))
            {
               # validate row and column
               if (!isDigits($row))
               {
                  errorLineMsg ("bad row (", $row, ")\n");
               }
               elsif (!($column =~ /^(?:\d+|[a-z]+)$/i))
               {
                  errorLineMsg ("bad column (", $column, ")\n");
               }
               else
               {
                  # get a handle to the cell
                  print ("checking for cell(", ColumnNumber($column), ",", $row, ") in sheet (", $sheet, ") file (", $xlFile->{'fullpath'}, ")\n");
                  my $xlCellHandle = $xlFile->{'sheets'}->{$sheet}->Cells(int($row), int(ColumnNumber($column)));
                  if (!defined($xlCellHandle))
                  {
                     errorLineMsg ("could not access cell(", $column, ",", $row, ") in sheet (", $sheet, ") file (", $xlFile->{'fullpath'}, ")\n");
                  }
                  else
                  {
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
                        if ($g_format =~ /^c_escape$/i)
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
                           $result = encode ($g_encoding, $temp);
                        }
                        elsif ($g_format =~ /^c_fullescape$/i)
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
                           $result = encode ($g_encoding, $temp);
                           $result =~ s/(.)/
                                       sprintf ('\\x%02x', ord($1));
                                       /sge;
                        }
                        elsif ($g_format =~ /^c_bytes$/i)
                        {
                           # convert to desired encoding
                           $result = encode ($g_encoding, $temp);
                           $result =~ s/(.)/
                                       sprintf ('0x%02x,', ord($1));
                                       /sge;
                        }
                        else
                        {
                           failMsg ("unknown format (", $g_format, ")\n");
                        }
                     }
                  }
               }
            }
         }
      }
   }
   elsif ($keyword =~ /^set_def$/i)
   {
      if ($numArgs == 2)
      {
         if ($arg[0] =~ /^sheet$/i)
         {
            $g_defSheet = $arg[1];
         }
         elsif ($arg[0] =~ /^file$/i)
         {
            $g_defFile = $arg[1];
         }
         elsif ($arg[0] =~ /^column$/i)
         {
            $g_defColumn = $arg[1];
         }
         else
         {
            errorLineMsg ("unknown option (", $arg[0], ") for SET_DEF\n")
         }
      }
      else
      {
         errorLineMsg ("wrong number of args (", $numArgs, ") for SET_DEF\n")
      }
   }
   else
   {
      errorLineMsg ("unknown keyword (", $keyword, ")\n");
   }

   $result;
}

#***********************************  ************************************
#********************************* main **********************************
#***********************************  ************************************
{
   my $man = 0;
   my $help = 0;
   my $showenc = 0;

   GetOptions(
      'help|?'        => \$help,
      'man'           => \$man,
      'showencodings' => \$showenc,
      'define|d=s'    => \%g_defines,
      'encoding=s'    => \$g_encoding,
      'format=s'      => \$g_format,
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

   my ($tmpltfile, $outfile) = @ARGV;

   if (!defined($tmpltfile)) { failMsg ("no template file specified\n"); }
   if (!defined($outfile))   { failMsg ("no output file specfified\n");  }

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
#      eval
      {
         # open the template file
         my $infh = IO::Handle->new();
         if (!open($infh, $tmpltfile))
         {
            errorMsg ("cannot open file (", $tmpltfile, ")\n");
         }
         else
         {
            # open the output file
            my $outfh = IO::Handle->new();
            if (!open($outfh, ">" . $outfile))
            {
               errorMsg ("cannot write file (", $outfile, ")\n");
            }
            else
            {
               # read each line
               while (<$infh>)
               {
                  $g_lineNo++;

                  # search for our keywords
                  s/\$(CELL|SET_DEF)\[([^\]]*)\]/
                     DoCommand($1,$2);
                     /sgie;
                  print $outfh $_;
               }

               close ($outfh);
            }
            close ($infh);
         }
      };
      $g_xl->Quit;
   }

   print "finished\n";
}

__END__

=head1 xlextractcells

extract the contents of cells from Excel files and generate a new file based on a template

=head1 SYNOPSIS

xlextractcells [options] template outfile

 Options:
   -help           brief help message
   -man            full documentation
   -encoding <enc> output encoding (see full docs)
   -format         format for output (see full docs)
   -define         define a constant (just like C)

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

Use -showencodings to see all available encodings.  See perl docs for
the module "Encode" for more information.
http://www.perldoc.com/perl5.8.0/lib/Encode.html

=item B<-define>

used to define constants.  Useful for specifying different files,
sheets or columns using the same template.  For example your
template might look like this

   $SET_DEF[FILE,%FILENAME%]
   $SET_DEF[COLUMN,%COLUMN%]

and you could call xlextractcells like this

   xlextractcells.pl mytmplt.txt myjtext.c -d column=B -d filename=dialog_japanese.xls
   xlextractcells.pl mytmplt.txt myktext.c -d column=D -d filename=dialog_korean.xls

=back

=head1 DESCRIPTION

B<xlextractcells> will read the given template file
and open the excel file referenced their in, extract
referenced cells and generate an output file

=cut
