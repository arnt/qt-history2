@rem = '--*-PERL-*--';
@rem = '
@echo off
rem setlocal
set ARGS=
:loopA
if .%1==. goto endloopA
set ARGS=%ARGS% %1
shift
goto loopA
:endloopA
rem ***** This assumes PERL is in the PATH *****
perl.exe -S pl2bat.bat %ARGS%
goto endofperl
@rem ';


$tail = "__END__\n:endofperl\n";

if ( @ARGV ) {
   LOOP:
    foreach ( @ARGV ) {
	open( FILE, $_ );
	@file = <FILE>;
	if ( grep( /:endofperl/, @file ) ) {
	    warn "$_ has already been converted to a batch file!!\n";
	    next LOOP;
	}            
	close( FILE, $_ );
	s/\.pl//i;
	s/\.bat//i;
	open( FILE, ">$_.bat" );

print FILE <<"--end--";
\@rem = '--*-PERL-*--';
\@rem = '
\@echo off
rem setlocal
set ARGS=
:loop
if .%1==. goto endloop
set ARGS=%ARGS% %1
shift
goto loop
:endloop
rem ***** This assumes PERL is in the PATH *****
perl.exe -S $_.bat %ARGS%
goto endofperl
\@rem ';
--end--
		print FILE @file, $tail;

	}
}

close FILE;
 

__END__
:endofperl
