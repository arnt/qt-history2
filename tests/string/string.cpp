#include <stdio.h>
#include <qlabel.h>
#include <qfile.h>
#include <qmultilinedit.h>
#include <qstring.h>
#include <qregexp.h>
#include <qapplication.h>
#include <qdatastream.h>
#include <qtextstream.h>

//#define USE_QCSTRING

#if QT_VERSION < 200
#define TESTING_1XSTRING
#else
#ifdef USE_QCSTRING
#define QString QCString
#define TESTING_1XSTRING
#endif
#endif

main(int argc, char** argv)
{
    // Tests every QString function.
    int err=0;
    #define TEST(A,E) /*printf("%d\n",__LINE__);*/\
	if ( (A)!=(E) ) { err++; printf("TEST(%s,%s) failed at %d\n",#A,#E,__LINE__); }

    // In a perfect world, these would all be defined, and QString would work.
    //
    #ifdef TESTING_1XSTRING
	//#define IMPLICIT
	//#define MAXLEN_EXCLUDES_NULL
	//#define RELATIONS_WORK
	//#define TOSHORT_WORKS
	//#define TOUINT_WORKS
	//#define SAFE_INDEXING
	//#define SELF_INSERT_WORKS
	//#define FINDREV_INDEX_CHECK
    #else
	#define IMPLICIT
	//#define MAXLEN_EXCLUDES_NULL   // Needs discussion
	#define RELATIONS_WORK
	#define TOSHORT_WORKS          // Needs discussion
	#define TOUINT_WORKS           // Needs discussion
	#define SAFE_INDEXING
	#define SELF_INSERT_WORKS
	//#define FINDREV_INDEX_CHECK
    #endif

    #ifndef IMPLICIT
	printf("WARNING: Testing assuming EXPLICIT SHARING\n");
    #endif
    #ifndef MAXLEN_EXCLUDES_NULL
	printf("WARNING: Assuming (char*,size) constructor includes \\0 in size (as in Qt 1.x)\n");
	// Let's just SCRAP this constructor - it's TOO DANGEROUS, as it acts
	// as a CAST from int!
	// No it doesn't - you're thinking of the QString( int ) constructor,
	// which _is_ evil.
    #endif
    #ifndef RELATIONS_WORK
	printf("WARNING: Not testing <, <=, etc, because they don't compile\n");
    #endif
    #ifndef TOSHORT_WORKS
	printf("WARNING: Expecting toShort() to return broken result, not 0\n");
    #endif
    #ifndef TOUINT_WORKS
	printf("WARNING: Expecting toUInt() to give broken result for big valid uints\n");
    #endif
    #ifndef SAFE_INDEXING
	printf("WARNING: Not indexing outside allocated space - it's broken\n");
    #endif
    #ifndef SELF_INSERT_WORKS
	printf("WARNING: Not inserting into self - it's broken\n");
    #endif
    #ifndef FINDREV_INDEX_CHECK
	printf("WARNING: Expecting findRev('c',length()) to succeed (as in Qt 1.x)\n");
    #endif

    QString a;

    QString b(10);
    QString bb((int)0);
    QString c("String C");
    #ifdef MAXLEN_EXCLUDES_NULL
	QString d("String D[last bit should not be seen]",8);
    #else
	QString d("String D[last bit should not be seen]",9);
    #endif
    QString ca(a);
    QString cb(b);
    QString cc(c);
    QString n;

    TEST(a.isNull(),TRUE)
    TEST(b.isNull(),FALSE)
    TEST(bb.isNull(),TRUE)
    TEST(a.isEmpty(),TRUE)
    TEST(b.isEmpty(),TRUE)
    TEST(bb.isEmpty(),TRUE)
    TEST(c.isEmpty(),FALSE)
    TEST(a.length(),0)
    TEST(b.length(),0)
    TEST(c.length(),8)
    TEST(d.length(),8)
    TEST(a,ca)
    TEST(b,cb)
    TEST(c,cc)
    TEST(d,"String D")

    QString e("String E");
    QString f;
    f = e;
    f[7]='F';
    #ifdef IMPLICIT
	TEST(e,"String E")
    #else
	TEST(e,"String F")
    #endif
    char text[]="String f";
    f = text;
    text[7]='!';
    TEST(f,"String f")
    f[7]='F';
    TEST(text[7],'!')

    f.resize(4);
    TEST(f,"Str")
    e.truncate(4);
    TEST(e,"Stri")
    e.fill('e',1);
    TEST(e,"e")
    f.fill('f',3);
    TEST(f,"fff")
    f.fill('F');
    TEST(f,"FFF");

    e = "String E";
    e.truncate(0);
    TEST(e,"")
    TEST(e.isEmpty(),TRUE)
    TEST(e.isNull(),FALSE)
    e = "String E";
    QString ce = e.copy();
    e = "XXX";
    TEST(ce,"String E")
    QString j;
    j.simplifyWhiteSpace();

    a.sprintf(0);
    TEST(a.isNull(),FALSE)   // I thought it would be TRUE
    a.sprintf("Test");
    TEST(a,"Test")
    a.sprintf("%%%d",1);
    TEST(a,"%1")
    TEST(a.sprintf("X%dY",2),"X2Y")
    TEST(a.sprintf("X%sY","hello"),"XhelloY");
    TEST(a.sprintf("X%9sY","hello"),"X    helloY");
    TEST(a.sprintf("X%9iY", 50000 ),"X    50000Y");
    TEST(a.sprintf("X%-9sY","hello"),"Xhello    Y");
    TEST(a.sprintf("X%-9iY", 50000 ),"X50000    Y");
    //QString fmt("X%-10SY");
    //QString txt("hello");
    //TEST(a.sprintf(fmt,&txt),"X     helloY");

    a="";
    #ifdef SAFE_INDEXING
	a[0]='A';
	TEST(a,"A");
	TEST(a.length(),1);
	a[1]='B';
	TEST(a,"AB");
	TEST(a.length(),2);
	a[2]='C';
	TEST(a,"ABC");
	TEST(a.length(),3);
	a = QString();
	TEST(a.isNull(),TRUE);
	a[0]='A';
	TEST(a,"A");
	TEST(a.length(),1);
	a[1]='B';
	TEST(a,"AB");
	TEST(a.length(),2);
	a[2]='C';
	TEST(a,"ABC");
	TEST(a.length(),3);
    #endif

    a="";
    TEST(a.find('A'),-1)
    a=QString();
    TEST(a.find('A'),-1)
    a="ABCDEFGHIEfGEFG"; // 15 chars
    TEST(a.find('A'),0)
    TEST(a.find('C'),2)
    TEST(a.find('Z'),-1)
    TEST(a.find('E'),4)
    TEST(a.find('E',4),4)
    TEST(a.find('E',5),9)
    //TEST(a.find('G',-1),14)    // -ve does what?  Parameter should be uint?
    //TEST(a.find('G',-2),11)    // -ve does what?  Parameter should be uint?
    TEST(a.find('f'),10)
    TEST(a.find('f',0,FALSE),5)
    TEST(a.find('g',0,TRUE),-1)
    TEST(a.find("fgh",0,FALSE),5)
    TEST(a.find("fgh",0,TRUE),-1)
    //TEST(a.find("efg",-1,FALSE),12)    // -ve does what?  Parameter should be uint?
    //TEST(a.find("efg",-2,FALSE),12)    // -ve does what?  Parameter should be uint?
    //TEST(a.find("efg",-3,FALSE),12)    // -ve does what?  Parameter should be uint?
    //TEST(a.find("efg",-4,FALSE),9)    // -ve does what?  Parameter should be uint?
    TEST(a.find(QRegExp("[EFG][EFG]"),0),4)
    TEST(a.find(QRegExp("[EFG][EFG]"),4),4)
    TEST(a.find(QRegExp("[EFG][EFG]"),5),5);
    TEST(a.find(QRegExp("[EFG][EFG]"),6),11);
    TEST(a.find(QRegExp("G"),14),14);

    TEST(a.findRev('G'),14)
    TEST(a.findRev('G',14),14)
    TEST(a.findRev('G',13),11)
    TEST(a.findRev('B'),1)
    TEST(a.findRev('B',1),1)
    TEST(a.findRev('B',0),-1)
    TEST(a.findRev("efg",99,FALSE),-1)
#ifdef FINDREV_INDEX_CHECK
    TEST(a.findRev("efg",15,FALSE),-1)
#else
    TEST(a.findRev("efg",15,FALSE),12)
#endif
    TEST(a.findRev("efg",16,FALSE),-1)
    TEST(a.findRev("efg",14,FALSE),12)
    TEST(a.findRev("efg",12,FALSE),12)
    TEST(a.findRev("efg",11,FALSE),9)
    TEST(a.findRev(QRegExp("[EFG][EFG]"),14),13)
    TEST(a.findRev(QRegExp("[EFG][EFG]"),11),11)

    a="ABCDEFGHIEfGEFG"; // 15 chars
    TEST(a.contains('A'),1);
    TEST(a.contains('Z'),0);
    TEST(a.contains('E'),3);
    TEST(a.contains('F'),2);
    TEST(a.contains('F',FALSE),3);
    TEST(a.contains("FG"),2);
    TEST(a.contains("FG",FALSE),3);
    TEST(a.contains(QRegExp("[FG][HI]")),1);
    TEST(a.contains(QRegExp("[G][HE]")),2);

    TEST(a.left(3),"ABC");
    TEST(a.right(3),"EFG");
    TEST(a.mid(3,3),"DEF");
    TEST(a.left(0).isNull(),FALSE);
    TEST(a.left(0),"");
    TEST(a.right(0),"");
    TEST(a.mid(0,0),"");
    TEST(a.mid(9999,0).isNull(),TRUE); // I expected "" (ie. FALSE)
    TEST(a.mid(9999,1).isNull(),TRUE); // I expected "" (ie. FALSE)

    TEST(n.left(3).isNull(),TRUE);
    TEST(n.right(3).isNull(),TRUE);
    TEST(n.mid(3,3).isNull(),TRUE);
    TEST(n.left(0).isNull(),TRUE);
    TEST(n.left(0).isNull(),TRUE);
    TEST(n.right(0).isNull(),TRUE);
    TEST(n.mid(0,0).isNull(),TRUE);
    TEST(n.mid(9999,0).isNull(),TRUE);
    TEST(n.mid(9999,1).isNull(),TRUE);

    a="ABC";
    TEST(a.leftJustify(5,'-'),"ABC--");
    TEST(a.leftJustify(4,'-'),"ABC-");
    TEST(a.leftJustify(4),"ABC ");
    TEST(a.leftJustify(3),"ABC");
    TEST(a.leftJustify(2),"ABC");
    TEST(a.leftJustify(1),"ABC");
    TEST(a.leftJustify(0),"ABC");
    TEST(n.leftJustify(3).isNull(),FALSE);    // I expected TRUE
    TEST(a.leftJustify(4,' ',TRUE),"ABC ");
    TEST(a.leftJustify(3,' ',TRUE),"ABC");
    TEST(a.leftJustify(2,' ',TRUE),"AB");
    TEST(a.leftJustify(1,' ',TRUE),"A");
    TEST(a.leftJustify(0,' ',TRUE),"");

    TEST(a.rightJustify(5,'-'),"--ABC");
    TEST(a.rightJustify(4,'-'),"-ABC");
    TEST(a.rightJustify(4)," ABC");
    TEST(a.rightJustify(3),"ABC");
    TEST(a.rightJustify(2),"ABC");
    TEST(a.rightJustify(1),"ABC");
    TEST(a.rightJustify(0),"ABC");
    TEST(n.rightJustify(3).isNull(),FALSE);  // I expected TRUE
    TEST(a.rightJustify(4,'-',TRUE),"-ABC");
    TEST(a.rightJustify(4,' ',TRUE)," ABC");
    TEST(a.rightJustify(3,' ',TRUE),"ABC");
    TEST(a.rightJustify(2,' ',TRUE),"AB");
    TEST(a.rightJustify(1,' ',TRUE),"A");
    TEST(a.rightJustify(0,' ',TRUE),"");
    TEST(a,"ABC");

    a="Text";
    TEST(a.upper(),"TEXT");
    TEST(a.lower(),"text");
    TEST(a,"Text");
    TEST(a.stripWhiteSpace(),"Text");
    TEST(a,"Text");
    a=" ";
    TEST(a.stripWhiteSpace(),"");
    TEST(a," ");
    a=" a   ";
    TEST(a.stripWhiteSpace(),"a");
    TEST(a.simplifyWhiteSpace(),"a");
    a=" a   b ";
    TEST(a.simplifyWhiteSpace(),"a b");
    
    a = "Ys";
    TEST(a.insert(1,'e'),"Yes")
    TEST(a.insert(3,'!'),"Yes!")
    TEST(a.insert(5,'?'),"Yes! ?")
    
    a="ABC";
    TEST(a.insert(5,"DEF"),"ABC  DEF");
    a="ABC";
    TEST(a.insert(0,"ABC"),"ABCABC");
    TEST(a,"ABCABC");
    #ifdef SELF_INSERT_WORKS
	TEST(a.insert(0,a),"ABCABCABCABC");
    #else
	a="ABCABCABCABC";
    #endif
	
    TEST(a,"ABCABCABCABC");
    TEST(a.insert(0,'<'),"<ABCABCABCABC");
    TEST(a.insert(1,'>'),"<>ABCABCABCABC");
    TEST(a.append(">"),"<>ABCABCABCABC>");
    TEST(a.prepend("-"),"-<>ABCABCABCABC>");
    TEST(a.remove(0,3),"ABCABCABCABC>");
    TEST(a.remove(1,4),"ACABCABC>");
    TEST(a.remove(999,4),"ACABCABC>");
    TEST(a.remove(9,4),"ACABCABC>");
    TEST(a.remove(8,4),"ACABCABC");
    TEST(a.remove(7,1),"ACABCAB");
    TEST(a.remove(4,0),"ACABCAB");
    TEST(a.replace(4,0,"X"),"ACABXCAB");
    TEST(a.replace(4,1,"Y"),"ACABYCAB");
    TEST(a.replace(4,1,""),"ACABCAB");
    TEST(a.replace(0,9999,"XX"),"XX");
    TEST(a.replace(0,9999,""),"");
    a="ABC";
    TEST(a.replace(1,9999,0),"A");  // Rather unexpected
    TEST(a.append(">>"),"A>>");
    TEST(a.prepend("<["),"<[A>>");
    a="123";
    b="456";
    a[0]=a[1];
    TEST(a,"223");
    a[1]=b[1];
    TEST(b,"456");
    TEST(a,"253");

    bool ok;
    a="TEST";
    TEST(a.toShort(&ok),0);
    TEST(ok,FALSE);
    a="123";
    TEST(a.toShort(&ok),123);
    TEST(ok,TRUE);
    a="123A";
    TEST(a.toShort(&ok),0); // I expected partial value (123), like atoi()
    TEST(ok,FALSE);
    a="1234567";
    #ifdef TOSHORT_WORKS
	TEST(a.toShort(&ok),0);
    #else
	TEST(a.toShort(&ok),-10617);  // This is SURELY an error
    #endif
    TEST(ok,FALSE);

    a="TEST";
    TEST(a.toInt(&ok),0);
    TEST(ok,FALSE);
    a="123";
    TEST(a.toInt(),123);
    TEST(a.toInt(&ok),123);
    TEST(ok,TRUE);
    a="123A";
    TEST(a.toInt(&ok),0);
    TEST(ok,FALSE);
    a="1234567";
    TEST(a.toInt(&ok),1234567);
    TEST(ok,TRUE);
    a="12345678901234";
    TEST(a.toInt(&ok),0);
    TEST(ok,FALSE);
    a="3234567890";
    TEST(a.toInt(&ok),0);
    TEST(ok,FALSE);
    #ifdef TOUINT_WORKS
	a="3234567890";
	TEST(a.toUInt(&ok),3234567890UL);
	TEST(ok,TRUE);
    #else
	a="3234567890";
	TEST(a.toUInt(&ok),0);
	TEST(ok,FALSE);
    #endif
    TEST(a.toULong(&ok),3234567890UL);
    TEST(ok,TRUE);
    TEST(a.toLong(&ok),0);
    TEST(ok,FALSE);

    a="0.000000000931322574615478515625";
    TEST(a.toFloat(&ok),(float)(0.000000000931322574615478515625));
    TEST(ok,TRUE);
    TEST(a.toDouble(&ok),(double)(0.000000000931322574615478515625));
    TEST(ok,TRUE);

    char t[]="TEXT";
    a="A";
    a.setStr(t);
    TEST(a,"TEXT");
    TEST(a,t);
    a[0]='X';
    TEST(a,"XEXT");
    TEST(t[0],'T');
    t[0]='Z';
    TEST(a,"XEXT");
    TEST(a.setNum(123),"123");
    TEST(a.setNum((short)123),"123");
    TEST(a.setNum(123UL),"123");
    TEST(a.setNum(123UL),"123");
    TEST(a.setNum(0.000000000931322574615478515625),"9.31323e-10");
    TEST(a.setNum(0.000000000931322574615478515625,'g',30),"9.31322574615478515625e-10");
    TEST(a.setNum(0.000000000931322574615478515625,'f',30),"0.000000000931322574615478515625");

    a="ABC";
    a.setExpand(0,'X');
    TEST(a,"XBC");
    a.setExpand(4,'Z');
    TEST(a,"XBC Z");
    a.setExpand(3,'Y');
    TEST(a,"XBCYZ");

    a="ABC";
    TEST(((const char*)a)[1],'B');
    TEST(strcmp(a,"ABC"),0);
    TEST(a+="DEF","ABCDEF");
    TEST(a+='G',"ABCDEFG");
    TEST(a+=((const char*)(0)),"ABCDEFG");

    // non-member operators

    a="ABC";
    b="ABC";
    c="ACB";
    d="ABCD";
    TEST(a==b,1);
    TEST(a==d,0);
    TEST(a!=b,0);
    TEST(a!=d,1);
    #ifdef RELATIONS_WORK
	TEST(a<b,0);
	TEST(a<c,1);
	TEST(a<d,1);
	TEST(d<a,0);
	TEST(c<a,0);
	TEST(a<=b,1);
	TEST(a<=d,1);
	TEST(a<=c,1);
	TEST(c<=a,0);
	TEST(d<=a,0);
    #endif
    TEST(a+b,"ABCABC");
    TEST(a+"XXXX","ABCXXXX");
    TEST(a+'X',"ABCX");
    TEST("XXXX"+a,"XXXXABC");
    TEST('X'+a,"XABC");

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
#ifndef TESTING_1XSTRING
	out << QString("This is Test Text");
#else
	out << (const char*)QString("This is Test Text");
#endif
    }
    {
	QTextStream in(ar,IO_ReadOnly);
	in >> a;
	TEST(a,"This");
    }
#if QT_VERSION >= 200
    {
	a="";
	QTextOStream ts(a);
	ts << "pi = " << 3.125;
	TEST(a,"pi = 3.125");
    }
    {
	a="123 456";
	int x,y;
	QTextIStream(a) >> x >> y;
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
#endif

    printf("\n%d error%s\n",err,"s"+(err==1));


#if 0 //QT_VERSION >= 200

    QApplication app(argc,argv);

    QString s;
    for (int hi=0; hi<0xff; hi+=1) {
	for (int lo=0; lo<=0xff; lo+=1) {
	    s += QChar(lo,hi);
	}
	s += "\n";
    }

    QMultiLineEdit m;
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
