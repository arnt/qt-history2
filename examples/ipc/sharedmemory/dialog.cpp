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

/*!
  \class Dialog

  \brief This class is a simple example of how to use QSharedMemory.

  It is a simple dialog that presents a few buttons. To compile the
  example, run make in qt/examples/ipc. Then run the executable twice
  to create two processes running the dialog. In one of the processes,
  press the button to load an image into a shared memory segment, and
  then select an image file to load. Once the first process has loaded
  and displayed the image, in the second process, press the button to
  read the same image from shared memory. The second process displays
  the same image loaded from its new loaction in shared memory.
*/

/*!
  The class contains a data member \l {QSharedMemory} {sharedMemory},
  which is initialized with the key "QSharedMemoryExample" to force
  all instances of Dialog to access the same shared memory segment.
  The constructor also connects the clicked() signal from each of the
  three dialog buttons to the slot function appropriate for handling
  each button.
*/
Dialog::Dialog(QWidget *parent)
  : QDialog(parent), sharedMemory("QSharedMemoryExample")
{
    ui.setupUi(this);
    connect(ui.loadFromFileButton, SIGNAL(clicked()), SLOT(loadFromFile()));
    connect(ui.loadFromSharedMemoryButton,
	    SIGNAL(clicked()),
	    SLOT(loadFromMemory()));
    connect(ui.detachButton, SIGNAL(clicked()), SLOT(detach()));
    setWindowTitle(tr("SharedMemory Example"));
}

/*!
  This slot function is called when the \tt {Load Image From File...}
  button is pressed on one of the Dialog processes. First, it prompts
  the user to select an image file. Then it loads the selected image
  from the file into a QImage, displays it in the Dialog, and streams
  it into a QBuffer using a QDataStream. 

  Next, it requests a shared memory segment large enough to hold the
  data in the QBuffer, locks the segment to prevent the other Dialog
  process from accessing it, and copies the data from the QBuffer into
  the shared memory. Finally, it unlocks the shared memory segment so
  the other Dialog process can access it.
 */
void Dialog::loadFromFile()
{
    if (sharedMemory.isAttached())
        detach();

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

/*!
  This slot function is called when the \tt {Load Image from Shared
  Memory} button is pressed. First, it attaches the Dialog process to
  the shared memory segment. Then it locks the segment for exclusive
  access, copies the data from the segment into a QBuffer, and streams
  the QBuffer into a QImage. Then it unlocks the shared memory segment,
  detaches from it, and finally displays the QImage in the Dialog.
 */
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

/*!
  This is the slot function called when the \tt {Detatch Image from
  Shared Memory} button is pressed.
 */
void Dialog::detach()
{
    if (!sharedMemory.detach())
        ui.label->setText(tr("Unable to detach from shared memory: " \
                             "\n%1.\n").arg(sharedMemory.errorString()));
    ui.detachButton->setEnabled(false);
}

