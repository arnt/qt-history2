#include <QtGui>

#include "finddialog.h"

FindDialog::FindDialog(QWidget *parent)
    : QDialog(parent)
{
    label = new QLabel(tr("Find &what:"), this);
    lineEdit = new QLineEdit(this);
    label->setBuddy(lineEdit);

    caseCheckBox = new QCheckBox(tr("Match &case"), this);
    fromStartCheckBox = new QCheckBox(tr("Search from &start"), this);
    fromStartCheckBox->setChecked(true);

    findButton = new QPushButton(tr("&Find"), this);
    findButton->setDefault(true);

    closeButton = new QPushButton(tr("Close"), this);

    moreButton = new QPushButton(tr("&More"), this);
    moreButton->setCheckable(true);
    moreButton->setAutoDefault(false);

    extension = new QWidget(this);
    extension->hide();

    wholeWordsCheckBox = new QCheckBox(tr("&Whole words"), extension);
    backwardCheckBox = new QCheckBox(tr("Search &backward"), extension);
    searchSelectionCheckBox = new QCheckBox(tr("Search se&lection"), extension);

    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(moreButton, SIGNAL(toggled(bool)), extension, SLOT(setVisible(bool)));

    QVBoxLayout *extensionLayout = new QVBoxLayout(extension);
    extensionLayout->setMargin(0);
    extensionLayout->addWidget(wholeWordsCheckBox);
    extensionLayout->addWidget(backwardCheckBox);
    extensionLayout->addWidget(searchSelectionCheckBox);

    QHBoxLayout *topLeftLayout = new QHBoxLayout;
    topLeftLayout->addWidget(label);
    topLeftLayout->addWidget(lineEdit);

    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addLayout(topLeftLayout);
    leftLayout->addWidget(caseCheckBox);
    leftLayout->addWidget(fromStartCheckBox);
    leftLayout->addStretch(1);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(findButton);
    rightLayout->addWidget(closeButton);
    rightLayout->addWidget(moreButton);
    rightLayout->addStretch(1);

    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    mainLayout->addLayout(leftLayout, 0, 0);
    mainLayout->addLayout(rightLayout, 0, 1);
    mainLayout->addWidget(extension, 1, 0, 1, 2);

    setWindowTitle(tr("Extension"));
}
