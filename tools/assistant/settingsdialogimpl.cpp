/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Assistant.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "settingsdialogimpl.h"
#include "docuparser.h"
#include "config.h"

#include <qapplication.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qcolordialog.h>
#include <qdir.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qmessagebox.h>
#include <qptrstack.h>
#include <qsettings.h>
#include <qtimer.h>
#include <qtoolbutton.h>
#include <qtabwidget.h>
#include <qmap.h>


SettingsDialog::SettingsDialog( QWidget *parent, const char* name )
    : SettingsDialogBase( parent, name )
{
    init();
}

void SettingsDialog::init()
{
    Config *config = Config::configuration();

    browserApp->setText( config->webBrowser() );
    homePage->setText( config->homePage() );
    pdfApp->setText( config->pdfReader() );
}

void SettingsDialog::selectColor()
{
    QColor c = QColorDialog::getColor( colorButton->paletteBackgroundColor(), this );
    colorButton->setPaletteBackgroundColor( c );
}

void SettingsDialog::browseWebApp()
{
    setFile( browserApp, tr( "Qt Assistant - Set Web Browser" ) );
}

void SettingsDialog::browsePDFApplication()
{
    setFile( pdfApp, tr( "Qt Assistant - Set PDF Browser" ) );
}

void SettingsDialog::browseHomepage()
{
    setFile( homePage, tr( "Qt Assistant - Set Homepage" ) );
}

void SettingsDialog::setFile( QLineEdit *le, const QString &caption )
{
    QFileDialog *fd = new QFileDialog( this );
    fd->setCaption( caption );
    fd->setMode( QFileDialog::AnyFile );
    fd->setDir( QDir::homeDirPath() );

    if ( fd->exec() == QDialog::Accepted ) {
	if ( !fd->selectedFile().isEmpty() )
	   le->setText( fd->selectedFile() );
    }
}

void SettingsDialog::accept()
{
    Config *config = Config::configuration();

    config->setWebBrowser( browserApp->text() );
    config->setHomePage( homePage->text() );
    config->setPdfReader( pdfApp->text() );

    hide();
    done( Accepted );
}

void SettingsDialog::reject()
{
    init();
    done( Rejected );
}
