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

#include <q3buttongroup.h>
#include <qradiobutton.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qaccel.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qstatusbar.h>
#include <qapplication.h>

#include "mywidget.h"
#include <qmessagebox.h>

MyWidget::MyWidget( QWidget* parent, const char* name )
        : Q3MainWindow( parent, name )
{
    QVBox* central = new QVBox(this);
    central->setMargin( 5 );
    central->setSpacing( 5 );
    setCentralWidget(central);

    QPopupMenu* file = new QPopupMenu(this);
    file->insertItem( tr("E&xit"), qApp, SLOT(quit()),
            QAccel::stringToKey(tr("Ctrl+Q")) );
    menuBar()->insertItem( tr("&File"), file );

    setWindowTitle( tr( "Internationalization Example" ) );

    QString l;
    QMessageBox::information( 0, tr("Language: English"), tr("Language: English"), tr( "E&xit" ) );
    statusBar()->message( tr("Language: English") );

    ( void )new QLabel( tr( "The Main Window" ), central );

    Q3ButtonGroup* gbox = new Q3ButtonGroup( 1, Qt::Horizontal,
                                      tr( "View" ), central );
    (void)new QRadioButton( tr( "Perspective" ), gbox );
    (void)new QRadioButton( tr( "Isometric" ), gbox );
    (void)new QRadioButton( tr( "Oblique" ), gbox );

    initChoices(central);
}

static const char* choices[] = {
    QT_TRANSLATE_NOOP( "MyWidget", "First" ),
    QT_TRANSLATE_NOOP( "MyWidget", "Second" ),
    QT_TRANSLATE_NOOP( "MyWidget", "Third" ),
    0
};

void MyWidget::initChoices(QWidget* parent)
{
    QListBox* lb = new QListBox( parent );
    for ( int i = 0; choices[i]; i++ )
        lb->insertItem( tr( choices[i] ) );
}

void MyWidget::closeEvent(QCloseEvent* e)
{
    QWidget::closeEvent(e);
    emit closed();
}
