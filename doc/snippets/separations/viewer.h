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

#include <QImage>
#include <QMainWindow>
#include <qmap.h>

class QAction;
class QFrame;
class QGridLayout;
class QLabel;
class QMenu;
class ScreenWidget;

class Viewer : public QMainWindow
{
    Q_OBJECT
public:
    enum Brightness { None, Quarter, Half, ThreeQuarters, Full };
    Viewer();

public slots:
    void chooseFile();
    void setBrightness(QAction *action);
    void createImage();
    void saveImage();

private:
    void createMenus();
    QFrame *createCentralWidget();
    void openImageFile(QString &filePath);

    QAction *saveAction;
    QGridLayout *grid;
    QImage scaledImage;
    QLabel *finalWidget;
    QMenu *brightnessMenu;
    QMenu *fileMenu;
    ScreenWidget *cyanWidget;
    ScreenWidget *magentaWidget;
    ScreenWidget *yellowWidget;

    int brightness;
    QAction *currentBrightness;
    QMap <QAction*,Brightness> menuMap;
};

#endif
