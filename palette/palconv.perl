#!/usr/bin/perl

while(<>)
{
  chomp;
  ($r,$g,$b)= split(/ /);
  if (! defined ($g) || ! defined($b)){ print "$_\n"; next;};
 
  printf "%d %d %d\n",$r<<11,$g<<11,$b<<11;

}
