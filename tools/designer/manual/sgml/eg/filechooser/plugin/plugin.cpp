#include "plugin.h"
#include "../widget/filechooser.h"

FileChooserPlugin::FileChooserPlugin()
    : ref( 0 )
{
}

QStringList FileChooserPlugin::featureList() const
{
    QStringList list;
    list << "FileChooser";
    return list;
}

QWidget* FileChooserPlugin::create( const QString &description, QWidget* parent, const char* name )
{
    QWidget* w = 0;

    if ( description == "FileChooser" )
	w = new FileChooser( parent, name );

    objects.add( w );
    return w;
}

QString FileChooserPlugin::group( const QString& description ) const
{
    if ( description == "FileChooser" )
	return "Input";
    return QString::null;
}

QString FileChooserPlugin::iconSet( const QString& description ) const
{
    if ( description == "FileChooser" )
	return "lineedit.xpm";
    return QString::null;
}

QIconSet FileChooserPlugin::iconset( const QString& ) const
{
    return QIconSet();
}

QString FileChooserPlugin::includeFile( const QString& description ) const
{
    if ( description == "FileChooser" )
	return "filechooser.h";
    return QString::null;
}

QString FileChooserPlugin::toolTip( const QString& description ) const
{
    if ( description == "FileChooser" )
	return "File Chooser Widget";
    return QString::null;
}

QString FileChooserPlugin::whatsThis( const QString& description ) const
{
    if ( description == "FileChooser" )
	return "A widget to choose a file or directory";
    return QString::null;
}

bool FileChooserPlugin::isContainer( const QString& ) const
{
    return FALSE;
}

QUnknownInterface *FileChooserPlugin::queryInterface( const QUuid& uuid )
{
    QUnknownInterface *iface = 0;

    if ( uuid == IID_QUnknownInterface )
	iface = (QUnknownInterface*)this;
    else if ( uuid == IID_WidgetInterface )
	iface = (WidgetInterface*)this;

    if ( iface )
	iface->addRef();

    return iface;
}

unsigned long FileChooserPlugin::addRef()
{
    return ref++;
}

unsigned long FileChooserPlugin::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }
    return ref;
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( FileChooserPlugin );
}
