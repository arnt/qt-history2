/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "sourceeditor.h"

#include "formwindow.h"
#include "metadatabase.h"

SourceEditor::SourceEditor( QWidget *parent, EditorInterface *iface )
    : QVBox( parent ), iFace( iface ), formWindow( 0 )
{
    iFace->editor( this );
    resize( 600, 400 );
}

void SourceEditor::setForm( FormWindow *fw )
{
    formWindow = fw;
    setCaption( tr( "Edit %1" ).arg( formWindow->name() ) );
    QValueList<MetaDataBase::Slot> slotList = MetaDataBase::slotList( formWindow );
    QString txt;
    for ( QValueList<MetaDataBase::Slot>::Iterator it = slotList.begin(); it != slotList.end(); ++it ) {
	txt += "function " + QString( formWindow->name() ) + "::" + QString( (*it).slot );
	txt += "\n{\n    \n}\n\n";
    }
    iFace->setText( txt );
}

void SourceEditor::setFunction( const QString &func )
{
}
