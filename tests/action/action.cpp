#include "action.h"

#include <qtoolbutton.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qtoolbar.h>
#include <qcombobox.h>
#include <qvalidator.h>
#include <qalgorithms.h>

QToggleAction::QToggleAction( const QString& text, int accel, QObject* parent, const char* name )
    : QAction( text, accel, parent, name )
{
    m_checked = FALSE;
    m_lock = FALSE;
}

QToggleAction::QToggleAction( const QString& text, int accel,
	       QObject* receiver, const char* slot, QObject* parent, const char* name )
    : QAction( text, accel, receiver, slot, parent, name )
{
    m_checked = FALSE;
    m_lock = FALSE;
}

QToggleAction::QToggleAction( const QString& text, const QIconSet& pix, int accel,
	       QObject* parent, const char* name )
    : QAction( text, pix, accel, parent, name )
{
    m_checked = FALSE;
    m_lock = FALSE;
}

QToggleAction::QToggleAction( const QString& text, const QIconSet& pix, int accel,
	       QObject* receiver, const char* slot, QObject* parent, const char* name )
    : QAction( text, pix, accel, receiver, slot, parent, name )
{
    m_checked = FALSE;
    m_lock = FALSE;
}

QToggleAction::QToggleAction( QObject* parent, const char* name )
    : QAction( parent, name )
{
    m_checked = FALSE;
    m_lock = FALSE;
}

int QToggleAction::plug( QWidget* widget )
{
    if ( !widget->inherits("QPopupMenu") && !widget->inherits("QActionWidget" ) &&
	 !widget->inherits("QToolBar") )
    {
	qDebug("Can not plug QToggleAction in %s", widget->className() );
	return -1;	
    }
    
    int index = QAction::plug( widget );
    if ( index == -1 )
	return index;
    
    if ( widget->inherits("QPopupMenu") )
    {
	int id = menuId( index );

	popupMenu( index )->setItemChecked( id, m_checked );
    }
    else if ( widget->inherits("QActionWidget" ) )
    {
    }
    else if ( widget->inherits("QToolBar") )
    {
	QToolButton* b = (QToolButton*)representative( index );
	b->setToggleButton( TRUE );
	b->setOn( m_checked );
    }

    return index;
}

void QToggleAction::setChecked( bool checked )
{
    if ( m_checked == checked )
	return;

    int len = containerCount();
    for( int i = 0; i < len; ++i )
    {
	QWidget* w = container( i );
	QWidget* r = representative( i );
	if ( w->inherits( "QToolBar" ) && r->inherits( "QToolButton" ) )
	    ((QToolButton*)r)->setOn( checked );
	else if ( w->inherits( "QPopupMenu" ) )
	    ((QPopupMenu*)w)->setItemChecked( menuId( i ), checked );
	else if ( w->inherits( "QMenuBar" ) )
	    ((QMenuBar*)w)->setItemChecked( menuId( i ), checked );
	else if ( w->inherits( "QActionWidget" ) )
	    ((QActionWidget*)w)->updateAction( this );	
    }
    
    m_checked = checked;
    
    emit activated();
    emit toggled( m_checked );
}

bool QToggleAction::isChecked()
{
    return m_checked;
}

void QToggleAction::slotActivated()
{
    if ( m_lock )
	return;
    
    m_lock = TRUE;
    setChecked( !m_checked );
    m_lock = FALSE;    
}

// ------------------------------------------------------------

QSelectAction::QSelectAction( const QString& text, int accel, QObject* parent, const char* name )
    : QAction( text, accel, parent, name )
{
    m_lock = FALSE;
    m_menu = 0;
}

QSelectAction::QSelectAction( const QString& text, int accel,
	       QObject* receiver, const char* slot, QObject* parent, const char* name )
    : QAction( text, accel, receiver, slot, parent, name )
{
    m_lock = FALSE;
    m_menu = 0;
    m_current = -1;
}

QSelectAction::QSelectAction( const QString& text, const QIconSet& pix, int accel,
	       QObject* parent, const char* name )
    : QAction( text, pix, accel, parent, name )
{
    m_lock = FALSE;
    m_menu = 0;
    m_current = -1;
}

QSelectAction::QSelectAction( const QString& text, const QIconSet& pix, int accel,
	       QObject* receiver, const char* slot, QObject* parent, const char* name )
    : QAction( text, pix, accel, receiver, slot, parent, name )
{
    m_lock = FALSE;
    m_menu = 0;
    m_current = -1;
}

QSelectAction::QSelectAction( QObject* parent, const char* name )
    : QAction( parent, name )
{
    m_lock = FALSE;
    m_menu = 0;
    m_current = -1;
}

void QSelectAction::setCurrentItem( int id )
{
    if ( id >= (int)m_list.count() )
	return;
    
    if ( m_menu )
    {
	if ( m_current >= 0 )
	    m_menu->setItemChecked( m_current, FALSE );
	if ( id >= 0 )
	    m_menu->setItemChecked( id, TRUE );
    }

    m_current = id;
	
    int len = containerCount();
    for( int i = 0; i < len; ++i )
    {
	QWidget* w = container( i );
	QWidget* r = representative( i );
	if ( w->inherits( "QToolBar" ) && r->inherits( "QComboBox" ) )
        {
	    QComboBox* b = (QComboBox*)r;
	    b->setCurrentItem( m_current );
	}
	else if ( w->inherits( "QActionWidget" ) )
	    ((QActionWidget*)w)->updateAction( this );	
    }
	    
    emit QAction::activated();
    emit activated( currentItem() );
    emit activated( currentText() );
}

QPopupMenu* QSelectAction::popupMenu()
{
    if ( !m_menu )
    {
	m_menu = new QPopupMenu;
	QStringList::ConstIterator it = m_list.begin();
	int id = 0;
	for( ; it != m_list.end(); ++it )
	    m_menu->insertItem( *it, this, SLOT( slotActivated( int ) ), 0, id++ );
    }
    
    return m_menu;
}

void QSelectAction::setItems( const QStringList& lst )
{
    m_list = lst;
    m_current = -1;
    
    if ( m_menu )
    {
	m_menu->clear();
	QStringList::ConstIterator it = m_list.begin();
	int id = 0;
	for( ; it != m_list.end(); ++it )
	    m_menu->insertItem( *it, this, SLOT( slotActivated( int ) ), 0, id++ );
    }

    int len = containerCount();
    for( int i = 0; i < len; ++i )
    {
	QWidget* w = container( i );
	QWidget* r = representative( i );
	if ( w->inherits( "QToolBar" ) && r->inherits( "QComboBox" ) )
        {
	    QComboBox* b = (QComboBox*)r;
	    b->clear();
	    QStringList::ConstIterator it = m_list.begin();
	    for( ; it != m_list.end(); ++it )
		b->insertItem( *it );
	}
	else if ( w->inherits( "QActionWidget" ) )
	    ((QActionWidget*)w)->updateAction( this );	
    }
}

QStringList QSelectAction::items()
{
    return m_list;
}

QString QSelectAction::currentText()
{
    if ( currentItem() < 0 )
	return QString::null;
    
    return m_list[ currentItem() ];
}

int QSelectAction::currentItem()
{
    return m_current;
}

int QSelectAction::plug( QWidget* widget )
{
    if ( widget->inherits("QPopupMenu") )
    {
	// Create the PopupMenu and store it in m_menu
	(void)popupMenu();
	
	QPopupMenu* menu = (QPopupMenu*)widget;
	int id;
	if ( !pixmap().isNull() )
        {
	    id = menu->insertItem( pixmap(), m_menu );
	}
	else
        {
	    if ( hasIconSet() )
		id = menu->insertItem( iconSet(), text(), m_menu );
	    else
		id = menu->insertItem( text(), m_menu );
	}

	menu->setItemEnabled( id, isEnabled() );
	menu->setWhatsThis( id, whatsThis() );

	addContainer( menu, id );
	connect( menu, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );
	
	return containerCount() - 1;
    }
    else if ( widget->inherits("QActionWidget" ) )
    {
	connect( widget, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );	
	addContainer( widget, (int)0 );
	
	return containerCount() - 1;
    }
    else if ( widget->inherits("QToolBar") )
    {
	QToolBar* bar = (QToolBar*)widget;
	QComboBox* b = new QComboBox( bar );

	QStringList::ConstIterator it = m_list.begin();
	for( ; it != m_list.end(); ++it )
	    b->insertItem( *it );
	
	b->setEnabled( isEnabled() );
	
	connect( b, SIGNAL( activated( int ) ), this, SLOT( slotActivated( int ) ) );
	
	addContainer( bar, b );
	connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );
	
	return containerCount() - 1;
    }

    qDebug("Can not plug QAction in %s", widget->className() );
    return -1;
}

void QSelectAction::slotActivated( int id )
{
    if ( m_current == id )
	return;

    if ( m_lock )
	return;
    
    m_lock = TRUE;
    setCurrentItem( id );
    m_lock = FALSE;
}

void QSelectAction::clear()
{
    if ( m_menu )
	m_menu->clear();
    
    int len = containerCount();
    for( int i = 0; i < len; ++i )
    {
	QWidget* w = container( i );
	QWidget* r = representative( i );
	if ( w->inherits( "QToolBar" ) && r->inherits( "QComboBox" ) )
        {
	    QComboBox* b = (QComboBox*)r;
	    b->clear();
	}
	else if ( w->inherits( "QActionWidget" ) )
	    ((QActionWidget*)w)->updateAction( this );	
    }
}

// ------------------------------------------------------------

QFontAction::QFontAction( const QString& text, int accel, QObject* parent, const char* name )
    : QSelectAction( text, accel, parent, name )
{
    setItems( m_fdb.families() );
}

QFontAction::QFontAction( const QString& text, int accel,
	       QObject* receiver, const char* slot, QObject* parent, const char* name )
    : QSelectAction( text, accel, receiver, slot, parent, name )
{
    setItems( m_fdb.families() );
}

QFontAction::QFontAction( const QString& text, const QIconSet& pix, int accel,
	       QObject* parent, const char* name )
    : QSelectAction( text, pix, accel, parent, name )
{
    setItems( m_fdb.families() );
}

QFontAction::QFontAction( const QString& text, const QIconSet& pix, int accel,
	       QObject* receiver, const char* slot, QObject* parent, const char* name )
    : QSelectAction( text, pix, accel, receiver, slot, parent, name )
{
    setItems( m_fdb.families() );
}

QFontAction::QFontAction( QObject* parent, const char* name )
    : QSelectAction( parent, name )
{
    setItems( m_fdb.families() );
}

// ------------------------------------------------------------

QFontSizeAction::QFontSizeAction( const QString& text, int accel, QObject* parent, const char* name )
    : QSelectAction( text, accel, parent, name )
{
    init();
}

QFontSizeAction::QFontSizeAction( const QString& text, int accel,
	       QObject* receiver, const char* slot, QObject* parent, const char* name )
    : QSelectAction( text, accel, receiver, slot, parent, name )
{
    init();
}

QFontSizeAction::QFontSizeAction( const QString& text, const QIconSet& pix, int accel,
	       QObject* parent, const char* name )
    : QSelectAction( text, pix, accel, parent, name )
{
    init();
}

QFontSizeAction::QFontSizeAction( const QString& text, const QIconSet& pix, int accel,
	       QObject* receiver, const char* slot, QObject* parent, const char* name )
    : QSelectAction( text, pix, accel, receiver, slot, parent, name )
{
    init();
}

QFontSizeAction::QFontSizeAction( QObject* parent, const char* name )
    : QSelectAction( parent, name )
{
    init();
}

void QFontSizeAction::init()
{
    m_lock = FALSE;
    
    QStringList lst;
    lst.append( "8" );
    lst.append( "9" );
    lst.append( "10" );
    lst.append( "11" );
    lst.append( "12" );
    lst.append( "14" );
    lst.append( "16" );
    lst.append( "18" );
    lst.append( "20" );
    lst.append( "24" );
    lst.append( "32" );
    lst.append( "48" );
    lst.append( "64" );
    
    setItems( lst );
}

void QFontSizeAction::setFontSize( int size )
{
    if ( size == fontSize() )
	return;
    
    if ( size < 1 || size > 128 )
    {
	qDebug( "QFontSizeAction: Size %i is out of range", size );
	return;
    }
    
    int index = items().findIndex( QString::number( size ) );
    if ( index == -1 )
    {
	QStringList lst = items();
	lst.append( QString::number( size ) );
	qHeapSort( lst );
	setItems( lst );
	index = lst.findIndex( QString::number( size ) );
	setCurrentItem( index );
    }
    else
    {
	// Avoid duplicates in combo boxes ...
	setItems( items() );
	setCurrentItem( index );
    }
    
    emit QAction::activated();
    emit activated( index );
    emit activated( QString::number( size ) );
    emit fontSizeChanged( size );
}

int QFontSizeAction::fontSize()
{
    return currentText().toInt();
}

void QFontSizeAction::slotActivated( int index )
{
    QSelectAction::slotActivated( index );
    
    emit fontSizeChanged( items()[ index ].toInt() );
}

void QFontSizeAction::slotActivated( const QString& size )
{
    if ( m_lock )
	return;

    if ( size.toInt() < 1 || size.toInt() > 128 )
    {
	qDebug( "QFontSizeAction: Size %s is out of range", size.latin1() );
	return;
    }

    m_lock = TRUE;
    setFontSize( size.toInt() );
    m_lock = FALSE;
}

int QFontSizeAction::plug( QWidget* widget )
{
    if ( widget->inherits("QToolBar") )
    {
	QToolBar* bar = (QToolBar*)widget;
	QComboBox* b = new QComboBox( TRUE, bar );
	b->setValidator( new QIntValidator( b ) );
	
	QStringList lst = items();
	QStringList::ConstIterator it = lst.begin();
	for( ; it != lst.end(); ++it )
	    b->insertItem( *it );
	
	b->setEnabled( isEnabled() );
	
	connect( b, SIGNAL( activated( const QString& ) ), this, SLOT( slotActivated( const QString& ) ) );
	
	addContainer( bar, b );
	connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );
	
	return containerCount() - 1;
    }
    
    return QSelectAction::plug( widget );
}
