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

#ifndef Q3BUTTONGROUP_H
#define Q3BUTTONGROUP_H

#ifndef QT_H
#include "qbuttongroup.h"
#include "q3groupbox.h"
#include "qmap.h"
#endif // QT_H


class QAbstractButton;

class Q_COMPAT_EXPORT Q3ButtonGroup : public Q3GroupBox
{
    Q_OBJECT
    Q_PROPERTY(bool exclusive READ isExclusive WRITE setExclusive)
    Q_PROPERTY(bool radioButtonExclusive READ isRadioButtonExclusive WRITE setRadioButtonExclusive)
    Q_PROPERTY(int selectedId READ selectedId WRITE setButton)

public:
    Q3ButtonGroup(QWidget* parent=0, const char* name=0);
    Q3ButtonGroup(const QString &title,
                  QWidget* parent=0, const char* name=0);
    Q3ButtonGroup(int columns, Qt::Orientation o,
                  QWidget* parent=0, const char* name=0);
    Q3ButtonGroup(int columns, Qt::Orientation o, const QString &title,
                  QWidget* parent=0, const char* name=0);
    ~Q3ButtonGroup();

    bool isExclusive() const;
    bool isRadioButtonExclusive() const { return radio_excl; }
    void setExclusive(bool);
    void setRadioButtonExclusive(bool);

public:
    int insert(QAbstractButton *, int id=-1);
    void remove(QAbstractButton *);
    QAbstractButton    *find(int id) const;
    int id(QAbstractButton *) const;
    int count() const;

    void setButton(int id);

    QAbstractButton    *selected() const;
    int selectedId() const;

signals:
    void pressed(int id);
    void released(int id);
    void clicked(int id);

protected slots:
    void buttonPressed();
    void buttonReleased();
    void buttonClicked();

protected:
    bool event(QEvent * e);

private:
    void init();

    bool excl_grp;
    bool radio_excl;
    QMap<int, QAbstractButton*> buttonIds;
    QButtonGroup group;

private:
#if defined(Q_DISABLE_COPY)
    Q3ButtonGroup(const Q3ButtonGroup &);
    Q3ButtonGroup &operator=(const Q3ButtonGroup &);
#endif
};

class Q_COMPAT_EXPORT QVButtonGroup : public Q3ButtonGroup
{
    Q_OBJECT
public:
    inline QVButtonGroup(QWidget* parent=0, const char* name=0)
        : Q3ButtonGroup(1, Qt::Horizontal /* sic! */, parent, name) {}
    inline QVButtonGroup(const QString &title, QWidget* parent=0, const char* name=0)
        : Q3ButtonGroup(1, Qt::Horizontal /* sic! */, title, parent, name) {}

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QVButtonGroup(const QVButtonGroup &);
    QVButtonGroup &operator=(const QVButtonGroup &);
#endif
};


class Q_COMPAT_EXPORT QHButtonGroup : public Q3ButtonGroup
{
    Q_OBJECT
public:
    inline QHButtonGroup(QWidget* parent=0, const char* name=0)
        : Q3ButtonGroup(1, Qt::Vertical /* sic! */, parent, name) {}
    inline QHButtonGroup(const QString &title, QWidget* parent=0, const char* name=0)
        : Q3ButtonGroup(1, Qt::Vertical /* sic! */, title, parent, name) {}

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QHButtonGroup(const QHButtonGroup &);
    QHButtonGroup &operator=(const QHButtonGroup &);
#endif
};


#endif // Q3BUTTONGROUP_H
