/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef SVGWINDOW_H
#define SVGWINDOW_H

#include <QPoint>
#include <QScrollArea>
#include <QString>

class QKeyEvent;
class QMouseEvent;

class SvgWindow : public QScrollArea
{
public:
    enum RendererType { Native, OpenGL, Image };

    SvgWindow();
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void openFile(const QString &file);
    void setRenderer(RendererType type = Native);
    void setFastAntialiasing(bool fastAntialiasing);

private:
    QPoint mousePressPos;
    QPoint scrollBarValuesOnMousePress;
    QString currentPath;
    RendererType renderer;

    bool fastAntialiasing;
};

#endif
