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
#include "qbutton.h"
#endif // QT_H

#ifndef QT_NO_CHECKBOX

class Q_GUI_EXPORT QCheckBox : public QButton
{
    Q_OBJECT
    Q_PROPERTY(bool checked READ isChecked WRITE setChecked)
    Q_PROPERTY(bool tristate READ isTristate WRITE setTristate)
    Q_OVERRIDE(bool autoMask DESIGNABLE true SCRIPTABLE true)

public:
    QCheckBox(QWidget *parent=0, const char* name=0);
    QCheckBox(const QString &text, QWidget *parent=0, const char* name=0);

    bool    isChecked() const;

    void    setNoChange();

    void    setTristate(bool y=true);
    bool    isTristate() const;

    QSize   sizeHint() const;

public slots:
    void    setChecked(bool check);

protected:
    void    resizeEvent(QResizeEvent*);
    void    drawButton(QPainter *);
    void    drawButtonLabel(QPainter *);
    void    updateMask();
    bool    hitButton(const QPoint &pos) const;
private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QCheckBox(const QCheckBox &);
    QCheckBox &operator=(const QCheckBox &);
#endif
};


inline bool QCheckBox::isChecked() const
{ return isOn(); }

inline void QCheckBox::setChecked(bool check)
{ setOn(check); }


#endif // QT_NO_CHECKBOX

#endif // QCHECKBOX_H
