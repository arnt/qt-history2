#include <qwidgetplugin.h>
#include <qlistbox.h>
#include <qapplication.h>

#include "qactivex.h"
#include "qactivexselect.h"

class QActiveXSelector : public QActiveXSelect
{
public:
    QActiveXSelector( QWidget *parent )
	: QActiveXSelect( parent, 0, TRUE )
    {
    }

    QString controlName() const
    {
	return control;
    }
};

/* XPM */
static const char *icon[]={
"15 13 4 1",
"a c #000000",
"# c #808080",
". c #d4d0c8",
"b c #ffffff",
"......#a#.#a#..",
"......aba.aba..",
"......#a#.#a#..",
".......a...a...",
"....aaaaaaaaaaa",
"....a.........a",
"....a........#a",
"#a#.a.a.aaaa.#a",
"abaaaaaa.a.a.#a",
"#a#.aa.a.a.aa#a",
"....a........#a",
"....a.########a",
"....aaaaaaaaaaa"};

class QActiveXPlugin : public QWidgetPlugin
{
public:
    QActiveXPlugin();

    QStringList keys() const;
    QWidget* create( const QString &classname, QWidget* parent = 0, const char* name = 0 );
    QString group( const QString& ) const;
    QIconSet iconSet( const QString& ) const;
    QString includeFile( const QString& ) const;
    QString toolTip( const QString& ) const;
    QString whatsThis( const QString& ) const;
    bool isContainer( const QString& ) const;
};

QActiveXPlugin::QActiveXPlugin()
{
}

QStringList QActiveXPlugin::keys() const
{
    QStringList list;

    list << "QActiveX";

    return list;
}

QWidget* QActiveXPlugin::create( const QString &key, QWidget* parent, const char* name )
{
    QWidget* w = 0;

    if ( key == "QActiveX" ) {
	w = new QActiveX( parent, name );
	if ( parent && parent->topLevelWidget()->inherits( "MainWindow" ) && qApp->isA( "DesignerApplication") ) {
	    QActiveXSelector dialog( parent->topLevelWidget() );
	    if ( dialog.exec() ) {
		((QActiveX*)w)->setControl( dialog.controlName() );
	    }
	}
    }

    return w;
}

QString QActiveXPlugin::group( const QString& key ) const
{
    if ( key == "QActiveX" )
	return "Views";
    return QString::null;
}

QIconSet QActiveXPlugin::iconSet( const QString &key ) const
{
    if ( key == "QActiveX" )
	return QIconSet( icon );
    return QIconSet();
}

QString QActiveXPlugin::includeFile( const QString& key ) const
{
    if ( key == "QActiveX" )
        return "qactivex.h";
    return QString::null;
}

QString QActiveXPlugin::toolTip( const QString& key ) const
{
    if ( key == "QActiveX" )
	return QT_TR_NOOP("ActiveX control");
    return QString::null;
}

QString QActiveXPlugin::whatsThis( const QString& key ) const
{
    if ( key == "QActiveX" )
	return "ActiveX control widget";
    return QString::null;
}

bool QActiveXPlugin::isContainer( const QString& ) const
{
    return FALSE;
}

Q_EXPORT_PLUGIN( QActiveXPlugin );
