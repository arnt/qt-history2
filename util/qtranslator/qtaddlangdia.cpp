/****************************************************************************
** $Id: //depot/qt/main/util/qtranslator/qtaddlangdia.cpp#1 $
**
** This is a utility program for translating Qt applications
**
**
** Copyright (C) 1999 by Trolltech AS.  All rights reserved.
**
*****************************************************************************/

#include "qtaddlangdia.h"

#include <qhbox.h>
#include <qvbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qpushbutton.h>

/****************************************************************************
 *
 * Class: QTAddLangDia
 *
 ****************************************************************************/

QTAddLangDia::QTAddLangDia( QWidget *parent, const char *name )
    : QDialog( parent, name, TRUE )
{
    back = new QVBox( this );
    back->setMargin( 5 );
    back->setSpacing( 5 );
    
    QHBox *row1 = new QHBox( back );
    row1->setSpacing( 5 );
    
    ( void )new QLabel( tr( "Enter Language Shortcut (e.g. de, no, ..)" ), row1 );    
    langCombo = new QComboBox( TRUE, row1 );

    langCombo->insertItem( "br" );
    langCombo->insertItem( "ca" );
    langCombo->insertItem( "cs" );
    langCombo->insertItem( "da" );
    langCombo->insertItem( "de" );
    langCombo->insertItem( "el" );
    langCombo->insertItem( "eo" );
    langCombo->insertItem( "es" );
    langCombo->insertItem( "et" );
    langCombo->insertItem( "fi" );
    langCombo->insertItem( "fr" );
    langCombo->insertItem( "he" );
    langCombo->insertItem( "hr" );
    langCombo->insertItem( "hu" );
    langCombo->insertItem( "is" );
    langCombo->insertItem( "it" );
    langCombo->insertItem( "ko" );
    langCombo->insertItem( "mk" );
    langCombo->insertItem( "nl" );
    langCombo->insertItem( "no" );
    langCombo->insertItem( "pl" );
    langCombo->insertItem( "pt" );
    langCombo->insertItem( "pt_BR" );
    langCombo->insertItem( "ro" );
    langCombo->insertItem( "ru" );
    langCombo->insertItem( "sk" );
    langCombo->insertItem( "sv" );
    langCombo->insertItem( "tr" );
    langCombo->insertItem( "zh_CN.GB2312" );
    
    QHBox *row2 = new QHBox( back );
    row2->setSpacing( 5 );
    
    // HACK: Win some space - I should use a real GM here
    ( void )new QWidget( row2 );
    ( void )new QWidget( row2 );
    ( void )new QWidget( row2 );
    QPushButton *ok = new QPushButton( tr( "&OK" ), row2 );
    ok->setDefault( TRUE );
    QPushButton *cancel = new QPushButton( tr( "&Cancel" ), row2 );
    
    connect( ok, SIGNAL( clicked() ),
             this, SLOT( slotOK() ) );
    connect( cancel, SIGNAL( clicked() ),
             this, SLOT( reject() ) );
    
    setCaption( tr( "Add new language" ) );
}

void QTAddLangDia::resizeEvent( QResizeEvent *e )
{
    QDialog::resizeEvent( e );
    back->resize( size() );
}

void QTAddLangDia::slotOK()
{
    if ( langCombo->currentText().isEmpty() ) {
        reject();
        return;
    }
    
    accept();
    emit newLangChosen( langCombo->currentText() );
}
