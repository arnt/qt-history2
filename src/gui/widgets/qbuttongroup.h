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
class Q4ButtonGroupPrivate;

class Q_GUI_EXPORT Q4ButtonGroup : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Q4ButtonGroup);
    Q_PROPERTY(bool exlusive READ exclusive WRITE setExclusive)
public:
    Q4ButtonGroup(QObject *parent = 0);
    ~Q4ButtonGroup();

    void setExclusive(bool);
    bool exclusive() const;

    void addButton(QAbstractButton *);
    void removeButton(QAbstractButton *);

    QAbstractButton * checkedButton() const;
    // no setter on purpose!

signals:
    void buttonChecked(QAbstractButton *);

private:
    friend class QAbstractButton;
    friend class QAbstractButtonPrivate;
};


#ifndef QT_H
#include "qgroupbox.h"
#endif // QT_H

#ifndef QT_NO_BUTTONGROUP

class QButton;
class QButtonMap;

class Q_GUI_EXPORT QButtonGroup : public QGroupBox
{
    Q_OBJECT
    Q_PROPERTY(bool exclusive READ isExclusive WRITE setExclusive)
    Q_PROPERTY(bool radioButtonExclusive READ isRadioButtonExclusive WRITE setRadioButtonExclusive)
    Q_PROPERTY(int selectedId READ selectedId WRITE setButton)

public:
    QButtonGroup(QWidget* parent=0, const char* name=0);
    QButtonGroup(const QString &title,
                  QWidget* parent=0, const char* name=0);
    QButtonGroup(int columns, Orientation o,
                  QWidget* parent=0, const char* name=0);
    QButtonGroup(int columns, Orientation o, const QString &title,
                  QWidget* parent=0, const char* name=0);
    ~QButtonGroup();

    bool        isExclusive() const;
    bool        isRadioButtonExclusive() const { return radio_excl; }
    virtual void setExclusive(bool);
    virtual void setRadioButtonExclusive(bool);

public:
    int                insert(QButton *, int id=-1);
    void        remove(QButton *);
    QButton    *find(int id) const;
    int                id(QButton *) const;
    int                count() const;

    virtual void setButton(int id);

    virtual void moveFocus(int);

    QButton    *selected() const;
    int    selectedId() const;

signals:
    void        pressed(int id);
    void        released(int id);
    void        clicked(int id);

protected slots:
    void        buttonPressed();
    void        buttonReleased();
    void        buttonClicked();
    void        buttonToggled(bool on);

protected:
    bool         event(QEvent * e);

private:
    void        init();

    bool        excl_grp;
    bool        radio_excl;
    QButtonMap *buttons; // ### 4.0: move to d-pointer

private:
#if defined(Q_DISABLE_COPY)
    QButtonGroup(const QButtonGroup &);
    QButtonGroup &operator=(const QButtonGroup &);
#endif
};

#endif // QT_NO_BUTTONGROUP

#endif // QBUTTONGROUP_H
