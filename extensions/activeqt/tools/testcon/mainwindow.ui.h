/****************************************************************************
** $Id: $
**
** Copyright (C) 2001-2002 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for the ActiveQt integration.
** This example program may be used, distributed and modified without 
** limitation.
**
*****************************************************************************/

#include <qaxobject.h>
#include <qinputdialog.h>
#include <qlabel.h>
#include "docuwindow.h"

#include <qt_windows.h>
#include <oaidl.h>
#include "../../shared/types.h"
#include <qaxfactory.h>

QAxObject *ax_mainWindow = 0;

static QTextEdit *debuglog = 0;

static void redirectDebugOutput( QtMsgType type, const char*msg )
{
    debuglog->append( msg );
}

void MainWindow::init()
{
    QAxScriptManager::registerEngine("PerlScript", ".pl");
    QAxScriptManager::registerEngine("Python", ".py");

    dlgInvoke = 0;
    dlgProperties = 0;
    dlgAmbient = 0;
    scripts = 0;
    debuglog = logDebug;
    oldDebugHandler = qInstallMsgHandler( redirectDebugOutput );
    QHBoxLayout *layout = new QHBoxLayout( Workbase );
    workspace = new QWorkspace( Workbase );
    layout->addWidget( workspace );
    connect( workspace, SIGNAL(windowActivated(QWidget*)), this, SLOT(windowActivated(QWidget*)) );
}

void MainWindow::destroy()
{
    qInstallMsgHandler( oldDebugHandler );
    debuglog = 0;
}

void MainWindow::changeProperties()
{
    QAxWidget *container = 0;
    if ( workspace->activeWindow() )
	container = (QAxWidget*)workspace->activeWindow()->qt_cast("QAxWidget");

    if ( !dlgProperties ) {
	dlgProperties = new ChangeProperties( this, 0, FALSE );
	connect(container, SIGNAL(propertyChanged(const QString&)), dlgProperties, SLOT(updateProperties()) );
    }
    dlgProperties->setControl( container );
    dlgProperties->show();
}

void MainWindow::clearControl()
{
    QAxWidget *container = 0;
    if ( workspace->activeWindow() )
	container = (QAxWidget*)workspace->activeWindow()->qt_cast("QAxWidget");
    if ( container )
	container->clear();
    updateGUI();
}

void MainWindow::containerProperties()
{
    if ( !dlgAmbient ) {
	dlgAmbient = new AmbientProperties( this, 0, FALSE );
	dlgAmbient->setControl( workspace );
    }
    dlgAmbient->show();
}

void MainWindow::invokeMethods()
{
    QAxWidget *container = 0;
    if ( workspace->activeWindow() )
	container = (QAxWidget*)workspace->activeWindow()->qt_cast("QAxWidget");

    if ( !dlgInvoke )
	dlgInvoke = new InvokeMethod( this, 0, FALSE );
    dlgInvoke->setControl( container );
    dlgInvoke->show();    
}

void MainWindow::logPropertyChanged( const QString &prop )
{
    QAxWidget *container = (QAxWidget*)((QObject*)sender())->qt_cast("QAxWidget");
    if ( !container )
	return;

    QVariant var = container->property( prop );
    logProperties->append( container->caption() + ": Property Change: " + prop + " - { " + var.toString() + " }" );
}

void MainWindow::logSignal( const QString &signal, int argc, void *argv )
{
    QAxWidget *container = (QAxWidget*)((QObject*)sender())->qt_cast("QAxWidget");
    if ( !container )
	return;

    QString paramlist;
    VARIANT *params = (VARIANT*)argv;
    for ( int a = argc-1; a >= 0; --a ) {
	if ( a == argc-1 )
	    paramlist = " - {";
	QVariant qvar = VARIANTToQVariant( params[a], 0 );
	paramlist += " " + qvar.toString();
	if ( a > 0 )
	    paramlist += ",";
	else
	    paramlist += " ";
    }
    if ( argc )
	paramlist += "}";
    logSignals->append( container->caption() + ": " + signal + paramlist );
}

void MainWindow::logException( int code, const QString&source, const QString&desc, const QString&help )
{
    QAxWidget *container = (QAxWidget*)((QObject*)sender())->qt_cast("QAxWidget");
    if ( !container )
	return;

    QString str = QString( "%1: Exception code %2 thrown by %3" ).
	arg( container->caption() ).arg( code ).arg( source );
    logDebug->append( str );

    if ( !help.isEmpty() )
	logDebug->append( "\tHelp available at " + help );
    else
	logDebug->append( "\tNo help available." );
}

void MainWindow::setControl()
{
    QAxWidget *container = 0;
    if ( workspace->activeWindow() )
	container = (QAxWidget*)workspace->activeWindow()->qt_cast("QAxWidget");
    if ( !container )
	return;

    QActiveXSelect select( this, 0, TRUE );
    if ( select.exec() ) {
	container->setControl( select.selectedControl() );
    }
    updateGUI();
}

void MainWindow::controlInfo()
{
    QAxWidget *container = 0;
    if ( workspace->activeWindow() )
	container = (QAxWidget*)workspace->activeWindow()->qt_cast("QAxWidget");
    if ( !container )
	return;

    ControlInfo info( this, 0, TRUE );
    info.setControl( container );
    info.exec();
}

void MainWindow::fileLoad()
{
    QString fname = QFileDialog::getOpenFileName( QString::null, "*.qax", this );
    if ( fname.isEmpty() )
	return;

    QFile file( fname );
    if ( !file.open( IO_ReadOnly ) ) {
	QMessageBox::information( this, "Error Loading File", QString("The file could not be opened for reading.\n%1").arg(fname) );
	return;
    }

    QAxWidget *container = new QAxWidget( workspace );
    
    QDataStream d( &file );
    d >> *container;

    container->show();

    updateGUI();
}

void MainWindow::fileSave()
{
    QAxWidget *container = 0;
    if ( workspace->activeWindow() )
	container = (QAxWidget*)workspace->activeWindow()->qt_cast("QAxWidget");
    if ( !container )
	return;

    QString fname = QFileDialog::getSaveFileName( QString::null, "*.qax", this );
    if ( fname.isEmpty() )
	return;

    QFile file( fname );
    if ( !file.open( IO_WriteOnly ) ) {
	QMessageBox::information( this, "Error Saving File", QString("The file could not be opened for writing.\n%1").arg(fname) );
	return;
    }
    QDataStream d( &file );
    d << *container;
}

void MainWindow::updateGUI()
{
    QAxWidget *container = 0;
    if ( workspace->activeWindow() )
	container = (QAxWidget*)workspace->activeWindow()->qt_cast("QAxWidget");

    bool hasControl = container && !container->isNull();
    actionFileNew->setEnabled( TRUE );
    actionFileLoad->setEnabled( TRUE );
    actionFileSave->setEnabled( hasControl );
    actionContainerSet->setEnabled( container != 0 );
    actionContainerClear->setEnabled( hasControl );
    actionControlProperties->setEnabled( hasControl );
    actionControlMethods->setEnabled( hasControl );
    actionControlInfo->setEnabled( hasControl );
    actionControlDocumentation->setEnabled( hasControl );
    actionControlPixmap->setEnabled( hasControl );
    if ( dlgInvoke )
	dlgInvoke->setControl( hasControl ? container : 0 );
    if ( dlgProperties )
	dlgProperties->setControl( hasControl ? container : 0 );

    QWidgetList list = workspace->windowList();
    QWidgetListIt it( list );
    while ( it.current() ) {
	QWidget *container = it.current();

	QAxWidget *ax = (QAxWidget*)container->qt_cast( "QAxWidget" );
	if ( ax ) {
	    container->disconnect( SIGNAL(signal(const QString&, int, void*)) );
	    if ( actionLogSignals->isOn() )
		connect( container, SIGNAL(signal(const QString&, int, void*)), this, SLOT(logSignal(const QString&, int, void*)) );

	    container->disconnect( SIGNAL(exception(int,const QString&,const QString&,const QString&)) );
	    connect( container, SIGNAL(exception(int,const QString&,const QString&,const QString&)),
		this, SLOT(logException(int,const QString&,const QString&,const QString&)) );

	    container->disconnect( SIGNAL(propertyChanged(const QString&)) );
	    if ( actionLogProperties->isOn() ) 
		connect( container, SIGNAL(propertyChanged(const QString&)), this, SLOT(logPropertyChanged(const QString&)) );
	    container->blockSignals( actionFreezeEvents->isOn() );
	}

	++it;
    }
}


void MainWindow::fileNew()
{
    QActiveXSelect select( this, 0, TRUE );
    if ( select.exec() ) {
	QAxWidget *container = new QAxWidget( select.selectedControl(), workspace, 0, WDestructiveClose );
	container->setName(container->caption().latin1());
	container->show();
    }
    updateGUI();
}


void MainWindow::windowActivated( QWidget *window )
{
    updateGUI();
}

void MainWindow::showDocumentation()
{
    QAxWidget *container = 0;
    if ( workspace->activeWindow() )
	container = (QAxWidget*)workspace->activeWindow()->qt_cast("QAxWidget");
    if ( !container )
	return;
    
    QString docu = container->generateDocumentation();
    if ( docu.isEmpty() )
	return;

    DocuWindow *docwindow = new DocuWindow( docu, workspace, container );
    docwindow->show();
}


void MainWindow::renderPixmap()
{
    QAxWidget *container = 0;
    if ( workspace->activeWindow() )
	container = (QAxWidget*)workspace->activeWindow()->qt_cast("QAxWidget");
    if ( !container )
	return;

    QPixmap pm = QPixmap::grabWidget( container );

    QLabel *label = new QLabel( workspace, "pixmap_label", WDestructiveClose );
    label->setPixmap( pm );
    label->setCaption( container->caption() + " - Pixmap" );

    label->show();
}

void MainWindow::runMacro()
{
    if (!scripts)
	return;

    // If we have only one script loaded we can use the cool dialog
    QStringList scriptList = scripts->scriptNames();
    if (scriptList.count() == 1) {
	InvokeMethod scriptInvoke( this, 0, TRUE );
	scriptInvoke.setCaption("Execute Script Function");
	scriptInvoke.setControl(scripts->script(scriptList[0])->scriptEngine());
	scriptInvoke.exec();
	return;
    }

    bool ok = FALSE;
    QStringList macroList = scripts->functions(QAxScript::FunctionNames);
    QString macro = QInputDialog::getItem("Select Macro", "Macro:", macroList, 0, TRUE, &ok, this);

    if (!ok)
	return;

    QVariant result = scripts->call(macro);
    if (result.isValid())
	logMacros->append(QString("Return value of %1: %2").arg(macro).arg(result.asString()));
}

void MainWindow::loadScript()
{
    QString file = QFileDialog::getOpenFileName(QString::null, QAxScriptManager::scriptFileFilter(),
						this, 0, "Open Script");

    if (file.isEmpty())
	return;

    if (!scripts) {
	scripts = new QAxScriptManager(this);
	scripts->addObject(this);
    }

    QWidgetList widgets = workspace->windowList();
    QWidgetListIt it(widgets);
    while (it.current()) {
	QAxBase *ax = (QAxBase*)it.current()->qt_cast("QAxBase");
	++it;
	if (!ax)
	    continue;
	scripts->addObject(ax);
    }

    QAxScript *script = scripts->load(file, file);
    if (script) {
	connect(script, SIGNAL(error(int, const QString&, int, const QString&)),
		this,   SLOT(logMacro(int,  const QString&, int, const QString&)));
	actionScriptingRun->setEnabled(TRUE);
    }
}

void MainWindow::logMacro(int code, const QString &description, int sourcePosition, const QString &sourceText )
{
    QString message = "Script: ";
    if (code)
	message += QString::number(code) + " ";
    message += "'" + description + "'";
    if (sourcePosition)
	message += " at position " + QString::number(sourcePosition);
    if (!sourceText.isEmpty())
	message += " '" + sourceText + "'";
    logMacros->append(message);
}
