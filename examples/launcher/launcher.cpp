#include <qapplication.h>
#include <qvbox.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qimage.h>
#include <qtimer.h>
#include <qlistbox.h>
#include <qgroupbox.h>
#include <qwindowsystem_qws.h>

#include <stdlib.h>
#include <unistd.h>

struct {
    const char* label;
    const char* file;
} command[] = {
    { "Desktop-in-an-application", "( cd ../multidoc; exec ./multidoc )" },
    { "Info Kiosk - MPEGs", "( cd ../kiosk; exec ./kiosk)" },
    { "Help Text Browser", "exec ../helpviewer/helpviewer" },
    { "Canvas - alpha-blending", "( cd ../canvas; exec ./canvas )" },
    { "Text Editor", "exec ../qwerty/qwerty ../qwerty/unicode.txt" },
    { "Scribble Editor", "exec ../scribble/scribble" },
    { "Internationalization", "( cd ../i18n; exec ./i18n all )" },
    { "Magnifier", "exec ../qmag/qmag" },
    /*{ "Dumb Terminal", "exec ../dumbterm/dumbterm" },*/
    { 0, 0 }
};

struct {
    const char* label;
    const char* file;
} other_command[] = {
    { "aclock", "( cd ../aclock; exec ./aclock; )" },
    { "addressbook", "( cd ../addressbook; exec ./addressbook; )" },
    { "buttongroups", "( cd ../buttongroups; exec ./buttongroups; )" },
    { "checklists", "( cd ../checklists; exec ./checklists; )" },
    { "cursor", "( cd ../cursor; exec ./cursor; )" },
    { "customlayout", "( cd ../customlayout; exec ./customlayout; )" },
    { "dclock", "( cd ../dclock; exec ./dclock; )" },
    { "dirview", "( cd ../dirview; exec ./dirview; )" },
    //{ "drawdemo", "( cd ../drawdemo; exec ./drawdemo; )" },
    { "drawlines", "( cd ../drawlines; exec ./drawlines; )" },
    //{ "forever", "( cd ../forever; exec ./forever; )" },
    { "hello", "( cd ../hello; exec ./hello; )" },
    { "layout", "( cd ../layout; exec ./layout; )" },
    { "life", "( cd ../life; exec ./life; )" },
    { "lineedits", "( cd ../lineedits; exec ./lineedits; )" },
    { "listbox", "( cd ../listbox; exec ./listbox; )" },
    { "listboxcombo", "( cd ../listboxcombo; exec ./listboxcombo; )" },
    { "menu", "( cd ../menu; exec ./menu; )" },
    { "movies", "( cd ../movies; exec ./movies; )" },
    //{ "picture", "( cd ../picture; exec ./picture; )" },
    { "popup", "( cd ../popup; exec ./popup; )" },
    { "progress", "( cd ../progress; exec ./progress; )" },
    { "progressbar", "( cd ../progressbar; exec ./progressbar; )" },
    { "qfd", "( cd ../qfd; exec ./qfd; )" },
    { "rangecontrols", "( cd ../rangecontrols; exec ./rangecontrols; )" },
    { "richtext", "( cd ../richtext; exec ./richtext; )" },
    { "scrollview", "( cd ../scrollview; exec ./scrollview; )" },
    { "showimg", "( cd ../showimg; exec ./showimg; )" },
    //{ "sound", "( cd ../sound; exec ./sounds; )" },
    { "splitter", "( cd ../splitter; exec ./splitter; )" },
    { "tabdialog", "( cd ../tabdialog; exec ./tabdialog; )" },
    { "table", "( cd ../table; exec ./table; )" },
    { "tetrix", "( cd ../tetrix; exec ./tetrix; )" },
    { "tictac", "( cd ../tictac; exec ./tictac; )" },
    { "tooltip", "( cd ../tooltip; exec ./tooltip; )" },
    { "validator", "( cd ../validator; exec ./validator; )" },
    { "widgets", "( cd ../widgets; exec ./widgets; )" },
    { "wizard", "( cd ../wizard; exec ./wizard; )" },
    //{ "xform", "( cd ../xform; exec ./xform; )" },
    { 0, 0 }
};


class Launcher : public QHBox {
    Q_OBJECT
public:
    Launcher() :
	QHBox(0,0,WStyle_Tool | WStyle_Customize)
    {
	setSpacing(10);
	setMargin(10);
	QMimeSourceFactory::defaultFactory()
	    ->setImage("logo",QImage("qtlogo.gif"));
	QVBox* vb;
        vb = new QVBox(this);
	setStretchFactor(vb,1);
	QLabel* label = new QLabel(
	    "<center><img src=logo>"
	    "<h1><b>Qt/Embedded</b></h1>"
	    "</center>"
"This display is a simple Qt/Embedded launcher application, running directly
on the Linux console. The buttons and listbox
to the right invoke additional applications.<p><br><hr>"
		, vb);
	info = new QLabel(vb);
	info->setFont(QFont("smoothtimes",17));
	info->setBackgroundColor(white);
	info->setAlignment(AlignTop);
	nextInfo();
	QTimer* infotimer = new QTimer(this);
	connect(infotimer,SIGNAL(timeout()),this,SLOT(nextInfo()));
	infotimer->start(20000);
	setBackgroundColor(white);
	label->setBackgroundColor(white);

	vb = new QVBox(this);
	vb->setBackgroundColor(white);
	QPushButton *pb;
	int i;
	for (i=0; command[i].label; i++) {
	    pb = new QPushButton(command[i].label,vb,command[i].file);
	    connect(pb, SIGNAL(clicked()), this, SLOT(execute()));
	}
	QListBox *lb = new QListBox(vb);
	lb->setFont(QFont("smoothtimes",17));
	for (i=0; other_command[i].label; i++) {
	    lb->insertItem(other_command[i].label);
	}
	lb->setMaximumHeight(pb->height()*8);
	connect(lb, SIGNAL(highlighted(int)), this, SLOT(executeOther(int)));
	connect(lb, SIGNAL(selected(int)), this, SLOT(executeOther(int)));
	
	QHBox* hb = new QHBox(vb);
	hb->setBackgroundColor(white);
	pb = new QPushButton("Restart",hb);
	connect(pb, SIGNAL(clicked()), this, SLOT(quit3()));
	pb = new QPushButton("Quit",hb);
	connect(pb, SIGNAL(clicked()), qApp, SLOT(quit()));
	hb->setSpacing(10);
	hb->setFixedHeight(hb->sizeHint().height());
    }

private slots:
    void nextInfo()
    {
	static int i = 0;
	static const int ninfotext = 3;
	static const char* infotext[ninfotext] = {
"<h2>No X11 or MS-Windows</h2>
Since Qt/Embedded runs directly on the device display (such as the
Linux Console in this example), huge amounts of
RAM and ROM are saved on the embedded device.
<p>
<table border=1>
<tr><th><th>RAM<th>ROM
<tr><th>Qt/Embedded<td align=right>1M<td align=right>4M
<tr><th>X11<td align=right>8M<td align=right>8M
</table>
<p>
Qt applications are smaller because they share and reuse functionality
from the library. Without the added cost of the X11 server and client
libraries, this means a suite of Qt/Embedded applications is light
on memory.
",

"<h2>Faster Custom Graphics</h2>
Because clients have direct access to the video display, advanced graphics
operations are easily implemented. Qt/Embedded therefore has support for
features not available on X11, such as:
<ul>
 <li>Anti-aliased text
 <li>Alpha-blended pixmaps
 <li>Color cursors
</ul>
Client applications can make their own use of direct display access,
such as:
<ul>
 <li>MPEG directly to the display
 <li>Real-time games
</ul>
The Qt/Embedded graphics kernel is structured for adding 
hardware acceleration for additional devices.
",

"<h2>Scalability</h2>
Qt/Embedded has exactly the same API as Qt/X11 and Qt/Windows, so your
applications can be on X11, Windows, and the embedded device.
<p>
<ul>
<li>Develop on X11 or Windows.
<li>Make software demonstrations that anyone can run.
</ul>

<p>
Qt is highly modular, and so can be customized to smaller
devices. The minimal configuration is about 4M RAM and 4M ROM - including
applications, Linux and Qt/Embedded.
",
	};
	QString t=infotext[i];
	if ( i==1 && getenv("QWS_NOACCEL") )
	    t += " This display has acceleration turned off.";
	info->setText(
	    "<blockquote>"
	    +t+
	    "</blockquote>"
	);
	i = (i+1)%ninfotext;
    }

    void quit3()
    {
	qApp->exit(3);
    }

    void run(const char* cmd)
    {
	QString c = cmd;
	c += " 2>/dev/null >/dev/null &";
	system(c.latin1());
    }
    void execute()
    {
	run(sender()->name());
    }
    void executeOther(int i)
    {
	run(other_command[i].file);
    }

    void setAntiAliasedText(bool on)
    {
	if ( on ) {
	    unsetenv("QWS_NO_SMOOTH_FONTS");
	} else {
	    setenv("QWS_NO_SMOOTH_FONTS","1",1);
	}
    }
    void setAcceleration(bool on)
    {
	if ( on ) {
	    unsetenv("QWS_NOACCEL");
	} else {
	    setenv("QWS_NOACCEL","1",1);
	}
    }
private:
    QLabel* info;
};

#include "launcher.moc"




#include <qmap.h>
#include <qfile.h>
#include <qtextstream.h>

class SimpleIM : public QWSServer::KeyboardFilter
{
public:
    SimpleIM( const QString &fn );
    bool filter(int unicode, int modifiers, bool isPress,
		      bool autoRepeat);

private:
    bool active;
    int maxlen;
    QMap<QString,QString> map;
    QString soFar;
};




SimpleIM::SimpleIM( const QString &fn )
{
    active = FALSE;
    maxlen = 0;
    QFile f( fn );
    if ( f.open(IO_ReadOnly) ) {
	
	QTextStream t( &f );        // use a text stream
	QString s;
	while ( !t.eof() ) {        // until end of file...
	    s = t.readLine();       // line of text excluding '\n'
	    int p = s.find( '\t' );
	    if ( p >= -1 ) {
		QString s1 = s.left( p );
		QString s2 = s.mid( p+1 );
		map[s1] = s2;
		maxlen = QMAX( maxlen, s1.length() );
	    }
	}
	f.close();
    }
    qDebug( "maxlen = %d", maxlen );
}



static void fakeKey( int unicode )
{
    QWSServer::sendKeyEvent( unicode, 0, TRUE, FALSE );
    QWSServer::sendKeyEvent( unicode, 0, FALSE, FALSE );
}

static void fakeKey( const QString &s )
{
    for ( int i = 0; i < s.length(); i++ )
	fakeKey( s[i].unicode() );
}

bool SimpleIM::filter(int code, int modifiers, bool isPress,
		       bool autoRepeat)
{
    int unicode = code & 0xffff;
    if ( modifiers == Qt::ShiftButton && unicode  == ' ' ) {
	if ( !isPress ) {
	    active = !active;
	}
	return TRUE; //filter
    }
    if ( active  && !modifiers ) {
	if ( !isPress ) {
	    soFar = soFar + QChar( unicode );
	    soFar = soFar.right( maxlen );
	
	    int n = soFar.length();
	    while ( n > 0 ) {
		QString candidate = soFar.right( n );
		if ( map.contains( candidate ) ) {
		    for ( int i = 0; i < n; i++ )
			fakeKey( Qt::Key_Backspace << 16 );
		    fakeKey( map[candidate] );
		    return TRUE;
		}
		n--;
	    }
	}
	QWSServer::sendKeyEvent( code, modifiers, isPress, autoRepeat );
	return TRUE;
    }
    soFar = "";
    return FALSE;
}


void silent(QtMsgType, const char *)
{
}


main(int argc, char** argv)
{
    QApplication app(argc,argv, QApplication::GuiServer);

    qInstallMsgHandler(silent);

    app.setFont(QFont("smoothtimes",22));
    if ( QString(argv[1]) == "-im" && argv[2] ) {
	SimpleIM* im = new SimpleIM( argv[2] );
	QWSServer::setKeyboardFilter( im );
    }

    Launcher l;
    app.setMainWidget(&l);
    l.showMaximized();
    return app.exec();
}
