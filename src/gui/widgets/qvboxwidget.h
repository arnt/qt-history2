/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QVBOXWIDGET_H
#define QVBOXWIDGET_H

#include "QtGui/qhboxwidget.h"

#ifndef QT_NO_VBOXWIDGET

class Q_GUI_EXPORT QVBoxWidget : public QHBoxWidget
{
    Q_OBJECT
public:
    explicit QVBoxWidget(QWidget* parent=0, Qt::WFlags f=0);

#ifdef QT3_SUPPORT
public:
    QT3_SUPPORT_CONSTRUCTOR QVBoxWidget(QWidget* parent, const char* name, Qt::WFlags f=0);
#endif

private:
    Q_DISABLE_COPY(QVBoxWidget)
};

#endif // QT_NO_VBOX

#endif // QVBOXWIDGET_H
