/****************************************************************
**
** Implementation of PrintPanel class, translation tutorial 3
**
****************************************************************/

#include "printpanel.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QButtonGroup>
#include <QRadioButton>

PrintPanel::PrintPanel(QWidget *parent)
    : QVBoxWidget(parent)
{
    setMargin( 10 );
    setSpacing( 10 );

/*
    QLabel *lab = new QLabel( tr("<b>TROLL PRINT</b>"), this );
    lab->setAlignment( AlignCenter );
*/

    QRadioButton *but;

    QGroupBox *twoSided = new QGroupBox(this);
    twoSided->setTitle(tr("2-sided"));

    QButtonGroup *g = new QButtonGroup(twoSided);
    QHBoxLayout *l = new QHBoxLayout(twoSided);

    but = new QRadioButton(tr("Enabled"), twoSided);
    g->addButton(but);
    l->addWidget(but);

    but = new QRadioButton(tr("Disabled"), twoSided);
    g->addButton(but);
    but->toggle();
    l->addWidget(but);

    QGroupBox *colors = new QGroupBox(this);
    colors->setTitle(tr("Colors"));

    g = new QButtonGroup(colors);
    l = new QHBoxLayout(colors);

    but = new QRadioButton(tr("Enabled"), colors);
    g->addButton(but);
    l->addWidget(but);

    but = new QRadioButton(tr("Disabled"), colors);
    g->addButton(but);
    l->addWidget(but);

    but->toggle();
}
