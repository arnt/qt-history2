#include <stdio.h>
#include <qlabel.h>
#include <qfile.h>
#include <qstring.h>
#include <qregexp.h>
#include <qapplication.h>
#include <qdatastream.h>
#include <qtextstream.h>

#include <qmultilineedit.h>
#include <qhbox.h>


main(int argc, char** argv)
{
    // Tests every QString function.
    int err=0;
    #define TEST(A,E) /*printf("%d\n",__LINE__);*/\
	if ( (A)!=(E) ) { err++; printf("TEST(%s,%s) failed at %d\n",#A,#E,__LINE__); }

	QString a;
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

