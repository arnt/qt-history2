#include "plugin.h"
#include "../widget/filechooser.h"

CustomWidgetInterface::CustomWidgetInterface()
    : ref( 0 )
{
}

QStringList CustomWidgetInterface::featureList() const
{
    QStringList list;
    list << "FileChooser";
    return list;
}

QWidget* CustomWidgetInterface::create( const QString &feature, QWidget* parent, const char* name )
{
    QWidget* w = 0;

    if ( feature == "FileChooser" )
	w = new FileChooser( parent, name );

    objects.add( w );
    return w;
}

QString CustomWidgetInterface::group( const QString& feature ) const
{
    if ( feature == "FileChooser" )
	return "Input";
    return QString::null;
}

QString CustomWidgetInterface::iconSet( const QString& feature ) const
{
    if ( feature == "FileChooser" )
	return "lineedit.xpm";
    return QString::null;
}

QIconSet CustomWidgetInterface::iconset( const QString& ) const
{
    return QIconSet();
}

QString CustomWidgetInterface::includeFile( const QString& feature ) const
{
    if ( feature == "FileChooser" )
	return "filechooser.h";
    return QString::null;
}

QString CustomWidgetInterface::toolTip( const QString& feature ) const
{
    if ( feature == "FileChooser" )
	return "File Chooser Widget";
    return QString::null;
}

QString CustomWidgetInterface::whatsThis( const QString& feature ) const
{
    if ( feature == "FileChooser" )
	return "A widget to choose a file or directory";
    return QString::null;
}

bool CustomWidgetInterface::isContainer( const QString& ) const
{
    return FALSE;
}

QUnknownInterface *CustomWidgetInterface::queryInterface( const QUuid& uuid )
{
    QUnknownInterface *iface = 0;

    if ( uuid == IID_QUnknownInterface )
	iface = (QUnknownInterface*)this;
    else if ( uuid == IID_QFeatureListInterface )
	iface = (QFeatureListInterface*)this;
    else if ( uuid == IID_WidgetInterface )
	iface = (WidgetInterface*)this;

    if ( iface )
	iface->addRef();

    return iface;
}

unsigned long CustomWidgetInterface::addRef()
{
    return ref++;
}

unsigned long CustomWidgetInterface::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }
    return ref;
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( CustomWidgetInterface );
}
