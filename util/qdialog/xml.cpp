#include "xml.h"
#include "formeditor.h"

#include <qobject.h>
#include <qproperty.h>
#include <qstring.h>
#include <qmetaobject.h>
#include <qresource.h>

QResourceItem* qObjectToXML( QObject* o, bool _layouted )
{
  QResourceItem* t = new QResourceItem( o->className() );

  QMetaObject* m = o->metaObject();
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
	if ( !_layouted || strcmp( p->name, "geometry" ) != 0 )
	{
	  QProperty prop;
	  o->property( p->name, &prop );
	  t->setProperty( p->name, prop );
	}
      }
    }
  }

  // TODO embedded DFormWidgets

  return t;
}

QResourceItem* qObjectToXML( DObjectInfo* o, bool _layouted )
{
  QResourceItem *t = qObjectToXML( o->widget(), _layouted );

  QValueList<DObjectInfo::Connection>::Iterator it = o->connectionsBegin();
  QValueList<DObjectInfo::Connection>::Iterator end = o->connectionsEnd();
  for( ; it != end; ++it )
  {
    QResourceItem* c = new QResourceItem( "Connection" );
    c->insertAttrib( "sender", it->senderName );
    c->insertAttrib( "signal", it->signal );
    c->insertAttrib( "receiver", it->receiverName );
    if ( it->slotIsSignal )
      c->insertAttrib( "transmitter", it->slot );
    else
      c->insertAttrib( "slot", it->slot );
    t->append( c );
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
