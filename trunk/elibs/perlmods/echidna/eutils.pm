#!/usr/bin/perl -w
use strict;
use warnings;

#  ************************************************************************
#
#                                EUTILS.PM
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
#
#    DESCRIPTION
#  	
#
#    PROGRAMMERS
#  	Gregg Tavares
#
#    TABS : 9
#
#
#    HISTORY
#	09/15/01 GAT: Created.
#
#

package echidna::eutils;
require Exporter;

use Getopt::Long;
use Pod::Usage;
use Text::ParseWords;
use IO::Handle;                     # 5.004 or higher
use File::Copy;
use echidna::nocasehash;

#    BEGIN {
        use Exporter   ();
        our ($VERSION, @ISA, @EXPORT, @EXPORT_OK, %EXPORT_TAGS);
        # set the version for version checking
        $VERSION     = 1.00;
        @ISA         = qw(Exporter);
        @EXPORT      = qw(
            errormsg
            warnmsg
            failmsg
            vprint
            dprint
            dumphash
            dumparray
            line_error
            line_warn
            line_fail
            parse_error
            parse_warn
            parse_fail
            cmd_execute
            copy_a_file
            isdigits
            split_string
            labelfy
            eio_fnmerge
            eio_fnsplit
            eio_path
            eio_filename
            eio_name
            eio_ext
            %scriptargs
            %globalvars
            $error_count
            $warn_count
            );
        %EXPORT_TAGS = ( );     # eg: TAG => [ qw!name1 name2! ],
        # your exported package globals go here,
        # as well as any optionally exported functions
        @EXPORT_OK   = qw(
            );
#    }

    our %scriptargs;
    our %globalvars;
    our $error_count;
    our $warn_count;

$error_count = 0;
$warn_count  = 0;

%scriptargs = (); # args that were passed into script
tie %scriptargs, 'Tie::NoCaseHash';
%globalvars = (); # for text substitutions
tie %globalvars, 'Tie::NoCaseHash';

#
# print an error and count
#
sub errormsg
{
   $error_count++;

   print "ERROR:";
   print @_;
}

#
# print a warning and count
#
sub warnmsg
{
   $warn_count++;

   print "WARNING:";
   print @_;
}

#
# just fail!
#
sub failmsg
{
   print "FAILED:";
   print @_;
   exit 0;
}

#
# print if verbose
#
sub vprint
{
   if ($scriptargs{'verbose'})
   {
      print @_;
   }
}

#
# print if debug
#
sub dprint
{
   if ($scriptargs{'debug'})
   {
      print @_;
   }
}

#
# dumphash (msg, hashref)
#
sub dumphash
{
   if ($scriptargs{'debug'})
   {
      my $key;
      my $msg = $_[0];
      my $hashref = $_[1];

      print $_[0], "\n";
      for $key (keys %$hashref)
      {
         print "key:", $key, ":", $_[1]->{$key}, "\n";
      }
   }
}

#
# dumparray (msg, arrayref[, sep, end])
#
sub dumparray
{
   if ($scriptargs{'debug'})
   {
      my $ii;
      my $array = $_[1];

      if (!defined ($array))
      {
         print $_[0], "undefined", (defined ($_[2]) ? $_[3] : "\n");
      }
      else
      {
         print $_[0], (defined ($_[2]) ? "" : "\n");

         for ($ii = 0; $ii < scalar (@{$array}); $ii++)
         {
            if (defined ($_[2]))
            {
               print $$array[$ii], (($ii != scalar (@{$array}) - 1) ? $_[2] : $_[3]);
            }
            else
            {
               print "  ", $ii, ":(", $$array[$ii], ")\n";
            }
         }
      }
   }
}

#
# print an error with filename and line number
#
# This is a function to keep the format consistent
#
sub line_error
{
   my $filename   = shift(@_);
   my $linenumber = shift(@_);

   errormsg ($filename, ":line ", $linenumber, ": ", @_);
}

#
# print an warning with filename and line number
#
# This is a function to keep the format consistent
#
sub line_warn
{
   my $filename   = shift(@_);
   my $linenumber = shift(@_);

   warnmsg ($filename, ":line ", $linenumber, ": ", @_);
}

#
# print an fail with filename and line number
#
# This is a function to keep the format consistent
#
sub line_fail
{
   my $filename   = shift(@_);
   my $linenumber = shift(@_);

   failmsg ($filename, ":line ", $linenumber, ": ", @_);
}

#
# print an error while parsing
#
sub parse_error
{
   my $context = shift(@_);

   line_error ($context->{'filename'}, $context->{'linenumber'}, "(", $context->{'keyword'}, ") ", @_);
}

#
# print an warning while parsing
#
sub parse_warn
{
   my $context = shift(@_);

   line_warn ($context->{'filename'}, $context->{'linenumber'}, "(", $context->{'keyword'}, ") ", @_);
}

#
# print an failure msg while parsing
#
sub parse_fail
{
   my $context = shift(@_);

   failmsg ($context->{'filename'}, $context->{'linenumber'}, "(", $context->{'keyword'}, ")", @_);
   exit 0;
}

#
# run a program and stop if failure
sub cmd_execute
{
   my $commandline = $_[0];

   system ($commandline) == 0 || die "system ", $commandline, " failed: $?";
}

#
# copy_a_file (src,dst)

sub copy_a_file
{
   my $src = $_[0];
   my $dst = $_[1];

   copy ($src,$dst) == 1 || die ("could not copy (", $src, ")->(", $dst, ") | ", $!);
}

#
# return TRUE of all characters in string are digits
#
sub isdigits
{
   my $check = $_[0];

   return ($check =~ /^\d+$/);
}

#
# split a string into an array of arguments
# split at whitespace or at comma unless surrounded by quotes
#
sub split_string
{
   my $str  = $_[0];
   my @args = ();

   if (defined ($str) && $str cmp "")
   {
      ((defined $+) ? push(@args, $+) : 0) while $str =~ m{
          "([^\"\\]*(?:\\.[^\"\\]*)*)"[,\s]*  # groups the phrase inside the quotes
        | ([^,\s]+)[,\s]*
        | [,\s]+
      }gx;
      push(@args, undef) if substr($str,-1,1) eq ',';
   }

   return @args;
}

#
# labelfy
#
# return a string with all non A-Z0-9_- turned into _
#
sub labelfy
{
   my $str = $_[0];

   $str =~ s/[^A-Z0-9-_]/_/gs;

   return $str;
}

#
# eio_fnsplit(filespec)
#
# split a filespec into path, name, etc (cross-platform)
#
# returns array (path, name, ext)
#
#   eio_filespec("d:\\greggdir\foo.bar") = ("d:\\greggdir\\", "foo", ".bar")
#
sub eio_fnsplit
{
   my $filespec = $_[0];
   my $path;
   my $filename;
   my $name;
   my $ext;

   # get the path if there is one
   if ($filespec =~ /^(.*)(?:\\|\/)([^\\\/]+)$/)
   {
      $path     = $1;
      $filename = $2;
   }
   else
   {
      $path = "";
      $filename = $filespec;
   }

   # get the ext if there is one
   if ($filename =~ /^(.*)\.([^\.]+)$/)
   {
      $name = $1;
      $ext  = $2;
   }
   else
   {
      $name = $filename;
      $ext  = "";
   }

   return ($path, $name, $ext);
}

#
# eio_name($filespec)
#
# return the just the name portion of a filespec
#
#   eio_name("d:\\greggdir\foo.bar") = "foo"
#
sub eio_name
{
   return (eio_fnsplit($_[0]))[1];
}

#
# eio_filename($filespec)
#
# return the just the filename portion of a filespec
#
#   eio_filename("d:\\greggdir\foo.bar") = "foo.bar"
#
sub eio_filename
{
   my $path;
   my $name;
   my $ext;

   ($path,$name,$ext) = eio_fnsplit($_[0]);
   return $name . $ext;
}

#
# eio_path($filespec)
#
# return the just the path portion of a filespec
#
#   eio_path("d:\\greggdir\foo.bar") = "d:\\greggdir\"
#
sub eio_path
{
   return (eio_fnsplit($_[0]))[0];
}

#
# eio_ext($filespec)
#
# return the just the name portion of a filespec
#
#   eio_ext("d:\\greggdir\foo.bar") = ".bar"
#
sub eio_ext
{
   return (eio_fnsplit($_[0]))[2];
}

#
# eio_fnmerge(path,name[,ext])
#
# merge a path filename and an extention cross-platform.  Add the slash if needed
#
sub eio_fnmerge
{
   my $path = $_[0];
   my $name = $_[1];
   my $ext  = $_[2];

   return (($path cmp "") ?
             ($path .
               ((substr ($path, -1) =~ /\\|\//) ?
                  '' :
                  '/'))
             : "")
          . $name . (defined $ext ? $ext : "");
}

1;

