#include "qwidgetfactory.h"
#include <widgetdatabase.h>
#include <widgetplugin.h>
#include <qmodules.h>
#include "../integration/kdevelop/kdewidgets.h"
#include <qdom.h>
#include <qfile.h>
#include <qlayout.h>
#include <qmetaobject.h>
#include <domtool.h>
#include <qapplication.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <zlib.h>
#include <qobjectlist.h>

// include all Qt widgets
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qgroupbox.h>
#include <qbuttongroup.h>
#include <qiconview.h>
#include <qtable.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qmultilineedit.h>
#include <qlabel.h>
#include <qwidget.h>
#include <qtabwidget.h>
#include <qcombobox.h>
#include <qdialog.h>
#include <qwizard.h>
#include <qlcdnumber.h>
#include <qprogressbar.h>
#include <qtextview.h>
#include <qtextbrowser.h>
#include <qdial.h>
#include <qslider.h>
#include <qframe.h>
#include <qwidgetstack.h>
#include <qtextedit.h>
#include <qscrollbar.h>

static QList<QWidgetFactory> widgetFactories;


/*!
  \class QWidgetFactory

  \brief A class to dynamically create widgets from Qt Designer user
  interface description files

  This class basically offers two things:

  <ul>

  <li>Dynamically creating widgets from Qt Designer user interface
  description files. You can do that using the static function
  QWidgetFactory::create(). This function also performs signal and
  slot connections, tab ordering, etc. as defined in the ui file and
  returns the toplevel widget of the ui file. After that you can use
  QObject::child() and QObject::queryList() to access child widgets of
  this returned widget.

  <li>Adding additional widget factories to be able to create custom
  widgets. See createWidget() for details.

  </ul>
*/

/*! Constructs a QWidgetFactory. */

QWidgetFactory::QWidgetFactory()
{
}

/*! Loads the Qt Designer user interface description file \a uiFile
  and returns the toplevel widget of that description. \a parent and
  \a name are passed to the constructor of the toplevel widget.

  This function also performs signal and slot connections, tab
  ordering, etc. as described in the ui file. In the Qt Designer it is
  possible to add custom slots to a form and connect to them. If you
  want that these connections are performed as well, you have to
  create a class derived from QObject, which implementes all these
  slots. Then pass an instance of it as \a connector to this
  function. This way these connections to custom slots will be done
  using the \a connector as slot.

  If something fails, 0 is returned.

  The ownership of the returned widget is passed to the caller.
*/

QWidget *QWidgetFactory::create( const QString &uiFile, QObject *connector, QWidget *parent, const char *name )
{
    QFile f( uiFile );
    if ( !f.open( IO_ReadOnly ) )
	return 0;

    QDomDocument doc;
    if ( !doc.setContent( &f ) )
	return 0;

    QWidgetFactory *widgetFactory = new QWidgetFactory;
    widgetFactory->toplevel = 0;

    QDomElement firstWidget = doc.firstChild().toElement().firstChild().toElement();

    while ( firstWidget.tagName() != "widget" )
	firstWidget = firstWidget.nextSibling().toElement();

    QDomElement connections = firstWidget;
    while ( connections.tagName() != "connections" && !connections.isNull() )
	connections = connections.nextSibling().toElement();

    QDomElement imageCollection = firstWidget;
    while ( imageCollection.tagName() != "images" && !imageCollection.isNull() )
	imageCollection = imageCollection.nextSibling().toElement();

    QDomElement tabOrder = firstWidget;
    while ( tabOrder.tagName() != "tabstops" && !tabOrder.isNull() )
	tabOrder = tabOrder.nextSibling().toElement();

    if ( !imageCollection.isNull() )
	widgetFactory->loadImageCollection( imageCollection );

    QWidget *w = widgetFactory->createWidgetInternal( firstWidget, parent, 0 );
    if ( !w ) {
	delete widgetFactory;
	return 0;
    }
    if ( w && name && qstrlen( name ) > 0 )
	w->setName( name );

    if ( !connections.isNull() )
	widgetFactory->loadConnections( connections, connector );
    if ( !tabOrder.isNull() )
	widgetFactory->loadTabOrder( tabOrder );

    delete widgetFactory;

    return w;
}

/*! Installs a widget factory \a factory, which normally contains
  additional widgets that can be created using a QWidgetFactory
  then. See createWidget() for further details.
*/

void QWidgetFactory::addWidgetFactory( QWidgetFactory *factory )
{
    widgetFactories.append( factory );
}

/*!  Creates the widget of the type \c className passing \a parent and
  \a name to its constructor. If \a className is a widget of the Qt
  library, it is directly created in this function. if this fails, all
  installed widget plugins are asked to create that widget. If this
  fails all installed widget factories are asked to create it (see
  addWidgetFactory()). If this fails as well, 0 is returned.

  If you have a custom widget, and want it to be created using the
  widget factory, you have two possibilities to add it:

  <ul>

  <li>First you can write a widget plugin. This allows you to use that
  widget in the Qt Designer and in this QWidgetFactory. See the widget
  plugin documentation for further details .

  <li>The other possibility is to subclass QWidgetFactory. Then
  reimplement this function. There create and return an instance of
  your custom widget, if \a className equals the name of your widget,
  otherwise return 0. Then at the beginning of your program where you
  want to use the widget factory to create widgets do a

  \code
  QWidgetFactory::addWidgetFactory( new MyWidgetFactory );
  \endcode

  (where MyWidgetFactory is your QWidgetFactory subclass)

  </ul>

*/

QWidget *QWidgetFactory::createWidget( const QString &className, QWidget *parent, const char *name ) const
{
    // create widgets we know
    if ( className == "QPushButton" ) {
	return new QPushButton( parent, name );
    } else if ( className == "QToolButton" ) {
	return new QToolButton( parent, name );
    } else if ( className == "QCheckBox" ) {
	return new QCheckBox( parent, name );
    } else if ( className == "QRadioButton" ) {
	return new QRadioButton( parent, name );
    } else if ( className == "QGroupBox" ) {
	return new QGroupBox( parent, name );
    } else if ( className == "QButtonGroup" ) {
	return new QButtonGroup( parent, name );
    } else if ( className == "QIconView" ) {
#if defined(QT_MODULE_ICONVIEW)
	return new QIconView( parent, name );
#endif
    } else if ( className == "QTable" ) {
#if defined(QT_MODULE_TABLE)
	return new QTable( parent, name );
#endif
    } else if ( className == "QListBox" ) {
	return new QListBox( parent, name );
    } else if ( className == "QListView" ) {
	return new QListView( parent, name );
    } else if ( className == "QLineEdit" ) {
	return new QLineEdit( parent, name );
    } else if ( className == "QSpinBox" ) {
	return new QSpinBox( parent, name );
    } else if ( className == "QMultiLineEdit" ) {
	return new QMultiLineEdit( parent, name );
    } else if ( className == "QLabel"  || className == "TextLabel" || className == "PixmapLabel" ) {
	return new QLabel( parent, name );
    } else if ( className == "QLayoutWidget" ) {
	return new QWidget( parent, name );
    } else if ( className == "QTabWidget" ) {
	return new QTabWidget( parent, name );
    } else if ( className == "QComboBox" ) {
	return new QComboBox( FALSE, parent, name );
    } else if ( className == "QWidget" ) {
	return new QWidget( parent, name );
    } else if ( className == "QDialog" ) {
	return new QDialog( parent, name );
    } else if ( className == "QWizard" ) {
	return  new QWizard( parent, name );
    } else if ( className == "QLCDNumber" ) {
	return new QLCDNumber( parent, name );
    } else if ( className == "QProgressBar" ) {
	return new QProgressBar( parent, name );
    } else if ( className == "QTextView" ) {
	return new QTextView( parent, name );
    } else if ( className == "QTextBrowser" ) {
	return new QTextBrowser( parent, name );
    } else if ( className == "QDial" ) {
	return new QDial( parent, name );
    } else if ( className == "QSlider" ) {
	return new QSlider( parent, name );
    } else if ( className == "QFrame" ) {
	return new QFrame( parent, name );
    } else if ( className == "Line" ) {
	QFrame *f = new QFrame( parent, name );
	f->setFrameStyle( QFrame::HLine | QFrame::Sunken );
	return f;
    } else if ( className == "QTextEdit" ) {
	return new QTextEdit( parent, name );
    } else if ( className == "QScrollBar" ) {
	return new QScrollBar( parent, name );
    }

    // maybe it is a KDE widget we support
    QWidget *w = qt_create_kde_widget( className, parent, name, FALSE );
    if ( w )
	return w;

    // try to create it using the loaded widget plugins
    w = widgetManager()->create( className, parent, name );
    if ( w )
	return w;

    // hope we have a factory which can do it
    for ( QWidgetFactory* f = widgetFactories.first(); f; f = widgetFactories.next() ) {
	QWidget *w = f->createWidget( className, parent, name );
	if ( w )
	    return w;
    }

    // no success
    return 0;
}

QWidget *QWidgetFactory::createWidgetInternal( const QDomElement &e, QWidget *parent, QLayout* layout )
{
    lastItem = 0;
    QDomElement n = e.firstChild().toElement();
    QWidget *w = 0; // the widget that got created
    QObject *obj = 0; // gets the properties

    int row = e.attribute( "row" ).toInt();
    int col = e.attribute( "column" ).toInt();
    int rowspan = e.attribute( "rowspan" ).toInt();
    int colspan = e.attribute( "colspan" ).toInt();
    if ( rowspan < 1 )
	rowspan = 1;
    if ( colspan < 1 )
	colspan = 1;
    while ( !n.isNull() ) {
	if ( n.tagName() == "class" ) {
	    QString className = n.firstChild().toText().data();
	    if ( !layout && className  == "QLayoutWidget" )
		className = "QWidget";
	    if ( layout && className == "QLayoutWidget" ) {
		// hide layout widgets
		w = parent;
	    } else {
		obj = QWidgetFactory::createWidget( className, parent, 0 );
		if ( !obj )
		    return 0;
		w = (QWidget*)obj;
		if ( !toplevel )
		    toplevel = w;
		if ( layout ) {
		    switch( layoutType( layout ) ) {
		    case HBox:
			( (QHBoxLayout*)layout )->addWidget( w );
			break;
		    case VBox:
			( (QVBoxLayout*)layout )->addWidget( w );
			break;
		    case Grid:
			( (QGridLayout*)layout )->addMultiCellWidget( w, row, row + rowspan - 1,
								      col, col + colspan - 1 );
			break;
		    default:
			break;
		    }
		}
	
		layout = 0;
	    }
	} else if ( n.tagName() == "spacer" ) {
	    createSpacer( n, layout );
	} else if ( n.tagName() == "widget" ) {
	    createWidgetInternal( n, w, layout );
	} else if ( n.tagName() == "hbox" ) {
	    QLayout *parentLayout = layout;
	    if ( layout && layout->inherits( "QGridLayout" ) )
		layout = createLayout( 0, 0, HBox );
	    else
		layout = createLayout( w, layout, HBox );
	    obj = layout;
	    n = n.firstChild().toElement();
	    if ( parentLayout && parentLayout->inherits( "QGridLayout" ) )
		( (QGridLayout*)parentLayout )->addMultiCellLayout( layout, row, row + rowspan - 1, col, col + colspan - 1 );
	    continue;
	} else if ( n.tagName() == "grid" ) {
	    QLayout *parentLayout = layout;
	    if ( layout && layout->inherits( "QGridLayout" ) )
		layout = createLayout( 0, 0, Grid );
	    else
		layout = createLayout( w, layout, Grid );
	    obj = layout;
	    n = n.firstChild().toElement();
	    if ( parentLayout && parentLayout->inherits( "QGridLayout" ) )
		( (QGridLayout*)parentLayout )->addMultiCellLayout( layout, row, row + rowspan - 1, col, col + colspan - 1 );
	    continue;
	} else if ( n.tagName() == "vbox" ) {
	    QLayout *parentLayout = layout;
	    if ( layout && layout->inherits( "QGridLayout" ) )
		layout = createLayout( 0, 0, VBox );
	    else
		layout = createLayout( w, layout, VBox );
	    obj = layout;
	    n = n.firstChild().toElement();
	    if ( parentLayout && parentLayout->inherits( "QGridLayout" ) )
		( (QGridLayout*)parentLayout )->addMultiCellLayout( layout, row, row + rowspan - 1, col, col + colspan - 1 );
	    continue;
 	} else if ( n.tagName() == "property" ) {
	    QDomElement n2 = n.firstChild().toElement();
	    if ( n2.tagName() == "name" && obj )
		setProperty( obj, n2.firstChild().toText().data(), n2.nextSibling().toElement() );
	} else if ( n.tagName() == "attribute" ) {
	    QDomElement n2 = n.firstChild().toElement();
	    if ( n2.tagName() == "name" ) {
		QString attrib = n2.firstChild().toText().data();
		QVariant v = DomTool::elementToVariant( n2.nextSibling().toElement(), QVariant() );

		if ( parent->inherits( "QTabWidget" ) ) {
		    if ( attrib == "title" )
			( (QTabWidget*)parent )->insertTab( (QWidget*)obj, v.toString() );
		} else if ( parent->inherits( "QWizard" ) ) {
		    if ( attrib == "title" )
			( (QWizard*)parent )->addPage( (QWidget*)obj, v.toString() );
		}
	    }
	} else if ( n.tagName() == "item" ) {
	    createItem( n, w );
	} else if ( n.tagName() == "column" ) {
	    createColumn( n, w );
	}
	
	n = n.nextSibling().toElement();
    }

    return w;
}

#define BOXLAYOUT_DEFAULT_MARGIN 11
#define BOXLAYOUT_DEFAULT_SPACING 6

QLayout *QWidgetFactory::createLayout( QWidget *widget, QLayout*  layout, LayoutType type )
{
    int spacing = BOXLAYOUT_DEFAULT_SPACING;
    int margin = BOXLAYOUT_DEFAULT_MARGIN;
	
    if ( !layout && widget && widget->inherits( "QTabWidget" ) )
	widget = ((QTabWidget*)widget)->currentPage();

    if ( !layout && widget && widget->inherits( "QWizard" ) )
	widget = ((QWizard*)widget)->currentPage();

    if ( !layout && widget && widget->inherits( "QWidgetStack" ) )
	widget = ((QWidgetStack*)widget)->visibleWidget();

    if ( !layout && widget && widget->inherits( "QGroupBox" ) ) {
	QGroupBox *gb = (QGroupBox*)widget;
	gb->setColumnLayout( 0, Qt::Vertical );
	gb->layout()->setMargin( 0 );
	gb->layout()->setSpacing( 0 );
	QLayout *l;
	switch ( type ) {
	case HBox:
	    l = new QHBoxLayout( gb->layout() );
	    l->setAlignment( Qt::AlignTop );
	    return l;
	case VBox:
	    l = new QVBoxLayout( gb->layout(), spacing );
	    l->setAlignment( Qt::AlignTop );
	    return l;
	case Grid:
	    l = new QGridLayout( gb->layout() );
	    l->setAlignment( Qt::AlignTop );
	    return l;
	default:
	    return 0;
	}
    } else {
	if ( layout ) {
	    QLayout *l;
	    switch ( type ) {
	    case HBox:
		l = new QHBoxLayout( layout );
		l->setSpacing( spacing );
		l->setMargin( margin );
		return l;
	    case VBox:
		l = new QVBoxLayout( layout );
		l->setSpacing( spacing );
		l->setMargin( margin );
		return l;
	    case Grid: {
		l = new QGridLayout( layout );
		l->setSpacing( spacing );
		l->setMargin( margin );
		return l;
	    }
	    default:
		return 0;
	    }
	} else {
	    QLayout *l;
	    switch ( type ) {
	    case HBox:
		l = new QHBoxLayout( widget );
		if ( !widget ) {
		    l->setMargin( margin );
		    l->setSpacing( margin );
		}
		return l;
	    case VBox:
		l = new QVBoxLayout( widget );
		if ( !widget ) {
		    l->setMargin( margin );
		    l->setSpacing( margin );
		}
		return l;
	    case Grid: {
		l = new QGridLayout( widget );
		if ( !widget ) {
		    l->setMargin( margin );
		    l->setSpacing( margin );
		}
		return l;
	    }
	    default:
		return 0;
	    }
	}
    }
}

QWidgetFactory::LayoutType QWidgetFactory::layoutType( QLayout *layout ) const
{
    if ( layout->inherits( "QHBoxLayout" ) )
	return HBox;
    else if ( layout->inherits( "QVBoxLayout" ) )
	return VBox;
    else if ( layout->inherits( "QGridLayout" ) )
	return Grid;
    return NoLayout;
}

void QWidgetFactory::setProperty( QObject* obj, const QString &prop, const QDomElement &e )
{
    const QMetaProperty *p = obj->metaObject()->property( prop, TRUE );

    QVariant defVarient;
    if ( e.tagName() == "font" ) {
	QFont f( qApp->font() );
	if ( obj->isWidgetType() && ( (QWidget*)obj )->parentWidget() )
	    f = ( (QWidget*)obj )->parentWidget()->font();
	defVarient = QVariant( f );
    }

    QString comment;
    QVariant v( DomTool::elementToVariant( e, defVarient, comment ) );

    if ( !p ) {
	if ( obj->isWidgetType() ) {
	    if ( prop == "toolTip" ) {
		if ( !v.toString().isEmpty() )
		    QToolTip::add( (QWidget*)obj, v.toString() );
	    } else if ( prop == "whatsThis" ) {
		if ( !v.toString().isEmpty() )
		    QWhatsThis::add( (QWidget*)obj, v.toString() );
	    }
	    return;
	}
    }

    if ( e.tagName() == "pixmap" ) {
	QPixmap pix = loadPixmap( e );
	v = QVariant( pix );
    } else if ( e.tagName() == "iconset" ) {
	QPixmap pix;
	pix.convertFromImage( loadFromCollection( v.toString() ) );
	v = QVariant( QIconSet( pix ) );
    } else if ( e.tagName() == "image" ) {
	v = QVariant( loadFromCollection( v.toString() ) );
    } else if ( e.tagName() == "palette" ) {
	QDomElement n = e.firstChild().toElement();
	QPalette p;
	while ( !n.isNull() ) {
	    QColorGroup cg;
	    if ( n.tagName() == "active" ) {
		cg = loadColorGroup( n );
		p.setActive( cg );
	    } else if ( n.tagName() == "inactive" ) {
		cg = loadColorGroup( n );
		p.setInactive( cg );
	    } else if ( n.tagName() == "disabled" ) {
		cg = loadColorGroup( n );
		p.setDisabled( cg );
	    }
	    n = n.nextSibling().toElement();
	}
	v = QPalette( p );
    } else if ( e.tagName() == "enum" && p && p->isEnumType() ) {
	QString key( v.toString() );
	v = QVariant( p->keyToValue( key ) );
    } else if ( e.tagName() == "set" && p && p->isSetType() ) {
	QString keys( v.toString() );
	QStringList lst = QStringList::split( '|', keys );
	QStrList l;
	for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it )
	    l.append( *it );
	v = QVariant( p->keysToValue( l ) );
    }	

    if ( prop == "geometry" ) {
	if ( obj == toplevel ) {
	    toplevel->resize( v.toRect().size() );
	    return;
	}
    }

    obj->setProperty( prop, v );
}

void QWidgetFactory::createSpacer( const QDomElement &e, QLayout *layout )
{
    QDomElement n = e.firstChild().toElement();
    int row = e.attribute( "row" ).toInt();
    int col = e.attribute( "column" ).toInt();
    int rowspan = e.attribute( "rowspan" ).toInt();
    int colspan = e.attribute( "colspan" ).toInt();

    Qt::Orientation orient;
    int w = 0, h = 0;
    QSizePolicy::SizeType sizeType = QSizePolicy::Preferred;
    while ( !n.isNull() ) {
	if ( n.tagName() == "property" ) {
	    QDomElement n2 = n.firstChild().toElement();
	    if ( n2.tagName() == "name" ) {
		if ( n2.firstChild().toText().data() == "orientation" ) {
		    if ( n2.nextSibling().firstChild().toText().data() == "Horizontal" )
			orient = Qt::Horizontal;
		    else
			orient = Qt::Vertical;
		} else if ( n2.firstChild().toText().data() == "sizeType" ) {
		    if ( n2.nextSibling().firstChild().toText().data() == "Fixed" )
			sizeType = QSizePolicy::Fixed;
		    else if ( n2.nextSibling().firstChild().toText().data() == "Minimum" )
			sizeType = QSizePolicy::Minimum;
		    else if ( n2.nextSibling().firstChild().toText().data() == "Maximum" )
			sizeType = QSizePolicy::Maximum;
		    else if ( n2.nextSibling().firstChild().toText().data() == "Preferred" )
			sizeType = QSizePolicy::Preferred;
		    else if ( n2.nextSibling().firstChild().toText().data() == "MinimumExpanding" )
			sizeType = QSizePolicy::MinimumExpanding;
		    else if ( n2.nextSibling().firstChild().toText().data() == "Expanding" )
			sizeType = QSizePolicy::Expanding;
		} else if ( n2.firstChild().toText().data() == "sizeHint" ) {
		    w = n2.nextSibling().firstChild().firstChild().toText().data().toInt();
		    h = n2.nextSibling().firstChild().nextSibling().firstChild().toText().data().toInt();
		}
	    }
	}
	n = n.nextSibling().toElement();
    }

    if ( rowspan < 1 )
	rowspan = 1;
    if ( colspan < 1 )
	colspan = 1;
    QSpacerItem *item = new QSpacerItem( w, h, orient == Qt::Horizontal ? sizeType : QSizePolicy::Minimum,
					 orient == Qt::Vertical ? sizeType : QSizePolicy::Minimum );
    if ( layout ) {
	if ( layout->inherits( "QBoxLayout" ) )
	    ( (QBoxLayout*)layout )->addItem( item );
	else
	    ( (QGridLayout*)layout )->addMultiCell( item, row, row + rowspan - 1, col, col + colspan - 1,
						    orient == Qt::Horizontal ? Qt::AlignVCenter : Qt::AlignHCenter );
    }
}

static QImage loadImageData( QDomElement &n2 )
{
    QImage img;
    QString data = n2.firstChild().toText().data();
    char *ba = new char[ data.length() / 2 ];
    for ( int i = 0; i < (int)data.length() / 2; ++i ) {
	char h = data[ 2 * i ].latin1();
	char l = data[ 2 * i  + 1 ].latin1();
	uchar r = 0;
	if ( h <= '9' )
	    r += h - '0';
	else
	    r += h - 'a' + 10;
	r = r << 4;
	if ( l <= '9' )
	    r += l - '0';
	else
	    r += l - 'a' + 10;
	ba[ i ] = r;
    }
    QString format = n2.attribute( "format", "PNG" );
    if ( format == "XPM.GZ" ) {
	ulong len = n2.attribute( "length" ).toULong();
	if ( len < data.length() * 5 )
	    len = data.length() * 5;
	QByteArray baunzip( len );
	::uncompress( (uchar*) baunzip.data(), &len, (uchar*) ba, data.length()/2 );
	img.loadFromData( (const uchar*)baunzip.data(), len, "XPM" );
    }  else {
	img.loadFromData( (const uchar*)ba, data.length() / 2, format );
    }
    delete [] ba;

    return img;
}

void QWidgetFactory::loadImageCollection( const QDomElement &e )
{
    QDomElement n = e.firstChild().toElement();
    while ( !n.isNull() ) {
	if ( n.tagName() == "image" ) {
	    QDomElement n2 = n.firstChild().toElement();
	    Image img;
	    while ( !n2.isNull() ) {
		if ( n2.tagName() == "name" )
		    img.name = n2.firstChild().toText().data();
		else if ( n2.tagName() == "data" )
		    img.img = loadImageData( n2 );
		n2 = n2.nextSibling().toElement();
	    }
	    images.append( img );
	    n = n.nextSibling().toElement();
	}
    }
}

QImage QWidgetFactory::loadFromCollection( const QString &name )
{
    QValueList<Image>::Iterator it = images.begin();
    for ( ; it != images.end(); ++it ) {
	if ( ( *it ).name == name )
	    return ( *it ).img;
    }
    return QImage();
}

QPixmap QWidgetFactory::loadPixmap( const QDomElement &e )
{
    QString arg = e.firstChild().toText().data();
    QImage img = loadFromCollection( arg );
    QPixmap pix;
    pix.convertFromImage( img );
    return pix;
}

QColorGroup QWidgetFactory::loadColorGroup( const QDomElement &e )
{
    QColorGroup cg;
    int r = -1;
    QDomElement n = e.firstChild().toElement();
    QColor col;
    while ( !n.isNull() ) {
	if ( n.tagName() == "color" ) {
	    r++;
	    cg.setColor( (QColorGroup::ColorRole)r, (col = DomTool::readColor( n ) ) );
	} else if ( n.tagName() == "pixmap" ) {
	    QImage img = loadFromCollection( n.firstChild().toText().data() );
	    QPixmap pix = loadPixmap( n );
	    cg.setBrush( (QColorGroup::ColorRole)r, QBrush( col, pix ) );
	}
	n = n.nextSibling().toElement();
    }
    return cg;
}

struct Connection
{
    QObject *sender, *receiver;
    QCString signal, slot;
    bool operator==( const Connection &c ) const {
	return sender == c.sender && receiver == c.receiver &&
	       signal == c.signal && slot == c.slot ;
    }	
};

void QWidgetFactory::loadConnections( const QDomElement &e, QObject *connector )
{
    QDomElement n = e.firstChild().toElement();
    while ( !n.isNull() ) {
	if ( n.tagName() == "connection" ) {
	    QDomElement n2 = n.firstChild().toElement();
	    Connection conn;
	    while ( !n2.isNull() ) {
		if ( n2.tagName() == "sender" ) {
		    QString name = n2.firstChild().toText().data();
		    if ( name == "this" || qstrcmp( toplevel->name(), name ) == 0 ) {
			conn.sender = toplevel;
		    } else {
			if ( name == "this" )
			    name = toplevel->name();
			QObjectList *l = toplevel->queryList( 0, name, FALSE );
			if ( l ) {
			    if ( l->first() )
				conn.sender = l->first();
			    delete l;
			}
		    }
		} else if ( n2.tagName() == "signal" ) {
		    conn.signal = n2.firstChild().toText().data();
		} else if ( n2.tagName() == "receiver" ) {
		    QString name = n2.firstChild().toText().data();
		    if ( name == "this" || qstrcmp( toplevel->name(), name ) == 0 ) {
			conn.receiver = toplevel;
		    } else {
			QObjectList *l = toplevel->queryList( 0, name, FALSE );
			if ( l ) {
			    if ( l->first() )
				conn.receiver = l->first();
			    delete l;
			}
		    }
		} else if ( n2.tagName() == "slot" ) {
		    conn.slot = n2.firstChild().toText().data();
		}
		n2 = n2.nextSibling().toElement();
	    }
	
	    QObject *sender = 0, *receiver = 0;
	    QObjectList *l = toplevel->queryList( 0, conn.sender->name(), FALSE );
	    if ( qstrcmp( conn.sender->name(), toplevel->name() ) == 0 ) {
		sender = toplevel;
	    } else {
		if ( !l || !l->first() ) {
		    delete l;
		    n = n.nextSibling().toElement();
		    continue;
		}
		sender = l->first();
		delete l;
	    }
	
	    if ( qstrcmp( conn.receiver->name(), toplevel->name() ) == 0 ) {
		receiver = toplevel;
	    } else {
		l = toplevel->queryList( 0, conn.receiver->name(), FALSE );
		if ( !l || !l->first() ) {
		    delete l;
		    n = n.nextSibling().toElement();
		    continue;
		}
		receiver = l->first();
		delete l;
	    }
	    QString s = "2""%1";
	    s = s.arg( conn.signal );
	    QString s2 = "1""%1";
	    s2 = s2.arg( conn.slot );

	    QStrList signalList = sender->metaObject()->signalNames( TRUE );
	    QStrList slotList = receiver->metaObject()->slotNames( TRUE );

	    // if this is a connection to a custom slot and we have a connector, try this as receiver
	    if ( slotList.find( conn.slot ) == -1 && receiver == toplevel && connector )
		slotList = connector->metaObject()->slotNames( TRUE );
	
	    // avoid warnings
	    if ( signalList.find( conn.signal ) == -1 ||
		 slotList.find( conn.slot ) == -1 ) {
		n = n.nextSibling().toElement();
		continue;
	    }
	
	    QObject::connect( sender, s, receiver, s2 );
	}
	n = n.nextSibling().toElement();
    }
}

void QWidgetFactory::loadTabOrder( const QDomElement &e )
{
    QWidget *last = 0;
    QDomElement n = e.firstChild().toElement();
    while ( !n.isNull() ) {
	if ( n.tagName() == "tabstop" ) {
	    QString name = n.firstChild().toText().data();
	    QObjectList *l = toplevel->queryList( 0, name, FALSE );
	    if ( l ) {
		if ( l->first() ) {
		    QWidget *w = (QWidget*)l->first();
		    if ( last )
			toplevel->setTabOrder( last, w );
		    last = w;
		}
		delete l;
	    }
	}
	n = n.nextSibling().toElement();
    }
}

void QWidgetFactory::createColumn( const QDomElement &e, QWidget *widget )
{
    if ( widget->inherits( "QListView" ) ) {
	QListView *lv = (QListView*)widget;
	QDomElement n = e.firstChild().toElement();
	QPixmap pix;
	bool hasPixmap = FALSE;
	QString txt;
	bool clickable = TRUE, resizeable = TRUE;
	while ( !n.isNull() ) {
	    if ( n.tagName() == "property" ) {
		QDomElement n2 = n.firstChild().toElement();
		if ( n2.tagName() == "name" ) {
		    QString attrib = n2.firstChild().toText().data();
		    QVariant v = DomTool::elementToVariant( n2.nextSibling().toElement(), QVariant() );
		    if ( attrib == "text" )
			txt = v.toString();
		    else if ( attrib == "pixmap" ) {
			pix = loadPixmap( n2.nextSibling().toElement() );
			hasPixmap = TRUE;
		    } else if ( attrib == "clickable" )
			clickable = v.toBool();
		    else if ( attrib == "resizeable" )
			resizeable = v.toBool();
		}
	    }
	    n = n.nextSibling().toElement();
	}
	lv->addColumn( txt );
	int i = lv->header()->count() - 1;
	if ( hasPixmap ) {
	    lv->header()->setLabel( i, pix, txt );
	}
	if ( !clickable )
	    lv->header()->setClickEnabled( clickable, i );
	if ( !resizeable )
	    lv->header()->setResizeEnabled( resizeable, i );
    }
}

void QWidgetFactory::loadItem( const QDomElement &e, QPixmap &pix, QString &txt, bool &hasPixmap )
{
    QDomElement n = e;
    hasPixmap = FALSE;
    while ( !n.isNull() ) {
	if ( n.tagName() == "property" ) {
	    QDomElement n2 = n.firstChild().toElement();
	    if ( n2.tagName() == "name" ) {
		QString attrib = n2.firstChild().toText().data();
		QVariant v = DomTool::elementToVariant( n2.nextSibling().toElement(), QVariant() );
		if ( attrib == "text" )
		    txt = v.toString();
		else if ( attrib == "pixmap" ) {
		    pix = loadPixmap( n2.nextSibling().toElement() );
		    hasPixmap = TRUE;
		}
	    }
	}
	n = n.nextSibling().toElement();
    }
}

void QWidgetFactory::createItem( const QDomElement &e, QWidget *widget, QListViewItem *i )
{
    if ( widget->inherits( "QListBox" ) || widget->inherits( "QComboBox" ) ) {
	QDomElement n = e.firstChild().toElement();
	QPixmap pix;
	bool hasPixmap = FALSE;
	QString txt;
	loadItem( n, pix, txt, hasPixmap );
	QListBox *lb = 0;
	if ( widget->inherits( "QListBox" ) )
	    lb = (QListBox*)widget;
	else
	    lb = ( (QComboBox*)widget)->listBox();
	if ( hasPixmap ) {
	    new QListBoxPixmap( lb, pix, txt );
	} else {
	    new QListBoxText( lb, txt );
	}
    } else if ( widget->inherits( "QIconView" ) ) {
	QDomElement n = e.firstChild().toElement();
	QPixmap pix;
	bool hasPixmap = FALSE;
	QString txt;
	loadItem( n, pix, txt, hasPixmap );

	QIconView *iv = (QIconView*)widget;
	new QIconViewItem( iv, txt, pix );
    } else if ( widget->inherits( "QListView" ) ) {
	QDomElement n = e.firstChild().toElement();
	QPixmap pix;
	QValueList<QPixmap> pixmaps;
	QStringList textes;
	QListViewItem *item = 0;
	QListView *lv = (QListView*)widget;
	if ( i )
	    item = new QListViewItem( i, lastItem );
	else
	    item = new QListViewItem( lv, lastItem );
	while ( !n.isNull() ) {
	    if ( n.tagName() == "property" ) {
		QDomElement n2 = n.firstChild().toElement();
		if ( n2.tagName() == "name" ) {
		    QString attrib = n2.firstChild().toText().data();
		    QVariant v = DomTool::elementToVariant( n2.nextSibling().toElement(), QVariant() );
		    if ( attrib == "text" )
			textes << v.toString();
		    else if ( attrib == "pixmap" ) {
			QString s = v.toString();
			if ( s.isEmpty() ) {
			    pixmaps << QPixmap();
			} else {
			    pix = loadPixmap( n2.nextSibling().toElement() );
			    pixmaps << pix;
			}
		    }
		}
	    } else if ( n.tagName() == "item" ) {
		item->setOpen( TRUE );
		createItem( n, widget, item );
	    }
		
	    n = n.nextSibling().toElement();
	}
	
	for ( int i = 0; i < lv->columns(); ++i ) {
	    item->setText( i, textes[ i ] );
	    item->setPixmap( i, pixmaps[ i ] );
	}
	lastItem = item;
    }
}
