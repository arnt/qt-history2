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

#ifndef ABSTRACTFORMWINDOWCURSOR_H
#define ABSTRACTFORMWINDOWCURSOR_H

#include "sdk_global.h"

class AbstractFormWindow;
class QWidget;
class QVariant;
class QString;

class QT_SDK_EXPORT AbstractFormWindowCursor
{
public:
    enum MoveOperation
    {
        NoMove,

        Start,
        End,
        Next,
        Prev,
        Left,
        Right,
        Up,
        Down
    };

    enum MoveMode
    {
        MoveAnchor,
        KeepAnchor
    };

public:
    virtual ~AbstractFormWindowCursor() {}

    virtual AbstractFormWindow *formWindow() const = 0;

    virtual bool movePosition(MoveOperation op, MoveMode mode = MoveAnchor) = 0;

    virtual int position() const = 0;
    virtual void setPosition(int pos, MoveMode mode = MoveAnchor) = 0;

    virtual QWidget *current() const = 0;

    virtual int widgetCount() const = 0;
    virtual QWidget *widget(int index) const = 0;

    virtual bool hasSelection() const = 0;
    virtual int selectedWidgetCount() const = 0;
    virtual QWidget *selectedWidget(int index) const = 0;

    virtual void setProperty(const QString &name, const QVariant &value) = 0;
    virtual void setWidgetProperty(QWidget *widget, const QString &name, const QVariant &value) = 0;

    bool isWidgetSelected(QWidget *widget) const;
};

#endif // ABSTRACTFORMWINDOWCURSOR_H
