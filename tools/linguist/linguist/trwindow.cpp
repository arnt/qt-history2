/**********************************************************************
**   Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
**   trwindow.cpp
**
**   This file is part of Qt Linguist.
**
**   See the file LICENSE included in the distribution for the usage
**   and distribution terms.
**
**   The file is provided AS IS with NO WARRANTY OF ANY KIND,
**   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR
**   A PARTICULAR PURPOSE.
**
**********************************************************************/

/*  TRANSLATOR TrWindow

  This is the application's main window.
*/

#include "trwindow.h"
#include "listviews.h"
#include "finddialog.h"
#include "msgedit.h"
#include "phrasebookbox.h"
#include "printout.h"

#include <qaccel.h>
#include <qaction.h>
#include <qapplication.h>
#include <qdict.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qstatusbar.h>
#include <qtextbrowser.h>
#include <qtextstream.h>
#include <qtoolbar.h>
#include <qwhatsthis.h>
#include <qsettings.h>
#include <qfontdialog.h>
#include <qheader.h>

#include <qdockarea.h>
#include <qdockwindow.h>
#include <qbitmap.h>
#include "phraselv.h"

#include <images.h>

static const char *logo_xpm[] = {
/* width height num_colors chars_per_pixel */
"21 16 213 2",
"   c white",
".  c #A3C511",
"+  c #A2C511",
"@  c #A2C611",
"#  c #A2C510",
"$  c #A2C513",
"%  c #A2C412",
"&  c #A2C413",
"*  c #A2C414",
"=  c #A2C515",
"-  c #A2C50F",
";  c #A3C510",
">  c #A2C410",
",  c #A2C411",
"'  c #A2C314",
")  c #A2C316",
"!  c #A2C416",
"~  c #A0C315",
"{  c #A1C313",
"]  c #A1C412",
"^  c #A2C40F",
"/  c #A1C410",
"(  c #A0C510",
"_  c #A0C511",
":  c #A1C414",
"<  c #9FC30E",
"[  c #98B51B",
"}  c #5F7609",
"|  c #5C6E0E",
"1  c #5B6E10",
"2  c #5C6C14",
"3  c #5A6E0A",
"4  c #839E16",
"5  c #A0C515",
"6  c #A0C513",
"7  c #A2C512",
"8  c #A1C512",
"9  c #A1C511",
"0  c #A1C50F",
"a  c #91AE12",
"b  c #505E11",
"c  c #1F2213",
"d  c #070606",
"e  c #040204",
"f  c #040306",
"g  c #15160F",
"h  c #2F3A0D",
"i  c #859F1B",
"j  c #A1C215",
"k  c #A0C50F",
"l  c #A1C510",
"m  c #A0C110",
"n  c #839C1B",
"o  c #1E240A",
"p  c #050205",
"q  c #030304",
"r  c #323917",
"s  c #556313",
"t  c #56680B",
"u  c #536609",
"v  c #4A561B",
"w  c #0B0D04",
"x  c #030208",
"y  c #090A05",
"z  c #5F6F18",
"A  c #A0C117",
"B  c #91AF10",
"C  c #1E2209",
"D  c #030205",
"E  c #17190D",
"F  c #7D981C",
"G  c #9ABA12",
"H  c #A3C411",
"I  c #A3C713",
"J  c #95B717",
"K  c #7F9A18",
"L  c #8FAE1B",
"M  c #394413",
"N  c #040305",
"O  c #090807",
"P  c #6C7E19",
"Q  c #A6C614",
"R  c #A1C411",
"S  c #64761F",
"T  c #030105",
"U  c #070707",
"V  c #728513",
"W  c #A2C40C",
"X  c #A2C70B",
"Y  c #89A519",
"Z  c #313B11",
"`  c #101409",
" . c #586A19",
".. c #97B620",
"+. c #1B2207",
"@. c #282D11",
"#. c #A6C41B",
"$. c #A1C413",
"%. c #A3C512",
"&. c #2E370B",
"*. c #030108",
"=. c #21260F",
"-. c #A5C21A",
";. c #A0C60D",
">. c #6D841A",
",. c #0F1007",
"'. c #040207",
"). c #0E1009",
"!. c #515F14",
"~. c #A2C41B",
"{. c #5E701B",
"]. c #030203",
"^. c #0B0B04",
"/. c #87A111",
"(. c #A0C411",
"_. c #A0C316",
":. c #212907",
"<. c #222C0B",
"[. c #A3C516",
"}. c #9CBE1A",
"|. c #5E6F1B",
"1. c #0E0F0B",
"2. c #040205",
"3. c #181B0D",
"4. c #93AE25",
"5. c #A0C610",
"6. c #617715",
"7. c #030306",
"8. c #070704",
"9. c #809818",
"0. c #A1C415",
"a. c #475416",
"b. c #030309",
"c. c #12170B",
"d. c #91B01E",
"e. c #5C721F",
"f. c #05050B",
"g. c #33371D",
"h. c #0E0F08",
"i. c #040405",
"j. c #758921",
"k. c #46511B",
"l. c #030207",
"m. c #131409",
"n. c #9FB921",
"o. c #859D21",
"p. c #080809",
"q. c #030305",
"r. c #46521C",
"s. c #8EB017",
"t. c #627713",
"u. c #4D5F17",
"v. c #97B71D",
"w. c #77901D",
"x. c #151708",
"y. c #0D0D0B",
"z. c #0C0B08",
"A. c #455216",
"B. c #A5C616",
"C. c #A0C114",
"D. c #556118",
"E. c #050307",
"F. c #050407",
"G. c #363E17",
"H. c #5D7309",
"I. c #A2BF28",
"J. c #A2C417",
"K. c #A4C620",
"L. c #60701D",
"M. c #030103",
"N. c #030303",
"O. c #809A1B",
"P. c #A0C310",
"Q. c #A0C410",
"R. c #A3C415",
"S. c #9CB913",
"T. c #6F801F",
"U. c #1A210A",
"V. c #1D1E0D",
"W. c #1D220F",
"X. c #1E210F",
"Y. c #0F0F07",
"Z. c #0E1007",
"`. c #090906",
" + c #2B360E",
".+ c #97B813",
"++ c #A2C50E",
"@+ c #A5C517",
"#+ c #90AD20",
"$+ c #5D6C1A",
"%+ c #394115",
"&+ c #050704",
"*+ c #040304",
"=+ c #202807",
"-+ c #5E6B21",
";+ c #728D0C",
">+ c #65791D",
",+ c #29330F",
"'+ c #7A911D",
")+ c #A2C614",
"!+ c #A1C513",
"~+ c #A3C50E",
"{+ c #A3C414",
"]+ c #9CBD11",
"^+ c #95B40C",
"/+ c #94B50F",
"(+ c #95B510",
"_+ c #99B913",
":+ c #A0C414",
"<+ c #9ABC11",
"[+ c #A0C314",
"}+ c #A1C40F",
"|+ c #A3C513",
". + + @ + # # $ % & * = & - + + + + + # # ",
"; > , > # > > $ ' ) ! ~ { ] ^ , - > , > # ",
"+ + / ( _ : < [ } | 1 2 3 4 5 6 : 7 8 # # ",
"+ 9 # ( 0 a b c d e e e f g h i j 9 k l + ",
"+ + > m n o p q r s t u v w x y z A & # # ",
"# % k B C D E F G H I J K L M N O P Q ] , ",
"$ R > S T U V W , X Y Z `  ...+.T @.#.$.] ",
"% %.* &.*.=.-.;.> >.,.'.).!.~.{.].^./.R 7 ",
"7 (._.:.D <.[.}.|.1.2.2.3.4.5.6.7.8.9._ 8 ",
". % 0.a.b.c.d.e.f.N g.h.2.i.j.k.l.m.n.$ # ",
"; + ; o.p.q.r.s.t.u.v.w.x.2.y.z.].A.B.l : ",
"# # R C.D.E.F.G.H.I.J.K.L.2.M.M.N.O.P.; l ",
"# / Q.R.S.T.U.].8.V.W.X.Y.e Z.`.]. +.+++7 ",
"+ + 9 / ; @+#+$+%+&+e *+=+-+;+>+,+'+)+, # ",
"# + > % & !+~+{+]+^+/+(+_+) Q.:+<+[+$ R # ",
"7 + > }+# % k |+8 + > + * $ _ / , 7 8 ] - "};

#define check_danger_mask_width 17
#define check_danger_mask_height 13
static const uchar check_danger_mask_bits[] = {
   0x00, 0x00, 0x00, 0x80, 0x03, 0x00, 0x80, 0x03, 0x00, 0x80, 0x03, 0x00,
   0x80, 0x03, 0x00, 0x80, 0x03, 0x00, 0x80, 0x03, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x80, 0x03, 0x00, 0x80, 0x03, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00 };

#define check_off_mask_width 17
#define check_off_mask_height 13
static const uchar check_off_mask_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x07, 0x00, 0xe0, 0x0e, 0x00,
   0xe0, 0x0e, 0x00, 0x00, 0x07, 0x00, 0x80, 0x03, 0x00, 0x80, 0x03, 0x00,
   0x00, 0x00, 0x00, 0x80, 0x03, 0x00, 0x80, 0x03, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00 };

#define check_on_mask_width 17
#define check_on_mask_height 13
static const uchar check_on_mask_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x1c, 0x00,
   0x00, 0x1e, 0x00, 0x10, 0x0f, 0x00, 0xb0, 0x07, 0x00, 0xf0, 0x03, 0x00,
   0xe0, 0x01, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00 };

#define pagecurl_mask_width 53
#define pagecurl_mask_height 51
static const uchar pagecurl_mask_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0x0f, 0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0xfe, 0xff,
   0xff, 0xff, 0xff, 0x0f, 0x00, 0xf0, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00,
   0xc0, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x80, 0xff, 0xff, 0xff, 0xff,
   0x0f, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xfe, 0xff,
   0xff, 0xff, 0x0f, 0x00, 0x00, 0xfc, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00,
   0xfc, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xf8, 0xff, 0xff, 0xff, 0x0f,
   0x00, 0x00, 0xf0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xf0, 0xff, 0xff,
   0xff, 0x0f, 0x00, 0x00, 0xf0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xf0,
   0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00,
   0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff,
   0x0f, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xe0, 0xff,
   0xff, 0xff, 0x0f, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00,
   0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f,
   0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xe0, 0xff, 0xff,
   0xff, 0x0f, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xe0,
   0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00,
   0x00, 0xe0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xc0, 0xff, 0xff, 0xff,
   0x0f, 0x00, 0x00, 0xc0, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0xfc,
   0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0x0f, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xfc, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x0f,
   0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xe0, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x0f, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08 };

typedef QValueList<MetaTranslatorMessage> TML;

static const int ErrorMS = 600000; // for error messages
static const int MessageMS = 2500;

static QDict<Embed> *imageDict = 0;

QPixmap * TrWindow::pxOn = 0;
QPixmap * TrWindow::pxOff = 0;
QPixmap * TrWindow::pxObsolete = 0;
QPixmap * TrWindow::pxDanger = 0;

enum Ending { End_None, End_FullStop, End_Interrobang, End_Colon,
	      End_Ellipsis };

static Ending ending( const QString& str )
{
    int ch = 0;
    if ( !str.isEmpty() )
	ch = str.right( 1 )[0].unicode();

    switch ( ch ) {
    case 0x002e: // full stop
	if ( str.right(3) == QString("...") )
	    return End_Ellipsis;
	else
	    return End_FullStop;
    case 0x0589: // armenian full stop
    case 0x06d4: // arabic full stop
    case 0x3002: // ideographic full stop
	return End_FullStop;
    case 0x0021: // exclamation mark
    case 0x003f: // question mark
    case 0x00a1: // inverted exclamation mark
    case 0x00bf: // inverted question mark
    case 0x01c3: // latin letter retroflex click
    case 0x037e: // greek question mark
    case 0x061f: // arabic question mark
    case 0x203c: // double exclamation mark
    case 0x203d: // interrobang
    case 0x2048: // question exclamation mark
    case 0x2049: // exclamation question mark
    case 0x2762: // heavy exclamation mark ornament
	return End_Interrobang;
    case 0x003a: // colon
	return End_Colon;
    case 0x2026: // horizontal ellipsis
	return End_Ellipsis;
    default:
	return End_None;
    }
}

class Action : public QAction
{
public:
    Action( QPopupMenu *pop, const QString& menuText, QObject *receiver,
	    const char *member, int accel = 0, bool toggle = FALSE );
    Action( QPopupMenu *pop, const QString& menuText, int accel = 0,
	    bool toggle = FALSE );

    virtual void setWhatsThis( const QString& whatsThis );

    bool addToToolbar( QToolBar *tb, const QString& text, const char *xpmName );
};

Action::Action( QPopupMenu *pop, const QString& menuText, QObject *receiver,
		const char *member, int accel, bool toggle )
    : QAction( pop->parent(), (const char *) 0, toggle )
{
    QAction::addTo( pop );
    setMenuText( menuText );
    setAccel( accel );
    connect( this, SIGNAL(activated()), receiver, member );
}

Action::Action( QPopupMenu *pop, const QString& menuText, int accel,
		bool toggle )
    : QAction( pop->parent(), (const char *) 0, toggle )
{
    QAction::addTo( pop );
    setMenuText( menuText );
    setAccel( accel );
}

void Action::setWhatsThis( const QString& whatsThis )
{
    QAction::setWhatsThis( whatsThis );
    setStatusTip( whatsThis );
}

bool Action::addToToolbar( QToolBar *tb, const QString& text,
			   const char *xpmName )
{
    Embed *ess = 0;
    Embed *ell = 0;

    setText( text );
    ess = imageDict->find( QString("small/") + QString(xpmName) );
    ell = imageDict->find( QString("large/") + QString(xpmName) );
    if ( ess != 0 && ell != 0 ) {
	QPixmap small, large;
	small.loadFromData( ess->data, ess->size );
	large.loadFromData( ell->data, ell->size );
	QIconSet s( small, large );
	setIconSet( s );
    }
    return QAction::addTo( tb );
}

const QPixmap TrWindow::splash()
{
    Embed *ess = 0;

    setupImageDict();
    ess = imageDict->find( QString("splash.png") );
    if ( ess == 0 )
	return 0;
    QPixmap pixmap;
    pixmap.loadFromData( ess->data, ess->size );
    return pixmap;
}

const QPixmap TrWindow::pageCurl()
{
    Embed *ess = 0;

    setupImageDict();
    ess = imageDict->find( QString("pagecurl.png") );
    if ( ess == 0 )
	return 0;
    QPixmap pixmap;
    pixmap.loadFromData( ess->data, ess->size );
	QBitmap pageCurlMask( pagecurl_mask_width, pagecurl_mask_height,
			pagecurl_mask_bits, TRUE );
	pixmap.setMask( pageCurlMask );
    return pixmap;
}

TrWindow::TrWindow()
    : QMainWindow( 0, "translation window",
		   WType_TopLevel | WDestructiveClose )
{
    setIcon( QPixmap(logo_xpm) );

    setupImageDict();

    // Set up the Scope dock window
    QDockWindow * dwScope = new QDockWindow( QDockWindow::InDock, this,
					     "scope");
    dwScope->setResizeEnabled( TRUE );
    dwScope->setCloseMode( QDockWindow::Always );
    addDockWindow( dwScope, tr("Scope"), Qt::Left );
    dwScope->setCaption( tr("Scope") );
    dwScope->setFixedExtentWidth( 200 );
    lv = new QListView( dwScope, "scope list view" );
    lv->setShowSortIndicator( TRUE );
    lv->setAllColumnsShowFocus( TRUE );
    lv->header()->setStretchEnabled( TRUE, 1 );
    lv->addColumn( tr("Done"), 40 );
    lv->addColumn( tr("Scope") );
    lv->addColumn( tr("Items"), 55 );
    lv->setColumnAlignment( 0, Qt::AlignCenter );
    lv->setColumnAlignment( 2, Qt::AlignRight );
    lv->setSorting( 0 );
    lv->setHScrollBarMode( QScrollView::AlwaysOff );
    dwScope->setWidget( lv );

    messageIsShown = FALSE;
    me = new MessageEditor( &tor, this, "message editor" );
    setCentralWidget( me );
    slv = me->sourceTextList();
    plv = me->phraseList();

    setupMenuBar();
    setupToolBars();
    progress = new QLabel( statusBar(), "progress" );
    statusBar()->addWidget( progress, 0, TRUE );
    modified = new QLabel( tr(" MOD "), statusBar(), "modified?" );
    statusBar()->addWidget( modified, 0, TRUE );

    numFinished = 0;
    numNonobsolete = 0;
    numMessages = 0;
    updateProgress();

    dirty = FALSE;
    updateCaption();

    phraseBooks.setAutoDelete( TRUE );

    ov = new QDialog( this, "overview dialog" );

    QTextBrowser *tv = new QTextBrowser( ov, "overview browser" );
    tv->setText( tr("<p> <b>Qt Linguist</b> is a utility for translating"
    " applications written with Qt. </p>\n\n"

    "<p> Translation of a Qt application is a three-step process: <ol>\n"
    " <li> Run <b>lupdate</b> to extract translatable text from the C++ source"
    " code of the Qt application, resulting in a translation source file (a TS"
    " file).\n"
    " <li> Provide translations for the source texts in the TS file.\n"
    " <li> Run <b>lrelease</b> to obtain a light-weight message file (a QM"
    " file) from the TS file, suitable only for released applications.\n"
    "</ol></p>\n\n"

    "<p> <b>Qt Linguist</b>'s role is to let you provide translations for"
    " source texts (step 2 above).  Each source text is characterized by the"
    " text itself, a <i>context</i> (usually the name of the C++ class"
    " containing the text), and a <i>comment</i> to help you distinguish the"
    " text.  Thus, distinct occurrences of terms such as \"Enabled\" may be"
    " translated differently according to the comment and the context in which"
    " they appear (\"Activado\", \"Activada\", etc., in Spanish).</p>\n\n"

    "<p> When translating an item, you have the following information close at"
    " hand:<ul>\n"
    " <li> the source text;\n"
    " <li> the comment for the source text;\n"
    " <li> the context in which the source text appears, possibly with an"
    " explanation of where to find the context in the Qt application or other"
    " comment;\n"
    " <li> relevant phrases from phrase books, with translations and"
    " definitions.\n"
    "</ul> Phrase books provide instant information about technical terms, and"
    " can be edited using <b>Qt Linguist</b>.  There is also a <i>done</i>"
    " flag that you may turn on when you are satisfied with a translation, in"
    " which case the translation is used instead of the source text when"
    " running the Qt application with the QM file.</p>\n\n"

    "<p> When the Qt application evolves (say, from release 1.0 to release"
    " 1.1), the utility <b>lupdate</b> merges the current source texts with the"
    " previous translation source file, to reuse existing translations. "
    " In some typical"
    " cases, <b>lupdate</b> may suggest translations.  These translations are"
    " marked as <i>unfinished</i>, so you can easily go over them.  In any"
    " case, you can press the <i>Guess Again</i> button and obtain existing"
    " translations for similar source text.</p>\n\n"

    "<p> Furthermore, <b>Qt Linguist</b> can flag the following three common"
    " errors: <ol>\n"
    " <li> An accelerator is present in the source text but is absent from the"
    " translation (or vice versa).  Thus, \"&amp;File\" translated as"
    " \"Fichier\" in French is flagged as suspicious (it should probably read"
    " \"&amp;Fichier\").\n"
    " <li> The translation does not end with the same punctuation as the source"
    " text.  Thus, \"Open...\" translated as \"Ouvrir\" is flagged as"
    " suspicious (it should probably read \"Ouvrir...\").\n"
    " <li> A suggested translation from the phrase book is ignored.  Thus,"
    " \"Save\" translated as \"Sauver\" is flagged as suspicious, if a phrase"
    " book entry gives \"Sauvegarder\" as the translation for \"Save\"."
    "</p>\n") );

    QPushButton *ok = new QPushButton( tr("OK"), ov, "ok overview" );
    ok->setDefault( TRUE );
    connect( ok, SIGNAL(clicked()), ov, SLOT(accept()) );

    QVBoxLayout *vlay = new QVBoxLayout( ov, 11, 11 );
    QHBoxLayout *hlay = new QHBoxLayout;
    vlay->add( tv );
    vlay->addLayout( hlay );
    hlay->addStretch( 1 );
    hlay->add( ok );
    hlay->addStretch( 1 );
    ov->resize( 480, 400 );

    f = new FindDialog( this, "find", FALSE );
    f->setCaption( tr("Qt Linguist") );
    findMatchCase = FALSE;
    findWhere   = 0;
    foundItem   = 0;
    foundScope  = 0;
    foundWhere  = 0;
    foundOffset = 0;

    connect( lv, SIGNAL(currentChanged(QListViewItem *)),
	     this, SLOT(showNewScope(QListViewItem *)) );

    connect( slv, SIGNAL(currentChanged(QListViewItem *)),
	     this, SLOT(showNewCurrent(QListViewItem *)) );

    connect( slv, SIGNAL(clicked(QListViewItem *, const QPoint&, int)),
	     this, SLOT(toggleFinished(QListViewItem *, const QPoint&, int)) );

    connect( me, SIGNAL(translationChanged(const QString&)),
	     this, SLOT(updateTranslation(const QString&)) );
    connect( me, SIGNAL(finished(bool)), this, SLOT(updateFinished(bool)) );
    connect( me, SIGNAL(prevUnfinished()), this, SLOT(prevUnfinished()) );
    connect( me, SIGNAL(nextUnfinished()), this, SLOT(nextUnfinished()) );
    connect( f, SIGNAL(findNext(const QString&, int, bool)),
	     this, SLOT(findNext(const QString&, int, bool)) );

    QWhatsThis::add( lv, tr("This panel lists the source contexts.") );

    QWhatsThis::add( slv, tr("This panel lists the source texts. "
			    "Items that violate validation rules "
			    "are marked with a warning.") );
    showNewCurrent( 0 );

    QSize as( qApp->desktop()->size() );
    as -= QSize( 30, 30 );
    resize( QSize( 1000, 800 ).boundedTo( as ) );
    readConfig();
}

TrWindow::~TrWindow()
{
    writeConfig();
}

void TrWindow::openFile( const QString& name )
{
    if ( !name.isEmpty() ) {
	statusBar()->message( tr("Loading...") );
	qApp->processEvents();
	if ( tor.load(name) ) {
	    slv->clear();
	    slv->repaint();
	    slv->viewport()->repaint();
	    slv->setUpdatesEnabled( FALSE );
	    slv->viewport()->setUpdatesEnabled( FALSE );
	    lv->clear();
	    lv->repaint();
	    lv->viewport()->repaint();
	    lv->setUpdatesEnabled( FALSE );
	    lv->viewport()->setUpdatesEnabled( FALSE );
	    setEnabled( FALSE );
	    numFinished = 0;
	    numNonobsolete = 0;
	    numMessages = 0;
	
	    TML all = tor.messages();
	    TML::Iterator it;
	    QDict<ContextLVI> contexts( 1009 );

	    for ( it = all.begin(); it != all.end(); ++it ) {
		qApp->processEvents();
		ContextLVI *c = contexts.find( QString((*it).context()) );
		if ( c == 0 ) {
		    c = new ContextLVI( lv, tor.toUnicode((*it).context()) );
		    contexts.insert( QString((*it).context()), c );
		}
		if ( (*it).sourceText()[0] == '\0' ) {
		    c->appendToComment( tor.toUnicode((*it).comment()) );
		} else {
		    MessageLVI * tmp = new MessageLVI( slv,
					   (*it),
 					   tor.toUnicode((*it).sourceText()),
 					   tor.toUnicode((*it).comment()), c );
		    tmp->setDanger( danger(tmp->sourceText(),
					   tmp->translation()) &&
		                           tmp->message().type() == MetaTranslatorMessage::Finished );
 		    c->instantiateMessageItem( slv, tmp );

		    if ( (*it).type() != MetaTranslatorMessage::Obsolete ) {
			numNonobsolete++;
			if ( (*it).type() == MetaTranslatorMessage::Finished )
			    numFinished++;
		    } else {
			c->incrementObsoleteCount();
		    }
		    numMessages++;
		}
		c->updateStatus();
	    }
	    slv->viewport()->setUpdatesEnabled( TRUE );
	    slv->setUpdatesEnabled( TRUE );
	    lv->viewport()->setUpdatesEnabled( TRUE );
	    lv->setUpdatesEnabled( TRUE );
	    setEnabled( TRUE );
	    slv->repaint();
	    slv->viewport()->repaint();
	    lv->repaint();
	    lv->viewport()->repaint();
	    updateProgress();
	    filename = name;
	    dirty = FALSE;
	    updateCaption();
	    me->showNothing();
	    finishedAndNextAct->setEnabled( FALSE );
	    messageIsShown = FALSE;
	    statusBar()->message(
		tr("%1 source phrase(s) loaded.").arg(numMessages),
		MessageMS );

	    foundItem = 0;
	    foundWhere = 0;
	    foundOffset = 0;
	    findAct->setEnabled( TRUE );
	    findAgainAct->setEnabled( FALSE );
	    addRecentlyOpenedFile( name, recentFiles );
	} else {
	    statusBar()->clear();
	    QMessageBox::warning( this, tr("Qt Linguist"),
				  tr("Cannot open '%1'.").arg(name) );
	}
    }
}

void TrWindow::exitApp()
{
    if ( maybeSave() )
	close();
}

void TrWindow::open()
{
    if ( maybeSave() ) {
	QString newFilename = QFileDialog::getOpenFileName( filename,
				      tr("Qt translation source (*.ts)\n"
				      "All files (*)"), this );
	openFile( newFilename );
    }
}

void TrWindow::save()
{
    if ( filename.isEmpty() )
	return;

    if ( tor.save(filename) ) {
	dirty = FALSE;
	updateCaption();
	statusBar()->message( tr("File saved."), MessageMS );
    } else {
	QMessageBox::warning( this, tr("Qt Linguist"), tr("Cannot save '%1'.")
			      .arg(filename) );
    }
}

void TrWindow::saveAs()
{
    QString newFilename = QFileDialog::getSaveFileName( filename,
	    tr("Qt translation source (*.ts)\n"
	       "All files (*)") );
    if ( !newFilename.isEmpty() ) {
	filename = newFilename;
	save();
	updateCaption();
    }
}

void TrWindow::release()
{
    QString newFilename = filename;
    newFilename.replace( QRegExp(".ts$"), "" );
    newFilename += QString( ".qm" );

    newFilename = QFileDialog::getSaveFileName( newFilename,
	    tr("Qt message files for released applications (*.qm)\n"
	       "All files (*)") );
    if ( !newFilename.isEmpty() ) {
	if ( tor.release(newFilename) )
	    statusBar()->message( tr("File created."), MessageMS );
	else
	    QMessageBox::warning( this, tr("Qt Linguist"),
				  tr("Cannot save '%1'.").arg(newFilename) );
    }
}

void TrWindow::print()
{
    int pageNum = 0;


    // ### Fix this! - printing is currently broken in 3.0!

    if ( printer.setup(this) ) {
	printer.setDocName( filename );
	statusBar()->message( tr("Printing...") );
	PrintOut pout( &printer );
	ContextLVI *c = (ContextLVI *) lv->firstChild();
	while ( c != 0 ) {
	    setCurrentContextItem( c );
	    pout.vskip();
	    pout.setRule( PrintOut::ThickRule );
	    pout.setGuide( c->context() );
	    pout.addBox( 100, tr("Context: %1").arg(c->context()),
			 PrintOut::Strong );
	    pout.flushLine();
	    pout.addBox( 4 );
	    pout.addBox( 92, c->comment(), PrintOut::Emphasis );
	    pout.flushLine();
	    pout.setRule( PrintOut::ThickRule );

	    MessageLVI *m = (MessageLVI *) slv->firstChild();
	    while ( m != 0 ) {
		pout.setRule( PrintOut::ThinRule );

		QString type;
		switch ( m->message().type() ) {
		case MetaTranslatorMessage::Finished:
		    type = m->danger() ? tr( "(?)" ) : tr( "finished" );
		    break;
		case MetaTranslatorMessage::Obsolete:
		    type = tr( "obsolete" );
		    break;
		default:
		    type = QString( "" );
		}
		pout.addBox( 40, m->sourceText() );
		pout.addBox( 4 );
		pout.addBox( 40, m->translation() );
		pout.addBox( 4 );
		pout.addBox( 12, type, PrintOut::Normal, Qt::AlignRight );
		if ( !m->comment().isEmpty() ) {
		    pout.flushLine();
		    pout.addBox( 4 );
		    pout.addBox( 92, m->comment(), PrintOut::Emphasis );
		}
		pout.flushLine( TRUE );

		if ( pout.pageNum() != pageNum ) {
		    pageNum = pout.pageNum();
		    statusBar()->message( tr("Printing... (page %1)")
					  .arg(pageNum) );
		}
		m = (MessageLVI *) m->nextSibling();
	    }
	    c = (ContextLVI *) c->nextSibling();
	}
	pout.flushLine( TRUE );
	statusBar()->message( tr("Printing completed"), MessageMS );
    } else {
	statusBar()->message( tr("Printing aborted"), MessageMS );
    }
}

void TrWindow::find()
{
    f->show();
    f->setActiveWindow();
    f->raise();
}

void TrWindow::findAgain()
{
    int pass = 0;
    int oldItemNo = itemToIndex( slv, slv->currentItem() );
    QListViewItem * j = foundScope;
    QListViewItem * k = indexToItem( slv, foundItem );
    QListViewItem * oldScope = lv->currentItem();

#if 1
    /*
      As long as we don't implement highlighting of the text in the QTextView,
      we may have only one match per message.
    */
    foundWhere = FindDialog::Comments;
    foundOffset = (int) 0x7fffffff;
#else
    foundOffset++;
#endif

    slv->setUpdatesEnabled( FALSE );
    do {
	// Iterate through every item in all scopes
	if ( j == 0 ) {
	    j = lv->firstChild();
	    setCurrentContextItem( j );
	    if ( foundScope != 0 )
		statusBar()->message( tr("Search wrapped."), MessageMS );
	}
	if ( k == 0 ) {
	    k = slv->firstChild();
	}

	while ( k ) {
	    MessageLVI * m = (MessageLVI *) k;
	    switch ( foundWhere ) {
		case 0:
		    foundWhere  = FindDialog::SourceText;
		    foundOffset = 0;
		    // fall-through
		case FindDialog::SourceText:
		    if ( searchItem( m->sourceText(), j, k ) )
			return;
		    foundWhere  = FindDialog::Translations;
		    // fall-through
		case FindDialog::Translations:
		    if ( searchItem( m->translation(), j, k ) )
			return;
		    foundWhere  = FindDialog::Comments;
		    // fall-through
		case FindDialog::Comments:
		    if( searchItem( ((ContextLVI *) j)->fullContext(), j, k))
			return;
 		    foundWhere  = 0;
	    }
	    k = k->nextSibling();
	}
	
	j = j->nextSibling();
	if ( j ) {
	    setCurrentContextItem( j );
	    k = slv->firstChild();
	}
    } while ( ++pass != lv->childCount() );

    // This is just to keep the current scope and source text item
    // selected after a search failed.
    if ( oldScope ){
	setCurrentContextItem( oldScope );
	QListViewItem * tmp = indexToItem( slv, oldItemNo );
	if( tmp )
	    setCurrentMessageItem( tmp );
    } else {
	if( lv->firstChild() )
	    setCurrentContextItem( lv->firstChild() );
	if( slv->firstChild() )
	    setCurrentMessageItem( slv->firstChild() );
    }

    slv->setUpdatesEnabled( TRUE );
    slv->triggerUpdate();
    statusBar()->message( tr("No match."), MessageMS );
    qApp->beep();
    foundItem   = 0;
    foundWhere  = 0;
    foundOffset = 0;
}

int TrWindow::itemToIndex( QListView * view, QListViewItem * item )
{
    int no = 0;
    QListViewItem * tmp;

    if( view && item ){
	if( (tmp = view->firstChild()) != 0 )
	    do {
		no++;
		tmp = tmp->nextSibling();
	    } while( tmp && (tmp != item) );
    }
    return no;
}

QListViewItem * TrWindow::indexToItem( QListView * view, int index )
{
    QListViewItem * item = 0;

    if( view && (index > 0) ){
	item = view->firstChild();
	while( item && (index-- > 0) )
	    item = item->nextSibling();
    }
    return item;
}

bool TrWindow::searchItem( const QString & searchWhat,  QListViewItem * j,
			   QListViewItem * k )
{
    if ( (findWhere & foundWhere) != 0 ) {
	foundOffset = searchWhat.find( findText, foundOffset, findMatchCase );
	if ( foundOffset >= 0 ) {
	    foundItem  = itemToIndex( slv, k );
	    foundScope = j;
	    setCurrentMessageItem( k );
	    slv->setUpdatesEnabled( TRUE );
	    slv->triggerUpdate();
	    return TRUE;
	}
    }
    foundOffset = 0;
    return FALSE;
}

void TrWindow::newPhraseBook()
{
    QString name;
    while ( TRUE ) {
	name = QFileDialog::getSaveFileName( QString::null,
		       tr("Qt phrase books (*.qph)\n"
			  "All files (*)") );
	if ( !QFile::exists(name) )
	    break;
	QMessageBox::warning( this, tr("Qt Linguist"),
			      tr("A file called '%1' already exists."
				 "  Please choose another name.").arg(name) );
    }
    if ( !name.isEmpty() ) {
	PhraseBook pb;
	if ( savePhraseBook(name, pb) ) {
	    if ( openPhraseBook(name) )
		statusBar()->message( tr("Phrase book created."), MessageMS );
	}
    }
}

void TrWindow::openPhraseBook()
{
    QString name = QFileDialog::getOpenFileName( QString::null,
	    tr("Qt phrase books (*.qph)\n"
	       "All files (*)") );

    if ( !name.isEmpty() && !phraseBookNames.contains(name) ) {
	if ( openPhraseBook(name) ) {
	    int n = phraseBooks.at( phraseBooks.count() - 1 )->count();
	    statusBar()->message( tr("%1 phrase(s) loaded.").arg(n),
				  MessageMS );
	}
    }
}

void TrWindow::closePhraseBook( int id )
{
    int index = closePhraseBookp->indexOf( id );
    phraseBooks.remove( index );
    phraseBookNames.remove( phraseBookNames.at(index) );
    updatePhraseDict();

    closePhraseBookp->removeItem( id );
    editPhraseBookp->removeItem( editPhraseBookp->idAt(index) );
    printPhraseBookp->removeItem( printPhraseBookp->idAt(index) );
}

void TrWindow::editPhraseBook( int id )
{
    int index = editPhraseBookp->indexOf( id );
    PhraseBookBox box( phraseBookNames[index], *phraseBooks.at(index), this,
		       "phrase book box", TRUE );
    box.setCaption( tr("%1 - %2").arg(tr("Qt Linguist"))
				 .arg(friendlyPhraseBookName(index)) );
    box.exec();
    *phraseBooks.at( index ) = box.phraseBook();
    updatePhraseDict();
}

void TrWindow::printPhraseBook( int id )
{
    int index = printPhraseBookp->indexOf( id );
    int pageNum = 0;

    if ( printer.setup(this) ) {
	printer.setDocName( phraseBookNames[index] );
	statusBar()->message( tr("Printing...") );
	PrintOut pout( &printer );
	PhraseBook *phraseBook = phraseBooks.at( index );
	PhraseBook::Iterator p;
	pout.setRule( PrintOut::ThinRule );
	for ( p = phraseBook->begin(); p != phraseBook->end(); ++p ) {
	    pout.setGuide( (*p).source() );
	    pout.addBox( 29, (*p).source() );
	    pout.addBox( 4 );
	    pout.addBox( 29, (*p).target() );
	    pout.addBox( 4 );
	    pout.addBox( 34, (*p).definition(), PrintOut::Emphasis );

	    if ( pout.pageNum() != pageNum ) {
		pageNum = pout.pageNum();
		statusBar()->message( tr("Printing... (page %1)")
				      .arg(pageNum) );
	    }
	    pout.setRule( PrintOut::ThinRule );
	    pout.flushLine( TRUE );
	}
	pout.flushLine( TRUE );
	statusBar()->message( tr("Printing completed"), MessageMS );
    } else {
	statusBar()->message( tr("Printing aborted"), MessageMS );
    }
}

void TrWindow::revertSorting()
{
    lv->setSorting( 0 );
    slv->setSorting( 0 );
}

void TrWindow::overview()
{
    ov->show();
    ov->setActiveWindow();
    ov->raise();
}

void TrWindow::about()
{
    Embed *ess = 0;

    setupImageDict();
    ess = imageDict->find( QString("splash.png") );
    if ( ess == 0 )
	return;

    QPixmap pixmap;
    pixmap.loadFromData( ess->data, ess->size );

    QDialog about( this, 0, TRUE );
    about.setCaption( tr("Qt Linguist") );
    about.resize( 447, 464 );

    QLabel *splash = new QLabel( &about );
    splash->setPixmap( pixmap );
    splash->setAlignment( QLabel::AlignCenter );

    QLabel *version = new QLabel( tr("Version pre1.0"), &about );
    version->setAlignment( QLabel::AlignCenter );
    QLabel *copyright = new QLabel( tr("Copyright (C) 2000 Trolltech AS"),
				    &about );
    copyright->setAlignment( QLabel::AlignCenter );
    QPushButton *ok = new QPushButton( tr("OK"), &about, "ok about" );
    ok->setDefault( TRUE );
    about.setFocusProxy( ok );
    connect( ok, SIGNAL(clicked()), &about, SLOT(accept()) );

    QVBoxLayout *vlay = new QVBoxLayout( &about, 11, 6 );
    QHBoxLayout *hlay = new QHBoxLayout;

    vlay->addWidget( splash );
    vlay->add( version );
    vlay->add( copyright );
    vlay->addStretch( 1 );
    vlay->addLayout( hlay );
    hlay->addStretch( 1 );
    hlay->add( ok );
    hlay->addStretch( 1 );

    about.exec();
}

void TrWindow::aboutQt()
{
    QMessageBox::aboutQt( this );
}

void TrWindow::setupPhrase()
{
    bool enabled = !phraseBooks.isEmpty();
    phrasep->setItemEnabled( closePhraseBookId, enabled );
    phrasep->setItemEnabled( editPhraseBookId, enabled );
    phrasep->setItemEnabled( printPhraseBookId, enabled );
}

bool TrWindow::maybeSave()
{
    if ( dirty ) {
	switch ( QMessageBox::information(this, tr("Qt Linguist"),
				  tr("Do you want to save '%1'?")
				  .arg(filename),
				  QMessageBox::Yes | QMessageBox::Default,
				  QMessageBox::No,
				  QMessageBox::Cancel ) )
	{
	    case QMessageBox::Cancel:
		return FALSE;
	    case QMessageBox::Yes:
		save();
		// fall-through
	    case QMessageBox::No:
		// fall-through
		break;
	}
    }
    return TRUE;
}

void TrWindow::updateCaption()
{
    QString cap;
    bool enable = !filename.isEmpty();
    saveAct->setEnabled( enable );
    saveAsAct->setEnabled( enable );
    releaseAct->setEnabled( enable );
    printAct->setEnabled( enable );
    acceleratorsAct->setEnabled( enable );
    endingPunctuationAct->setEnabled( enable );
    phraseMatchesAct->setEnabled( enable );
    revertSortingAct->setEnabled( enable );

    if ( filename.isEmpty() )
	cap = tr( "Qt Linguist by Trolltech" );
    else
	cap = tr( "%1 - %2" ).arg( tr("Qt Linguist by Trolltech") )
			     .arg( filename );
    setCaption( cap );
    modified->setEnabled( dirty );
}

//
// New scope selected - build a new list of source text items
// for that scope.
//
void TrWindow::showNewScope( QListViewItem *item )
{
    static ContextLVI * oldContext = 0;

    if( item != 0 ) {
      	ContextLVI * c = (ContextLVI *) item;
	bool upe = slv->isUpdatesEnabled();	
	slv->setUpdatesEnabled( FALSE );
	slv->viewport()->setUpdatesEnabled( FALSE );
	if ( oldContext != 0 ) {
	    MessageLVI * tmp;
	    slv->blockSignals( TRUE );
	    while ( (tmp = (MessageLVI *) slv->firstChild()) != 0 )
		oldContext->appendMessageItem( slv, tmp );
	    slv->blockSignals( FALSE );
	}
	while ( c->messageItemsInList() )
	    slv->insertItem( c->takeMessageItem( c->messageItemsInList() - 1) );
	
	slv->setUpdatesEnabled( upe );
	if( upe )
	    slv->triggerUpdate();
	oldContext = (ContextLVI *) item;
	statusBar()->clear();
    }
}

void TrWindow::showNewCurrent( QListViewItem *item )
{
    messageIsShown = (item != 0);
    MessageLVI *m = (MessageLVI *) item;
    ContextLVI *c = (ContextLVI *) m ? m->contextLVI() : 0;

    if ( messageIsShown ) {
	me->showMessage( m->sourceText(), m->comment(), c->fullContext(),
			 m->translation(), m->message().type(),
			 getPhrases(m->sourceText()) );	
	if ( (m->message().type() != MetaTranslatorMessage::Finished) &&
	     m->danger() )
	    danger( m->sourceText(), m->translation(), TRUE );
	else
	    statusBar()->clear();
	finishedAndNextAct->setEnabled( (m->message().type() != MetaTranslatorMessage::Obsolete) );
    } else {
	if ( item == 0 )
	    me->showNothing();
	else
 	    me->showContext( c->fullContext(), c->finished() );
	finishedAndNextAct->setEnabled( FALSE );
    }
    deleteAct->setEnabled( messageIsShown );
    selectAllAct->setEnabled( messageIsShown );
}

void TrWindow::updateTranslation( const QString& translation )
{
    QListViewItem *item = slv->currentItem();
    if ( item != 0 ) {
	MessageLVI *m = (MessageLVI *) item;
	QString stripped = translation.stripWhiteSpace();
	if ( stripped != m->translation() ) {
	    bool dngr;
	    m->setTranslation( stripped );
	    if ( m->finished() &&
		 (dngr = danger( m->sourceText(), m->translation(), TRUE )) ) {
		m->setDanger( dngr );
		m->setFinished( FALSE );
		m->contextLVI()->updateStatus();
	    }
	    tor.insert( m->message() );
	    if ( !dirty ) {
		dirty = TRUE;
		updateCaption();
	    }
	}
    }
}

void TrWindow::updateFinished( bool finished )
{
    QListViewItem *item = slv->currentItem();
    if ( item != 0 ) {
	MessageLVI *m = (MessageLVI *) item;
	if ( finished != m->finished() ) {
	    numFinished += finished ? +1 : -1;
	    updateProgress();
	    m->setFinished( finished );
	    bool oldDanger = m->danger();
	    m->setDanger( /*m->finished() &&*/
	    		  danger(m->sourceText(), m->translation(),
	    		 !oldDanger) );
	    if ( !oldDanger && m->danger() )
		qApp->beep();
	    tor.insert( m->message() );
	    if ( !dirty ) {
		dirty = TRUE;
		updateCaption();
	    }
	}
    }
}

void TrWindow::doneAndNext()
{
    MessageLVI * m = (MessageLVI *) slv->currentItem();
    bool dngr = FALSE;

    if ( !m ) return;
    dngr = danger( m->sourceText(), m->translation(), TRUE );
    if ( !dngr ) {
	me->finishAndNext();
	m->contextLVI()->updateStatus();
    } else {
	if ( m->danger() != dngr )
	    m->setDanger( dngr );
	tor.insert( m->message() );
	if ( !dirty ) {
	    dirty = TRUE;
	    updateCaption();
	}
	qApp->beep();
    }
}

void TrWindow::toggleFinished( QListViewItem *item, const QPoint& /* p */,
			       int column )
{
    if ( item != 0 && column == 0 ) {
	MessageLVI *m = (MessageLVI *) item;
	bool dngr = FALSE;
	
	if ( m->message().type() == MetaTranslatorMessage::Unfinished ) {
	    dngr = danger( m->sourceText(), m->translation(), TRUE );
	}
	if ( !dngr && m->message().type() != MetaTranslatorMessage::Obsolete) {
	    setCurrentMessageItem( m );
	    me->setFinished( !m->finished() );
	    m->contextLVI()->updateStatus();
	} else {
	    bool oldDanger = m->danger();
	    m->setDanger( danger(m->sourceText(), m->translation(),
				 !oldDanger) );
	    if ( !oldDanger && m->danger() )
		qApp->beep();
	    tor.insert( m->message() );
	    if ( !dirty ) {
		dirty = TRUE;
		updateCaption();
	    }
	}
    }
}

void TrWindow::nextUnfinished()
{
    // Select a message to translate, grab the first available if
    // there are no current selection.
    QListViewItem * cItem = lv->currentItem();  // context item
    QListViewItem * mItem = slv->currentItem(); // message item

    // Make sure an item is selected from both the context and the
    // message list.
    if( (mItem == 0) && !(mItem = slv->firstChild()) ) {
	if( (cItem == 0) && !(cItem = lv->firstChild()) ) {
	    statusBar()->message( tr("No phrase to translate."), MessageMS );
 	    qApp->beep();
	    return;
	} else {
	    showNewScope( cItem );
	    while( cItem && !(mItem = slv->firstChild()) ) {
		// no children in this node - try next one
		cItem = cItem->nextSibling();
		showNewScope( cItem );
	    }
	    setCurrentContextItem( cItem );
	    if( mItem ) {
		setCurrentMessageItem( cItem );
	    } else {
		statusBar()->message( tr("No phrase to translate."), MessageMS );
		qApp->beep();
		return;
	    }
	}
    } else {
	setCurrentMessageItem( mItem );
    }

    MessageLVI * m = (MessageLVI *) mItem;
    MessageLVI * n;
    ContextLVI * p = (ContextLVI *) cItem;
    ContextLVI * q;

    // Find the next Unfinished sibling within the same context.
    m = (MessageLVI *) mItem->nextSibling();
    n = m;
    do {
	if ( n == 0 )
	    break;
	if ( n && !n->finished() && n != mItem ) {
	    setCurrentMessageItem( n );
	    return;
	}
	n = (MessageLVI *) n->nextSibling();
    } while ( n != m );

    // If all siblings are Finished or Obsolete, look in the first Unfinished
    // context.
    p = (ContextLVI *) p->nextSibling();
    q = p;
    do {
	if ( q == 0 )
	    q = (ContextLVI *) lv->firstChild();
	if ( q && !q->finished() ) {
	    showNewScope( q );
	    setCurrentContextItem( q );
	    n = (MessageLVI *) slv->firstChild();
	    while ( n && n->finished() )
		n = (MessageLVI *) n->nextSibling();
	    if ( n && q ) {
		setCurrentMessageItem( n );
		return;
	    }
	}
	q = (ContextLVI *) q->nextSibling();
    } while ( q != p );


    // If no Unfinished message is left, the user has finished the job.  We
    // congratulate on a job well done with this ringing bell.
    statusBar()->message( tr("No untranslated phrases left."), MessageMS );
    qApp->beep();
}

static QListViewItem * lastChild( QListView * view )
{
    if ( view ) {
	QListViewItem * ret, * tmp;
	ret = view->firstChild();
	while ( ret ) {
	    tmp = ret->nextSibling();
	    if ( tmp == 0 )
		return ret;
	    ret = tmp;
	}
    }
    return 0;
}

void TrWindow::prevUnfinished()
{
    // Select a message to translate, grab the first available if
    // there are no current selection.
    QListViewItem * cItem = lv->currentItem();  // context item
    QListViewItem * mItem = slv->currentItem(); // message item

    // Make sure an item is selected from both the context and the
    // message list.
    if( (mItem == 0) && !(mItem = slv->firstChild()) ) {
	if( (cItem == 0) && !(cItem = lv->firstChild()) ) {
	    statusBar()->message( tr("No phrase to translate."), MessageMS );
 	    qApp->beep();
	    return;
	} else {
	    showNewScope( cItem );
	    while( cItem && !(mItem = slv->firstChild()) ) {
		// no children in this node - try next one
		cItem = cItem->nextSibling();
		showNewScope( cItem );
	    }
	    setCurrentContextItem( cItem );
	    if( mItem ) {
		setCurrentMessageItem( cItem );
	    } else {
		statusBar()->message( tr("No phrase to translate."), MessageMS );
		qApp->beep();
		return;
	    }
	}
    } else {
	setCurrentMessageItem( mItem );
    }

    MessageLVI * m = (MessageLVI *) mItem;
    MessageLVI * n;
    ContextLVI * p = (ContextLVI *) cItem;
    ContextLVI * q;

    // Find the next Unfinished sibling within the same context.
    n = m;
    do {
	n = (MessageLVI * ) n->itemAbove();
	if ( n == 0 )
	    break;
	if ( n && !n->finished() ) {
	    setCurrentMessageItem( n );
	    return;
	}
    } while ( !((ContextLVI *) cItem)->finished() && n != 0 );

    // If all siblings are Finished or Obsolete, look in the prev Unfinished
    // context.
    q = p;
    do {
	q = (ContextLVI *) q->itemAbove();
	if ( q == 0 )
	    q = (ContextLVI *) lastChild( lv );
	if ( q && !q->finished() ) {
	    showNewScope( q );
	    setCurrentContextItem( q );
	    n = (MessageLVI *) lastChild( slv );
	    while ( n && n->finished() )
		n = (MessageLVI *) n->itemAbove();
	    if ( n && q ) {
		setCurrentMessageItem( n );
		return;
	    }
	}
    } while ( q != 0 );

    statusBar()->message( tr("No untranslated phrases left."), MessageMS );
    qApp->beep();
}

void TrWindow::prev()
{
    QListViewItem * cItem = lv->currentItem();  // context item
    QListViewItem * mItem = slv->currentItem(); // message item
    QListViewItem * tmp;
	
    if ( !cItem ) {
	cItem = lv->firstChild();
	if ( !cItem ) return;
	setCurrentContextItem( cItem );
    }

    if ( !mItem ) {
	mItem = lastChild( slv );
	if ( !mItem ) return;
	setCurrentMessageItem( mItem );
    } else {
	if ( (tmp = mItem->itemAbove()) != 0 ) {
	    setCurrentMessageItem( tmp );
	    return;
	} else {
	    if ( (tmp = cItem->itemAbove()) == 0 ) {
		tmp = lastChild( lv );
	    }
	    if ( !tmp ) return;
	    setCurrentContextItem( tmp );
	    setCurrentMessageItem( lastChild( slv ) );
	}
    }
}

void TrWindow::next()
{
    QListViewItem * cItem = lv->currentItem();  // context item
    QListViewItem * mItem = slv->currentItem(); // message item
    QListViewItem * tmp;

    if ( !cItem ) {
	cItem = lv->firstChild();
	if ( !cItem ) return;
	setCurrentContextItem( cItem );
    }

    if ( !mItem ) {
	mItem = slv->firstChild();
	if ( !mItem ) return;
	setCurrentMessageItem( mItem );
    } else {
	if ( (tmp = mItem->nextSibling()) != 0 ) {
	    setCurrentMessageItem( tmp );
	    return;
	} else {
	    if ( (tmp = cItem->nextSibling()) == 0 ) {
		tmp = lv->firstChild();
	    }
	    if ( !tmp ) return;
	    setCurrentContextItem( tmp );
	    setCurrentMessageItem( slv->firstChild() );
	}
    }
}


void TrWindow::findNext( const QString& text, int where, bool matchCase )
{
    findText = text;
    if ( findText.isEmpty() )
	findText = QString( "magicwordthatyoushouldavoid" );
    findWhere = where;
    findMatchCase = matchCase;
    findAgainAct->setEnabled( TRUE );
    findAgain();
}

void TrWindow::revalidate()
{
    ContextLVI *c = (ContextLVI *) lv->firstChild();
    QListViewItem * oldScope = lv->currentItem();
    int oldItemNo = itemToIndex( slv, slv->currentItem() );
    slv->setUpdatesEnabled( FALSE );

    while ( c != 0 ) {
	showNewScope( c );
	MessageLVI *m = (MessageLVI *) slv->firstChild();
	while ( m != 0 ) {
	    m->setDanger( danger(m->sourceText(), m->translation()) &&
		    m->message().type() == MetaTranslatorMessage::Finished );
	    m = (MessageLVI *) m->nextSibling();
	}
	c = (ContextLVI *) c->nextSibling();
    }

    if ( oldScope ){
	showNewScope( oldScope );
	QListViewItem * tmp = indexToItem( slv, oldItemNo );
	if( tmp )
	    setCurrentMessageItem( tmp );
    }
    slv->setUpdatesEnabled( TRUE );
    slv->triggerUpdate();
}

void TrWindow::setupImageDict()
{
    if ( imageDict == 0 ) {
	imageDict = new QDict<Embed>( 101 );
	Embed *em;
	for ( em = embed_vec; em->size > 0; em++ )
	    imageDict->insert( em->name, em );

	// Create the application global listview symbols
	pxOn  = new QPixmap;
	pxOff = new QPixmap;
	pxObsolete = new QPixmap;
	pxDanger   = new QPixmap;
	
    	em = imageDict->find( QString("symbols/check_on.xpm") );
	pxOn->loadFromData( em->data, em->size );
	em = imageDict->find( QString("symbols/check_off.xpm") );
	pxOff->loadFromData( em->data, em->size );
	em = imageDict->find( QString("symbols/check_obs.xpm") );
	pxObsolete->loadFromData( em->data, em->size );
	em = imageDict->find( QString("symbols/check_danger.xpm") );
	pxDanger->loadFromData( em->data, em->size );

	QBitmap onMask( check_on_mask_width, check_on_mask_height,
			check_on_mask_bits, TRUE );
	QBitmap offMask( check_off_mask_width, check_off_mask_height,
			 check_off_mask_bits, TRUE );
	QBitmap dangerMask( check_danger_mask_width, check_danger_mask_height,
			    check_danger_mask_bits, TRUE );
	pxOn->setMask( onMask );
	pxOff->setMask( offMask );
	pxObsolete->setMask( onMask );
	pxDanger->setMask( dangerMask );
    }
}

QString TrWindow::friendlyString( const QString& str )
{
    QString f = str.lower();
    f.replace( QRegExp(QString("[.,:;!?()-]")), QString(" ") );
    f.replace( QRegExp(QString("&")), QString("") );
    f = f.simplifyWhiteSpace();
    f = f.lower();
    return f;
}

void TrWindow::setupMenuBar()
{
    QMenuBar * m = menuBar();
    QPopupMenu * filep        = new QPopupMenu( this );
    QPopupMenu * editp        = new QPopupMenu( this );
    QPopupMenu * translationp = new QPopupMenu( this );
    QPopupMenu * validationp  = new QPopupMenu( this );
    validationp->setCheckable( TRUE );
    phrasep = new QPopupMenu( this );
    closePhraseBookp   = new QPopupMenu( this );
    editPhraseBookp    = new QPopupMenu( this );
    printPhraseBookp   = new QPopupMenu( this );
    QPopupMenu * viewp = new QPopupMenu( this );
    viewp->setCheckable( TRUE );
    QPopupMenu * helpp   = new QPopupMenu( this );

    m->insertItem( tr("&File"), filep );
    m->insertItem( tr("&Edit"), editp );
    m->insertItem( tr("&Translation"), translationp );
    m->insertItem( tr("V&alidation"), validationp );
    m->insertItem( tr("&Phrases"), phrasep );
    m->insertItem( tr("&View"), viewp );
    m->insertSeparator();
    m->insertItem( tr("&Help"), helpp );

    connect( closePhraseBookp, SIGNAL(activated(int)),
	     this, SLOT(closePhraseBook(int)) );
    connect( editPhraseBookp, SIGNAL(activated(int)),
	     this, SLOT(editPhraseBook(int)) );
    connect( printPhraseBookp, SIGNAL(activated(int)),
	     this, SLOT(printPhraseBook(int)) );
    // File menu
    openAct = new Action( filep, tr("&Open..."), this, SLOT(open()),
			  QAccel::stringToKey(tr("Ctrl+O")) );

    filep->insertSeparator();

    saveAct = new Action( filep, tr("&Save"), this, SLOT(save()),
			  QAccel::stringToKey(tr("Ctrl+S")) );
    saveAsAct = new Action( filep, tr("Save &As..."), this, SLOT(saveAs()) );
    releaseAct = new Action( filep, tr("&Release..."), this, SLOT(release()) );
    filep->insertSeparator();
    printAct = new Action( filep, tr("&Print..."), this, SLOT(print()),
			   QAccel::stringToKey(tr("Ctrl+P")) );

    filep->insertSeparator();

    recentFilesMenu = new QPopupMenu( this );
    filep->insertItem( tr("Re&cently opened files"), recentFilesMenu );
    connect( recentFilesMenu, SIGNAL(aboutToShow()), this,
	     SLOT(setupRecentFilesMenu()) );
    connect( recentFilesMenu, SIGNAL(activated( int )), this,
	     SLOT(recentFileActivated( int )) );
		
    filep->insertSeparator();

    exitAct = new Action( filep, tr("E&xit"), this, SLOT(exitApp()),
			  QAccel::stringToKey(tr("Ctrl+Q")) );
    // Edit menu
    undoAct = new Action( editp, tr("&Undo"), me, SLOT(undo()), CTRL + Key_Z );
    undoAct->setEnabled( FALSE );
    connect( me, SIGNAL(undoAvailable(bool)), undoAct, SLOT(setEnabled(bool)) );
    redoAct = new Action( editp, tr("&Redo"), me, SLOT(redo()), CTRL + Key_Y );
    redoAct->setEnabled( FALSE );
    connect( me, SIGNAL(redoAvailable(bool)), redoAct, SLOT(setEnabled(bool)) );
    editp->insertSeparator();
    cutAct = new Action( editp, tr("Cu&t"), me, SLOT(cut()), CTRL + Key_X );
    cutAct->setEnabled( FALSE );
    connect( me, SIGNAL(cutAvailable(bool)), cutAct, SLOT(setEnabled(bool)) );
    copyAct = new Action( editp, tr("&Copy"), me, SLOT(copy()), CTRL + Key_C );
    copyAct->setEnabled( FALSE );
    connect( me, SIGNAL(copyAvailable(bool)), copyAct, SLOT(setEnabled(bool)) );
    pasteAct = new Action( editp, tr("&Paste"), me, SLOT(paste()),
			   CTRL + Key_V );
    pasteAct->setEnabled( FALSE );
    connect( me, SIGNAL(pasteAvailable(bool)),
	     pasteAct, SLOT(setEnabled(bool)) );
    deleteAct = new Action( editp, tr("&Delete"), me, SLOT(del()), Key_Delete );
    deleteAct->setEnabled( FALSE );
    selectAllAct = new Action( editp, tr("Select &All"), me, SLOT(selectAll()),
			       QAccel::stringToKey(tr("Ctrl+A")) );
    selectAllAct->setEnabled( FALSE );
    editp->insertSeparator();
    findAct = new Action( editp, tr("&Find..."), this, SLOT(find()),
			  QAccel::stringToKey(tr("Ctrl+F")) );
    findAct->setEnabled( FALSE );
    findAgainAct = new Action( editp, tr("Find &Next"),
			  this, SLOT(findAgain()), Key_F3 );
    findAgainAct->setEnabled( FALSE );

    // Translation menu
    startFromSourceAct = new Action( translationp, tr("&Start From Source"),
				     me, SLOT(startFromSource()), CTRL+Key_T );
    connect( me, SIGNAL(updateActions(bool)), startFromSourceAct,
	     SLOT(setEnabled(bool)) );
    guessAct = new Action( translationp, tr("&Guess"),
			   me, SLOT(guessAgain()), CTRL+Key_G );
    connect( me, SIGNAL(updateActions(bool)), guessAct,
	     SLOT(setEnabled(bool)) );
    prevUnfinishedAct = new Action( translationp, tr("&Prev Unfinished"),
				    this, SLOT(prevUnfinished()), CTRL+Key_K );
    nextUnfinishedAct = new Action( translationp, tr("&Next Unfinished"),
				    this, SLOT(nextUnfinished()), CTRL+Key_L );

    prevAct = new Action( translationp, tr("&Prev"),
			  this, SLOT(prev()), CTRL+SHIFT+Key_K );
    nextAct = new Action( translationp, tr("&Next"),
			  this, SLOT(next()), CTRL+SHIFT+Key_L );
    finishedAndNextAct = new Action( translationp, tr("&Done And Next"),
				     this, SLOT(doneAndNext()), CTRL+Key_N );

    // Phrasebook menu
    newPhraseBookAct = new Action( phrasep, tr("&New Phrase Book..."),
				   this, SLOT(newPhraseBook()) );
    openPhraseBookAct = new Action( phrasep, tr("&Open Phrase Book..."),
				    this, SLOT(openPhraseBook()),
				    QAccel::stringToKey(tr("Ctrl+B")) );
    closePhraseBookId = phrasep->insertItem( tr("&Close Phrase Book"),
					     closePhraseBookp );
    phrasep->insertSeparator();
    editPhraseBookId = phrasep->insertItem( tr("&Edit Phrase Book..."),
					    editPhraseBookp );
    printPhraseBookId = phrasep->insertItem( tr("&Print Phrase Book..."),
					     printPhraseBookp );
    connect( phrasep, SIGNAL(aboutToShow()), this, SLOT(setupPhrase()) );

    // Validation menu
    acceleratorsAct = new Action( validationp, tr("&Accelerators"),
				  this, SLOT(revalidate()), 0, TRUE );
    acceleratorsAct->setOn( TRUE );
    endingPunctuationAct = new Action( validationp, tr("&Ending Punctuation"),
				       this, SLOT(revalidate()), 0, TRUE );
    endingPunctuationAct->setOn( TRUE );
    phraseMatchesAct = new Action( validationp, tr("&Phrase Matches"),
				   this, SLOT(revalidate()), 0, TRUE );
    phraseMatchesAct->setOn( TRUE );

    // View menu
    revertSortingAct = new Action( viewp, tr("&Revert Sorting"),
				   this, SLOT(revertSorting()) );
    viewp->insertSeparator();
    bigIconsAct = new Action( viewp, tr("&Large Icons"), 0, TRUE );
    connect( bigIconsAct, SIGNAL(toggled(bool)),
	     this, SLOT(setUsesBigPixmaps(bool)) );
    textLabelsAct = new Action( viewp, tr("&Text Labels"), 0, TRUE );
    connect( textLabelsAct, SIGNAL(toggled(bool)),
	     this, SLOT(setUsesTextLabel(bool)) );

    overviewAct = new Action( helpp, tr("&Overview..."),
			      this, SLOT(overview()) );
    aboutAct = new Action( helpp, tr("&About..."), this, SLOT(about()),
			   Key_F1 );
    aboutQtAct = new Action( helpp, tr("About &Qt..."), this, SLOT(aboutQt()) );
    helpp->insertSeparator();
    whatsThisAct = new Action( helpp, tr("&What's This?"),
			       this, SLOT(whatsThis()), SHIFT + Key_F1 );

    openAct->setWhatsThis( tr("Open a Qt translation source file (TS file) for"
			      " editing.") );
    saveAct->setWhatsThis( tr("Save changes made to this Qt translation "
				"source file.") );
    saveAsAct->setWhatsThis( tr("Save changes made to this Qt translation"
			        "source file into a new file.") );
    releaseAct->setWhatsThis( tr("Create a Qt message file suitable for"
				 " released applications"
				 " from the current message file.") );
    printAct->setWhatsThis( tr("Print a list of all the phrases in the current"
			       " Qt translation source file.") );
    exitAct->setWhatsThis( tr("Close this window and exit.") );

    undoAct->setWhatsThis( tr("Undo the last editing operation performed on the"
			      " translation.") );
    redoAct->setWhatsThis( tr("Redo an undone editing operation performed on"
			      " the translation.") );
    cutAct->setWhatsThis( tr("Copy the selected translation text to the"
			     " clipboard and deletes it.") );
    copyAct->setWhatsThis( tr("Copy the selected translation text to the"
			      " clipboard.") );
    pasteAct->setWhatsThis( tr("Paste the clipboard text into the"
			       " translation.") );
    deleteAct->setWhatsThis( tr("Delete the selection or the character on the"
				" right of the cursor.") );
    selectAllAct->setWhatsThis( tr("Select the whole translation text.") );
    findAct->setWhatsThis( tr("Search for some text in the translation "
				"source file.") );
    findAgainAct->setWhatsThis( tr("Continue the search where it was left.") );

    newPhraseBookAct->setWhatsThis( tr("Create a new phrase book.") );
    openPhraseBookAct->setWhatsThis( tr("Open a phrase book to assist"
					" translation.") );
    acceleratorsAct->setWhatsThis( tr("Enable or disable validity checks of"
				      " accelerators.") );
    endingPunctuationAct->setWhatsThis( tr("Enable or disable validity checks"
					   " of ending punctuation.") );
    phraseMatchesAct->setWhatsThis( tr("Enable or disable checking that phrase"
				       " suggestions are used.") );

    revertSortingAct->setWhatsThis( tr("Sort the items back in the same order"
				       " as in the message file.") );
    bigIconsAct->setWhatsThis( tr("Enable or disable use of large icons in"
				  " toolbars.") );
    textLabelsAct->setWhatsThis( tr("Enable or disable use of text labels in"
				    " toolbars.") );

    overviewAct->setWhatsThis( tr("Display an introduction to %1.")
			       .arg(tr("Qt Linguist")) );
    aboutAct->setWhatsThis( tr("Display information about %1.")
			    .arg(tr("Qt Linguist")) );
    aboutQtAct->setWhatsThis( tr("Display information about the Qt toolkit by"
				 " Trolltech.") );
    whatsThisAct->setWhatsThis( tr("Enter What's This? mode.") );

    startFromSourceAct->setWhatsThis( tr("Copies the source text into"
					 " the translation field.") );
    guessAct->setWhatsThis( tr("Copies a guess translation into the"
			       " translation field." ) );
    nextAct->setWhatsThis( tr("Moves to the next item.") );
    prevAct->setWhatsThis( tr("Moves to the previous item.") );
    nextUnfinishedAct->setWhatsThis( tr("Moves to the next unfinished item.") );
    prevUnfinishedAct->setWhatsThis( tr("Moves to the previous unfinished item.") );
    finishedAndNextAct->setWhatsThis( tr("Marks this item as finished and "
	   			       "moves to the next unfinished item.") );

}

void TrWindow::setupToolBars()
{
    QToolBar *filet = new QToolBar( tr("File"), this );
    QToolBar *editt = new QToolBar( tr("Edit"), this );
    QToolBar *translationst = new QToolBar( tr("Translation"), this );
    QToolBar *validationt   = new QToolBar( tr("Validation"), this );
    QToolBar *helpt = new QToolBar( tr("Help"), this );

    openAct->addToToolbar( filet, tr("Open"), "fileopen.xpm" );
    saveAct->addToToolbar( filet, tr("Save"), "filesave.xpm" );
    printAct->addToToolbar( filet, tr("Print"), "print.xpm" );
    filet->addSeparator();
    openPhraseBookAct->addToToolbar( filet, tr("Open Phrase Book"),
				     "book.xpm" );

    undoAct->addToToolbar( editt, tr("Undo"), "undo.xpm" );
    redoAct->addToToolbar( editt, tr("Redo"), "redo.xpm" );
    editt->addSeparator();
    cutAct->addToToolbar( editt, tr("Cut"), "editcut.xpm" );
    copyAct->addToToolbar( editt, tr("Copy"), "editcopy.xpm" );
    pasteAct->addToToolbar( editt, tr("Paste"), "editpaste.xpm" );
    deleteAct->addToToolbar( editt, tr("Delete"), "editdelete.xpm" );
    editt->addSeparator();
    findAct->addToToolbar( editt, tr("Find"), "search.xpm" );

    startFromSourceAct->addToToolbar( translationst, tr("Start From Source"),
				      "search.xpm" );
    guessAct->addToToolbar( translationst, tr("Guess"), "search.xpm" );
    prevAct->addToToolbar( translationst, tr("Prev"), "search.xpm" );
    nextAct->addToToolbar( translationst, tr("Next"), "search.xpm" );
    prevUnfinishedAct->addToToolbar( translationst, tr("Prev Unfinished"),
					"search.xpm" );
    nextUnfinishedAct->addToToolbar( translationst, tr("Next Unfinished"),
					"search.xpm" );
    finishedAndNextAct->addToToolbar( translationst, tr("Done And Next"),
				      "search.xpm" );

    acceleratorsAct->addToToolbar( validationt, tr("Accelerators"),
				   "accel.xpm" );
    endingPunctuationAct->addToToolbar( validationt, tr("Punctuation"),
					"endpunct.xpm" );
    phraseMatchesAct->addToToolbar( validationt, tr("Phrases"), "phrase.xpm" );

    whatsThisAct->addToToolbar( helpt, tr("What's This?"), "whatsthis.xpm" );
}

void TrWindow::setCurrentContextItem( QListViewItem *item )
{
    lv->ensureItemVisible( item );
    lv->setSelected( item, TRUE );
}

void TrWindow::setCurrentMessageItem( QListViewItem *item )
{
    slv->ensureItemVisible( item );
    slv->setSelected( item, TRUE );
}

QString TrWindow::friendlyPhraseBookName( int k )
{
    return QFileInfo( phraseBookNames[k] ).fileName();
}

bool TrWindow::openPhraseBook( const QString& name )
{
    PhraseBook *pb = new PhraseBook;
    if ( !pb->load(name) ) {
	QMessageBox::warning( this, tr("Qt Linguist"),
			      tr("Cannot read from phrase book '%1'.")
			      .arg(name) );
	return FALSE;
    }

    int index = (int) phraseBooks.count();
    phraseBooks.append( pb );
    phraseBookNames.append( name );
    int id = closePhraseBookp->insertItem( friendlyPhraseBookName(index) );
    closePhraseBookp->setWhatsThis( id, tr("Close this phrase book.") );
    id = editPhraseBookp->insertItem( friendlyPhraseBookName(index) );
    editPhraseBookp->setWhatsThis( id, tr("Allow you to add, modify, or delete"
					  " phrases of this phrase book.") );
    id = printPhraseBookp->insertItem( friendlyPhraseBookName(index) );
    printPhraseBookp->setWhatsThis( id, tr("Print the entries of the phrase"
					   " book.") );
    updatePhraseDict();
    return TRUE;
}

bool TrWindow::savePhraseBook( const QString& name, const PhraseBook& pb )
{
    if ( !pb.save(name) ) {
	QMessageBox::warning( this, tr("Qt Linguist"),
			      tr("Cannot create phrase book '%1'.")
			      .arg(name) );
	return FALSE;
    }
    return TRUE;
}

void TrWindow::updateProgress()
{
    if ( numNonobsolete == 0 )
	progress->setText( QString("        ") );
    else
	progress->setText( QString(" %1/%2 ").arg(numFinished)
			   .arg(numNonobsolete) );
    prevUnfinishedAct->setEnabled( numFinished != numNonobsolete );
    nextUnfinishedAct->setEnabled( numFinished != numNonobsolete );
    prevAct->setEnabled( lv->firstChild() != 0 );
    nextAct->setEnabled( lv->firstChild() != 0 );
}

void TrWindow::updatePhraseDict()
{
    QListIterator<PhraseBook> pb = phraseBooks;
    PhraseBook::Iterator p;
    PhraseBook *ent;
    phraseDict.clear();
    while ( pb.current() != 0 ) {
	for ( p = (*pb)->begin(); p != (*pb)->end(); ++p ) {
	    QString f = friendlyString( (*p).source() );
	    if ( f.length() > 0 ) {
		f = QStringList::split( QChar(' '), f ).first();
		ent = phraseDict.find( f );
		if ( ent == 0 ) {
		    ent = new PhraseBook;
		    phraseDict.insert( f, ent );
		}
		ent->append( *p );
	    }
	}
	++pb;
    }
    revalidate();
}

PhraseBook TrWindow::getPhrases( const QString& source )
{
    PhraseBook phrases;
    QString f = friendlyString( source );
    QStringList lookupWords = QStringList::split( QChar(' '), f );
    QStringList::Iterator w;
    PhraseBook::Iterator p;

    for ( w = lookupWords.begin(); w != lookupWords.end(); ++w ) {
	PhraseBook *ent = phraseDict.find( *w );
	if ( ent != 0 ) {
	    for ( p = ent->begin(); p != ent->end(); ++p ) {
		if ( f.find(friendlyString((*p).source())) >= 0 )
		    phrases.append( *p );
	    }
	}
    }
    return phrases;
}

bool TrWindow::danger( const QString& source, const QString& translation,
		       bool verbose )
{
    if ( acceleratorsAct->isOn() ) {
	int sk = QAccel::shortcutKey( source );
	int tk = QAccel::shortcutKey( translation );
	if ( sk == 0 && tk != 0 ) {
	    if ( verbose )
		statusBar()->message( tr("Accelerator possibly superfluous in"
					 " translation."), ErrorMS );
	    return TRUE;
	} else if ( sk != 0 && tk == 0 ) {
	    if ( verbose )
		statusBar()->message( tr("Accelerator possibly missing in"
					 " translation."), ErrorMS );
	    return TRUE;
	}
    }
    if ( endingPunctuationAct->isOn() ) {
	if ( ending(source) != ending(translation) ) {
	    if ( verbose )
		statusBar()->message( tr("Translation does not end with the"
					 " same punctuation as the source"
					 " text."), ErrorMS );
	    return TRUE;
	}
    }
    if ( phraseMatchesAct->isOn() ) {
	QString fsource = friendlyString( source );
	QString ftranslation = friendlyString( translation );
	QStringList lookupWords = QStringList::split( QChar(' '), fsource );
	QStringList::Iterator w;
	PhraseBook::Iterator p;

	for ( w = lookupWords.begin(); w != lookupWords.end(); ++w ) {
	    PhraseBook *ent = phraseDict.find( *w );
	    if ( ent != 0 ) {
		for ( p = ent->begin(); p != ent->end(); ++p ) {
		    if ( fsource.find(friendlyString((*p).source())) < 0 ||
			 ftranslation.find(friendlyString((*p).target())) >= 0 )
			break;
		}
		if ( p == ent->end() ) {
		    if ( verbose )
			statusBar()->message( tr("A phrase book suggestion for"
						 " '%1' was ignored.")
						 .arg(*w), ErrorMS );
		    return TRUE;
		}
	    }
	}
    }
    if ( verbose )
	statusBar()->clear();
    return FALSE;
}

void TrWindow::readConfig()
{
    QString   keybase("/Qt Linguist/3.0/");
    QSettings config;

    config.insertSearchPath( QSettings::Windows, "/Trolltech" );

    recentFiles = config.readListEntry( keybase + "RecentlyOpenedFiles", ',' );
    QRect r( pos(), size() );
    r.setX( config.readNumEntry( keybase + "Geometry/MainwindowX", r.x() ) );
    r.setY( config.readNumEntry( keybase + "Geometry/MainwindowY", r.y() ) );
    r.setWidth( config.readNumEntry( keybase + "Geometry/MainwindowWidth",
				     r.width() ) );
    r.setHeight( config.readNumEntry( keybase + "Geometry/MainwindowHeight",
				      r.height() ) );
    QRect desk = QApplication::desktop()->geometry();
    QRect inter = desk.intersect( r );
    resize( r.size() );
    if ( inter.width() * inter.height() > ( r.width() * r.height() / 20 ) ) {
	move( r.topLeft() );
    }

    QDockWindow * dw;
    dw = (QDockWindow *) lv->parent();
    int place;
    place = config.readNumEntry( keybase + "Geometry/ScopewindowInDock" );
    r.setX( config.readNumEntry( keybase + "Geometry/ScopewindowX" ) );
    r.setY( config.readNumEntry( keybase + "Geometry/ScopewindowY" ) );
    r.setWidth( config.readNumEntry( keybase +
				     "Geometry/ScopewindowWidth" ) );
    r.setHeight( config.readNumEntry( keybase +
				      "Geometry/ScopewindowHeight" ) );
    if ( place == QDockWindow::OutsideDock )
	dw->undock();
    dw->setGeometry( r );

    dw = (QDockWindow *) slv->parent();
    place = config.readNumEntry( keybase + "Geometry/ScopewindowInDock" );
    r.setX( config.readNumEntry( keybase + "Geometry/SourcewindowX" ) );
    r.setY( config.readNumEntry( keybase + "Geometry/SourcewindowY" ) );
    r.setWidth( config.readNumEntry( keybase +
				     "Geometry/SourcewindowWidth" ) );
    r.setHeight( config.readNumEntry( keybase +
				      "Geometry/SourcewindowHeight" ) );
    if ( place == QDockWindow::OutsideDock )
	dw->undock();
    dw->setGeometry( r );

    dw = (QDockWindow *) plv->parent()->parent();
    place = config.readNumEntry( keybase + "Geometry/PhrasewindowInDock" );
    r.setX( config.readNumEntry( keybase + "Geometry/PhrasewindowX" ) );
    r.setY( config.readNumEntry( keybase + "Geometry/PhrasewindowY" ) );
    r.setWidth( config.readNumEntry( keybase +
				     "Geometry/PhrasewindowWidth" ) );
    r.setHeight( config.readNumEntry( keybase +
				      "Geometry/PhrasewindowHeight" ) );
    if ( place == QDockWindow::OutsideDock )
	dw->undock();
    dw->setGeometry( r );
    QApplication::sendPostedEvents();
//
//     QString fn = QDir::homeDirPath() + "/.linguistrc" + "tb2";
//     QFile f( fn );
//     if ( f.open( IO_ReadOnly ) ) {
// 	QTextStream ts( &f );
// 	ts >> *this;
// 	f.close();
//     }

}

void TrWindow::writeConfig()
{
    QString   keybase("/Qt Linguist/3.0/");
    QSettings config;


    config.insertSearchPath( QSettings::Windows, "/Trolltech" );
    config.writeEntry( keybase + "RecentlyOpenedFiles", recentFiles, ',' );
    config.writeEntry( keybase + "Geometry/MainwindowX", x() );
    config.writeEntry( keybase + "Geometry/MainwindowY", y() );
    config.writeEntry( keybase + "Geometry/MainwindowWidth", width() );
    config.writeEntry( keybase + "Geometry/MainwindowHeight", height() );

    QDockWindow * dw =(QDockWindow *) lv->parent();
    config.writeEntry( keybase + "Geometry/ScopewindowInDock", dw->place() );
    config.writeEntry( keybase + "Geometry/ScopewindowX", dw->x() );
    config.writeEntry( keybase + "Geometry/ScopewindowY", dw->y() );
    config.writeEntry( keybase + "Geometry/ScopewindowWidth", dw->width() );
    config.writeEntry( keybase + "Geometry/ScopewindowHeight", dw->height() );

    dw =(QDockWindow *) slv->parent();
    config.writeEntry( keybase + "Geometry/SourcewindowInDock",
		       dw->place() );
    config.writeEntry( keybase + "Geometry/SourcewindowX", dw->geometry().x() );
    config.writeEntry( keybase + "Geometry/SourcewindowY", dw->geometry().y() );
    config.writeEntry( keybase + "Geometry/SourcewindowWidth", dw->width() );
    config.writeEntry( keybase + "Geometry/SourcewindowHeight", dw->height() );

    dw =(QDockWindow *) plv->parent()->parent();
    config.writeEntry( keybase + "Geometry/PhrasewindowInDock",
		       dw->place() );
    config.writeEntry( keybase + "Geometry/PhrasewindowX", dw->geometry().x() );
    config.writeEntry( keybase + "Geometry/PhrasewindowY", dw->geometry().y() );
    config.writeEntry( keybase + "Geometry/PhrasewindowWidth", dw->width() );
    config.writeEntry( keybase + "Geometry/PhrasewindowHeight", dw->height() );

//     QString fn = QDir::homeDirPath() + "/.linguistrc" + "tb2";
//     QFile f( fn );
//     if ( f.open( IO_WriteOnly ) ) {
// 	QTextStream ts( &f );
// 	ts << *this;
// 	f.close();
//     }

}

void TrWindow::setupRecentFilesMenu()
{
    recentFilesMenu->clear();
    int id = 0;
    QStringList::Iterator it = recentFiles.begin();
    for ( ; it != recentFiles.end(); ++it )
    {
	recentFilesMenu->insertItem( *it, id );
	id++;
    }
}

void TrWindow::recentFileActivated( int id )
{
    if ( id != -1 ) {
	openFile( *recentFiles.at( id ) );
    }
}

void TrWindow::addRecentlyOpenedFile( const QString &fn, QStringList &lst )
{
    if ( lst.find( fn ) != lst.end() )
	return;
    if ( lst.count() >= 10 )
	lst.remove( lst.begin() );
    lst << fn;
}
