/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "informationwindow.h"

InformationWindow::InformationWindow(int id, QSqlRelationalTableModel *offices,
                                     QWidget *parent)
    : QDialog(parent)
{
    QLabel *locationLabel = new QLabel(tr("Location: "));
    QLabel *countryLabel = new QLabel(tr("Country: "));
    QLabel *descriptionLabel = new QLabel(tr("Description: "));
    QLabel *imageFileLabel = new QLabel(tr("Image file: "));

    locationText = new QLabel;
    countryText = new QLabel;
    descriptionEditor = new QTextEdit;

    imageFileEditor = new QComboBox;
    imageFileEditor->setModel(offices->relationModel(1));
    imageFileEditor->setModelColumn(offices->relationModel(1)->fieldIndex("file"));

    connect(descriptionEditor, SIGNAL(textChanged()),
            this, SLOT(enableButtons()));
    connect(imageFileEditor, SIGNAL(currentIndexChanged(int)),
            this, SLOT(enableButtons()));

    createButtons();

    mapper = new QDataWidgetMapper(this);
    mapper->setModel(offices);
    mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    mapper->setItemDelegate(new QSqlRelationalDelegate(mapper));
    mapper->addMapping(imageFileEditor, 1);
    mapper->addMapping(locationText, 2);
    mapper->addMapping(countryText, 3);
    mapper->addMapping(descriptionEditor, 4);
    mapper->setCurrentIndex(id);

    locationId = id;
    storedImage = imageFileEditor->currentText();

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(locationLabel, 0, 0, Qt::AlignLeft | Qt::AlignTop);
    layout->addWidget(countryLabel, 1, 0, Qt::AlignLeft | Qt::AlignTop);
    layout->addWidget(imageFileLabel, 2, 0, Qt::AlignLeft | Qt::AlignTop);
    layout->addWidget(descriptionLabel, 3, 0, Qt::AlignLeft | Qt::AlignTop);
    layout->addWidget(locationText, 0, 1);
    layout->addWidget(countryText, 1, 1);
    layout->addWidget(imageFileEditor, 2, 1);
    layout->addWidget(descriptionEditor, 3, 1);
    layout->addWidget(buttonBox, 4, 0, 1, 2);
    setLayout(layout);

    setWindowFlags(Qt::Window);

    enableButtons(false);

    setWindowTitle(tr("Trolltech Office: %1").arg(locationText->text()));
    resize(320, sizeHint().height());
}

void InformationWindow::createButtons()
{
    closeButton = new QPushButton(tr("&Close"));
    revertButton = new QPushButton(tr("&Revert"));
    submitButton = new QPushButton(tr("&Submit"));

    closeButton->setDefault(true);

    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(revertButton, SIGNAL(clicked()), this, SLOT(revert()));
    connect(submitButton, SIGNAL(clicked()), this, SLOT(submit()));

    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(submitButton, QDialogButtonBox::ResetRole);
    buttonBox->addButton(revertButton, QDialogButtonBox::ResetRole);
    buttonBox->addButton(closeButton, QDialogButtonBox::RejectRole);
}

int InformationWindow::id()
{
    return locationId;
}

void InformationWindow::revert()
{
    mapper->revert();
    enableButtons(false);
}

void InformationWindow::submit()
{
    QString newImage(imageFileEditor->currentText());

    if (storedImage != newImage) {
        storedImage = newImage;
        emit imageChanged(locationId, newImage);
    }

    mapper->submit();
    mapper->setCurrentIndex(locationId);

    enableButtons(false);
}

void InformationWindow::enableButtons(bool enable)
{
    revertButton->setEnabled(enable);
    submitButton->setEnabled(enable);
}


