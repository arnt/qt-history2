/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qwidgetplugin.h>
#include <qaxwidget.h>
#include <qdialog.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qapplication.h>
#include <qtimer.h>
#include <qaction.h>
#include <qtextbrowser.h>
#include <qwidgetlist.h>
#include <qobjectcleanuphandler.h>
#include <private/qwidgetinterface_p.h>
#include <actioninterface.h>
#include <designerinterface.h>
#include <qdict.h>
#include <ocidl.h> 
#include <olectl.h>

#include "../container/qactivexselect.h"


/* XPM */
static const char *widgetIcon[]={
"22 22 6 1",
"a c #000000",
"# c #808080",
"+ c #aaa5a0",
"b c #dddddd",
"* c #d4d0c8",
". c none",
".........#aa#...#aa#..",
".........abba...abba..",
".........abba...abba..",
".........#aa#...#aa#..",
"..........aa.....aa...",
"..........aa.....aa...",
"..........aa.....aa...",
".......aaaaaaaaaaaaaaa",
".......a*************a",
".......a************#a",
".......a***********+#a",
".......a***********+#a",
".......a***********+#a",
"#aa#...a***********+#a",
"abbaaaaa***********+#a",
"abbaaaaa***********+#a",
"#aa#...a***********+#a",
".......a***********+#a",
".......a***********+#a",
".......a**++++++++++#a",
".......a*############a",
".......aaaaaaaaaaaaaaa"};

static const char *helpIcon[]={
"22 22 7 1",
"a c #000000",
"# c #808080",
"+ c #aaa5a0",
"b c #dddddd",
"* c #d4d0c8",
"h c #1000ef",
". c none",
".........#aa#...#aa#..",
".........abba...abba..",
".........abba...abba..",
".........#aa#...#aa#..",
"..........aa.....aa...",
"..........aa.....aa...",
"..........aa.....aa...",
".......aaaaaaaaaaaaaaa",
".......a*************a",
".......a*****hhh****#a",
".......a****hhhhh**+#a",
".......a***hhh*hhh*+#a",
".......a***hh***hh*+#a",
"#aa#...a****h**hh**+#a",
"abbaaaaa******hh***+#a",
"abbaaaaa*****hh****+#a",
"#aa#...a***********+#a",
".......a*****hh****+#a",
".......a*****hh****+#a",
".......a**++++++++++#a",
".......a*############a",
".......aaaaaaaaaaaaaaa"};

static const char *propIcon[]={
"22 22 7 1",
"a c #000000",
"# c #808080",
"+ c #aaa5a0",
"b c #dddddd",
"* c #d4d0c8",
"h c #1000ef",
". c none",
".........#aa#...#aa#..",
".........abba...abba..",
".........abba...abba..",
".........#aa#...#aa#..",
"..........aa.....aa...",
"..........aa.....aa...",
"..........aa.....aa...",
".......aaaaaaaaaaaaaaa",
".......a*************a",
".......a************#a",
".......a*hh********+#a",
".......a*hh*aaaaaa*+#a",
".......a***********+#a",
"#aa#...a*hh********+#a",
"abbaaaaa*hh*aaaaaa*+#a",
"abbaaaaa***********+#a",
"#aa#...a*hh********+#a",
".......a*hh*aaaaaa*+#a",
".......a***********+#a",
".......a**++++++++++#a",
".......a*############a",
".......aaaaaaaaaaaaaaa"};

class QActiveXPlugin : public QObject,
		       public QLibraryInterface,
		       public QWidgetFactoryInterface, 
		       public ActionInterface
{
    Q_OBJECT
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

    bool init() { return TRUE; }
    void cleanup() {}
    bool canUnload() const { return objects.isEmpty(); }

protected slots:
    void onFormChange();
    void onSelectionChanged();
    void onShowDocumentation();
    void onShowProperties();
    void onPropertyChanged( const QString &name );

private:
    QActionGroup *actions;
    QAction *actionDocu;
    QAction *actionProp;

    QDict<QDialog> browserDict;
    bool    property_changed;
    QObjectCleanupHandler objects;

    static DesignerInterface *designer;
};

#include "plugin.moc"

DesignerInterface *QActiveXPlugin::designer = 0;
bool runsInDesignMode = FALSE;

QActiveXPlugin::QActiveXPlugin()
: actionDocu(0)
{
    browserDict.setAutoDelete( TRUE );
    if ( designer )
	designer->addRef();
}

QActiveXPlugin::~QActiveXPlugin()
{
    if ( designer )
	designer->release();
}

void QActiveXPlugin::onFormChange()
{
    DesignerFormWindow *currentForm = designer->currentForm();
    if ( !currentForm || !currentForm->form() )
	return;

    currentForm->form()->disconnect( SIGNAL(selectionChanged()), this );
    connect( currentForm->form(), SIGNAL(selectionChanged()), this, SLOT(onSelectionChanged()) );
}

void QActiveXPlugin::onSelectionChanged()
{
    if ( !designer || !designer->currentForm() || !actionDocu )
	return;
    
    QWidgetList widgets = designer->currentForm()->selectedWidgets();
    QWidgetListIt it( widgets );
    bool enabled = FALSE;
    while ( it.current() ) {
	QWidget *widget = it.current();
	++it;
	QAxWidget *axwidget = (QAxWidget*)widget->qt_cast( "QAxWidget" );
	if ( !axwidget ) {
	    actions->setEnabled( FALSE );
	    return;
	} else {
	    enabled = TRUE;
	}
    }
    actions->setEnabled( enabled );
}

void QActiveXPlugin::onShowDocumentation()
{
    if ( !designer || !designer->currentForm() )
	return;

    QWidgetList widgets = designer->currentForm()->selectedWidgets();
    QWidgetListIt it( widgets );
    while ( it.current() ) {
	QWidget *widget = it.current();
	++it;
	if ( widget->inherits( "QAxWidget" ) ) {
	    QAxWidget *axwidget = (QAxWidget*)widget;
	    if ( !axwidget->isNull() ) {
		QDialog *dialog = browserDict[ axwidget->control() ];
		if ( !dialog ) {
		    dialog = new QDialog( axwidget->topLevelWidget(), 0, FALSE, 0 );
		    QVBoxLayout *layout = new QVBoxLayout( dialog );
		    QTextBrowser *docuView = new QTextBrowser( dialog );
		    layout->addWidget( docuView );

		    dialog->setCaption( tr("API of %1").arg( axwidget->control() ) );
		    browserDict.insert( axwidget->control(), dialog );
		    docuView->setText( axwidget->generateDocumentation() );
		}
		dialog->show();
	    }
	}
    }
}

void QActiveXPlugin::onShowProperties()
{
    if ( !designer || !designer->currentForm() )
	return;

    QWidgetList widgets = designer->currentForm()->selectedWidgets();
    QWidgetListIt it( widgets );
    while ( it.current() ) {
	QWidget *widget = it.current();
	++it;
	if ( widget->inherits( "QAxWidget" ) ) {
	    QAxWidget *axwidget = (QAxWidget*)widget;
	    if ( !axwidget->isNull() ) {
		ISpecifyPropertyPages *spp = 0;
		axwidget->queryInterface( IID_ISpecifyPropertyPages, (void**)&spp );
		CAUUID pages;
		if ( !spp )
		    return;

		spp->GetPages( &pages );
		IUnknown *objects[1];
		axwidget->queryInterface( IID_IUnknown, (void**)&objects[0] );

		property_changed = FALSE;
		connect( axwidget, SIGNAL(propertyChanged(const QString&)), this, SLOT(onPropertyChanged(const QString&)) );

		OleCreatePropertyFrame( widget->topLevelWidget()->winId(), 0, 0, L"Properties", 
		    1, objects, pages.cElems, pages.pElems, LOCALE_USER_DEFAULT, 0, 0 );

		if ( property_changed ) {
		    QMetaObject *mo = axwidget->metaObject();
		    int pc = mo->numProperties( TRUE );

		    for ( int i = mo->propertyOffset(); i < pc; ++i ) {
			const QMetaProperty *mp = mo->property( i, TRUE );
			designer->currentForm()->setPropertyChanged( axwidget, mp->name(), TRUE );
		    }
		}
		designer->currentForm()->clearSelection();
		qApp->processEvents();
		designer->currentForm()->selectWidget( axwidget );
		designer->currentForm()->setCurrentWidget( axwidget );

		CoTaskMemFree( pages.pElems );

		disconnect( axwidget, SIGNAL(propertyChanged(const QString&)), this, SLOT(onPropertyChanged(const QString&)) );
		return;
	    }
	}
    }
}

void QActiveXPlugin::onPropertyChanged( const QString & )
{
    property_changed = TRUE;
}

QRESULT QActiveXPlugin::queryInterface( const QUuid &iid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( iid == IID_QUnknown )
	*iface = (QUnknownInterface*)(ActionInterface*) this;
    else if ( iid == IID_QLibrary )
	*iface = (QLibraryInterface*)this;
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

    list << "QAxWidget";

    return list;
}

QAction *QActiveXPlugin::create( const QString &key, QObject *parent )
{
    if ( key == "QAxWidget" ) {
	actions = new QActionGroup( parent, "qaxactions" );

	actionDocu = new QAction( "AxWidget Documentation", QIconSet( helpIcon ),
	                          "A&xWidget Documentation", 0, actions, "qaxdocu" );
	connect( actionDocu, SIGNAL(activated()), this, SLOT(onShowDocumentation()) );

	actionDocu = new QAction( "AxWidget Property Pages", QIconSet( propIcon ),
	                          "A&xWidget Property Pages", 0, actions, "qaxprop" );
	connect( actionDocu, SIGNAL(activated()), this, SLOT(onShowProperties()) );

	actions->setEnabled( FALSE );
	objects.add( actions );	
	return actions;
    }
    return 0;
}

bool QActiveXPlugin::location( const QString & /*name*/, Location /*l*/ ) const
{
    return TRUE;
}

void QActiveXPlugin::connectTo( QUnknownInterface *app )
{
    if ( !app )
	return;
    app->queryInterface( IID_Designer, (QUnknownInterface**)&designer );
    if ( !designer )
	return;

    runsInDesignMode = TRUE;
    designer->addRef();
    designer->onFormChange( this, SLOT(onFormChange()) );
}

QWidget* QActiveXPlugin::create( const QString &key, QWidget* parent, const char* name )
{
    QWidget* w = 0;

    if ( key == "QAxWidget" ) {
	w = new QAxWidget( parent, name );
	if ( designer ) {
	    QActiveXSelect *dialog = new QActiveXSelect( parent->topLevelWidget() );
	    dialog->setActiveX( (QAxWidget*)w );
	    dialog->setDesigner( designer );
	    QTimer::singleShot( 100, dialog, SLOT(openLater()) );
	}
    }

    return w;
}

QString QActiveXPlugin::group( const QString& key ) const
{
    if ( key == "QAxWidget" )
	return "Containers";
    return QString::null;
}

QIconSet QActiveXPlugin::iconSet( const QString &key ) const
{
    if ( key == "QAxWidget" )
	return QIconSet( widgetIcon );
    return QIconSet();
}

QString QActiveXPlugin::includeFile( const QString& key ) const
{
    if ( key == "QAxWidget" )
        return "qaxwidget.h";
    return QString::null;
}

QString QActiveXPlugin::toolTip( const QString& key ) const
{
    if ( key == "QAxWidget" )
	return QT_TR_NOOP("ActiveX control");
    return QString::null;
}

QString QActiveXPlugin::whatsThis( const QString& key ) const
{
    if ( key == "QAxWidget" )
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
