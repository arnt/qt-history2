/****************************************************************************
**
** Definition of QButtonGroup class.
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

#ifndef QBUTTONGROUP_H
#define QBUTTONGROUP_H

#ifndef QT_H
#include "qobject.h"
#endif

class QAbstractButton;
class QAbstractButtonPrivate;
class QButtonGroupPrivate;

class Q_GUI_EXPORT QButtonGroup : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QButtonGroup)
    Q_PROPERTY(bool exlusive READ exclusive WRITE setExclusive)
public:
    QButtonGroup(QObject *parent = 0);
    ~QButtonGroup();

    void setExclusive(bool);
    bool exclusive() const;

    void addButton(QAbstractButton *);
    void removeButton(QAbstractButton *);

    int count() const;

    QAbstractButton * checkedButton() const;
    // no setter on purpose!

signals:
    void buttonChecked(QAbstractButton *);

private:
    friend class QAbstractButton;
    friend class QAbstractButtonPrivate;
};



#endif // QBUTTONGROUP_H
