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
#include "project.h"

SourceEditor::SourceEditor( QWidget *parent, EditorInterface *iface )
    : QVBox( parent ), iFace( iface ), formWindow( 0 )
{
    iFace->editor( this );
    resize( 600, 400 );
}

SourceEditor::~SourceEditor()
{
    iFace->release();
}

class NormalizeObject : public QObject
{
public:
    NormalizeObject() : QObject() {}
    static QCString normalizeSignalSlot( const char *signalSlot ) { return QObject::normalizeSignalSlot( signalSlot ); }
};


void SourceEditor::setForm( FormWindow *fw )
{
    save();
    formWindow = fw;
    setCaption( tr( "Edit %1" ).arg( formWindow->name() ) );
    iFace->setText( sourceOfForm( formWindow, lang, iFace ) );
    if ( fw->project() )
	iFace->setContext( fw->project()->formList(), fw );
}

QString SourceEditor::sourceOfForm( FormWindow *fw, const QString &lang, EditorInterface *iface )
{
    QValueList<MetaDataBase::Slot> slotList = MetaDataBase::slotList( fw );
    QMap<QString, QString> bodies = MetaDataBase::functionBodies( fw );
    QString txt;
    for ( QValueList<MetaDataBase::Slot>::Iterator it = slotList.begin(); it != slotList.end(); ++it ) {
	if ( (*it).language != lang )
	    continue;
	QString sl( (*it).slot );
	txt += iface->createFunctionStart( fw->name(), sl );
	QMap<QString, QString>::Iterator bit = bodies.find( NormalizeObject::normalizeSignalSlot( (*it).slot ) );
	if ( bit != bodies.end() )
	    txt += "\n" + *bit + "\n\n";
	else
	    txt += "\n{\n    \n}\n\n";
    }
    return txt;
}

void SourceEditor::setFunction( const QString &func )
{
    iFace->scrollTo( func );
}

void SourceEditor::closeEvent( QCloseEvent *e )
{
    emit hidden();
    e->accept();
}

void SourceEditor::save()
{
    QMap<QString, QString> functions;
    iFace->functions( &functions );
    MetaDataBase::setFunctionBodies( formWindow, functions );
}

QString SourceEditor::language() const
{
    return lang;
}

void SourceEditor::setLanguage( const QString &l )
{
    lang = l;
}
