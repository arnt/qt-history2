/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "connectionitems.h"
#include "formwindow.h"
#include "mainwindow.h"
#include "metadatabase.h"
#include "widgetfactory.h"
#include "project.h"
#include <qpainter.h>
#include <qcombobox.h>
#include <qmetaobject.h>
#include <qcombobox.h>
#include <qlistbox.h>
#include <qaction.h>
#include <qdatabrowser.h>

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
    : QComboTableItem( table, QStringList(), FALSE ), formWindow( fw ), conn( 0 )
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

void ConnectionItem::paint( QPainter *p, const QPalette &pal, const QRect &cr, bool selected )
{
    p->fillRect( 0, 0, cr.width(), cr.height(),
		 selected ? pal.brush( QPalette::Highlight )
			  : pal.brush( QPalette::Base ) );

    int w = cr.width();
    int h = cr.height();

    int x = 0;

    if ( currentText()[0] == '<' )
	p->setPen( QObject::red );
    else if ( selected )
	p->setPen( pal.highlightedText() );
    else
	p->setPen( pal.text() );

    QFont f( p->font() );
    QFont oldf( p->font() );
    if ( conn && conn->isModified() ) {
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

static void appendChildActions( QAction *action, QStringList &lst )
{
    QObjectList l = action->children();
    for (int i = 0; i < l.size(); ++i) {
	QObject *o = l.at(i);
	if ( !qt_cast<QAction*>(o) )
	    continue;
	lst << o->name();
	if ( !o->children().isEmpty() && qt_cast<QActionGroup*>(o) )
	    appendChildActions( (QAction*)o, lst );
    }
}

static QStringList flatActions( const QList<QAction*> &l )
{
    QStringList lst;

    for(QList<QAction *>::ConstIterator it = l.begin(); it != l.end(); ++it) {
	QAction *action = (*it);
	lst << action->name();
	if ( !action->children().isEmpty() && qt_cast<QActionGroup*>(action) )
	    appendChildActions( action, lst );
	++it;
    }

    return lst;
}

// ------------------------------------------------------------------

SenderItem::SenderItem( QTable *table, FormWindow *fw )
    : ConnectionItem( table, fw )
{
    QStringList lst;

    QHash<QWidget*, QWidget*> *widgets = formWindow->widgets();
    for(QHash<QWidget*, QWidget*>::Iterator it = widgets->begin(); it != widgets->end(); ++it) {
	if ( lst.find( (*it)->name() ) != lst.end() ) 
	    continue;
	if ( !QString( (*it)->name() ).startsWith( "qt_dead_widget_" ) &&
	     !qt_cast<QLayoutWidget*>((*it)) &&
	     !qt_cast<Spacer*>((*it)) &&
	     !qt_cast<SizeHandle*>((*it)) &&
	     qstrcmp( (*it)->name(), "central widget" ) != 0 ) {
	    lst << (*it)->name();
	}
    }

    lst += flatActions( formWindow->actionList() );

    lst.prepend( "<No Sender>" );
    lst.sort();
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
	o = formWindow->findAction( sender );
    if ( !o )
	return;
    emit currentSenderChanged( o );
}



// ------------------------------------------------------------------

ReceiverItem::ReceiverItem( QTable *table, FormWindow *fw )
    : ConnectionItem( table, fw )
{
    QStringList lst;

    QHash<QWidget*, QWidget*> *widgets = formWindow->widgets();
    for(QHash<QWidget*, QWidget*>::Iterator it = widgets->begin(); it != widgets->end(); ++it) {
	if ( lst.find( (*it)->name() ) != lst.end() ) 
	    continue;
	if ( !QString( (*it)->name() ).startsWith( "qt_dead_widget_" ) &&
	     !qt_cast<QLayoutWidget*>((*it)) &&
	     !qt_cast<Spacer*>((*it)) &&
	     !qt_cast<SizeHandle*>((*it)) &&
	     qstrcmp( (*it)->name(), "central widget" ) != 0 ) {
	    lst << (*it)->name();
	}
    }

    lst += flatActions( formWindow->actionList() );

    lst.prepend( "<No Receiver>" );
    lst.sort();
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
	o = formWindow->findAction( receiver );
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
    lst.sort();
    setStringList( lst );
}

void SignalItem::senderChanged( QObject *sender )
{
    QStringList lst;
    int signalCount = sender->metaObject()->signalCount();
    for (int i = 0; i < signalCount; ++i) {
	QString s = sender->metaObject()->signal(i).signature();
	if (s == "destroyed()"
	    || s == "destroyed(QObject*)"
	    || s == "accessibilityChanged(int)"
	    || s == "accessibilityChanged(int,int)" )
	    continue;
	lst << s;
    }
    if ( qt_cast<CustomWidget*>(sender) ) {
	MetaDataBase::CustomWidget *w = ( (CustomWidget*)sender )->customWidget();
	for ( QList<QCString>::Iterator it = w->lstSignals.begin();
	      it != w->lstSignals.end(); ++it )
	    lst << MetaDataBase::normalizeFunction( *it );
    }

    if ( sender == formWindow->mainContainer() ) {
	QStringList extra = MetaDataBase::signalList( formWindow );
	if ( !extra.isEmpty() )
	    lst += extra;
    }

    lst.prepend( "<No Signal>" );

    lst.sort();
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
    lst.sort();
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
#ifndef QT_NO_SQL
    if ( qstrcmp( slot, "update()" ) == 0 &&
	 qt_cast<QDataBrowser*>(lastReceiver) )
	return FALSE;
#endif

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
	lst.sort();
	setStringList( lst );
	return;
    }

    QString signal = MetaDataBase::normalizeFunction( lastSignal );
    int n = lastReceiver->metaObject()->slotCount();
    QStringList slts;

    for( int i = 0; i < n; ++i ) {
	// accept only public slots. For the form window, also accept protected slots
	QMetaMember mm = lastReceiver->metaObject()->slot( i );
	if ( ( mm.access() == QMetaMember::Public ||
	       (formWindow->isMainContainer( (QWidget*)lastReceiver ) &&
		mm.access() == QMetaMember::Protected) )
	     && !ignoreSlot( mm.signature() )
	     && checkConnectArgs( signal.latin1(), lastReceiver, mm.signature() ) )
	    if ( lst.find( mm.signature() ) == lst.end() )
		lst << MetaDataBase::normalizeFunction( mm.signature() );
    }

    LanguageInterface *iface =
	MetaDataBase::languageInterface( formWindow->project()->language() );
    if ( !iface || iface->supports( LanguageInterface::ConnectionsToCustomSlots ) ) {
	if ( formWindow->isMainContainer( (QWidget*)lastReceiver ) ) {
	    QList<MetaDataBase::Function> moreSlots = MetaDataBase::slotList( formWindow );
	    if ( !moreSlots.isEmpty() ) {
		for ( QList<MetaDataBase::Function>::Iterator it = moreSlots.begin();
		      it != moreSlots.end(); ++it ) {
		    QCString s = (*it).function;
		    if ( !s.data() )
			continue;
		    s = MetaDataBase::normalizeFunction( s );
		    if ( checkConnectArgs( signal.latin1(), lastReceiver, s ) ) {
			if ( lst.find( (*it).function ) == lst.end() )
			    lst << s;
		    }
		}
	    }
	}
    }

    if ( qt_cast<CustomWidget*>(lastReceiver) ) {
	MetaDataBase::CustomWidget *w = ( (CustomWidget*)lastReceiver )->customWidget();
	for ( QList<MetaDataBase::Function>::Iterator it = w->lstSlots.begin();
	      it != w->lstSlots.end(); ++it ) {
	    QCString s = (*it).function;
	    if ( !s.data() )
		continue;
	    s = MetaDataBase::normalizeFunction( s );
	    if ( checkConnectArgs( signal.latin1(), lastReceiver, s ) ) {
		if ( lst.find( (*it).function ) == lst.end() )
		    lst << s;
	    }
	}
    }

    lst.prepend( "<No Slot>" );
    lst.sort();
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
