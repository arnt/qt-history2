// showincs.C

#include <qstring.h>
#include <qfile.h>
#include <qlist.h>


QString includePath = 				// default include path
    "/home/popcorn/hanord/quasar/src/include";
//    "/usr/g++-include:"
//    "/usr/include:";

// --------------------------------------------------------------------------
// Returns the complete filename (including file path)
// The path argument is in this format "path1:path2:path3" etc.

QString getFilePath( const QString &fileName, const QString &path )
{
    QString p = (char*)path;			// duplicates path
    QString file;
    int n = 0;
    while ( TRUE ) {
	int m = p.find( ':', n );		// split up path
	if ( m >= 0 )
	    p[m] = '\0';
	file = (char*)&p[n];
	file += "/" + fileName;
	if ( QFile::exists(file) )		// does file exist
	    break;
	if ( m < 0 ) {				// tried all paths
	    file = 0;
	    break;
        }
	n = m + 1;				// more trials
    }
    return file;
}


// --------------------------------------------------------------------------
// Very simple parsing routines and its data
//

#include <qstrlist.h>
#include <qdict.h>

struct FileInfo {
    QString   name;				// file name
    bool      in;				// included file
    bool      flag;				// utility flag, set to FALSE
    QStrList *incl;				// list of include files
};

typedef declare(QDict,FileInfo) FilesDict;
FilesDict *fdict = 0;
QStrList  *flist = 0;

void addFile( const QString &fileName, bool isIncluded=FALSE )
{
    if ( fdict == 0 ) {				// create main dict and list
	fdict = new FilesDict(23,TRUE,FALSE);
	fdict->autoDelete( TRUE );
	flist = new QStrList;
    }
    FileInfo *f = fdict->find(fileName);	// already added?
    if ( !f ) {
	f = new FileInfo;			// create new node
	f->name = fileName;
	f->in = isIncluded;
	f->flag = FALSE;
	f->incl = new QStrList;
	fdict->insert( f->name, f );
	flist->append( f->name );
    }
}

void parse( const QString &fileName, bool local=TRUE )
{
    QString file;				// path + file name
    file = fileName;

    if ( !local )				// get full pathname
	file = getFilePath( file, includePath );

    if ( file.isNull() || !QFile::exists(file) )
	return;					// cannot find file

    addFile( fileName, TRUE );

    QString buf(512);
    FILE *fh = fopen( file, "r" );
    while ( !feof(fh) ) {
	QString buf(240);
	fgets( buf, 512, fh );
	if ( buf.find( "#include" ) == 0 ) {	// include statement
	    char *b = buf;
	    bool loc = TRUE;
	    while ( *b && *b != '<' && *b != '\"' )
		b++;
	    if ( *b++ == '<' )
		loc = FALSE;
	    char fn[80];
	    char *p = fn;
	    while ( (*p = *b) && *b != '>' && *b != '\"' ) {
		b++;
		p++;
	    }
	    *p = 0;
	    FileInfo *f = fdict->find( fn );
	    if ( !f )				// not parsed yet
		parse( fn, loc );		// parse this file
	    fdict->find( fileName )->incl->append( fn );
	}
    }
    fclose( fh );
}

void runIt()					// starts hunting #includes
{
    QStrListIterator i(*flist);
    int n = i.count();				// NOTE: files will be added
    while ( n-- )
	parse( i() );
}


// --------------------------------------------------------------------------
// Output functions: The result can be displayed in a hierarchy or flat
//

void printHier( QStrList *list=flist, int level=0 )
{
    QStrListIterator i(*list);
    while ( i ) {
	FileInfo *f = fdict->find( i.get() );
	QString indent;
	indent.fill( ' ', level*4 );
	if ( level > 0 || (level==0 && !f->in) )
	    printf( "%s%s\n", (pcchar)indent, i.get() );
	if ( !f->flag ) {
	    f->flag = TRUE;
	    printHier( f->incl, level+1 );
	}
	++i;
    }
}

void printFlat()
{
    QStrListIterator i(*flist);
    while ( i ) {
	FileInfo *f = fdict->find( i.get() );
	if ( !f->in ) {
	    printf( "%s\n", i.get() );
	    QStrListIterator j(*f->incl);
	    while ( j )
		printf( "    %s\n", j() );
	}
	++i;
    }
}


// --------------------------------------------------------------------------
// main() routine
//

int main( int argc, char **argv )
{
    bool flat = TRUE;
    bool wasFiles = FALSE;
    if ( argc < 2 )
	fatal( "Usage:\n\tshowincs [-Ipath] [-hf] files ..." );

    for ( int n=1; n<argc; n++ ) {
	QString s = argv[n];
	if ( s.find("-I") == 0 ) {		// include path
	    includePath += ":";
	    includePath += &s[2];
	}
	else
        if ( s.find("-h") == 0 )
	    flat = FALSE;
	else
        if ( s.find("-f") == 0 )
	    flat = TRUE;
	else {
	    wasFiles = TRUE;
	    addFile( s );
	}
    }
    if ( !wasFiles )
	fatal( "Hey! You forgot to specify some files" );
    runIt();					// starts looking for includes
    if ( flat )
	printFlat();				// print flat results
    else
	printHier( flist, 0 );			// print hierarchical results
    return 0;
}
