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

#include "formsettingsimpl.h"
#include "formwindow.h"
#include "metadatabase.h"
#include "command.h"
#include "asciivalidator.h"
#include "mainwindow.h"
#include "project.h"

#include <qmultilineedit.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qcheckbox.h>

FormSettings::FormSettings( QWidget *parent, FormWindow *fw )
    : FormSettingsBase( parent, 0, true ), formwindow( fw )
{
    connect( buttonHelp, SIGNAL( clicked() ), MainWindow::self, SLOT( showDialogHelp() ) );
    MetaDataBase::MetaInfo info = MetaDataBase::metaInfo( fw );
    if ( info.classNameChanged && !info.className.isEmpty() )
	editClassName->setText( info.className );
    else
	editClassName->setText( fw->name() );
    editComment->setText( info.comment );
    editAuthor->setText( info.author );

    editClassName->setValidator( new AsciiValidator( QString( ":" ), editClassName ) );
    editPixmapFunction->setValidator( new AsciiValidator( QString( ":" ), editPixmapFunction ) );

    if ( formwindow->savePixmapInline() )
	radioPixmapInline->setChecked( true );
    else if ( formwindow->savePixmapInProject() )
	radioProjectImageFile->setChecked( true );
    else
	radioPixmapFunction->setChecked( true );
    editPixmapFunction->setText( formwindow->pixmapLoaderFunction() );
    radioProjectImageFile->setEnabled( !fw->project()->isDummy() );
    spinSpacing->setValue( formwindow->layoutDefaultSpacing() );
    spinMargin->setValue( formwindow->layoutDefaultMargin() );
    editSpacingFunction->setValidator( new AsciiValidator( QString( ":" ), editSpacingFunction ) );
    editMarginFunction->setValidator( new AsciiValidator( QString( ":" ), editMarginFunction ) ); 
    checkLayoutFunctions->setChecked( formwindow->hasLayoutFunctions() );
    editSpacingFunction->setText( formwindow->spacingFunction() );
    editMarginFunction->setText( formwindow->marginFunction() );
}

void FormSettings::okClicked()
{
    MetaDataBase::MetaInfo info;
    info.className = editClassName->text();
    info.classNameChanged = info.className != QString( formwindow->name() );
    info.comment = editComment->text();
    info.author = editAuthor->text();
    MetaDataBase::setMetaInfo( formwindow, info );

    formwindow->commandHistory()->setModified( true );

    if ( formwindow->savePixmapInline() ) {
	MetaDataBase::clearPixmapArguments( formwindow );
	MetaDataBase::clearPixmapKeys( formwindow );
    } else if ( formwindow->savePixmapInProject() ) {
	MetaDataBase::clearPixmapArguments( formwindow );
    } else {
	MetaDataBase::clearPixmapKeys( formwindow );
    }

    if ( radioPixmapInline->isChecked() ) {
	formwindow->setSavePixmapInline( true );
	formwindow->setSavePixmapInProject( false );
    } else if ( radioProjectImageFile->isChecked() ){
	formwindow->setSavePixmapInline( false );
	formwindow->setSavePixmapInProject( true );
    } else {
	formwindow->setSavePixmapInline( false );
	formwindow->setSavePixmapInProject( false );
    }
    
    if ( checkLayoutFunctions->isChecked() )
	formwindow->hasLayoutFunctions( true );
    else
	formwindow->hasLayoutFunctions( false );

    formwindow->setPixmapLoaderFunction( editPixmapFunction->text() );
    formwindow->setLayoutDefaultSpacing( spinSpacing->value() );
    formwindow->setSpacingFunction( editSpacingFunction->text() );
    formwindow->setLayoutDefaultMargin( spinMargin->value() );
    formwindow->setMarginFunction( editMarginFunction->text() );

    accept();
}
