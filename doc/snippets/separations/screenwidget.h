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

#ifndef SCREENWIDGET_H
#define SCREENWIDGET_H

#include <qcolor.h>
#include <qframe.h>
#include <qimage.h>

class QGridLayout;
class QLabel;
class QMouseEvent;
class QPushButton;
class QWidget;

class ScreenWidget : public QFrame
{
    Q_OBJECT
public:
    enum Separation { Cyan, Magenta, Yellow };

    ScreenWidget(QWidget *parent, QColor initialColor, QString name,
                 Separation mask);
    void setImage(QImage &image);
    QImage* image();

signals:
    void imageChanged();

public slots:
    void setColor();
    void invertImage();

private:
    void createImage();

    Separation maskColor;
    QColor paintColor;
    QImage originalImage;
    QImage newImage;

    QPushButton *invertButton;
    QLabel *imageLabel;
    QLabel *nameLabel;
    QPushButton *colorButton;
    QGridLayout *grid;
    bool inverted;
};

#endif
