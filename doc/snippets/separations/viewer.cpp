/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
viewer.cpp

Provides a main window for displaying a user-specified original image
with three color separations in a grid layout.

A main menu provides entries for selecting files, and adjusting the
brightness of the separations.
*/

#include <qapplication.h>
#include <qcolor.h>
#include <qdir.h>
#include <qdeepcopy.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qframe.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlist.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qstringlist.h>

#include "screenwidget.h"
#include "viewer.h"

/*!
    Constructor: initializes a default value for the brightness, creates
    the main menu entries, and constructs a central widget that contains
    enough space for images to be displayed.
*/

Viewer::Viewer() : QMainWindow()
{
    setWindowTitle(tr("QImage Color Separations"));

    brightness = 255;

    createMenus();
    setCentralWidget(createCentralWidget());
}

/*!
    Creates a main menu with two entries: a File menu, to allow the image
    to be selected, and a Brightness menu to allow the brightness of the
    separations to be changed.

    Initially, the Brightness menu items are disabled, but the first entry in
    the menu is checked to reflect the default brightness.
*/

void Viewer::createMenus()
{
    fileMenu = new QPopupMenu(this);
    fileMenu->insertItem(tr("&Open"), this, SLOT(chooseFile()), tr("Ctrl+O"));
    fileMenu->insertItem(tr("&Save"), this, SLOT(saveImage()), tr("Ctrl+S"));
    fileMenu->insertItem(tr("E&xit"), qApp, SLOT(quit()), tr("Ctrl+Q"));
    fileMenu->setItemEnabled(saveItemId, false);

    QMenuBar *menu = menuBar();
    menu->insertItem(tr("&File"), fileMenu);

    brightnessMenu = new QPopupMenu(this);
    brightnessMenu->insertItem(tr("&0%"), None, 0);
    brightnessMenu->insertItem(tr("&25%"), Quarter, 1);
    brightnessMenu->insertItem(tr("&50%"), Half, 2);
    brightnessMenu->insertItem(tr("&75%"), ThreeQuarters, 3);
    brightnessMenu->insertItem(tr("&100%"), Full, 4);

    currentBrightness = Full;
    brightnessMenu->setItemChecked(currentBrightness, true);

    connect(brightnessMenu, SIGNAL(activated(int)), this,
            SLOT(setBrightness(int)));
    brightnessMenuId = menu->insertItem(tr("&Brightness"), brightnessMenu);
    menuBar()->setItemEnabled(brightnessMenuId, false);
}

/*!
    Constructs a central widget for the window consisting of a two-by-two
    grid of labels, each of which will contain an image. We restrict the
    size of the labels to 256 pixels, and ensure that the window cannot
    be resized.
*/

QFrame* Viewer::createCentralWidget()
{
    QFrame* frame = new QFrame(this);
    grid = new QGridLayout(frame, 2, 2);
    grid->setSpacing(8);
    grid->setMargin(4);

    layout()->setResizeMode(QLayout::Fixed);

    QSize labelSize(256, 256);

    finalWidget = new QLabel(frame);
    finalWidget->setMinimumSize(labelSize);
    cyanWidget = new ScreenWidget(frame, Qt::cyan, tr("Cyan"), ScreenWidget::Cyan);
    cyanWidget->setMinimumSize(labelSize);
    magentaWidget = new ScreenWidget(frame, Qt::magenta, tr("Magenta"),
                              ScreenWidget::Magenta);
    magentaWidget->setMinimumSize(labelSize);
    yellowWidget = new ScreenWidget(frame, Qt::yellow, tr("Yellow"),
                              ScreenWidget::Yellow);
    yellowWidget->setMinimumSize(labelSize);

    connect(cyanWidget, SIGNAL(imageChanged()), this, SLOT(createImage()));
    connect(magentaWidget, SIGNAL(imageChanged()), this, SLOT(createImage()));
    connect(yellowWidget, SIGNAL(imageChanged()), this, SLOT(createImage()));

    grid->addWidget(finalWidget, 0, 0, Qt::AlignTop | Qt::AlignHCenter);
    grid->addWidget(cyanWidget, 0, 1, Qt::AlignTop | Qt::AlignHCenter);
    grid->addWidget(magentaWidget, 1, 0, Qt::AlignTop | Qt::AlignHCenter);
    grid->addWidget(yellowWidget, 1, 1, Qt::AlignTop | Qt::AlignHCenter);

    return frame;
}

/*!
    Provides a dialog window to allow the user to specify an image file.
    If a file is selected, the appropriate function is called to process
    and display it.
*/

void Viewer::chooseFile()
{
    QString imageFile = QFileDialog::getOpenFileName("", tr("Images (*.*)"),
        this, "locate images", tr("Choose an image file to open"));

    if (!imageFile.isEmpty())
        openImageFile(imageFile);
}

/*!
    Changes the value of the brightness according to the entry selected in the
    Brightness menu. The selected entry is checked, and the previously selected
    entry is unchecked.

    The color separations are updated to use the new value for the brightness.
*/

void Viewer::setBrightness(int amount)
{
    switch (amount) {
        case None:
            brightness = 0; break;
        case Quarter:
            brightness = 64; break;
        case Half:
            brightness = 128; break;
        case ThreeQuarters:
            brightness = 191; break;
        case Full:
            brightness = 255; break;
        default: return;
    }

    brightnessMenu->setItemChecked(currentBrightness, false);
    brightnessMenu->setItemChecked(amount, true);
    currentBrightness = amount;

    createImage();
}

/*!
    Load the image from the file given, and create four pixmaps based
    on the original image.

    The window caption is set, and the Brightness menu enabled if the image file
    can be loaded.
*/

void Viewer::openImageFile(QString &imageFile)
{
    QImage originalImage;

    if (originalImage.load(imageFile)) {
        setCaption(imageFile);
        menuBar()->setItemEnabled(brightnessMenuId, true);
        fileMenu->setItemEnabled(saveItemId, true);

        /* Note: the ScaleMin value may be different for Qt 4. */
        scaledImage = originalImage.convertDepth(32).scale(256, 256, QImage::ScaleFree);

        cyanWidget->setImage(scaledImage);
        magentaWidget->setImage(scaledImage);
        yellowWidget->setImage(scaledImage);
        createImage();
    }
    else
        (void) QMessageBox::warning(this, tr("Cannot open file"),
            tr("The selected file could not be opened."),
            QMessageBox::Cancel, QMessageBox::NoButton, QMessageBox::NoButton);
}

/*!
    Creates an image by combining the contents of the three screens
    to present a page preview.

    The image associated with each screen is separated into cyan,
    magenta, and yellow components. We add up the values for each
    component from the three screen images, and subtract the totals
    from the maximum value for each corresponding primary color.
*/

void Viewer::createImage()
{
    QImage newImage = scaledImage.copy();

    QImage *image1 = cyanWidget->image();
    QImage *image2 = magentaWidget->image();
    QImage *image3 = yellowWidget->image();
    int darkness = 255 - brightness;

    for (int y = 0; y < newImage.height(); ++y) {
        for (int x = 0; x < newImage.width(); ++x) {

            // Create three screens, using the quantities of the source
            // CMY components to determine how much of each of the
            // inks are to be put on each screen.
            QRgb p1(image1->pixel(x, y));
            float cyan1 = 255 - qRed(p1);
            float magenta1 = 255 - qGreen(p1);
            float yellow1 = 255 - qBlue(p1);

            QRgb p2(image2->pixel(x, y));
            float cyan2 = 255 - qRed(p2);
            float magenta2 = 255 - qGreen(p2);
            float yellow2 = 255 - qBlue(p2);

            QRgb p3(image3->pixel(x, y));
            float cyan3 = 255 - qRed(p3);
            float magenta3 = 255 - qGreen(p3);
            float yellow3 = 255 - qBlue(p3);

            QColor newColor(
                QMAX(255 - int(cyan1+cyan2+cyan3) - darkness, 0),
                QMAX(255 - int(magenta1+magenta2+magenta3) - darkness, 0),
                QMAX(255 - int(yellow1+yellow2+yellow3) - darkness, 0));

            newImage.setPixel(x, y, newColor.pixel());
        }
    }

    finalWidget->setPixmap(QPixmap(newImage));
}

/*!
    Provides a dialog window to allow the user to save the image file.
*/

void Viewer::saveImage()
{
    QString imageFile = QFileDialog::getSaveFileName("", tr("Images (*.png)"),
        this, "save image", tr("Choose a filename to save the image"));

    QFileInfo info(imageFile);

    if (!info.baseName(false).isEmpty()) {
            QString newImageFile = info.dirPath(true) + "/" +
                info.baseName(false) + ".png";

        if (!finalWidget->pixmap()->save(newImageFile, "PNG"))
            (void) QMessageBox::warning(this, tr("Cannot save file"),
                tr("The file could not be saved."),
                QMessageBox::Cancel, QMessageBox::NoButton,
                QMessageBox::NoButton);
    }
    else
        (void) QMessageBox::warning(this, tr("Cannot save file"),
            tr("Please enter a valid filename."),
            QMessageBox::Cancel, QMessageBox::NoButton,
            QMessageBox::NoButton);
}
