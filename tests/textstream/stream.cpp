#include <stdio.h>
#include <qlabel.h>
#include <qfile.h>
#include <qstring.h>
#include <qregexp.h>
#include <qapplication.h>
#include <qdatastream.h>
#include <qtextstream.h>
#include <qtextcodec.h>

#include <qmultilineedit.h>
#include <qhbox.h>

const char *str = "אנימנסהבימינו את KDE 3 BETA1, ויש לי בעיות עם זה, והייתי רוצה לשתף את";
const char *str2 = "אנימנסהבימינו";

main(int argc, char** argv)
{
    // Tests every QString function.
    int err=0;
    #define TEST(A,E) /*printf("%d\n",__LINE__);*/\
	if ( (A)!=(E) ) { err++; printf("TEST(%s,%s) failed at %d\n",#A,#E,__LINE__); }

	QString a;
	
	QString nonlatin = QString::fromUtf8( str );
	QString nonlatin2 = QString::fromUtf8( str2 );
    QByteArray ar;
    {
	QDataStream out(ar,IO_WriteOnly);
	out << QString("Test Text");
    }
    {
	QDataStream in(ar,IO_ReadOnly);
	in >> a;
	TEST(a,"Test Text");
    }
    {
	QTextStream out(ar,IO_WriteOnly);
	out << QString("This is Test Text");
    }
    {
	QTextStream in(ar,IO_ReadOnly);
	in >> a;
	TEST(a,"This");
    }
    {
	QTextStream out(ar,IO_WriteOnly);
	out.setEncoding( QTextStream::UnicodeUTF8 );
	out << QString("This is Test Text");
    }
    {
	QTextStream in(ar,IO_ReadOnly);
	in.setEncoding( QTextStream::UnicodeUTF8 );
	in >> a;
	TEST(a,"This");
    }
    {
	QTextStream out(ar,IO_WriteOnly);
	out.setEncoding( QTextStream::Unicode );
	out << QString("This is Test Text");
    }
    {
	QTextStream in(ar,IO_ReadOnly);
	in.setEncoding( QTextStream::Unicode );
	in >> a;
	TEST(a,"This");
    }
    {
	QTextStream out(ar,IO_WriteOnly);
	out.setEncoding( QTextStream::UnicodeReverse );
	out << QString("This is Test Text");
    }
    {
	QTextStream in(ar,IO_ReadOnly);
	in.setEncoding( QTextStream::UnicodeReverse );
	in >> a;
	TEST(a,"This");
    }
    {
	QTextStream out(ar,IO_WriteOnly);
	out.setCodec( QTextCodec::codecForName( "iso2022-jp" ) );
	out << QString("This is Test Text");
    }
    {
	QTextStream in(ar,IO_ReadOnly);
	in.setCodec( QTextCodec::codecForName( "iso2022-jp" ) );
	in >> a;
	TEST(a,"This");
    }

    // try with non latin1 string:
    {
	QTextStream out(ar,IO_WriteOnly);
	out.setEncoding( QTextStream::UnicodeUTF8 );
	out << nonlatin;
    }
    {
	QTextStream in(ar,IO_ReadOnly);
	in.setEncoding( QTextStream::UnicodeUTF8 );
	in >> a;
	TEST(a,nonlatin2);
    }
    {
	QTextStream out(ar,IO_WriteOnly);
	out.setEncoding( QTextStream::Unicode );
	out << nonlatin;
    }
    {
	QTextStream in(ar,IO_ReadOnly);
	in.setEncoding( QTextStream::Unicode );
	in >> a;
	TEST(a,nonlatin2);
    }
    {
	QTextStream out(ar,IO_WriteOnly);
	out.setEncoding( QTextStream::UnicodeReverse );
	out << nonlatin;
    }
    {
	QTextStream in(ar,IO_ReadOnly);
	in.setEncoding( QTextStream::UnicodeReverse );
	in >> a;
	TEST(a,nonlatin2);
    }
    {
	QTextStream out(ar,IO_WriteOnly);
	out.setCodec( QTextCodec::codecForName( "iso8859-8-i" ) );
	out << nonlatin;
    }
    {
	QTextStream in(ar,IO_ReadOnly);
	in.setCodec( QTextCodec::codecForName( "iso8859-8-i" ) );
	in >> a;
	TEST(a,nonlatin2);
    }

    {
	a="";
	QTextOStream ts( &a );
	ts << "pi = " << 3.125;
	TEST(a,"pi = 3.125");
    }

    {
	QString msg;
	QTextStream st(msg, IO_WriteOnly);
 	st << "Hello World" << endl;
	TEST( msg, "Hello World\n" );
    }

    {
	a="123 456";
	int x,y;
	QTextIStream( &a ) >> x >> y;
	TEST(x,123);
	TEST(y,456);
    }
    {
	QFile f("test.txt");
	f.open( IO_WriteOnly );
	QTextStream ts( &f );
	ts.setEncoding(QTextStream::Unicode);
	ts << "Abc";
    }

    printf("\n%d error%s\n",err,"s"+(err==1));


#if 1


    QApplication app(argc,argv);

    QFile f("stream.cpp");
    f.open( IO_ReadOnly );
    QTextStream ts( &f );

    QString s;
    while ( !ts.eof() ) {
	s += ts.readLine();
	if ( !ts.atEnd() )
	    s += "\n";
    }
#if 0
    for (int hi=0; hi<0xff; hi+=1) {
	for (int lo=0; lo<=0xff; lo+=1) {
	    s += QChar(lo,hi);
	}
	s += "\n";
    }
#endif
    QMultiLineEdit m(0);
    //QLabel m;
    m.setText(s);
    app.setMainWidget(&m);
    m.show();
    return app.exec();
#else
    argc = 0;
    argv = 0;
#endif
}

