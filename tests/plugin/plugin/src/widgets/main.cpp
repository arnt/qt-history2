#include "previewstack.h"
#include "styledbutton.h"
#include "../../../qwidgetinterface.h"

#ifdef _WS_WIN_
#undef LIBEXPORT
#define LIBEXPORT __declspec(dllexport)
#else
#define LIBEXPORT
#endif

class TestInterface : public QWidgetInterface
{
public:
    TestInterface() {}
    QString queryInterface() { return "QWidgetInterface"; }

    QString name() { return "Test Widgetplugin"; }
    QString description() { return "Test implementation of the QWidgetInterface"; }
    QString author() { return "vohi"; }

    QStringList featureList();
    QWidget* create( const QString &classname, QWidget* parent = 0, const char* name = 0 );
};

QStringList TestInterface::featureList()
{
    QStringList w;

    w << "StyledButton";
    w << "PreviewStack";
    
    return w;
}

QWidget* TestInterface::create( const QString &classname, QWidget* parent, const char* name )
{
    if ( classname == "StyledButton" )
	return new StyledButton( parent, name );
    else if ( classname == "PreviewStack" )
	return new PreviewStack( parent, name );
    else
	return 0;
}

#if defined(__cplusplus )
extern "C"
{
#endif

LIBEXPORT QWidgetInterface* loadInterface()
{
    return new TestInterface();
}

LIBEXPORT bool onConnect()
{
    qDebug("I've been loaded!");
    return TRUE;
}

LIBEXPORT bool onDisconnect()
{
    qDebug("I've been unloaded!");
    return TRUE;
}

#if defined(__cplusplus)
}
#endif // __cplusplus


