/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include <qsettings.h>
#include <qapplication.h>

void QActiveXSelect::init()
{
    activex = 0;
    QApplication::setOverrideCursor( WaitCursor );
    QSettings controls;
    QStringList clsids = controls.subkeyList( "/Classes/CLSID" );
    for ( QStringList::Iterator it = clsids.begin(); it != clsids.end(); ++it ) {
	QString clsid = *it;
	QStringList subkeys = controls.subkeyList( "/Classes/CLSID/" + clsid );
	if ( subkeys.contains( "Control" ) ) {
	    QString name = controls.readEntry( "/Classes/CLSID/" + clsid + "/Default" );
	    ActiveXList->insertItem( name );
	}
    }
    ActiveXList->sort();
    QApplication::restoreOverrideCursor();
}


void QActiveXSelect::controlSelected( const QString &ctrl )
{
    control = ctrl;
}

void QActiveXSelect::openLater()
{
    if ( !activex || !activex->isNull() || !designer ) {
	designer->release();
	delete this;
	return;
    }
    if ( exec() ) {
	activex->setControl( control );
	DesignerFormWindow *form = designer->currentForm();
	if ( form ) {
	    form->setPropertyChanged( activex, "control", TRUE );
	}
	designer->release();
	delete this;
    }
}

void QActiveXSelect::setActiveX( QActiveX *ax )
{
    activex = ax;
}

void QActiveXSelect::setDesigner( DesignerInterface *des )
{
    designer = des;
    designer->addRef();
}
