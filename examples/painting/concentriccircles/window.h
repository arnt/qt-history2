/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>

class QLabel;
class CircleWidget;

class Window : public QWidget
{
    Q_OBJECT

public:
    Window();

private:
    QLabel *createLabel(const QString &text);

    QLabel *aliasedLabel;
    QLabel *antialiasedLabel;
    QLabel *intLabel;
    QLabel *floatLabel;
    CircleWidget *circleWidgets[2][2];
};

#endif
