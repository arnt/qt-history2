/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "flow.h"
#include "border.h"
#include "card.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcolor.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qmultilineedit.h>
#include <qcolor.h>

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    QWidget *f = new QWidget;
    QBoxLayout *gm = new QVBoxLayout( f, 5 );

#if 1
    SimpleFlow *b1 = new SimpleFlow;
    gm->addLayout( b1 );

    b1->addWidget( new QPushButton( "Short" ) );
    b1->addWidget( new QPushButton( "Longer" ) );
    b1->addWidget( new QPushButton( "Different text" ) );
    b1->addWidget( new QPushButton( "More text" ) );
    b1->addWidget( new QPushButton( "Even longer button text" ) );
    QPushButton* qb = new QPushButton( "Quit" );
    a.connect( qb, SIGNAL( clicked() ), SLOT( quit() ) );
    b1->addWidget( qb );
#else
    gm->addWidget( new QLabel( "Testlabel" ) );
#endif

    BorderLayout *large = new BorderLayout;
    gm->addLayout( large );
    large->setSpacing( 5 );
    large->addWidget( new QPushButton( "North" ), BorderLayout::North );
    large->addWidget( new QPushButton( "West" ), BorderLayout::West );
    QMultiLineEdit* m = new QMultiLineEdit;
    m->setText( "Central\nWidget" );
    large->addWidget( m, BorderLayout::Center );
    QWidget *east1 = new QPushButton( "East" );
    large->addWidget( east1, BorderLayout::East );
    QWidget *east2 = new QPushButton( "East 2" );
    large->addWidget( east2 , BorderLayout::East );
    large->addWidget( new QPushButton( "South" ), BorderLayout::South );
    //Left-to-right tab order looks better:
    QWidget::setTabOrder( east2, east1 );

    CardLayout *card = new CardLayout( 10 );
    gm->addLayout( card );

    QWidget *crd = new QWidget;
    crd->setPalette(QPalette(Qt::red));
    card->addWidget(crd);
    crd = new QWidget;
    crd->setPalette(QPalette(Qt::green));
    card->addWidget( crd );
    crd = new QWidget;
    crd->setPalette(QPalette(Qt::blue));
    card->addWidget( crd );
    crd = new QWidget;
    crd->setPalette(QPalette(Qt::white));
    card->addWidget( crd );
    crd = new QWidget;
    crd->setPalette(QPalette(Qt::black));
    card->addWidget( crd );
    crd = new QWidget;
    crd->setPalette(QPalette(Qt::yellow));
    card->addWidget( crd );

    QLabel* s = new QLabel;
    s->setText( "outermost box" );
    s->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    s->setAlignment( Qt::AlignVCenter | Qt::AlignHCenter );
    gm->addWidget( s );
    a.setMainWidget( f );
    f->setWindowTitle("Qt Example - Custom Layout");
    f->show();

    int result = a.exec();
    delete f;
    return result;
}
