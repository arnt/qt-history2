#include "editorinterfaceimpl.h"
#include "languageinterfaceimpl.h"

class CommonInterface : public QUnknownInterface
{
public:
    CommonInterface();

    QUnknownInterface *queryInterface( const QUuid& );
    unsigned long addRef();
    unsigned long release();

private:
    unsigned long ref;
    EditorInterface *editorIface;
    LanguageInterfaceImpl *langIface;

};

CommonInterface::CommonInterface()
    : QUnknownInterface(), ref( 0 )
{
    editorIface = new EditorInterfaceImpl;
    langIface = new LanguageInterfaceImpl;
}

QUnknownInterface *CommonInterface::queryInterface( const QUuid &uuid )
{
    qDebug("CommonInterface::queryInterface");
    QUnknownInterface *iface = 0;
    if ( uuid == IID_QUnknownInterface )
	iface = (QUnknownInterface*)this;
    else if ( uuid == IID_EditorInterface ) {
	qDebug("returning IID_EditorInterface");
	iface = editorIface;
    } else if ( uuid == IID_LanguageInterface ) {
	qDebug("returning IID_LanguageInterface");
	iface = langIface;
    }

    if ( iface )
	iface->addRef();
    return iface;
}

unsigned long CommonInterface::addRef()
{
    return ref++;
}

unsigned long CommonInterface::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }
    return ref;
}

Q_EXPORT_INTERFACE()
{
     Q_CREATE_INSTANCE( CommonInterface )
}
