/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "filechooser.h"
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qfiledialog.h>
#include <qlayout.h>

FileChooser::FileChooser( QWidget *parent, const char *name )
    : QWidget( parent, name ), md( File )
{
    QHBoxLayout *layout = new QHBoxLayout( this );
    layout->setMargin( 0 );

    lineEdit = new QLineEdit( this, "filechooser_lineedit" );
    layout->addWidget( lineEdit );

    connect( lineEdit, SIGNAL( textChanged( const QString & ) ),
	     this, SIGNAL( fileNameChanged( const QString & ) ) );

    button = new QPushButton( "...", this, "filechooser_button" );
    button->setFixedWidth( button->fontMetrics().width( " ... " ) );
    layout->addWidget( button );

    connect( button, SIGNAL( clicked() ),
	     this, SLOT( chooseFile() ) );

    setFocusProxy( lineEdit );
}

void FileChooser::setMode( Mode m )
{
    md = m;
}

FileChooser::Mode FileChooser::mode() const
{
    return md;
}

void FileChooser::setFileName( const QString &fn )
{
    lineEdit->setText( fn );
}

QString FileChooser::fileName() const
{
    return lineEdit->text();
}

void FileChooser::chooseFile()
{
    QString fn;
    if ( mode() == File )
	fn = QFileDialog::getOpenFileName( lineEdit->text(), QString::null, this );
    else
	fn = QFileDialog::getExistingDirectory( lineEdit->text(),this );

    if ( !fn.isEmpty() ) {
	lineEdit->setText( fn );
	emit fileNameChanged( fn );
    }
}

