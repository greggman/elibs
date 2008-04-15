#  ************************************************************************
#
#                                NOCASEHASH.PM
#
#  ************************************************************************
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
package Tie::NoCaseHash;
use strict;
use Tie::Hash;
use vars qw(@ISA);
@ISA = qw(Tie::StdHash);
sub STORE {
    my ($self, $key, $value) = @_;
    return $self->{lc $key} = $value;
    } 
sub FETCH {
    my ($self, $key) = @_;
    return $self->{lc $key};
} 
sub EXISTS {
    my ($self, $key) = @_;
    return exists $self->{lc $key};
} 
sub DEFINED {
    my ($self, $key) = @_;
    return defined $self->{lc $key};
} 
1;
