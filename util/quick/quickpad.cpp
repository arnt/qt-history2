#include "quickpad.h"
#include <qtabbar.h>
#include <qbitmap.h>
#include <qpushbt.h>
#include <qbttngrp.h>
#include "qdolistbox.h"

static const int PMT_GAP = 1;

class QNamedWidgetFactory : public QWidgetFactory {
    QString name;
public:
    QNamedWidgetFactory( const char *classname ) :
	name(classname)
    {
    }

    QWidget* instance(QWidget *parent, const char *objname)
    {
	// ### vegetable luser implementation
	if ( name == "QWidget" ) return new QWidget(parent,objname);
	if ( name == "QPushButton" ) return new QPushButton("Button",parent,objname);
	if ( name == "QButtonGroup" ) return new QButtonGroup(parent,objname);
debug("QNamedWidgetFactory::instance() - what is a %s?",(const char*)name);
	return new QWidget(parent,objname);
    }
};

class QuickTab : public QTab {
public:
    QPixmap pm;
};

class QuickTabBar : public QTabBar {
public:
    QuickTabBar(QWidget *parent, const char *name=0) :
	QTabBar(parent, name)
    {
    }

    // ### the paint event in QTabBar does too much.
    // ### we had to copy code to override the text-drawing
    // ### portion.  QT_VERSION 200 fixes that, and this function
    // ### can be deleted.
    //
    void paint( QPainter * p, QTab * base_t, bool selected ) const
    {
	QuickTab* t = (QuickTab*)base_t;

	QRect r( t->r );

	if ( selected ) {
	    p->setPen( colorGroup().background() );
	    p->drawLine( r.left()+1, r.bottom(), r.right()-2, r.bottom() );
	    p->drawLine( r.left()+1, r.bottom(), r.left()+1, r.top()+2 );
	    p->setPen( colorGroup().light() );
	    QFont bold( font() );
	    bold.setWeight( QFont::Bold );
	    p->setFont( bold );
	} else {
	    p->setPen( colorGroup().light() );
	    p->drawLine( r.left(), r.bottom(), r.right(), r.bottom() );
	    r.setRect( r.left() + 2, r.top() + 2, r.width() - 4, r.height() - 2 );
	    p->setFont( font() );
	}

	// ###
	p->fillRect( r.x()+1, r.y()+1, r.width()-2, r.height()-2, lightGray );
	
	p->drawLine( r.left(), r.bottom(), r.left(), r.top() + 2 );
	p->drawPoint( r.left()+1, r.top() + 1 );
	p->drawLine( r.left()+2, r.top(),
		     r.right() - 2, r.top() );

	p->setPen( colorGroup().dark() );
	p->drawLine( r.right() - 1, r.top() + 2, r.right() - 1, r.bottom() - 1 ); 
	p->setPen( black );
	p->drawPoint( r.right() - 1, r.top() + 1 );
	p->drawLine( r.right(), r.top() + 2, r.right(), r.bottom() - 1 ); 

	QRect br = p->fontMetrics().boundingRect( t->label );
	br.setHeight( p->fontMetrics().height() );
	br.setRect( r.left() + (r.width()-br.width())/2 - 3,
		    r.top() + (r.height()-br.height())/2,
		    br.width() + 5,
		    br.height() + 2 );

	paintLabel( p, br, t, t->id == keyboardFocusTab() );
    }

    void paintLabel( QPainter* p, const QRect& br,
		     QTab* base_t, bool has_focus ) const
    {
	QuickTab* t = (QuickTab*)base_t;

	if ( t->enabled ) {
	    p->setPen( palette().normal().text() );
	    p->drawPixmap( br.x(),
		br.y() + (br.height() - t->pm.height()) / 2, t->pm );
	    p->drawText( br, AlignCenter | ShowPrefix, t->label );
	} else {
	    if ( style() == WindowsStyle ) {
		// Windows style, disabled
		p->setPen( colorGroup().light() );
		QRect wr = br;
		wr.moveBy( 1, 1 );
		p->drawText( wr, AlignCenter | ShowPrefix, t->label );
	    }
	    p->setPen( palette().disabled().text() );
	    if ( t->pm.mask() )
		p->drawPixmap( br.x(),
		    br.y() + (br.height() - t->pm.height()) / 2, *(t->pm.mask()) );
	    else
		p->drawPixmap( br.x(),
		    br.y() + (br.height() - t->pm.height()) / 2, t->pm );
	    p->drawText( br, AlignCenter | ShowPrefix, t->label );
	}

	if ( has_focus ) {
	    if ( style() == WindowsStyle )
		p->drawWinFocusRect( br );
	    else
		p->drawRect( br );
	}
    }
};

QuickPad::QuickPad(QWidget *parent, const char *name, bool modal, WFlags f ) :
    QTabDialog(parent, name, modal, f)
{
    setOKButton(0);
    QuickTabBar *qtb = new QuickTabBar(this);
    // ### Need setBackgroundMode to be virtual - QT_VERSION 200
    qtb->setBackgroundMode( PaletteDark );
    setTabBar(qtb);

    QDragOffListBox *classes = new QDragOffListBox(this);
    classes->insertItem("QWidget");
    classes->insertItem("QGrid");
    classes->insertItem("QPushButton");
    classes->insertItem("QButtonGroup");

    // ### want to use dragged() signal, but not quite right.
    connect( classes, SIGNAL(selected(const char*)),
	     this, SLOT(pullWidgetClassed(const char*)) );

    addTab( classes, QPixmap("classes.xpm"), "Classes" );
    addTab( new QListBox(this), QPixmap("objects.xpm"), "Objects" );
    addTab( new QWidget(this), QPixmap("clipboard.xpm"), "Clipboard" );

    setMargins(0,0,0,0);
}


void QuickPad::addTab( QWidget* child, QPixmap pm, const char* text )
{
    // ### pixmap has to be ignored until QTabBar is more flexible
    // ### we want to do the Windows trick of Elid... some of the text.
    // ### QT_VERSION 200

    QuickTab* t = new QuickTab;
    t->label = QString(text).left(3);
    //t->pm = pm;
    QTabDialog::addTab( child, t );
}

/*!
  Do not call this.
*/
void QuickPad::addTab( QWidget*, const char* )
{
    fatal("wrong QuickTab::addTab called");
}

QuickPad::~QuickPad()
{
}

/*!
  \fn void QuickPad::pulled(QWidgetFactory& factory);

  Emitted when a QWidget has been pulled from the pad.  The \a factory
  can provide as many instance of the pulled widget as required.
*/

/*!
  Drops a widget onto the pad.
*/
void QuickPad::dropWidget(QWidget*)
{
}

/*!
  Slot to hit to pull a widget factory given a class name.
*/
void QuickPad::pullWidgetClassed( const char* n )
{
    QNamedWidgetFactory factory(n);
    emit pulled(factory);
}
