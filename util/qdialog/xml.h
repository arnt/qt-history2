#ifndef _xml_h__
#define _xml_h__

class QResourceItem;
class DObjectInfo;
class QMenuBar;
class QToolBar;

QResourceItem* qObjectToXML( DObjectInfo* o );
QResourceItem* qMenuBarToXML( QMenuBar* mb );
QResourceItem* qToolBarToXML( QToolBar* tb );

#endif
