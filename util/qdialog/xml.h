#ifndef _xml_h__
#define _xml_h__

class QXMLTag;
class QObject;
class QProperty;
class QString;

class DObjectInfo;

QXMLTag* qObjectToXML( QObject*, bool _layouted );
QXMLTag* qObjectToXML( DObjectInfo* o, bool _layouted );
void qPropertyToXML( QXMLTag* tag, const QProperty& prop, const QString& _name );

#endif
