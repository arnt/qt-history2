/****************************************************************
**
** Implementation of PrintPanel class, translation tutorial 3
**
****************************************************************/

#include "printpanel.h"

#include <qlayout.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>

PrintPanel::PrintPanel( QWidget *parent )
    : QVBox( parent )
{
    setMargin( 10 );
    setSpacing( 10 );

/*
    QLabel *lab = new QLabel( tr("<b>TROLL PRINT</b>"), this );
    lab->setAlignment( AlignCenter );
*/

    QRadioButton *but;

    QGroupBox *twoSided = new QGroupBox( this );
    twoSided->setTitle( tr("2-sided") );

    QButtonGroup *g = new QButtonGroup(twoSided);
    QHBoxLayout *l = new QHBoxLayout(twoSided);

    but = new QRadioButton( tr("Enabled"), twoSided );
    g->addButton(but);
    l->addWidget(but);

    but = new QRadioButton( tr("Disabled"), twoSided );
    g->addButton(but);
    but->toggle();
    l->addWidget(but);

    QGroupBox *colors = new QGroupBox( this );
    colors->setTitle( tr("Colors") );

    g = new QButtonGroup(colors);
    l = new QHBoxLayout(twoSided);

    but = new QRadioButton( tr("Enabled"), colors );
    g->addButton(but);
    l->addWidget(but);

    but = new QRadioButton( tr("Disabled"), colors );
    g->addButton(but);
    l->addWidget(but);

    but->toggle();
}
