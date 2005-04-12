/****************************************************************
**
** Implementation of PrintPanel class, translation tutorial 3
**
****************************************************************/

#include "printpanel.h"

#include <QButtonGroup>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>

PrintPanel::PrintPanel(QWidget *parent)
    : QWidget(parent)
{
/*
    QLabel *lab = new QLabel(tr("<b>TROLL PRINT</b>"), this);
    lab->setAlignment(Qt::AlignCenter);
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

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->addWidget(twoSided);
    vbox->addWidget(colors);
}
