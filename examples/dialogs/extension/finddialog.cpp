#include <QtGui>

#include "finddialog.h"

FindDialog::FindDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Find"));

    label = new QLabel(tr("Find &what:"), this);
    lineEdit = new QLineEdit(this);
    label->setBuddy(lineEdit);

    caseCheckBox = new QCheckBox(tr("Match &case"), this);
    fromStartCheckBox = new QCheckBox(tr("Search from &start"), this);
    fromStartCheckBox->setChecked(true);

    findButton = new QPushButton(tr("&Find"), this);
    findButton->setDefault(true);

    closeButton = new QPushButton(tr("Close"), this);

    optionsButton = new QPushButton(tr("More &Options"), this);
    optionsButton->setCheckable(true);

    optionsWidget = new QWidget(this);
    optionsWidget->hide();

    wholeWordsCheckBox = new QCheckBox(tr("&Whole words"), optionsWidget);
    backwardCheckBox = new QCheckBox(tr("Search &backward"), optionsWidget);
    searchSelectionCheckBox = new QCheckBox(tr("Search se&lection"), optionsWidget);

    connect(closeButton, SIGNAL(clicked()),
            this, SLOT(close()));
    connect(optionsButton, SIGNAL(toggled(bool)), optionsWidget,
            SLOT(setShown(bool)));

    QHBoxLayout *topLeftLayout = new QHBoxLayout;
    topLeftLayout->addWidget(label);
    topLeftLayout->addWidget(lineEdit);

    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addLayout(topLeftLayout);
    leftLayout->addWidget(caseCheckBox);
    leftLayout->addWidget(fromStartCheckBox);
    leftLayout->addStretch(1);
    leftLayout->addWidget(optionsWidget);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(findButton);
    rightLayout->addWidget(closeButton);
    rightLayout->addWidget(optionsButton);
    rightLayout->addStretch(1);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setResizeMode(QLayout::Fixed);
    mainLayout->addLayout(leftLayout);
    mainLayout->addLayout(rightLayout);

    QVBoxLayout *optionsLayout = new QVBoxLayout(optionsWidget);
    optionsLayout->setMargin(0);
    optionsLayout->addWidget(wholeWordsCheckBox);
    optionsLayout->addWidget(backwardCheckBox);
    optionsLayout->addWidget(searchSelectionCheckBox);
}
