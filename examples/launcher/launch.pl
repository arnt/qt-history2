#!/usr/bin/perl

use POSIX ":sys_wait_h";

$file_qws = "../../util/qws/qws"; $args_qws="+k";
$file_launcher = "./launcher"; $args_launcher="";

#$silence="2>/dev/null >/dev/null";
$silence="2>/dev/null";

printf("\033[?25l\n"); # VT100 cursor off

system("killall $file_qws 2>/dev/null");

while ( 1 ) {
    if ( !$server ) {
	system("kill $client 2>/dev/null");
	system("kill -9 $client 2>/dev/null");
	unless ( $server = fork() ) {
	    exec "$file_qws $args_qws $silence";
	    $fail++;
	    print "Server exec failed\n";
	    exit 0;
	}
    }
    sleep 1;
    if ( !$client ) {
	unless ( $client = fork() ) {
	    exec "$file_launcher $args_launcher $silence";
	    $fail++;
	    print "Client exec failed\n";
	    exit 0;
	}
    }
    if ( $ARGV[0] eq "-1" ) {
	wait;
    } else {
	while ( $server && $client ) {
	    $dead=waitpid(-1,&WNOHANG);
	    $client = 0, $fail++ if $dead == $client;
	    $server = 0, $fail++ if $dead == $server;
	    sleep 5 if $dead == -1;
	}
    }
}
