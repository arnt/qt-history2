#define INCLUDE_MENUITEM_DEF

#include "xml.h"
#include "formeditor.h"

#include <qobject.h>
#include <qproperty.h>
#include <qstring.h>
#include <qmetaobject.h>
#include <qresource.h>
#include <qobjectlist.h>

// Needed since they need special handling
#include <qwizard.h>
#include <qdialog.h>
#include <qmainwindow.h>
#include <qtoolbar.h>
#include <qmenubar.h>
#include <qmenudata.h>
#include <qpopupmenu.h>
#include <qlistview.h>
#include <qheader.h>

void qListViewItemsToXML( QResourceItem* t, QListViewItem* item, int columns );

QResourceItem* qObjectToXML( DObjectInfo* o )
{
  QWidget* widget = o->widget();
  QResourceItem* t = new QResourceItem( widget->className() );

  /**
   * Save all properties of that object.
   */
  QMetaObject* m = widget->metaObject();
  if ( m )
  {
    QStringList props = m->propertyNames();
  
    QStringList::Iterator it = props.begin();
    for( ; it != props.end(); ++it )
    {
      QMetaProperty* p = m->property( *it, TRUE );
      if ( p && !p->readonly )
      {
	// Dont save absolute positions in this case
	if ( strcmp( p->name, "geometry" ) != 0 )
	{
	  // Save only custom propertie values
	  QProperty* prop = o->property( p->name );
	  if ( prop )
	    t->setProperty( p->name, *prop );
	  /* QProperty prop;
	  widget->property( p->name, &prop );
	  t->setProperty( p->name, prop ); */
	}
      }
    }
  }

  /**
   * Handle containers here. They contain DFormWidgets. We have to find
   * and save them.
   */
  if ( widget->inherits( "QWizard" ) )
  {
    QWizard* w = (QWizard*)widget;
    for( int i = 0; i < w->count(); ++i )
    {
      QWidget *pw = w->page( i );
      QResourceItem* p = new QResourceItem( "Page" );
      p->setProperty( "title", QProperty( w->title( pw ) ) );
      t->append( p );
      ASSERT( pw && pw->inherits("DFormWidget" ) );
      p->append( ((DFormWidget*)pw)->save() );
    }
  }
  else if ( widget->inherits( "QDialog" ) )
  {
    QDialog* w = (QDialog*)widget;

    const QObjectList *list = w->children();
    if ( list )
    {
      QObjectListIt it( *list );
      for( ; it.current(); ++it )
	if ( it.current()->inherits("DFormWidget") )
	{
	  QResourceItem* f = ((DFormWidget*)it.current())->save();
	  if ( f->type() == "Widget" )
	  {
	    QResourceItem* l = new QResourceItem( "Layout" );
	    t->append( l );
	    QResourceItem* v = new QResourceItem( "QVBoxLayout" );
	    l->append( v );
	    v->append( f );
	  }
	  else
	    t->append( f );
	}
    }
  }
  else if ( widget->inherits( "QMainWindow" ) )
  {
    QMainWindow* w = (QMainWindow*)widget;

    const QObjectList *list = w->children();
    if ( list )
    {
      QObjectListIt it( *list );
      for( ; it.current(); ++it )
	if ( it.current()->inherits("QToolBar") )
	  t->append( qToolBarToXML( (QToolBar*)it.current() ) );
      else if ( it.current()->inherits("QMenuBar") )
	t->append( qMenuBarToXML( (QMenuBar*)it.current() ) );
    }  

    if ( w->centralWidget() && w->centralWidget()->inherits( "DFormWidget" ) )
    {
      QResourceItem* c = new QResourceItem( "CentralWidget" );
      t->append( c );
      c->append( ((DFormWidget*)w->centralWidget())->save() );
    }
  }
  else if ( widget->inherits( "QListView" ) )
  {
    if ( t->type() == "DListView" )
      t->setType( "QListView" );
    QListView* w = (QListView*)widget;

    QResourceItem* h = new QResourceItem( "Head" );
    t->append( h );

    QHeader* header = w->header();
    for( int k = 0; k < header->count(); ++k )
    {
      QResourceItem* c = new QResourceItem( "Column" );
      h->append( c );
      c->append( new QResourceItem( header->label( k ), TRUE ) );
    }

    QResourceItem* l = new QResourceItem( "List" );
    t->append( l );
    QListViewItem* item = w->firstChild();
    if ( item )
      qListViewItemsToXML( l, item, header->count() );
  }

  QStringList::ConstIterator sit = o->customSignals().begin();
  QStringList::ConstIterator send = o->customSignals().end();
  for( ; sit != send; ++sit )
  {
    QResourceItem* c = new QResourceItem( "CustomSignal" );
    c->insertAttrib( "signature", *sit );
    t->append( c );
  }

  sit = o->customSlots().begin();
  send = o->customSlots().end();
  for( ; sit != send; ++sit )
  {
    QResourceItem* c = new QResourceItem( "CustomSlot" );
    c->insertAttrib( "signature", *sit );
    t->append( c );
  }

  return t;
}

QResourceItem* qToolBarToXML( QToolBar* tb )
{
  QResourceItem* t = new QResourceItem( "ToolBar" );

  const QObjectList *list = tb->children();
  if ( list )
  {
    QObjectListIt it( *list );
    for( ; it.current(); ++it )
      if ( it.current()->inherits( "DFormWidget" ) )
	t->append( ((DFormWidget*)it.current())->save() );
  }  

  return t;
}

void qMenuDataToXML( QResourceItem* m, QMenuData* mb )
{
  int count = mb->count();
  for( int i = 0; i < count; ++i )
  {
    int id = mb->idAt( i );
    QMenuItem* item = mb->findItem( id );
    ASSERT( item );

    QResourceItem* e = new QResourceItem( "Entry" );
    m->append( e );

    if ( !item->text().isEmpty() )
      e->setProperty( "text", QProperty( item->text() ) );
    if ( !item->whatsThis().isEmpty() )
      e->setProperty( "whatsthis", QProperty( item->whatsThis() ) );

    if ( item->popup() )
    {
      QResourceItem* p = new QResourceItem( "Menu" );
      e->append( p );
      QResourceItem* p2 = new QResourceItem( "QPopupMenu" );
      p->append( p2 );
      qMenuDataToXML( p2, item->popup() );
    }
  }
}

QResourceItem* qMenuBarToXML( QMenuBar* mb )
{
  QResourceItem* m = new QResourceItem( "MenuBar" );
  QResourceItem* m2 = new QResourceItem( "QMenuBar" );
  m->append( m2 );
  qMenuDataToXML( m2, mb );

  return m;
}

void qListViewItemsToXML( QResourceItem* t, QListViewItem* item, int columns )
{
  for( ; item; item = item->nextSibling() )
  {
    QResourceItem* li = new QResourceItem( "QListViewItem" );
    t->append( li );
    for( int i = 0; i < columns; ++i )
    {
      // TODO: Handle pixmaps here
      QResourceItem *text = new QResourceItem( "text" );
      li->append( text );
      text->append( new QResourceItem( item->text( i ), TRUE ) );
    }

    qListViewItemsToXML( li, item->firstChild(), columns );
  }
}
