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

class ColorWidget : public QWidget
{
public:
    ColorWidget(QColor col, QWidget *parent = 0) : QWidget(parent) { setPalette(col); }
};

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    QWidget mainWidget;
    
    QBoxLayout *gm = new QVBoxLayout( &mainWidget, 5 );

    SimpleFlow *b1 = new SimpleFlow(gm);

    b1->addWidget( new QPushButton( "Short" ) );
    b1->addWidget( new QPushButton( "Longer" ) );
    b1->addWidget( new QPushButton( "Different text" ) );
    b1->addWidget( new QPushButton( "More text" ) );
    b1->addWidget( new QPushButton( "Even longer button text" ) );
    QPushButton* qb = new QPushButton( "Quit" );
    a.connect( qb, SIGNAL( clicked() ), SLOT( quit() ) );
    b1->addWidget( qb );

    BorderLayout *large = new BorderLayout(gm);
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

    CardLayout *card = new CardLayout( gm, 10 );

    card->addWidget(new ColorWidget(Qt::red));
    card->addWidget(new ColorWidget(Qt::green));
    card->addWidget(new ColorWidget(Qt::blue));
    card->addWidget(new ColorWidget(Qt::white));
    card->addWidget(new ColorWidget(Qt::black));
    card->addWidget(new ColorWidget(Qt::yellow));

    QLabel* s = new QLabel;
    s->setText( "outermost box" );
    s->setFrameStyle( QFrame::Panel | QFrame::Sunken );
    s->setAlignment( Qt::AlignVCenter | Qt::AlignHCenter );
    gm->addWidget( s );
    a.setMainWidget( &mainWidget );
    mainWidget.setWindowTitle("Qt Example - Custom Layout");
    mainWidget.show();

    return a.exec();
}
