#!/usr/bin/perl
# -------------------------------------------------------------------
# Simple perl script to create the mkdist manifest file
# -------------------------------------------------------------------

# default values ----------------------------------------------------
#$exe          = "c:\\cygwin\\bin\\find.exe";
$exe            = "find";
$directory      = ".";
$manifestFile   = "manifest";
$manifestFilter = "manifest.filter";
$verbose        = 0;

# global variables --------------------------------------------------
$scriptname     = $0;
$scriptname     =~ s/(.*)\///; # basename
$keepCount      = 0;
$removeCount    = 0;

# these will be generated when loading when reading the filter file -
$removeExp      = '';
$keepExp        = '';

# functions ---------------------------------------------------------
sub showUsage
{
    print "$scriptname usage:\n";
    print "  -d, --directory <path>     Directory to create manifest for  (default: '$directory')\n";
    print "  -m, --manifest <filename>  Filename for manifest             (default: '$manifestFile')\n";
    print "  -f, --filter <filename>    Filename for manifest filter      (default: '$manifestFilter')\n";
    print "  -v, --verbose              Increase verbose output level     (default: $verbose)\n";
    print "                             Every -v will create more output\n";
    print "  -e, --exe                  Which find application to use     (default: '$exe')\n";
    print "  -h, --?, --help            This help\n";
}


sub parseArgs
{
    while ( @ARGV ) {
        $_ = shift @ARGV;
        if (s/^-+//) {
            if (/^h$/ || /^help$/ || /^\?$/) {
                showUsage();
                exit 0;
            } elsif (/^d$/ || /^directory$/) {
                $directory = shift @ARGV;
                $directory =~ s/\/+$//; # remove trailing /
            } elsif (/^m$/ || /^manifest$/) {
                $manifestFile = shift @ARGV;
            } elsif (/^f$/ || /^filter$/) {
                $manifestFilter = shift @ARGV;
            } elsif (/^e$/ || /^exe$/) {
                $exe = shift @ARGV;
            } elsif (/^v$/ || /^verbose$/) {
                $verbose++;
            } else {
                print "Unknown option: $_\n\n";
                showUsage();
                exit 1;
            }
        } else {
            print "Unknown parameter: $_\n\n";
            showUsage();
            exit 2;
        }
    }
}


# Loads the filter file, and puts Remove|Keep regexps into
#    @removeList
#    @keepList
sub loadFilter
{
    print "Loading filter file '$manifestFilter'.." if $verbose > 0;
    @removeList = ();
    @keepList   = ();

    $removeMode = 1;                      # it safest to remove by default
    open(FILT, "<$manifestFilter") || die "$0: Couldn't open $manifestFilter for reading $!\n";
    while (<FILT>)
    {
        chomp;                            # remove newlines
        s/#.*$//;                         #        comments
        s/^\s+//;                         #        leading whitespace
        s/\s+$//;                         #        trailing whitespace
        next if ($_ eq '');               # skip on blanks
        if ($_ eq '[REMOVE]') {
            $removeMode = 1;
            next;
        } elsif ($_ eq '[KEEP]') {
            $removeMode = 0;
            next;
        }
        if ($removeMode == 1) {
            @removeList = (@removeList, $_);
        } else {
            @keepList = (@keepList, $_);
        }
    }
    close(FILT);
    print "done\n" if $verbose > 0;

    # create single regular expressions of both removeList & keepList
    $removeCount = @removeList;
    $keepCount = @keepList;
    $removeExp .= '0';
    $removeExp .= '||' if ($removeCount > 0);
    $removeExp .= join("||", @removeList);
    $keepExp   .= '0';
    $keepExp   .= '||' if ($keepCount > 0);
    $keepExp   .= join("||", @keepList);
    print "  removeExp: $removeExp\n" if $verbose > 1;
    print "  keepExp  : $keepExp\n" if $verbose > 1;
}


# Returns 1 if the file matches the filter expression, otherwise 0
sub filterFile
{
    my($filterexp, $filepath, $path, $name) = @_;
    $_ = $filepath;             # Default matching
    $res = eval $filterexp;     # Do runtime evaluation
    die "Error  : $@\nPattern:\n$removeExp\n" if ($@);
    return $res ? 1 : 0;
}


# checks every file for match in removeExp, and keepExp
# if no match, ask user if it's supposed to be included in manifest
sub processFiles
{
    # search for all files, and open the manifest file
    open(FIND, "$exe $directory -xtype f |") || die "$0: Couldn't exec $exe $!\n";
    open(MANI, ">$manifestFile") || die "$0: Couldn't open $manifestFile for writing $!\n";

    # loop over all files found
    $count = 0;
    while (<FIND>)
    {
        $count++;
        chomp;
        $file = $_;
        if ( /(.*?)([^\/]+)$/ ) {
            $filepath = $_;               # full file path
            $path = $1;                   # path part
            $name = $2;                   # filename part
            $path     =~ s,$directory/,,; # remove $directory/ prefix
            $path     =~ s/\/+$//;        # remove trailing /
            $filepath =~ s,$directory/,,; # ditto -2
            $filepath =~ s/\/+$//;        # ditto -2

            $kept = 0;
            if (filterFile($removeExp, $filepath, $path, $name)) {
                $removeCount++;
            } elsif (filterFile($keepExp, $filepath, $path, $name)) {
                $keepCount++;
                $kept = 1;
            } else {
                print "'$filepath' not filtered, keep it? (yes or no): ";
                while(my $answer = <STDIN>) {
                    if ($answer =~ /^y(es)?$/i) {
                        $kept = 1;
                        $keepCount++;
                        last;
                    } elsif ($answer =~ /^n(o)?$/i) {
                        $kept = 0;
                        $removeCount++;
                        last;
                    } else {
                        print "uh, yes or no? ";
                    }
                }
            }

            # output file to manifest file, if kept
            print MANI "/$filepath\n" if $kept > 0;

            # verbose output
            print "$count:\tkept($kept)\tpath: $file\n"                 if $verbose > 2;
            print "--- filepath($filepath), path($path), name($name)\n" if $verbose > 3;
        }
    }

    # we're done, so close up!
    close(FIND);
    close(MANI);
}


# This is where it all happens ---------------------------------------------------------------------

parseArgs();
if ($directory =~ /^$/) {
    print "** Specify a directory to create a manifest for!\n";
    exit 0xDEAD;
}

loadFilter();

print "Directory      : '$directory'\nManifest       : '$manifestFile'\nVerbose level  : $verbose\n" if $verbose > 1;
print "> Creating manifest.. " if $verbose > 0;

processFiles();

if ($verbose >= 1) {
    print "done\n";
    $totalCount = $removeCount + $keepCount;
    print "Total files    : $totalCount\nKept           : $keepCount\nRemoved        : $removeCount\n";
}
