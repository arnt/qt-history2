#include "previewstack.h"
#include "styledbutton.h"
#include "customaction.h"

#ifndef DLLEXPORT
#define DLLEXPORT __declspec(dllexport)
#endif

#if defined(__cplusplus )
extern "C"
{
#endif

DLLEXPORT const char* enumerateWidgets()
{
    return "PreviewStack\nStyledButton";
}

DLLEXPORT QWidget* createWidget( const QString& classname, QWidget* parent, const char* name, Qt::WFlags f )
{
    if ( classname == "PreviewStack" ) {
	return new PreviewStack( parent, name, f );
    } else if ( classname == "StyledButton" ) {
	return new StyledButton( parent, name, f );
    } else {
	QStringList list = QStringList::split( '\n', enumerateWidgets() );
	if ( list.contains( classname ) )
	    qWarning(QString("Widget of class %1 not defined in this plugin").arg(classname));
	else
	    qWarning(QString("Widget of class %1 not implemented in this plugin").arg(classname));
	return 0;
    }
}

DLLEXPORT const char* enumerateActions()
{
    return "CustomAction";
}

DLLEXPORT QAction* createAction( const QString& actionname, QObject* parent )
{
    if ( actionname == "CustomAction" ) {
	return new CustomAction( parent );
    } else {
	QStringList list = QStringList::split( '\n', enumerateActions() );
	if ( list.contains( actionname ) )
	    qWarning(QString("Action %1 not defined in this plugin").arg(actionname));
	else
	    qWarning(QString("Action %1 not implemented in this plugin").arg(actionname));
	return 0;
    }
}

#if defined(__cplusplus)
}
#endif // __cplusplus
