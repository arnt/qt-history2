#ifndef _xml_h__
#define _xml_h__

class QObject;
class QResourceItem;
class DObjectInfo;

QResourceItem* qObjectToXML( QObject*, bool _layouted );
QResourceItem* qObjectToXML( DObjectInfo* o, bool _layouted );

#endif
