#! /usr/bin/perl
## crc5 nrzstream
## e.g.  crc5 1000111
## nrz stream is sent in left to right order
## generated crc should also be sent out in left to right order
sub xor5 {
   local(@x) = @_[0..4];
   local(@y) = @_[5..9];
   local(@results5) = ();
   for($j=0;$j<5;$j++) 
{
      if (shift(@x) eq shift(@y)) { push(@results5, '0'); }
      else { push(@results5, '1'); }
}
   return(@results5[0..4]);
}
{
   local($st_data) = $ARGV[0];
   local(@G) =    ('0','0','1','0','1');
   local(@data) = split (//,$st_data);
   local(@hold) = ('1','1','1','1','1');
   if (scalar(@data) > 0) 
{
   loop5: while (scalar(@data) > 0) 
{

$nextb=shift(@data);
if (($nextb ne "0") && ($nextb ne "1")) {next loop5} ## comment
if ($nextb eq shift(@hold)) {push(@hold, '0')}
else 
{ push(@hold, '0'); @hold = &xor5(@hold,@G); }
##   print (@hold); print "\n";
}
}
##   print (@hold); print "\n";
## invert shift reg contents to generate crc field
for ($i=0;$i<=$#hold;$i++) {if (@hold[$i] eq "1") {print("0")} else {
print("1")} }
print "\n";
}
