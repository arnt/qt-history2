/****************************************************************************
**
** Definition of QCheckBox class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QCHECKBOX_H
#define QCHECKBOX_H

#ifndef QT_H
#include "qabstractbutton.h"
#endif // QT_H

#ifndef QT_NO_CHECKBOX

class QCheckBoxPrivate;

class Q_GUI_EXPORT QCheckBox : public QAbstractButton
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QCheckBox);
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
    void drawBevel(QPainter *);
    void drawLabel(QPainter *);
    void paintEvent(QPaintEvent *);
    void updateMask();

#ifdef QT_COMPAT
public:
    inline QT_COMPAT void setNoChange() { setState(NoChange); }
    QCheckBox(QWidget *parent, const char* name);
    QCheckBox(const QString &text, QWidget *parent, const char* name);

#endif
private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QCheckBox(const QCheckBox &);
    QCheckBox &operator=(const QCheckBox &);
#endif
};

#endif // QT_NO_CHECKBOX

#endif // QCHECKBOX_H
