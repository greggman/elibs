#!/usr/bin/perl -w
use strict;
use warnings;

use Getopt::Long;
use Pod::Usage;
use Text::ParseWords;
use IO::Handle;                     # 5.004 or higher
require "nocasehash.pl";

BEGIN {
        use Exporter   ();
        our ($VERSION, @ISA, @EXPORT, @EXPORT_OK, %EXPORT_TAGS);
        
        # set the version for version checking
        $VERSION     = 1.00;
        # if using RCS/CVS, this may be preferred
        $VERSION = do { my @r = (q$Revision$ =~ /\d+/g); sprintf "%d."."%02d" x $#r, @r }; # must be all one line, for MakeMaker
        
        @ISA         = qw(Exporter);
        @EXPORT      = qw(%scriptargs %globalsvars);
        %EXPORT_TAGS = ( );     # eg: TAG => [ qw!name1 name2! ],
        # your exported package globals go here,
        # as well as any optionally exported functions
        @EXPORT_OK   = qw();
    }


my $error_count = 0;
my $warn_count  = 0;

my %scriptargs = (); # args that were passed into script
tie %scriptargs, 'Tie::NoCaseHash';
my %globalvars = (); # for text substitutions
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

1;

