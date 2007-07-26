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
    setWindowTitle(tr("SharedMemory Example"));
}

/*!
  This slot function is called when the \tt {Load Image From File...}
  button is pressed on the firs Dialog process. First, it tests
  whether the process is already connected to a shared memory segment
  and, if so, detaches from that segment. This ensures that we always
  start the example from the beginning if we run it multiple times
  with the same two Dialog processes. After detaching from an existing
  shared memory segment, the user is prompted to select an image file.
  The selected file is loaded into a QImage. The QImage is displayed
  in the Dialog and streamed into a QBuffer with a QDataStream.

  Next, it gets a new shared memory segment from the system big enough
  to hold the image data in the QBuffer, and it locks the segment to
  prevent the second Dialog process from accessing it. Then it copies
  the image from the QBuffer into the shared memory segment. Finally,
  it unlocks the shared memory segment so the second Dialog process
  can access it.

  After this function runs, the user is expected to press the \tt
  {Load Image from Shared Memory} button on the second Dialog process.

  \sa loadFromMemory()
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
}

/*!
  This slot function is called in the second Dialog process, when the
  user presses the \tt {Load Image from Shared Memory} button. First,
  it attaches the process to the shared memory segment created by the
  first Dialog process. Then it locks the segment for exclusive
  access, copies the image data from the segment into a QBuffer, and
  streams the QBuffer into a QImage. Then it unlocks the shared memory
  segment, detaches from it, and finally displays the QImage in the
  Dialog.

  \sa loadFromFile()
 */
void Dialog::loadFromMemory()
{
    if (!sharedMemory.attach()) {
        ui.label->setText(tr("The shared memory segment does not exist\n" \
			     "Press \"Load Image From File\""));
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
  Shared Memory} button is pressed. It is also called at the top of
  loadFromFile() to ensure thart the example can be run multiple times
  using the same two Dialog processes.
 */
void Dialog::detach()
{
    if (!sharedMemory.detach())
        ui.label->setText(tr("Unable to detach from shared memory."));
}

