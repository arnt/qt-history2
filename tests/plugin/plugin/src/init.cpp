#include "previewstack.h"
#include "styledbutton.h"
#include "customaction.h"
#include <qiodevice.h>
#include <qfileinfo.h>
#include <qtoolbar.h>

#ifdef _OS_WIN32_
#undef DLLEXPORT
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

#if defined(__cplusplus )
extern "C"
{
#endif

DLLEXPORT const char* infoString()
{
    return "Test plugin";
}

DLLEXPORT const char* enumerateWidgets()
{
    return "PreviewStack;StyledButton";
}

DLLEXPORT QWidget* createWidget( const QString& classname, bool init, QWidget* parent, const char* name )
{
    if ( classname == "PreviewStack" ) {
	return new PreviewStack( parent, name );
    } else if ( classname == "StyledButton" ) {
	return new StyledButton( parent, name );
    }

    return 0;
}

DLLEXPORT const char* enumerateActions()
{
    return "CustomAction";
}

DLLEXPORT QAction* createAction( const QString& actionname, bool& self, QObject* parent )
{
    if ( actionname == "CustomAction" ) {
	CustomAction* a = new CustomAction( parent );
	if ( self ) {
	    if ( parent && parent->inherits( "QMainWindow" ) ) {
		QMainWindow* mw = (QMainWindow*)parent;
		QToolBar* mytoolbar = new QToolBar( mw, infoString() );
		a->addTo( mytoolbar );
		mw->addToolBar( mytoolbar );
	    } else
		self = FALSE;
	}
	return a;
    }
    return 0;
}

DLLEXPORT const char* enumerateFileTypes()
{
    return "Visual Studio Resource script (*.rc;*.rc2);;Visual Studio Resource (*.res)\n.bla";
}

DLLEXPORT QWidget* processFile( QIODevice* f, const QString& filetype )
{
    qDebug("Imagine I process %s", filetype.latin1() );

    return 0;
}

#if defined(__cplusplus)
}
#endif // __cplusplus
