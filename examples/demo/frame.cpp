/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "frame.h"

#include <qapplication.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qaccel.h>
#include <qsplitter.h>
#include <qlistbox.h>
#include <qpainter.h>
#include <qwidgetstack.h>
#include <qstylefactory.h>
#include <qaction.h>
#include <qsignalmapper.h>
#include <qdict.h>
#include <qtextcodec.h>
#include <stdlib.h>

static QTranslator *translator = 0;
static QTranslator *qt_translator = 0;

class CategoryItem : public QListBoxItem {
public:
    CategoryItem( QListBox *parent, QWidget *widget,
		  const QPixmap &p, const QString &name, int id );
    CategoryItem(QListBox *parent, QWidget *widget,
		  const QPixmap &p1, const QPixmap &p2, const QString &name, int id );
    virtual QString key( int, bool ) const;
    virtual int height( const QListBox * ) const;
    virtual int width( const QListBox * )  const;

    int id() const { return _id; }
    void setWidget(QWidget *w) { Q_ASSERT(!_widget); _widget = w; }
    QWidget *widget() const { return _widget; }
protected:
    virtual void paint( QPainter * );
private:
    int _id;
    QWidget *_widget;
    QPixmap pm_Unsel;
    QPixmap pm_Sel;
};

CategoryItem::CategoryItem( QListBox *parent, QWidget *widget,
			    const QPixmap &p, const QString &name, int id )
    : QListBoxItem( parent ),
      _id( id ),
      _widget( widget ),
      pm_Unsel( p ),
      pm_Sel( p )
{
    setText( name );
}

CategoryItem::CategoryItem( QListBox * parent, QWidget *widget, const QPixmap &p1, const QPixmap &p2,
			    const QString &name, int id )
    : QListBoxItem( parent ),
      _id( id),
      _widget( widget ),
      pm_Unsel( p1 ),
      pm_Sel( p2 )
{
    setText( name );
}



QString CategoryItem::key( int, bool ) const
{
    QString tmp;
    return tmp.sprintf( "%03d", _id );
}

int CategoryItem::height( const QListBox * ) const
{
    return 100;
}

int CategoryItem::width( const QListBox * )  const
{
    return 150;
}

void CategoryItem::paint( QPainter *p )
{
    int w = width( listBox() );
    int tx = (w-p->fontMetrics().boundingRect(text()).width())/2;
    p->drawText( tx, 80, text() );
    if ( isSelected() )
	p->drawPixmap( (w-pm_Sel.width())/2, 10, pm_Sel );
    else
    	p->drawPixmap( (w-pm_Unsel.width())/2, 10, pm_Unsel );
}

Frame::Frame( QWidget *parent, const char *name )
    : QMainWindow( parent, name )
{
    title = tr( "Qt Demo Collection" );
    setCaption( title );

    // set up the menu bar
    QMenuBar *mainMenu = menuBar();
    QPopupMenu *fileMenu = new QPopupMenu( this, "file" );
    fileMenu->insertItem( tr( "&Exit" ), this, SLOT( close() ),
			  QAccel::stringToKey( tr( "Ctrl+Q" ) ) );

    QPopupMenu *styleMenu = new QPopupMenu( this, "style" );
    styleMenu->setCheckable( TRUE );
    QActionGroup *ag = new QActionGroup( this, 0 );
    ag->setExclusive( TRUE );
    QSignalMapper *styleMapper = new QSignalMapper( this );
    connect( styleMapper, SIGNAL( mapped( const QString& ) ), this, SLOT( setStyle( const QString& ) ) );

    QStringList list = QStyleFactory::keys();
    list.sort();
    QDict<int> stylesDict( 17, FALSE );
    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
	QString style = *it;
	QString styleAccel = style;
	if ( stylesDict[styleAccel.left(1)] ) {
	    for ( uint i = 0; i < styleAccel.length(); i++ ) {
		if ( !stylesDict[styleAccel.mid( i, 1 )] ) {
		    stylesDict.insert(styleAccel.mid( i, 1 ), (const int *)1);
		    styleAccel = styleAccel.insert( i, '&' );
		    break;
		}
	    }
	} else {
	    stylesDict.insert(styleAccel.left(1), (const int *)1);
	    styleAccel = "&"+styleAccel;
	}
	QAction *a = new QAction( style, QIconSet(), styleAccel, 0, ag, 0, ag->isExclusive() );
	connect( a, SIGNAL( activated() ), styleMapper, SLOT(map()) );
	styleMapper->setMapping( a, a->text() );
    }
    ag->addTo( styleMenu );

    mainMenu->insertItem( tr( "&File" ), fileMenu );
    mainMenu->insertItem( tr( "St&yle" ), styleMenu );
    mainMenu->setItemChecked( idEnglish, TRUE );

    // category chooser
    QSplitter *splitter = new QSplitter( this );
    categories = new QListBox( splitter );
    QFont f = categories->font();
    f.setWeight( QFont::Bold );
    categories->setFont( f );

    connect( categories, SIGNAL( clicked( QListBoxItem *) ),
                   	     SLOT( clickedCategory( QListBoxItem *) ) );

    // stack for the demo widgets
    stack = new QWidgetStack( splitter );

    setCentralWidget( splitter );
}

void Frame::addCategory( QWidget *w, const QPixmap &p, const QString &n )
{
    int i = categories->count();
    stack->addWidget( w, i );
    CategoryItem *item = new CategoryItem( categories, w, p, n, i );
    if ( !stack->visibleWidget() ) {
	categories->setCurrentItem( item );
	clickedCategory( item );
    }
    if ( i < 3 && categories->height() < 3 * item->height( categories ) )
	categories->setMinimumHeight( 3 * item->height( categories ) );
}

void Frame::addCategory( QWidget *w, const QPixmap &p1, const QPixmap &p2, const QString &n )
{
    int i = categories->count();
    if(w)
	stack->addWidget( w, i );
    CategoryItem *item = new CategoryItem( categories, w, p1, p2, n, i );
    if ( !stack->visibleWidget() ) {
	categories->setCurrentItem( item );
	clickedCategory( item );
    }
    if ( i < 3 && categories->height() < 3 * item->height( categories ) )
	categories->setMinimumHeight( 3 * item->height( categories ) );
}

void Frame::setStyle( const QString& style )
{
    QStyle *s = QStyleFactory::create( style );
    if ( s )
	QApplication::setStyle( s );
}

void Frame::clickedCategory( QListBoxItem *item )
{
    if ( item ) {
	CategoryItem *c = (CategoryItem*)item;
	topLevelWidget()->setCaption( title + " - " + item->text() );
	if ( !c->widget() ) {
	    QWidget *w = createCategory(item->text());
	    if ( w ) {
		c->setWidget(w);
		stack->addWidget(w, c->id());
	    } else {
		qDebug("Lazy creation of %s failed", item->text().latin1());
	    }
	}
	stack->raiseWidget( c->widget() );
    }
}

void Frame::updateTranslators()
{
    if ( !qt_translator ) {
	qt_translator = new QTranslator( qApp );
	translator = new QTranslator( qApp );
	qApp->installTranslator( qt_translator );
	qApp->installTranslator( translator );
    }

    QString QTDIR = getenv( "QTDIR" );

    qt_translator->load( QString( "qt_%1" ).arg( QTextCodec::locale() ), QTDIR + "/translations" );
    translator->load( QString( "translations/demo_%1" ).arg( QTextCodec::locale() ) );
}

bool Frame::event( QEvent *e )
{
    if ( e->type() == QEvent::LocaleChange )
	updateTranslators();

    return QMainWindow::event( e );
}

