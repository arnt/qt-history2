#include <qfile.h>
#include <qtextstream.h>
#include <qdatastream.h>
#include <stdlib.h>
#include <stdio.h>
#include <qbuffer.h>


void readLines(QIODevice& f)
{
    const int maxln=20;
    char s[maxln];
    int n = 1;
    while ( !f.status() ) {        // until past end of file...
	int prevat;

	int ch;
	prevat = f.at();
	ch = f.getch();         // character
	printf( "%03d->%03d %c (%o) -> %04x\n",
	    prevat, f.at(), ch, ch, f.status());

	prevat = f.at();
	f.readLine(s,maxln);       // line of text excluding '\n'
	printf( "%03d->%03d %3d: %s -> %04x\n",
	    prevat, f.at(), n++, s, f.status());
    }
}

void readTextLines(QIODevice& f)
{
    QTextStream t( &f );        // use a text stream
    QString s;
    int n = 1;
    while ( !t.eof() ) {        // until end of file...
	int prevat = f.at();
	s = t.readLine();       // line of text excluding '\n'
	printf( "%03d->%03d %3d: %s\n",
	    prevat, f.at(), n++, (const char *)s );
    }
    f.close();
}

int main()
{
    printf("Testing stdin...\n");
    {
	QFile file;
	file.open(IO_ReadOnly, stdin);
	if ( file.isSequentialAccess() )
	    printf("stdin is Sequential Access\n");
	if ( file.isDirectAccess() )
	    printf("stdin is Direct Access\n");
    }

    int r;

    for (r=1; r>=0; r--) {
	printf("Testing reading lines from %sfile...\n", r ? "raw " : "");
	QFile f("file.txt");
	if ( f.open((r?IO_Raw:0)|IO_ReadOnly) )     // file opened successfully
	    readTextLines(f);
    }

    for (r=0; r<=1; r++) {
	printf("Testing reading chars and lines from %sfile, to OVER eof...\n", r ? "raw " : "");
	QFile f("file.txt");
	if ( f.open((r?IO_Raw:0)|IO_ReadOnly) )     // file opened successfully
	    readLines(f);
    }

    printf("Testing reading chars and lines from buffer, to OVER eof...\n");
    {
	QFile f("file.txt");
	QBuffer b;
	b.open(IO_ReadWrite);
	char* blk = new char[f.size()];
	if ( f.open((r?IO_Raw:0)|IO_ReadOnly) ) {     // file opened successfully
	    f.readBlock(blk,f.size());
	    b.writeBlock(blk,f.size());
	    delete [] blk;
	    b.reset();
	    readLines(b);
	}
    }

    printf("Testing reading chars and lines from buffer...\n");
    {
	QFile f("file.txt");
	QBuffer b;
	b.open(IO_ReadWrite);
	char* blk = new char[f.size()];
	if ( f.open((r?IO_Raw:0)|IO_ReadOnly) ) {     // file opened successfully
	    f.readBlock(blk,f.size());
	    b.writeBlock(blk,f.size());
	    delete [] blk;
	    b.reset();
	    readTextLines(b);
	}
    }

    printf("Testing unsetDevice()...\n");
    {
          QFile tFile("empty.txt");
          tFile.open( IO_ReadOnly);
          QDataStream tStream(&tFile);
 
          printf("%d\n", tStream.eof());
          tStream.unsetDevice();
          printf("%d\n", tStream.eof());
          tFile.close();
          printf("%d\n", tStream.eof());
    }

    printf("Testing unsetDevice()...\n");
    {
          QFile tFile("file.txt");
          tFile.open( IO_ReadOnly);
          QDataStream tStream(&tFile);
 
          printf("%d\n", tStream.eof());
          tStream.unsetDevice();
          printf("%d\n", tStream.eof());
          tFile.close();
          printf("%d\n", tStream.eof());
    }
}
