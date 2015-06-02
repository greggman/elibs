# The Echidna Libraries

*   [Jump to the code](http://github.com/greggman/elibs/)
*   [What is this?](#what)
*   [Tell me the interesting bits.](#what-s-interesting-here)
*   [Utilities and Tools](#utilities)
*   [Command line argument system](#arguments-and-response-files)
*   [Reasons for going open source](#what-are-the-benefits-of-proprietary-utilities-vs-the-costs)
*   [Compiling / Setup Instructions](#instructions)
*   [License](#license--new-bsd)
*   [Download](#downloads)

<a name="what"></a>In 1990 my friends and I started a company called Echidna. At that time we created some
libraries of routines that we all shared. We needed to create titles for both the Amiga
and the PC and so we wanted to create a set of libraries that would allows us to write the
game once and just use a different version of the libraries for the game to run on a
different platform.

Since that time, some of those routines have turned out to be more re-usable then
others. Routines to do things like:

*   Parse a command line including support for nested response files.
*   Parse a Windows style .ini file and get values from it
*   Write formatted data to a text file
*   Split and combine filenames, paths and extensions
*   Get a directory listing optionally including subdirectories
*   Get a list of all environment variables
*   Walk the command a path so you can visit every directory
*   Find a file in the command path
*   Copy a file including date, attributes, commands (and resource fork)
*   Read/Set/Compare the date of a file
*   Implement a linked list
*   Report Errors
*   Track the call stack
*   Manage memory
*   Read/write 8bit, 24bit and 32bit images

And a few other things like

*   Ensure that all systems support all functions we need (examples: strdup and stricmp) are
    not ANSI and so are on some systems but not others.

*   Ensure a common set of basic types int8, uint8, int16, uint16, int32, uint32, BOOL

*   Support converting to and from the Native format of 16 and 32 bit values on the current
    machine to/from the target machine.

*   Supply a bunch of routines for handling common tasks that should fail on error. Like
    opening a file, allocating memory. In these cases, if we are writing a command line tool,
    we just want it to fail and exit with an error message. We don't want to have to
    check for errors.

*   Open a file in a system independent way. (ie, open() on a PC is different from open() on
    Unix which is different from open() on an Amiga. We have EIO_ReadOpen and EIO_WriteOpen
    and just so we don't have to remember the word order we also have EIO_OpenRead and
    EIO_OpenWrite

These libraries are far from perfect especially since they have been updated for many
years. There is still code in them for things we needed in 1990.

We learned many things on the way. One of the biggest is never use a common name. For
example calling something a `Point` pretty much guarantees that someday
you'll have to change that code when you try to use some example 3D program you found
on the net with your libraries because they also declared a type called Point.

Another example, having a define called DEBUG, TEST, WIN, MAC, WINDOWS, WIN, USA,
JAPAN, FINAL, STATUS, X, Y, Z, ROM, will almost surely clash with somebody someday. The
easiest solution we found is to use a prefix.

We find these libraries very useful no matter where we are working or who we are
working for, yet they are our libraries and as such, when we are working for somebody else
they get tense about us using them. They feel if they don't own them outright somehow
that sets them up for losing out in someway even though we've assured them they can
do anything they want with them.

So, in the self-interest of wanting to be able to use our libraries when and where we
please we are making them copyrighted freeware. That means the copyright stays with us but
anybody can use them royalty free and without restriction. We are not doing this to help
others with cool routines. We are doing this solely out of self-interest. It sucks to have
to re-write code. Our libraries make us more efficient and so we want to use them. This
should allow us to use them anywhere unrestricted.

For those people that just don't get it and still feel they need to own the code
if it is used in their product consider these facts. You don't own the code to the C
library that came with your compiler nor do you own the startup code that gets linked into
your program to start it nor do you own libraries like MFC, STL, OpenGL or DirectX some or
all of which you are using in your code. Yet somehow that doesn't seem to bother you.
Please consider our libraries to be exactly the same as those. If you want to use them go
ahead and use them and don't worry about it.

You are free to download them and see if there is anything useful in them that you
would like to use. Please do not ask us to support them. That is not why they are here.

## What's Interesting here?

These tools and libraries started in the early 1990 so some parts of it are
more relevant to today then others.  The parts you might find interesting

### Most Interesting:

*   The Echidna Data Linker.  Links together complex data for easy
    loading into a game

*   The GF tools, gfxinout, gfshrink, gfpal.  These 3 tools can for
    example make mips for textures and palletize them including having one palette
    across mips.  Sample a thousand images, make a one palette for all of
    them, then remap all the images to use that palette.

*   File format functions.  In the libraries there are functions for
    creading various graphic formats. .PCX, .PSD, .PIC, .TGA, .IFF.  Check out
    the functions in READGFX.H

#### Mildly Interesting:

*   EIO.c / EIO.h  this part of the libraries deals with IO across
    platforms including path manipulation, copying files, reading folders.
    As there is no consistent standard we wrote our own so our tools can be
    platform independent.
*   Definer.  A solution to the problem of having separate header files
    for 2 or more different languages but that have to remain in sync.  For
    example keeping your constants in sync and available to both C and assembly.

#### Less Interesting

*   Cutbytes.  It's a simple program but sometimes when you are trying to
    hack something apart it can be easier to just cut it up using a simple tool
    like this.

*   Argparse.c/Argparse.h.  It's our solution to argument parsing.
    Handles multiple arguments per field and passing the arguments in by files to
    get around the limits some system have with long command lines.

*   Maclang.cpp/Maclang.h.  A simple text based macro expansion language
    that can be applied to any string.

*   Readini.c/Readini.h.  Functions to read windows style .ini files.
    Can make it easy to make configuration files for example.

*   Ensure.c/Ensure.h  For examples of making your own printf and
    printing it's results to the VC++ debugger's output pane (see EL_printf())

## Utilities

There are also several utilities written with these libraries that we find useful.
Consider them the same as the libraries. They are also copyrighted freeware. Use them if
you find them useful, take some code out of them and stick them in your code, you have our
permission.

A lot of them were written back in the DOS days.  Currently we do most of our scripting
 in Perl so there isn't as much of a reason for things like sizefile, longest, change,
 newer and riteargs but you might get some ideas for your own scripts by reading what they
 do.


Examples of some of the utilities:


<dir>
  <table CELLSPACING="0">
  <tr>
    <td WIDTH="16%" VALIGN="TOP">Bin2list</td>
    <td WIDTH="84%" VALIGN="TOP">Copies a bunch of files into one file with
    offset pointers to them as a header table at the top of the file or as constants in a
    separate file.</td>
  </tr>
  <tr>
    <td WIDTH="16%" VALIGN="TOP">Bump</td>
    <td WIDTH="84%" VALIGN="TOP">Increments a version number in a .c/.h file</td>
  </tr>
  <tr>
    <td WIDTH="16%" VALIGN="TOP">Change</td>
    <td WIDTH="84%" VALIGN="TOP">Searches and replaces text. <p>Only simple
    search and replace, no fancy expressions</td>
  </tr>
  <tr>
    <td WIDTH="16%" VALIGN="TOP">Copyframes</td>
    <td WIDTH="84%" VALIGN="TOP">Copies a bunch of files in alphabetical order.<p>This
    was useful for copying files to the DPS Perception video card because if you used Windows
    to copy the files they might not get copied in alphabetical order and the card didn't
    like that when assembling a new video.</td>
  </tr>
  <tr>
    <td WIDTH="16%" VALIGN="TOP">Cutbytes</td>
    <td WIDTH="84%" VALIGN="TOP">Copies a specified number of bytes out of a
    file starting at a specified offset</td>
  </tr>
  <tr>
    <td WIDTH="16%" VALIGN="TOP">Definer</td>
    <td WIDTH="84%" VALIGN="TOP">Generates .h,.inc and other types of files
    based on a .def (definition) file and an input file.<p>The most common use for this would
    be to have one .d header file to define a structure and then have definer generate the .h
    and .inc files for C and assembly language. This prevents a common programming error where
    you change a structure or constant on your .h file but forget to change it in your .inc
    file and then spend hours or days trying to find the bug.</p>
    <p>This is also useful for generating .c/.h files from tables so for example of you want
    to make a table of monsters you can make a .md file and have definer generate a .c, .h,
    .inc files that have both the table and constants created. That way you only have to edit
    one file (.md) instead of 3 files when adding a monster to your game.</td>
  </tr>
  <tr>
    <td WIDTH="16%" VALIGN="TOP">Echecksize</td>
    <td WIDTH="84%" VALIGN="TOP">Adds up the sizes of a bunch of files and
    returns an error if they are over a certain size<p>This is useful in a build process were
    you want to make sure the total size of some data is not larger than some specified size.
    For example you want to make sure all your sounds are not more than 512K.</td>
  </tr>
  <tr>
    <td WIDTH="16%" VALIGN="TOP">Ecompare</td>
    <td WIDTH="84%" VALIGN="TOP">Does a binary compare of two files and returns
    error if they are different.<p>Useful in a build process for checking if something
    actually changed. For example, lets say you create a new palette and then all of your
    graphics need to be remapped. Well instead you could keep a copy of the old palette,
    generate the new palette, then use ecompare. If the new palette happens to be the same as
    the old palette then you don't need to re-map all your graphics.</td>
  </tr>
  <tr>
    <td WIDTH="16%" VALIGN="TOP">Egetsetdate</td>
    <td WIDTH="84%" VALIGN="TOP">Copies the date from one file to another</td>
  </tr>
  <tr>
    <td WIDTH="16%" VALIGN="TOP">Eggsfilt</td>
    <td WIDTH="84%" VALIGN="TOP">Converts a C / C++ file from DOS/Unix/Mac
    style to the native or specified style.<p>In otherwords if you run it on DOS it will
    default to converting the file to DOS. That means changing the line endings to DOS style
    endings and changing #includes from</p>
    <p>#include &lt;echidna:utils.h&gt;</p>
    <p>or</p>
    <p>#include &lt;echidna/strings.h&gt;</p>
    <p>to</p>
    <p>#include &lt;echidna\strings.h&gt;</p>
    <p>While at the same time keeping the date the same as the original</td>
  </tr>
  <tr>
    <td WIDTH="16%" VALIGN="TOP">Elcopy</td>
    <td WIDTH="84%" VALIGN="TOP">Copies a directory tree with options to copy
    only newer files and files that don't exist in the destination.<p>It doesn't
    fail on exit. This is useful for copying a very large directory on a regular basis like
    backing up your entire project from one server to another every night.</td>
  </tr>
  <tr>
    <td WIDTH="16%" VALIGN="TOP">Etap</td>
    <td WIDTH="84%" VALIGN="TOP">Same as unix tap. Copies stdin to a file and
    to stdout</td>
  </tr>
  <tr>
    <td WIDTH="16%" VALIGN="TOP">Fixlines</td>
    <td WIDTH="84%" VALIGN="TOP">Converts line ending from DOS/Unix/Mac to the
    native or specified format.</td>
  </tr>
  <tr>
    <td WIDTH="16%" VALIGN="TOP">Hexdump</td>
    <td WIDTH="84%" VALIGN="TOP">Prints a Hexdump of a file with various
    options for BYTE, WORDs, LONGs etc.</td>
  </tr>
  <tr>
    <td WIDTH="16%" VALIGN="TOP">Largest</td>
    <td WIDTH="84%" VALIGN="TOP">Checks the size of a file and if it's
    larger than the size saved in another file it updates that size.<p>In other words you can
    use it to generate a constant the size of the largest file. For example if you want to
    allocate a buffer the size of the largest image you could run all images through largest
    during your build processes and then use the result to allocate your buffer.</td>
  </tr>
  <tr>
    <td WIDTH="16%" VALIGN="TOP">Makelib</td>
    <td WIDTH="84%" VALIGN="TOP">Makelib splits a .C/.CPP file into a bunch of
    temporary files by looking for lines that start with `/**# MODULE`. The area
    before the first `/**# MODULE` and areas that start with `/** GLOBAL`
    are put in all temporary files. This allows you to create a library using one file but
    have it split up by routine so that when you link you get only those routines you actually
    use. Since the time this was written, some compiler companies do this for you
    automatically. Unfortunately I don't think Microsoft is one of them yet.</td>
  </tr>
  <tr>
    <td WIDTH="16%" VALIGN="TOP">Makelist</td>
    <td WIDTH="84%" VALIGN="TOP">Makes a list based on a word table.<p>Example.
    You have a file that looks like this (myspecfile)</p>
    <p>Explosion d:\sound\exl2.wav 44k<br>
    Engine d:\sound\grrr.wav 22k looping<br>
    Cough d:\sound\cgh43.wav 22k<br>
    Crash d:\sound\crsh.wav 11k</p>
    <p>You have another that looks like this (mywordfile)</p>
    <p>Engine<br>
    Crash</p>
    <p>If you run makelist like this:</p>
    <p>Makelist myspecfile "nothing 0k" mywordfile myresult</p>
    <p>Then myresult will look like this</p>
    <p>Nothing 0k<br>
    d:\sound\grrr.wav 22k looping<br>
    Nothing 0k<br>
    d:\sound\crsh.wav 11k</td>
  </tr>
  <tr>
    <td WIDTH="16%" VALIGN="TOP">
    [Mkloadob](mkloadob.html)</td>
    <td WIDTH="84%" VALIGN="TOP">Takes a "linker file" and builds a
    loadable binary file with lots of pointers that need to be fixed up :-)</td>
  </tr>
  <tr>
    <td WIDTH="16%" VALIGN="TOP">Mutate</td>
    <td WIDTH="84%" VALIGN="TOP">Renames files using numbers.<p>This was used to solve the
    problem that some 3D program numbered frames 0,1,2,...9,10,11...,99,100,101 which means
    they didn't actually sort correctly.  Mutate was used to change them to
    0001,0002,...0009,0010,0011 etc.</td>
  </tr>
  <tr>
    <td WIDTH="16%" VALIGN="TOP">Newer</td>
    <td WIDTH="84%" VALIGN="TOP">Checks if one file is newer than another. Used
    in batch files to check for dependencies except now I use Perl instead </td>
  </tr>
  <tr>
    <td WIDTH="16%" VALIGN="TOP">Parallel</td>
    <td WIDTH="84%" VALIGN="TOP">Creates parallel arrays from tabular data</td>
  </tr>
  <tr>
    <td WIDTH="16%" VALIGN="TOP">Riteargs</td>
    <td WIDTH="84%" VALIGN="TOP">Writes arguments to stdout and interprets C
    style escape sequences</td>
  </tr>
  <tr>
    <td WIDTH="16%" VALIGN="TOP">Sizefile</td>
    <td WIDTH="84%" VALIGN="TOP">Writes the size of one file as text into
    another.<p>Useful for computing the exact size needed for a buffer at compile time.</td>
  </tr>
  <tr>
    <td WIDTH="16%" VALIGN="TOP">Gfxinout</td>
    <td WIDTH="84%" VALIGN="TOP">Converts images to and from .PSD, .PCX, .TGA,
    .GFF (gff is our internal format)</td>
  </tr>
  <tr>
    <td WIDTH="16%" VALIGN="TOP">Gfshrink </td>
    <td WIDTH="84%" VALIGN="TOP">Shrinks a .GFF file</td>
  </tr>
  <tr>
    <td WIDTH="16%" VALIGN="TOP">Gf16bit</td>
    <td WIDTH="84%" VALIGN="TOP">Converts a .GFF file to a raw 16 bit per pixel
    file</td>
  </tr>
  <tr>
    <td WIDTH="16%" VALIGN="TOP">Gfpal </td>
    <td WIDTH="84%" VALIGN="TOP">Converts a .GFF file to a paletted file with a
    zillion options.<p>There are options to:<ul>
      <li>create a palette</li>
      <li>use an existing palette</li>
      <li>disallow some colors from being used</li>
      <li>allow only some colors to be chosen</li>
      <li>dither when converting</li>
      <li>output in different formats</li>
    </ul>
    </td>
  </tr>
</table>
</DIR>

## Arguments and Response files:

All utilities that use the echidna argument parsing functions can take response files.
If you use an @ sign on the command line the tool will read each line in that file as part
of command line. Since it is parsing it as a command line that means that if it encounters
another @ symbol inside that file it will start parsing that file also. This is most
useful for passing lots of arguments to programs like bin2list and echecksize. Example:

    --file-everylevelstuff.arp--
    ;
    ; these files need to be in every level
    ;
    %myprojectdatadir%\mainpalette.pal
    %myprojectdatadir%\statusbar.img
    %myprojectdatadir%\player.img
    --file-level1.arp&#151;
    ;
    ; these files are used in level 1
    ;
    %myprojectdatadir%\level1tiles.img
    %myprojectdatadir%\level1snds.snd

Now, you can run bin2list like this

    Bin2list level1.dat @%myprojectdatadir%everylevelstuff.arp
    @%myprojectdatadir%\level1.arp

As you can see you can use multiple response files and environment variables will get
expanded. (Note: Environment variables can also get expanded in our .INI file routines).
You can also use a semicolon for comments.

The argument routines decide which "argument position" to assign your
arguments to by filling them in from left to right. For example if you have a program that
takes an INFILE and an OUTFILE and you supply two arguments then the first argument will
go to INFILE and the second to OUTFILE. Arguments can be forced to a particular argument
position by prefixing it with the name of the position. Example:

    Myprogram OUTFILE mynewfile INFILE myoldfile

Or

    Myprogram OUTFILE=mynewfile INFILE=myoldfile

This is important especially in programs that can take multiple files in multiple
positions. In order for the program to know which position to put your arguments you may
have to prefix all arguments. For example of you had a program called imageprocess that
could take multiple files in for both images and palettes and you wanted to use a response
file it might need to look like this:

    INFILE=myimage1.tga
    INFILE=myimage2.tga
    INFILE=myimage3.tga
    PALETTE=mypalette1.pcx
    PALETTE=mypalette2.pcx
    PALETTE=mypalette3.pcx

And then you might run the program like this

    Imageprocess @response.arp myoutfile

## Notes about utilities and libraries

As we are usually using these libraries on someone else's behalf as well as ours
we expect that minor updates to them will not be considered proprietary to the company we
are working for at the time.

Expect us to update them from time to time though not often. Some examples, there are
still some defines and names that are too generic that need to be changed. For example
ERRORS should be changed to EL_ERRORS and GetLine() could be changed to EL_GetLine or
INI_GetLine etc. If we start using a new platform expect us to add support to these
libraries. If we run into another image format that we have to read/write expect it to get
added. Now that we are using C++ expect C++ classes for strings, lists, maps, arrays etc.
(unless someone can teach me STL :-)

## What are the benefits of proprietary utilities vs the costs?

Well that depends on what your goals are. For most game companies that goal should be
to make great games (and with the least amount of money). I worked for at least one
company that didn't take that goal into consideration when setting their utility
policy. They had an image processing utility. It ran from the command line and was script
driven and would convert images to several target formats with lots of various and useful
options specifically for games. Yet they would not let anybody outside of the company use
that tool including outside developers working for them. In their mind this utility was a
competitive advantage that they couldn't afford to let others see lest they develop
something similar. But wait a minute, name one game company that doesn't have image
processing tools? You can't! Why, because it's impossible to make a game without
them. Any and every game company will have image processing tools. If you hire an outside
developer who doesn't have them or doesn't have some that will generate data for
the target platform he will have to write them.

Now back to our goal. You've hired an outside developer and given him 9 months to
make a simple game for you. You have image processing tools. You have two choices: (1)
Make him write his own image processing tools. Result: He spends 1 month writing image
processing tools and 8 months making your game. (2) You let him use your tools. Result: He
gets 9 months to write the game meaning in case 2 the game will have 1 more month of
polish, or one more month of features or one more month of levels etc. Which policy meets
your company's goals better?

Now expand that to include other tools. Maybe your company has sound tools or a sound
library or level editing tools or 3D character exporting tools etc. Every tool/utility you
can supply gives your developer that much more time to spend on the actual game instead of
tools. Not giving him the tools means he had to write is own which means that not only did
you not protect any technology (since he now has the same tools even if you think yours
are cooler) but ultimately you screwed yourself by getting a worse game and hence worse
sales than you could have.

Now I'm not saying you should give your tools away publicly like we are here.
I'm saying that you should always think of what makes your bottom line: "I can
make the most money by creating great games." Now, every decision should be based on
will it make the game BETTER! If sharing your tools will make the game better then DO IT!
"Trade Secrets" do not help you make money if the way you make money is by
making GREAT games.

This partly explains why we made our libraries public. Our goals are also to make great
games. Since we work for various different companies, in order to spend more time making
games and less time writing utilities it's important that we be able to use our
existing libraries and utilities. If we kept them proprietary then we couldn't use
them. That would hurt nobody but us. Since it is not practical for us to market, sell and
support these tools, we are not losing money by making them free but we are allowing
ourselves to work more efficiently wherever we are.

## Instructions:

These are very brief and probably incomplete.

On any platform you install the libraries you will need to set the environment variable
ELIBS equal to the path to the directory of the libraries. Example: You installed them in
d:\work\elibs then

    set ELIBS=d:\work\elibs

In unix CSH (currently Irix only)

    setenv ELIBS ~/work/elibs

Every .c/.cpp/.h file must have at the top

    #include "platform.h"
    #include "switches.h"
    #include &lt;echidna\ensure.h&gt;

The header file "platform.h" in your local directory should look like this

    #include &lt;echidna/platform.h&gt;

There needs to be a switches.h in your local directory. I can be empty but you can use
it to set the following switches.


<table CELLSPACING="0" BORDER="0" CELLPADDING="7" WIDTH="568">
  <tr>
    <td WIDTH="33%" VALIGN="TOP">EL_NO_ENSURE</td>
    <td WIDTH="67%" VALIGN="TOP">There are a bunch of macros we call ENSURE
    macros (ensure.h). They are very similar to the ASSERT macro that C has but ours print
    more information and can be compiled out if this switch is set to 1</td>
  </tr>
  <tr>
    <td WIDTH="33%" VALIGN="TOP">EL_DEBUG_MESSAGES</td>
    <td WIDTH="67%" VALIGN="TOP">There are several macros for printing
    diagnostic messages (dbmess.h). Setting this to 1 will turn them on. Do not use these for
    printing errors for the user as they will print too much information depending on the
    settings.</td>
  </tr>
  <tr>
    <td WIDTH="33%" VALIGN="TOP">EL_USE_FUNC_NAMES</td>
    <td WIDTH="67%" VALIGN="TOP">The ENSURE macros have the option to print the
    current function by setting this to 1</td>
  </tr>
  <tr>
    <td WIDTH="33%" VALIGN="TOP">EL_CALLTRACE_ON</td>
    <td WIDTH="67%" VALIGN="TOP">The ENSURE macros have the ability to print
    the entire call stack if this is set to 1. You must use the BEGINFUNC/ ENDFUNC/ BEGINPROC/
    ENDPROC macros and you must use them consistently in order for this to work.</td>
  </tr>
  <tr>
    <td WIDTH="33%" VALIGN="TOP">EL_DEBUG_MEMORY</td>
    <td WIDTH="67%" VALIGN="TOP">The memory routines can print
    allocation/de-allocation info as well as attempt to track memory over writing, double
    freeing, etc if this is set to 1.</td>
  </tr>
  <tr>
    <td WIDTH="33%" VALIGN="TOP">EL_ERRORS_OFF</td>
    <td WIDTH="67%" VALIGN="TOP">Many routines use the error reporting
    mechanism in eerrors.h. Setting this to 1 turns the messages from those errors off.</td>
  </tr>
</table>

To build the libraries:

From Windows VC++ 5.0 load the project in &lt;installdir&gt;/lib/echidna and compile
it. You must have added &lt;installdir&gt;inc to your include path in the configuration
settings in VC++. To build any of the utilities you will need to also put
&lt;installdir&gt;/lib in your library path.

From an SGI go to the &lt;installdir&gt;/lib/echidna directory and type &#145;make
&#150;f makefile.sgi'

That's about it. The rest, you're on your own.

## License: (New-BSD)

Copyright (c) 2001-2008, Echidna

All rights reserved.

Redistribution and use in source and binary forms, with or
without modification, are permitted provided that the following
conditions are met:

*   Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

*   Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer
    in the documentation and/or other materials provided with the
    distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.


## Echidna is:

*   John Alvarado
*   Dan Chang
*   Greg Marquez
*   [Gregg Tavares](http://greggman.com/)

## Downloads:

The latest versions are available on github
[here](http://github.com/greggman/elibs/).
