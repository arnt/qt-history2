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

#ifndef QCHECKBOX_H
#define QCHECKBOX_H

#include "QtGui/qabstractbutton.h"

#ifndef QT_NO_CHECKBOX

class QCheckBoxPrivate;

class Q_GUI_EXPORT QCheckBox : public QAbstractButton
{
    Q_OBJECT

    Q_PROPERTY(bool tristate READ isTristate WRITE setTristate)
    Q_OVERRIDE(bool autoMask DESIGNABLE true SCRIPTABLE true)

public:
    QCheckBox(QWidget *parent=0);
    QCheckBox(const QString &text, QWidget *parent=0);


    QSize sizeHint() const;

    void setTristate(bool y=true);
    bool isTristate() const;

    enum ToggleState { Off, NoChange, On };
    ToggleState state() const;
    void setState(ToggleState state);

signals:
    void stateChanged(int);

protected:
    bool hitButton(const QPoint &pos) const;
    void checkStateSet();
    void nextCheckState();
    void paintEvent(QPaintEvent *);
    void updateMask();

#ifdef QT_COMPAT
public:
    inline QT_COMPAT void setNoChange() { setState(NoChange); }
    QT_COMPAT_CONSTRUCTOR QCheckBox(QWidget *parent, const char* name);
    QT_COMPAT_CONSTRUCTOR QCheckBox(const QString &text, QWidget *parent, const char* name);
#endif

private:
    Q_DECLARE_PRIVATE(QCheckBox)
    Q_DISABLE_COPY(QCheckBox)
};

#endif // QT_NO_CHECKBOX

#endif // QCHECKBOX_H
