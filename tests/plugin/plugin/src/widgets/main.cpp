#include "previewstack.h"
#include "styledbutton.h"
#include "../../../qwidgetinterface.h"
#include "../../../qcleanuphandler.h"

#include <qapplication.h>

#ifdef _WS_WIN_
#undef LIBEXPORT
#define LIBEXPORT __declspec(dllexport)
#else
#define LIBEXPORT
#endif

QCleanUpHandler<QWidget>* widgets = 0;

class TestInterface : public QWidgetInterface
{
public:
    TestInterface();
    ~TestInterface();

    QString queryInterface() { return "QWidgetInterface"; }

    QString name() { return "Test Widgetplugin"; }
    QString description() { return "Test implementation of the QWidgetInterface"; }
    QString author() { return "vohi"; }

    QStringList featureList();
    QWidget* create( const QString &classname, QWidget* parent = 0, const char* name = 0 );
};

TestInterface::TestInterface()
{
    widgets = new QCleanUpHandler<QWidget>;
}

TestInterface::~TestInterface()
{
    delete widgets;
}

QStringList TestInterface::featureList()
{
    QStringList w;

    w << "StyledButton";
    w << "PreviewStack";
    
    return w;
}

QWidget* TestInterface::create( const QString &classname, QWidget* parent, const char* name )
{
    if ( classname == "StyledButton" ) {
	QWidget* w = new StyledButton( parent, name );
	widgets->addCleanUp( w );
	return w;
    } else if ( classname == "PreviewStack" ) {
	QWidget* w = new PreviewStack( parent, name );
	widgets->addCleanUp( w );
	return w;
    } else {
	return 0;
    }
}

#if defined(__cplusplus )
extern "C"
{
#endif

LIBEXPORT QWidgetInterface* loadInterface()
{
    return new TestInterface();
}

LIBEXPORT bool onConnect( QApplication* theApp )
{
    qDebug("Widget-Plugin: I've been loaded by %p", theApp );
    return TRUE;
}

LIBEXPORT bool onDisconnect( QApplication* theApp )
{
    // this is wrong now, as the interface gets destroyed (indirectly) when 
    // the toplevel widget gets closed, so the application is of course
    // not closing down yet (or at least it doesn't know it)
    if ( theApp && !theApp->closingDown() && !widgets->clean() ) {
	qDebug("Widget-Plugin: Can't be unloaded. Library is still use!" );
	return FALSE;
    }
    qDebug("Widget-Plugin: I've been unloaded by %p", theApp);

    return TRUE;
}

#if defined(__cplusplus)
}
#endif // __cplusplus


