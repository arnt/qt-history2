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

#ifndef VIEWER_H
#define VIEWER_H

#include <qimage.h>
#include <qmainwindow.h>

class ScreenWidget;
class QFrame;
class QGridLayout;
class QLabel;
class QPopupMenu;

class Viewer : public QMainWindow
{
    Q_OBJECT
public:
    Viewer();

public slots:
    void chooseFile();
    void setBrightness(int amount);
    void createImage();
    void saveImage();

private:
    void createMenus();
    QFrame *createCentralWidget();
    void openImageFile(QString &filePath);

    QGridLayout *grid;
    QImage scaledImage;
    QLabel *finalWidget;
    ScreenWidget *cyanWidget;
    ScreenWidget *magentaWidget;
    ScreenWidget *yellowWidget;
    QPopupMenu *fileMenu;
    QPopupMenu *brightnessMenu;

    int brightnessMenuId;
    int saveItemId;
    int brightness;
    int currentBrightness;
    enum { None, Quarter, Half, ThreeQuarters, Full };
};

#endif
