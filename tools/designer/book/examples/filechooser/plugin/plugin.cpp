#include "plugin.h"
#include "../widget/filechooser.h"

static const char *filechooser_pixmap[] = {
    "22 22 8 1",
    "  c Gray100",
    ". c Gray97",
    "X c #4f504f",
    "o c #00007f",
    "O c Gray0",
    "+ c none",
    "@ c Gray0",
    "# c Gray0",
    "++++++++++++++++++++++",
    "++++++++++++++++++++++",
    "++++++++++++++++++++++",
    "++++++++++++++++++++++",
    "+OOOOOOOOOOOOOOOOOOOO+",
    "OOXXXXXXXXXXXXXXXXXXOO",
    "OXX.          OO OO  O",
    "OX.      oo     O    O",
    "OX.      oo     O   .O",
    "OX  ooo  oooo   O    O",
    "OX    oo oo oo  O    O",
    "OX  oooo oo oo  O    O",
    "OX oo oo oo oo  O    O",
    "OX oo oo oo oo  O    O",
    "OX  oooo oooo   O    O",
    "OX            OO OO  O",
    "OO..................OO",
    "+OOOOOOOOOOOOOOOOOOOO+",
    "++++++++++++++++++++++",
    "++++++++++++++++++++++",
    "++++++++++++++++++++++",
    "++++++++++++++++++++++"
};

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

QIconSet CustomWidgetInterface::iconSet( const QString& ) const
{
    return QIconSet( QPixmap( filechooser_pixmap ) );
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

QRESULT CustomWidgetInterface::queryInterface( const QUuid& uuid, QUnknownInterface **iface )
{
    if ( uuid == IID_QUnknown )
	*iface = (QUnknownInterface*)this;
    else if ( uuid == IID_QFeatureList )
	*iface = (QFeatureListInterface*)this;
    else if ( uuid == IID_Widget )
	*iface = (WidgetInterface*)this;

    if ( *iface )
	(*iface)->addRef();
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( CustomWidgetInterface );
}
