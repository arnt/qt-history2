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

#ifndef QRADIOBUTTON_H
#define QRADIOBUTTON_H

#include "QtGui/qabstractbutton.h"

#ifndef QT_NO_RADIOBUTTON

class Q_GUI_EXPORT QRadioButton : public QAbstractButton
{
    Q_OBJECT

    Q_OVERRIDE(bool autoExclusive DESIGNABLE true)
    Q_OVERRIDE(bool autoMask DESIGNABLE true SCRIPTABLE true)

public:
    QRadioButton(QWidget *parent=0);
    QRadioButton(const QString &text, QWidget *parent=0);

    QSize sizeHint() const;

protected:
    bool hitButton(const QPoint &) const;
    void paintEvent(QPaintEvent *);
    void updateMask();

#ifdef QT_COMPAT
public:
    QT_COMPAT_CONSTRUCTOR QRadioButton(QWidget *parent, const char* name);
    QT_COMPAT_CONSTRUCTOR QRadioButton(const QString &text, QWidget *parent, const char* name);
#endif

private:
    Q_DISABLE_COPY(QRadioButton)
};

#endif // QT_NO_RADIOBUTTON

#endif // QRADIOBUTTON_H
