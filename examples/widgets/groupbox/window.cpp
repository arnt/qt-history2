#include <QtGui>

#include "window.h"

Window::Window(QWidget *parent)
    : QWidget(parent)
{
    QGridLayout *grid = new QGridLayout(this);
    grid->addWidget(createFirstExclusiveGroup(), 0, 0);
    grid->addWidget(createSecondExclusiveGroup(), 1, 0);
    grid->addWidget(createNonExclusiveGroup(), 0, 1);
    grid->addWidget(createPushButtonGroup(), 1, 1);

    setWindowTitle(tr("Group Box"));
    resize(480, 320);
}

QGroupBox *Window::createFirstExclusiveGroup()
{
    QGroupBox *groupBox = new QGroupBox(tr("Exclusive Radio Buttons"), this);

    QRadioButton *radio1 = new QRadioButton(tr("&Radio button 1"), groupBox);
    QRadioButton *radio2 = new QRadioButton(tr("R&adio button 2"), groupBox);
    QRadioButton *radio3 = new QRadioButton(tr("Ra&dio button 3"), groupBox);

    radio1->setChecked(true);

    QVBoxLayout *vbox = new QVBoxLayout(groupBox);
    vbox->addWidget(radio1);
    vbox->addWidget(radio2);
    vbox->addWidget(radio3);
    vbox->addStretch(1);

    return groupBox;
}

QGroupBox *Window::createSecondExclusiveGroup()
{
    QGroupBox *groupBox = new QGroupBox(tr("E&xclusive Radio Buttons"), this);
    groupBox->setCheckable(true);
    groupBox->setChecked(false);

    QRadioButton *radio1 = new QRadioButton(tr("Rad&io button 1"), groupBox);
    QRadioButton *radio2 = new QRadioButton(tr("Radi&o button 2"), groupBox);
    QRadioButton *radio3 = new QRadioButton(tr("Radio &button 3"), groupBox);
    radio1->setChecked(true);
    QCheckBox *checkBox = new QCheckBox(tr("Ind&ependent checkbox"), groupBox);
    checkBox->setChecked(true);

    QVBoxLayout *vbox = new QVBoxLayout(groupBox);
    vbox->addWidget(radio1);
    vbox->addWidget(radio2);
    vbox->addWidget(radio3);
    vbox->addWidget(checkBox);
    vbox->addStretch(1);

    return groupBox;
}

QGroupBox *Window::createNonExclusiveGroup()
{
    QGroupBox *groupBox = new QGroupBox(tr("Non-Exclusive Checkboxes"), this);
    groupBox->setFlat(true);

    QCheckBox *checkBox1 = new QCheckBox(tr("&Checkbox 1"), groupBox);
    QCheckBox *checkBox2 = new QCheckBox(tr("C&heckbox 2"), groupBox);
    checkBox2->setChecked(true);
    QCheckBox *tristateBox = new QCheckBox(tr("Tri-&state button"), groupBox);
    tristateBox->setTristate(true);
    tristateBox->setCheckState(Qt::PartiallyChecked);

    QVBoxLayout *vbox = new QVBoxLayout(groupBox);
    vbox->addWidget(checkBox1);
    vbox->addWidget(checkBox2);
    vbox->addWidget(tristateBox);
    vbox->addStretch(1);

    return groupBox;
}

QGroupBox *Window::createPushButtonGroup()
{
    QGroupBox *groupBox = new QGroupBox(tr("&Push Buttons"), this);
    groupBox->setCheckable(true);
    groupBox->setChecked(true);

    QPushButton *pushButton = new QPushButton(tr("&Normal Button"), groupBox);
    QPushButton *toggleButton = new QPushButton(tr("&Toggle Button"), groupBox);
    toggleButton->setCheckable(true);
    toggleButton->setChecked(true);
    QPushButton *flatButton = new QPushButton(tr("&Flat Button"), groupBox);
    flatButton->setFlat(true);

    QPushButton *popupButton = new QPushButton(tr("Pop&up Button"), groupBox);
    QMenu *menu = new QMenu(this);
    menu->addAction(tr("&First Item"));
    menu->addAction(tr("&Second Item"));
    menu->addAction(tr("&Third Item"));
    menu->addAction(tr("F&ourth Item"));
    popupButton->setMenu(menu);

    QVBoxLayout *vbox = new QVBoxLayout(groupBox);
    vbox->addWidget(pushButton);
    vbox->addWidget(toggleButton);
    vbox->addWidget(flatButton);
    vbox->addWidget(popupButton);
    vbox->addStretch(1);

    return groupBox;
}
