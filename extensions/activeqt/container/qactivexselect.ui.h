/****************************************************************************
**
** Implementation of the Qt Designer integration plugin.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the Active Qt integration.
** EDITIONS: ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


#include <qsettings.h>
#include <qapplication.h>
#include <designerinterface.h>
#include <qt_windows.h>

class ListBoxText : public QListBoxText
{
public:
    ListBoxText( QListBox *box, const QString &name, const QString &id )
	: QListBoxText( box, name ), ID( id )
    {
    }

    QString clsid() const
    {
	return ID;
    }

private:
    QString ID;
};

void QActiveXSelect::init()
{
    QApplication::setOverrideCursor( WaitCursor );
    HKEY classes_key;
    RegOpenKeyA( HKEY_CLASSES_ROOT, "CLSID", &classes_key );
    if ( classes_key ) {
	DWORD index = 0;
	LONG result = 0;
	char buffer[256];
	DWORD szBuffer = 255;
	FILETIME ft;
	do {
	    result = RegEnumKeyExA( classes_key, index, (char*)&buffer, &szBuffer, 0, 0, 0, &ft );
	    szBuffer = 255;
	    if ( result == ERROR_SUCCESS ) {
		HKEY sub_key;
		QString clsid = QString::fromLocal8Bit( buffer );
		result = RegOpenKeyA( classes_key, QString(clsid + "\\Control").local8Bit(), &sub_key );
		if ( result == ERROR_SUCCESS ) {
		    RegCloseKey( sub_key );
		    RegQueryValueA( classes_key, buffer, (char*)&buffer, (LONG*)&szBuffer );
		    QString name = QString::fromLocal8Bit( buffer, szBuffer );
		    if ( !name.isEmpty() )
			(void)new ListBoxText( ActiveXList, name, clsid );

		}
		result = ERROR_SUCCESS;
	    }
	    szBuffer = 255;
	    ++index;
	} while ( result == ERROR_SUCCESS );
	RegCloseKey( classes_key );
    }

    ActiveXList->sort();
    QApplication::restoreOverrideCursor();

    ActiveXList->setFocus();
}


void QActiveXSelect::controlSelected( QListBoxItem *ctrl )
{
    if ( !ctrl )
	return;

    ActiveX->setText( ((ListBoxText*)ctrl)->clsid() );
}

QString QActiveXSelect::selectedControl()
{
    return ActiveX->text();
}
