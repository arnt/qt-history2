#include <qwidgetplugin.h>
#include <qlistbox.h>
#include <qapplication.h>
#include <qtimer.h>
#include <private/qwidgetinterface_p.h>
#include <actioninterface.h>
#include <designerinterface.h>

#include "qactivex.h"
#include "qactivexselect.h"

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

class QActiveXPlugin : public QWidgetFactoryInterface, 
		       public ActionInterface
{
public:
    QActiveXPlugin();
    ~QActiveXPlugin();

    QRESULT queryInterface( const QUuid &, QUnknownInterface ** );
    Q_REFCOUNT;

    QStringList featureList() const;

    QAction* create( const QString &classname, QObject* parent );
    QString group( const QString& ) const;
    bool location( const QString &name, Location l ) const;
    void connectTo( QUnknownInterface *appInterface );

    QWidget* create( const QString &classname, QWidget* parent = 0, const char* name = 0 );
    QIconSet iconSet( const QString& ) const;
    QString includeFile( const QString& ) const;
    QString toolTip( const QString& ) const;
    QString whatsThis( const QString& ) const;
    bool isContainer( const QString& ) const;

private:
    static DesignerInterface *designer;
};

DesignerInterface *QActiveXPlugin::designer = 0;

QActiveXPlugin::QActiveXPlugin()
{
    if ( designer )
	designer->addRef();
}

QActiveXPlugin::~QActiveXPlugin()
{
    if ( designer )
	designer->release();
}

QRESULT QActiveXPlugin::queryInterface( const QUuid &iid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( iid == IID_QUnknown )
	*iface = (QUnknownInterface*)(ActionInterface*) this;
    else if ( iid == IID_QFeatureList )
	*iface = (QFeatureListInterface*)(ActionInterface*) this;
    else if ( iid == IID_Action )
	*iface = (ActionInterface*)this;
    else if ( iid == IID_QWidgetFactory )
	*iface = (QWidgetFactoryInterface*)this;
    else 
	return QE_NOINTERFACE;

    addRef();
    return QS_OK;
}

QStringList QActiveXPlugin::featureList() const
{
    QStringList list;

    list << "QActiveX";

    return list;
}

QAction *QActiveXPlugin::create( const QString &key, QObject *parent )
{
    return 0;
}

bool QActiveXPlugin::location( const QString &name , Location l ) const
{
    return FALSE;
}

void QActiveXPlugin::connectTo( QUnknownInterface *app )
{
    if ( !app )
	return;
    app->queryInterface( IID_Designer, (QUnknownInterface**)&designer );
}

QWidget* QActiveXPlugin::create( const QString &key, QWidget* parent, const char* name )
{
    QWidget* w = 0;

    if ( key == "QActiveX" ) {
	w = new QActiveX( parent, name );
	if ( designer ) {
	    QActiveXSelect *dialog = new QActiveXSelect( parent->topLevelWidget() );
	    dialog->setActiveX( (QActiveX*)w );
	    dialog->setDesigner( designer );
	    QTimer::singleShot( 0, dialog, SLOT(openLater()) );
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

Q_EXPORT_COMPONENT()
{
    Q_CREATE_INSTANCE( QActiveXPlugin )
}
