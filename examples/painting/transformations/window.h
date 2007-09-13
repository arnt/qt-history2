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

#ifndef WINDOW_H
#define WINDOW_H

#include <QList>
#include <QPainterPath>
#include <QWidget>

#include "renderarea.h"

QT_DECLARE_CLASS(QComboBox)

class Window : public QWidget
{
    Q_OBJECT

public:
    Window();

public slots:
    void operationChanged();
    void shapeSelected(int index);

private:
    void setupShapes();

    enum { NumTransformedAreas = 3 };
    RenderArea *originalRenderArea;
    RenderArea *transformedRenderAreas[NumTransformedAreas];
    QComboBox *shapeComboBox;
    QComboBox *operationComboBoxes[NumTransformedAreas];
    QList<QPainterPath> shapes;
};

#endif
