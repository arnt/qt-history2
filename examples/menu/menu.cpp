/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "menu.h"
#include <qcursor.h>
#include <qpopupmenu.h>
#include <qapplication.h>
#include <qmessagebox.h>
#include <qpixmap.h>
#include <qpainter.h>

/* XPM */
static const char * p1_xpm[] = {
"16 16 3 1",
" 	c None",
".	c #000000000000",
"X	c #FFFFFFFF0000",
"                ",
"                ",
"         ....   ",
"        .XXXX.  ",
" .............. ",
" .XXXXXXXXXXXX. ",
" .XXXXXXXXXXXX. ",
" .XXXXXXXXXXXX. ",
" .XXXXXXXXXXXX. ",
" .XXXXXXXXXXXX. ",
" .XXXXXXXXXXXX. ",
" .XXXXXXXXXXXX. ",
" .XXXXXXXXXXXX. ",
" .XXXXXXXXXXXX. ",
" .............. ",
"                "};

/* XPM */
static const char * p2_xpm[] = {
"16 16 3 1",
" 	c None",
".	c #000000000000",
"X	c #FFFFFFFFFFFF",
"                ",
"   ......       ",
"   .XXX.X.      ",
"   .XXX.XX.     ",
"   .XXX.XXX.    ",
"   .XXX.....    ",
"   .XXXXXXX.    ",
"   .XXXXXXX.    ",
"   .XXXXXXX.    ",
"   .XXXXXXX.    ",
"   .XXXXXXX.    ",
"   .XXXXXXX.    ",
"   .XXXXXXX.    ",
"   .........    ",
"                ",
"                "};

/* XPM */
static const char * p3_xpm[] = {
"16 16 3 1",
" 	c None",
".	c #000000000000",
"X	c #FFFFFFFFFFFF",
"                ",
"                ",
"   .........    ",
"  ...........   ",
"  ........ ..   ",
"  ...........   ",
"  ...........   ",
"  ...........   ",
"  ...........   ",
"  ...XXXXX...   ",
"  ...XXXXX...   ",
"  ...XXXXX...   ",
"  ...XXXXX...   ",
"   .........    ",
"                ",
"                "};

#if 0
/*
  Auxiliary class to provide fancy menu items with different
  fonts. Used for the "bold" and "underline" menu items in the options
  menu.
 */
class MyMenuItem : public QCustomMenuItem
{
public:
    MyMenuItem( const QString& s, const QFont& f )
	: string( s ), font( f ){};
    ~MyMenuItem(){}

    void paint( QPainter* p, const QPalette& /*pal*/, bool /*act*/, bool /*enabled*/, int x, int y, int w, int h )
    {
	p->setFont ( font );
	p->drawText( x, y, w, h, AlignLeft | AlignVCenter | DontClip | ShowPrefix, string );
    }

    QSize sizeHint()
    {
	return QFontMetrics( font ).size( AlignLeft | AlignVCenter | ShowPrefix | DontClip,  string );
    }
private:
    QString string;
    QFont font;
};
#endif

MenuExample::MenuExample( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    QPixmap p1( p1_xpm );
    QPixmap p2( p2_xpm );
    QPixmap p3( p3_xpm );
    QPopupMenu *print = new QPopupMenu( this );
    Q_CHECK_PTR( print );
    print->insertTearOffHandle();
    print->insertItem( "&Print to printer", this, SLOT(printer()) );
    print->insertItem( "Print to &file", this, SLOT(file()) );
    print->insertItem( "Print to fa&x", this, SLOT(fax()) );
    print->insertSeparator();
    print->insertItem( "Printer &Setup", this, SLOT(printerSetup()) );

    QPopupMenu *file = new QPopupMenu( this );
    Q_CHECK_PTR( file );
    file->insertItem( p1, "&Open",  this, SLOT(open()), Qt::CTRL+Qt::Key_O );
    file->insertItem( p2, "&New", this, SLOT(news()), Qt::CTRL+Qt::Key_N );
    file->insertItem( p3, "&Save", this, SLOT(save()), Qt::CTRL+Qt::Key_S );
    file->insertItem( "&Close", this, SLOT(closeDoc()), Qt::CTRL+Qt::Key_W );
    file->insertSeparator();
    file->insertItem( "&Print", print, Qt::CTRL+Qt::Key_P );
    file->insertSeparator();
    file->insertItem( "E&xit",  qApp, SLOT(quit()), Qt::CTRL+Qt::Key_Q );

    QPopupMenu *edit = new QPopupMenu( this );
    Q_CHECK_PTR( edit );
    int undoID = edit->insertItem( "&Undo", this, SLOT(undo()) );
    int redoID = edit->insertItem( "&Redo", this, SLOT(redo()) );
    edit->setItemEnabled( undoID, FALSE );
    edit->setItemEnabled( redoID, FALSE );

    QPopupMenu* options = new QPopupMenu( this );
    Q_CHECK_PTR( options );
    options->insertTearOffHandle();
    options->setWindowTitle("Options");
    options->insertItem( "&Normal Font", this, SLOT(normal()) );
    options->insertSeparator();

#if 0
    QFont f = options->font();
    f.setBold( TRUE );
    boldID = options->insertItem( new MyMenuItem( "Bold", f ) );
    options->setAccel( Qt::CTRL+Qt::Key_B, boldID );
    options->connectItem( boldID, this, SLOT(bold()) );
    f = font();
    f.setUnderline( TRUE );
    underlineID = options->insertItem( new MyMenuItem( "Underline", f ) );
    options->setAccel( Qt::CTRL+Qt::Key_U, underlineID );
    options->connectItem( underlineID, this, SLOT(underline()) );
#else
//#warning "Do we want something like this?! -Sam"
#endif

    isBold = FALSE;
    isUnderline = FALSE;
    options->setCheckable( TRUE );


    QPopupMenu *help = new QPopupMenu( this );
    Q_CHECK_PTR( help );
    help->insertItem( "&About", this, SLOT(about()), Qt::CTRL+Qt::Key_H );
    help->insertItem( "About &Qt", this, SLOT(aboutQt()) );

    // If we used a QMainWindow we could use its built-in menuBar().
    menu = new QMenuBar( this );
    Q_CHECK_PTR( menu );
    menu->insertItem( "&File", file );
    menu->insertItem( "&Edit", edit );
    menu->insertItem( "&Options", options );
    menu->insertSeparator();
    menu->insertItem( "&Help", help );
    menu->setSeparator( QMenuBar::InWindowsStyle );


    QLabel *msg = new QLabel( this );
    Q_CHECK_PTR( msg );
    msg->setText( "A context menu is available.\n"
		  "Invoke it by right-clicking or by"
		  " pressing the 'context' button." );
    msg->setGeometry( 0, height() - 60, width(), 60 );
    msg->setAlignment( Qt::AlignCenter );

    label = new QLabel( this );
    Q_CHECK_PTR( label );
    label->setGeometry( 20, rect().center().y()-20, width()-40, 40 );
    label->setFrameStyle( QFrame::Box | QFrame::Raised );
    label->setLineWidth( 1 );
    label->setAlignment( Qt::AlignCenter );

    connect( this,  SIGNAL(explain(const QString&)),
	     label, SLOT(setText(const QString&)) );

    setMinimumSize( 100, 80 );
    setFocusPolicy( Qt::ClickFocus );
}

class MyFancyMenu : public QMenu
{
    const QString text;
    int margin;
    QFont myFont() const {
        QFont ret = font();
        ret.setPixelSize(30);
        ret.setBold(true);
        return ret;
    }
public:
    MyFancyMenu(const QString &t, QWidget *widget) : QMenu(widget), text(t) {
        margin = QFontMetrics(myFont()).height() + 10;
        setContentsMargins(margin, 0, 0, 0);
    }

    QSize sizeHint() const {
        QSize ret = QMenu::sizeHint();
        int length = QFontMetrics(myFont()).width(text);
        if(ret.height() < length)
            ret.setHeight(length);
        return ret;
    }
protected:

    void paintEvent(QPaintEvent *e) {
        QMenu::paintEvent(e);

        QPainter p(this);
        //draw gradiant
        p.setClipRect(QRect(0, 0, margin, height()));
        p.setBrush(QBrush(QPoint(margin/2, 0), Qt::black, QPoint(margin/2, height()), Qt::red));
        p.drawRect(0, 0, margin, height());
        //draw text
        p.setPen(Qt::white);
        p.setFont(myFont());
        p.translate((margin/2)+(p.fontMetrics().ascent()/2), height());
        p.rotate(-90);
        p.drawText(0, 0, text);
    }
};

void MenuExample::contextMenuEvent( QContextMenuEvent * )
{
    MyFancyMenu contextMenu(tr("Context!"), this);
    contextMenu.addAction("&New",  this, SLOT(news()))->setShortcut( Qt::CTRL+Qt::Key_N);
    contextMenu.addAction("&Open...", this, SLOT(open()))->setShortcut( Qt::CTRL+Qt::Key_O);
    contextMenu.addAction("&Save", this, SLOT(save()))->setShortcut( Qt::CTRL+Qt::Key_S);
    QMenu *submenu = new QMenu(this);
    submenu->addAction("&Print to printer", this, SLOT(printer()));
    submenu->addAction("Print to &file", this, SLOT(file()));
    submenu->addAction("Print to fa&x", this, SLOT(fax()));
    contextMenu.addMenu("&Print", submenu);
    contextMenu.exec(QCursor::pos());
}


void MenuExample::open()
{
    emit explain( "File/Open selected" );
}


void MenuExample::news()
{
    emit explain( "File/New selected" );
}

void MenuExample::save()
{
    emit explain( "File/Save selected" );
}


void MenuExample::closeDoc()
{
    emit explain( "File/Close selected" );
}


void MenuExample::undo()
{
    emit explain( "Edit/Undo selected" );
}


void MenuExample::redo()
{
    emit explain( "Edit/Redo selected" );
}


void MenuExample::normal()
{
    isBold = FALSE;
    isUnderline = FALSE;
    QFont font;
    label->setFont( font );
    menu->setItemChecked( boldID, isBold );
    menu->setItemChecked( underlineID, isUnderline );
    emit explain( "Options/Normal selected" );
}


void MenuExample::bold()
{
    isBold = !isBold;
    QFont font;
    font.setBold( isBold );
    font.setUnderline( isUnderline );
    label->setFont( font );
    menu->setItemChecked( boldID, isBold );
    emit explain( "Options/Bold selected" );
}


void MenuExample::underline()
{
    isUnderline = !isUnderline;
    QFont font;
    font.setBold( isBold );
    font.setUnderline( isUnderline );
    label->setFont( font );
    menu->setItemChecked( underlineID, isUnderline );
    emit explain( "Options/Underline selected" );
}


void MenuExample::about()
{
    QMessageBox::about( this, "Qt Menu Example",
			"This example demonstrates simple use of Qt menus.\n"
			"You can cut and paste lines from it to your own\n"
			"programs." );
}


void MenuExample::aboutQt()
{
    QMessageBox::aboutQt( this, "Qt Menu Example" );
}


void MenuExample::printer()
{
    emit explain( "File/Printer/Print selected" );
}

void MenuExample::file()
{
    emit explain( "File/Printer/Print To File selected" );
}

void MenuExample::fax()
{
    emit explain( "File/Printer/Print To Fax selected" );
}

void MenuExample::printerSetup()
{
    emit explain( "File/Printer/Printer Setup selected" );
}


void MenuExample::resizeEvent( QResizeEvent * )
{
    label->setGeometry( 20, rect().center().y()-20, width()-40, 40 );
}


int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    MenuExample m;
    m.setWindowTitle("Qt Examples - Menus");
    a.setMainWidget( &m );
    m.show();
    return a.exec();
}
