#ifndef _xml_h__
#define _xml_h__

class QXMLTag;
class QObject;
class QProperty;
class QString;

QXMLTag* qObjectToXML( QObject*, bool _layouted );
void qPropertyToXML( QXMLTag* tag, const QProperty& prop, const QString& _name );

#endif
