/****************************************************************************
** $Id: $
**
** Implementation of the Qt Designer integration plugin
**
** Copyright (C) 2001-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Active Qt integration.
**
** Licensees holding valid Qt Enterprise Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifdef QT_PLUGIN

#include <qwidgetplugin.h>
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

#include "qaxwidget.h"
#include "qactivexselect.h"

/* XPM */
static const char *widgetIcon[]={
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

static const char *helpIcon[]={
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

private:
    QAction *actionDocu;

    QDict<QDialog> browserDict;
    QObjectCleanupHandler objects;

    static DesignerInterface *designer;
};

#include "plugin.moc"

DesignerInterface *QActiveXPlugin::designer = 0;

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
	    actionDocu->setEnabled( FALSE );
	    return;
	} else {
	    enabled = TRUE;
	}
    }
    actionDocu->setEnabled( enabled );
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
	actionDocu = new QAction( "QAxWidget Documentation", QIconSet( helpIcon ),
	                          "QA&xWidget Documentation", 0, parent, "qaxdocu" );
	actionDocu->setEnabled( FALSE );
	objects.add( actionDocu );
	connect( actionDocu, SIGNAL(activated()), this, SLOT(onShowDocumentation()) );
	return actionDocu;
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

#endif // QT_PLUGIN
