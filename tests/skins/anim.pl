#!/usr/bin/perl

$initial = 0;
$increment = 4;
$count = 360 / $increment;
$count += 1;
#$count = 2;

while ($count > 0) {

   #/* update file */
   open(ORIG, "<ui.pov");
   open(SCRATCH, ">uitemp.pov");
   while(<ORIG>) {
       $line = $_;
       if ($line =~ /declare C_V_angle = /) {
           $line =~ s/0/$initial/;
       }
       if ($line =~ /declare C_H_angle = /) {
           $line =~ s/0/$initial/;
       }
       print SCRATCH $line;
   }
   close(ORIG);
   close(SCRATCH);

   `povray +w800 +h480 +FN +A -d -v -iuitemp.pov`;
   `pngtopnm uitemp.png > uitemp.pnm`;
   `pnmcrop uitemp.pnm > uitemp2.pnm`;
   if(defined ($runonce)) {
       `pnmcat -lr current.pnm uitemp2.pnm > current2.pnm`;
       `cp current2.pnm current.pnm`;
   } else {
       `cp uitemp2.pnm current.pnm`;
   }
   $runonce = TRUE;
   $initial += $increment;

   $count -= 1;
}
