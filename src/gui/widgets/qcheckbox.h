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
    explicit QCheckBox(QWidget *parent=0);
    explicit QCheckBox(const QString &text, QWidget *parent=0);


    QSize sizeHint() const;

    void setTristate(bool y = true);
    bool isTristate() const;

    Qt::CheckState checkState() const;
    void setCheckState(Qt::CheckState state);

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
    enum ToggleState {
        Off =      Qt::Unchecked,
        NoChange = Qt::PartiallyChecked,
        On =       Qt::Checked
    };
    inline QT_COMPAT ToggleState state() const
        { return static_cast<ToggleState>(checkState()); }
    inline QT_COMPAT void setState(ToggleState state)
        { setCheckState(static_cast<Qt::CheckState>(state)); }
    inline QT_COMPAT void setNoChange()
        { setCheckState(Qt::PartiallyChecked); }
    QT_COMPAT_CONSTRUCTOR QCheckBox(QWidget *parent, const char* name);
    QT_COMPAT_CONSTRUCTOR QCheckBox(const QString &text, QWidget *parent, const char* name);
#endif

private:
    Q_DECLARE_PRIVATE(QCheckBox)
    Q_DISABLE_COPY(QCheckBox)
};

#endif // QT_NO_CHECKBOX

#endif // QCHECKBOX_H
