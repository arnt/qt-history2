/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


#include "dialog.h"

#include <QFileDialog>
#include <QBuffer>

Dialog::Dialog(QWidget *parent) : QDialog(parent), sharedMemory("QSharedMemoryExample")
{
    ui.setupUi(this);
    connect(ui.loadFromFileButton, SIGNAL(clicked()), SLOT(loadFromFile()));
    connect(ui.loadFromSharedMemoryButton, SIGNAL(clicked()), SLOT(loadFromMemory()));
    connect(ui.detachButton, SIGNAL(clicked()), SLOT(detach()));
    setWindowTitle(tr("SharedMemory Example"));
}

void Dialog::loadFromFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, QString(), QString(),
                                        tr("Images (*.png *.xpm *.jpg)"));
    QImage image;
    if (!image.load(fileName))
        return;
    ui.label->setPixmap(QPixmap::fromImage(image));

    // load into shared memory
    QBuffer buffer;
    buffer.open(QBuffer::ReadWrite);
    QDataStream out(&buffer);
    out << image;
    int size = buffer.size();

    if (!sharedMemory.create(size)) {
        ui.label->setText(tr("Unable to store in shared memory: \n%1.\n" \
            "Is there already an image there?").arg(sharedMemory.errorString()));
        return;
    }
    sharedMemory.lock();
    char *to = (char*)sharedMemory.data();
    const char *from = buffer.data().data();
    memcpy(to, from, qMin(sharedMemory.size(), size));
    sharedMemory.unlock();

    ui.detachButton->setEnabled(true);
}

void Dialog::loadFromMemory()
{
    if (!sharedMemory.attach()) {
        ui.label->setText(tr("Unable to load from shared memory: \n%1.\n" \
            "Try loading an image first.").arg(sharedMemory.errorString()));
        return;
    }

    QBuffer buffer;
    QDataStream in(&buffer);
    QImage image;

    sharedMemory.lock();
    buffer.setData((char*)sharedMemory.constData(), sharedMemory.size());
    buffer.open(QBuffer::ReadOnly);
    in >> image;
    sharedMemory.unlock();

    sharedMemory.detach();
    ui.label->setPixmap(QPixmap::fromImage(image));
}

void Dialog::detach()
{
    if (!sharedMemory.detach())
        ui.label->setText(tr("Unable to detach from shared memory: " \
                             "\n%1.\n").arg(sharedMemory.errorString()));
    ui.detachButton->setEnabled(false);
}

