#include "connectionitems.h"
#include "formwindow.h"
#include "mainwindow.h"
#include <qpainter.h>
#include "metadatabase.h"
#include <qcombobox.h>
#include <qmetaobject.h>
#include "widgetfactory.h"
#include "project.h"
#include <qcombobox.h>
#include <qlistbox.h>
#include <qaction.h>

static const char* const ignore_slots[] = {
    "destroyed()",
    "setCaption(const QString&)",
    "setIcon(const QPixmap&)",
    "setIconText(const QString&)",
    "setMouseTracking(bool)",
    "clearFocus()",
    "setUpdatesEnabled(bool)",
    "update()",
    "update(int,int,int,int)",
    "update(const QRect&)",
    "repaint()",
    "repaint(bool)",
    "repaint(int,int,int,int,bool)",
    "repaint(const QRect&,bool)",
    "repaint(const QRegion&,bool)",
    "show()",
    "hide()",
    "iconify()",
    "showMinimized()",
    "showMaximized()",
    "showFullScreen()",
    "showNormal()",
    "polish()",
    "constPolish()",
    "raise()",
    "lower()",
    "stackUnder(QWidget*)",
    "move(int,int)",
    "move(const QPoint&)",
    "resize(int,int)",
    "resize(const QSize&)",
    "setGeometry(int,int,int,int)",
    "setGeometry(const QRect&)",
    "focusProxyDestroyed()",
    "showExtension(bool)",
    "setUpLayout()",
    "showDockMenu(const QPoint&)",
    "init()",
    "destroy()",
    "deleteLater()",
    0
};

ConnectionItem::ConnectionItem( QTable *table, FormWindow *fw )
    : QComboTableItem( table, QStringList(), FALSE ), formWindow( fw )
{
    setReplaceable( FALSE );
}

void ConnectionItem::senderChanged( QObject * )
{
    emit changed();
    QWidget *w = table()->cellWidget( row(), col() );
    if ( w )
	setContentFromEditor( w );
}

void ConnectionItem::receiverChanged( QObject * )
{
    emit changed();
    QWidget *w = table()->cellWidget( row(), col() );
    if ( w )
	setContentFromEditor( w );
}

void ConnectionItem::signalChanged( const QString & )
{
    emit changed();
    QWidget *w = table()->cellWidget( row(), col() );
    if ( w )
	setContentFromEditor( w );
}

void ConnectionItem::slotChanged( const QString & )
{
    emit changed();
    QWidget *w = table()->cellWidget( row(), col() );
    if ( w )
	setContentFromEditor( w );
}

void ConnectionItem::setSender( SenderItem *i )
{
    connect( i, SIGNAL( currentSenderChanged( QObject * ) ),
	     this, SLOT( senderChanged( QObject * ) ) );
}

void ConnectionItem::setReceiver( ReceiverItem *i )
{
    connect( i, SIGNAL( currentReceiverChanged( QObject * ) ),
	     this, SLOT( receiverChanged( QObject * ) ) );
}

void ConnectionItem::setSignal( SignalItem *i )
{
    connect( i, SIGNAL( currentSignalChanged( const QString & ) ),
	     this, SLOT( signalChanged( const QString & ) ) );
}

void ConnectionItem::setSlot( SlotItem *i )
{
    connect( i, SIGNAL( currentSlotChanged( const QString & ) ),
	     this, SLOT( slotChanged( const QString & ) ) );
}

void ConnectionItem::paint( QPainter *p, const QColorGroup &cg,
			    const QRect &cr, bool selected )
{
    p->fillRect( 0, 0, cr.width(), cr.height(),
		 selected ? cg.brush( QColorGroup::Highlight )
			  : cg.brush( QColorGroup::Base ) );

    int w = cr.width();
    int h = cr.height();

    int x = 0;

    if ( selected )
	p->setPen( cg.highlightedText() );
    else
	p->setPen( cg.text() );

    QFont f( p->font() );
    QFont oldf( p->font() );
    if ( conn->isModified() ) {
	f.setBold( TRUE );
	p->setFont( f );
    }

    p->drawText( x + 2, 0, w - x - 4, h, alignment(), currentText() );
    p->setFont( oldf );
}

void ConnectionItem::setConnection( ConnectionContainer *c )
{
    conn = c;
}


// ------------------------------------------------------------------

SenderItem::SenderItem( QTable *table, FormWindow *fw )
    : ConnectionItem( table, fw )
{
    QStringList lst;

    QPtrDictIterator<QWidget> it( *formWindow->widgets() );
    while ( it.current() ) {
	if ( lst.find( it.current()->name() ) != lst.end() ) {
	    ++it;
	    continue;
	}
	if ( !QString( it.current()->name() ).startsWith( "qt_dead_widget_" ) &&
	     !it.current()->inherits( "QLayoutWidget" ) &&
	     !it.current()->inherits( "Spacer" ) &&
	     !it.current()->inherits( "SizeHandle" ) &&
	     qstrcmp( it.current()->name(), "central widget" ) != 0 ) {
	    lst << it.current()->name();
	}
	++it;
    }

    QPtrListIterator<QAction> it2( formWindow->actionList() );
    while ( it2.current() ) {
	lst << it2.current()->name();
	++it2;
    }

    lst.prepend( "<No Sender>" );
    setStringList( lst );
}

QWidget *SenderItem::createEditor() const
{
    QComboBox *cb = (QComboBox*)ConnectionItem::createEditor();
    cb->listBox()->setMinimumWidth( cb->fontMetrics().width( "01234567890123456789012345678901234567890123456789" ) );
    connect( cb, SIGNAL( activated( const QString & ) ),
	     this, SLOT( senderChanged( const QString & ) ) );
    return cb;
}

void SenderItem::setSenderEx( QObject *sender )
{
    setCurrentItem( sender->name() );
    emit currentSenderChanged( sender );
}

void SenderItem::senderChanged( const QString &sender )
{
    QObject *o = formWindow->child( sender, "QObject" );
    if ( !o )
	return;
    emit currentSenderChanged( o );
}



// ------------------------------------------------------------------

ReceiverItem::ReceiverItem( QTable *table, FormWindow *fw )
    : ConnectionItem( table, fw )
{
    QStringList lst;

    QPtrDictIterator<QWidget> it( *formWindow->widgets() );
    while ( it.current() ) {
	if ( lst.find( it.current()->name() ) != lst.end() ) {
	    ++it;
	    continue;
	}
	if ( !QString( it.current()->name() ).startsWith( "qt_dead_widget_" ) &&
	     !it.current()->inherits( "QLayoutWidget" ) &&
	     !it.current()->inherits( "Spacer" ) &&
	     !it.current()->inherits( "SizeHandle" ) &&
	     qstrcmp( it.current()->name(), "central widget" ) != 0 ) {
	    lst << it.current()->name();
	}
	++it;
    }

    QPtrListIterator<QAction> it2( formWindow->actionList() );
    while ( it2.current() ) {
	lst << it2.current()->name();
	++it2;
    }

    lst.prepend( "<No Receiver>" );
    setStringList( lst );
}

QWidget *ReceiverItem::createEditor() const
{
    QComboBox *cb = (QComboBox*)ConnectionItem::createEditor();
    cb->listBox()->setMinimumWidth( cb->fontMetrics().width( "01234567890123456789012345678901234567890123456789" ) );
    connect( cb, SIGNAL( activated( const QString & ) ),
	     this, SLOT( receiverChanged( const QString & ) ) );
    return cb;
}

void ReceiverItem::setReceiverEx( QObject *receiver )
{
    setCurrentItem( receiver->name() );
    emit currentReceiverChanged( receiver );
}

void ReceiverItem::receiverChanged( const QString &receiver )
{
    QObject *o = formWindow->child( receiver, "QObject" );
    if ( !o )
	return;
    emit currentReceiverChanged( o );
}



// ------------------------------------------------------------------

SignalItem::SignalItem( QTable *table, FormWindow *fw )
    : ConnectionItem( table, fw )
{
    QStringList lst;
    lst << "<No Signal>";
    setStringList( lst );
}

void SignalItem::senderChanged( QObject *sender )
{
    QStrList sigs = sender->metaObject()->signalNames( TRUE );
    sigs.remove( "destroyed()" );
    sigs.remove( "destroyed(QObject*)" );
    sigs.remove( "accessibilityChanged(int)" );
    sigs.remove( "accessibilityChanged(int,int)" );

    QStringList lst = QStringList::fromStrList( sigs );

    if ( sender->inherits( "CustomWidget" ) ) {
	MetaDataBase::CustomWidget *w = ( (CustomWidget*)sender )->customWidget();
	for ( QValueList<QCString>::Iterator it = w->lstSignals.begin();
	      it != w->lstSignals.end(); ++it )
	    lst << MetaDataBase::normalizeSlot( *it );
    }

    if ( sender == formWindow->mainContainer() ) {
	QStringList extra = MetaDataBase::signalList( formWindow );
	if ( !extra.isEmpty() )
	    lst += extra;
    }

    lst.prepend( "<No Signal>" );

    setStringList( lst );

    ConnectionItem::senderChanged( sender );
}

QWidget *SignalItem::createEditor() const
{
    QComboBox *cb = (QComboBox*)ConnectionItem::createEditor();
    cb->listBox()->setMinimumWidth( cb->fontMetrics().width( "01234567890123456789012345678901234567890123456789" ) );
    connect( cb, SIGNAL( activated( const QString & ) ),
	     this, SIGNAL( currentSignalChanged( const QString & ) ) );
    return cb;
}

// ------------------------------------------------------------------

SlotItem::SlotItem( QTable *table, FormWindow *fw )
    : ConnectionItem( table, fw )
{
    QStringList lst;
    lst << "<No Slot>";
    setStringList( lst );

    lastReceiver = 0;
    lastSignal = "<No Signal>";
}

void SlotItem::receiverChanged( QObject *receiver )
{
    lastReceiver = receiver;
    updateSlotList();
    ConnectionItem::receiverChanged( receiver );
}

void SlotItem::signalChanged( const QString &signal )
{
    lastSignal = signal;
    updateSlotList();
    ConnectionItem::signalChanged( signal );
}

bool SlotItem::ignoreSlot( const char* slot ) const
{
    if ( qstrcmp( slot, "update()" ) == 0 &&
	 lastReceiver->inherits( "QDataBrowser" ) )
	return FALSE;

    for ( int i = 0; ignore_slots[i]; i++ ) {
	if ( qstrcmp( slot, ignore_slots[i] ) == 0 )
	    return TRUE;
    }

    if ( !formWindow->isMainContainer( (QWidget*)lastReceiver ) ) {
	if ( qstrcmp( slot, "close()" ) == 0  )
	    return TRUE;
    }

    if ( qstrcmp( slot, "setFocus()" ) == 0  )
	if ( lastReceiver->isWidgetType() &&
	     ( (QWidget*)lastReceiver )->focusPolicy() == QWidget::NoFocus )
	    return TRUE;

    return FALSE;
}

void SlotItem::updateSlotList()
{
    QStringList lst;
    if ( !lastReceiver || lastSignal == "<No Signal>" ) {
	lst << "<No Slot>";
	setStringList( lst );
	return;
    }

    QString signal = MetaDataBase::normalizeSlot( lastSignal );
    int n = lastReceiver->metaObject()->numSlots( TRUE );
    QStringList slts;

    for( int i = 0; i < n; ++i ) {
	// accept only public slots. For the form window, also accept protected slots
	const QMetaData* md = lastReceiver->metaObject()->slot( i, TRUE );
	if ( ( (lastReceiver->metaObject()->slot( i, TRUE )->access == QMetaData::Public) ||
	       (formWindow->isMainContainer( (QWidget*)lastReceiver ) &&
		lastReceiver->metaObject()->slot(i, TRUE)->access ==
		QMetaData::Protected) ) &&
	     !ignoreSlot( md->name ) &&
	     checkConnectArgs( signal.latin1(), lastReceiver, md->name ) )
	    if ( lst.find( md->name ) == lst.end() )
		lst << MetaDataBase::normalizeSlot( md->name );
    }

    LanguageInterface *iface =
	MetaDataBase::languageInterface( formWindow->project()->language() );
    if ( !iface || iface->supports( LanguageInterface::ConnectionsToCustomSlots ) ) {
	if ( formWindow->isMainContainer( (QWidget*)lastReceiver ) ) {
	    QValueList<MetaDataBase::Slot> moreSlots = MetaDataBase::slotList( formWindow );
	    if ( !moreSlots.isEmpty() ) {
		for ( QValueList<MetaDataBase::Slot>::Iterator it = moreSlots.begin();
		      it != moreSlots.end(); ++it ) {
		    QCString s = (*it).slot;
		    if ( !s.data() )
			continue;
		    s = MetaDataBase::normalizeSlot( s );
		    if ( checkConnectArgs( signal.latin1(), lastReceiver, s ) ) {
			if ( lst.find( (*it).slot ) == lst.end() )
			    lst << s;
		    }
		}
	    }
	}
    }

    if ( lastReceiver->inherits( "CustomWidget" ) ) {
	MetaDataBase::CustomWidget *w = ( (CustomWidget*)lastReceiver )->customWidget();
	for ( QValueList<MetaDataBase::Slot>::Iterator it = w->lstSlots.begin();
	      it != w->lstSlots.end(); ++it ) {
	    QCString s = (*it).slot;
	    if ( !s.data() )
		continue;
	    s = MetaDataBase::normalizeSlot( s );
	    if ( checkConnectArgs( signal.latin1(), lastReceiver, s ) ) {
		if ( lst.find( (*it).slot ) == lst.end() )
		    lst << s;
	    }
	}
    }

    lst.prepend( "<No Slot>" );
    setStringList( lst );
}

QWidget *SlotItem::createEditor() const
{
    QComboBox *cb = (QComboBox*)ConnectionItem::createEditor();
    cb->listBox()->setMinimumWidth( cb->fontMetrics().width( "01234567890123456789012345678901234567890123456789" ) );
    connect( cb, SIGNAL( activated( const QString & ) ),
	     this, SIGNAL( currentSlotChanged( const QString & ) ) );
    return cb;
}

void SlotItem::customSlotsChanged()
{
    QString currSlot = currentText();
    updateSlotList();
    setCurrentItem( "<No Slot>" );
    setCurrentItem( currSlot );
    emit currentSlotChanged( currentText() );
}
