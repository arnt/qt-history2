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

#ifndef QBUTTONGROUP_H
#define QBUTTONGROUP_H

#include "qobject.h"

class QAbstractButton;
class QAbstractButtonPrivate;
class QButtonGroupPrivate;

class Q_GUI_EXPORT QButtonGroup : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool exclusive READ exclusive WRITE setExclusive)
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

#ifdef QT_COMPAT
public:
    inline QT_COMPAT void insert(QAbstractButton *b) { addButton(b); }
    inline QT_COMPAT void remove(QAbstractButton *b) { removeButton(b); }
#endif

private:
    Q_DISABLE_COPY(QButtonGroup)
    Q_DECLARE_PRIVATE(QButtonGroup)
    friend class QAbstractButton;
    friend class QAbstractButtonPrivate;
};



#endif // QBUTTONGROUP_H
