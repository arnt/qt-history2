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

#ifndef QACTIONGROUP_H
#define QACTIONGROUP_H
#include "QtGui/qaction.h"

class QActionGroupPrivate;

class Q_GUI_EXPORT QActionGroup : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QActionGroup)

    Q_PROPERTY(bool exclusive READ isExclusive WRITE setExclusive)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible)

public:
    explicit QActionGroup(QObject* parent);
    ~QActionGroup();

    QAction *addAction(QAction* a);
    QAction *addAction(const QString &text);
    QAction *addAction(const QIcon &icon, const QString &text);
    void removeAction(QAction *a);
    QList<QAction*> actions() const;

    QAction *checkedAction() const;
    bool isExclusive() const;
    bool isEnabled() const;
    bool isVisible() const;

#ifdef QT_COMPAT
    inline QT_COMPAT void add(QAction* a) { addAction(a); }
    inline QT_COMPAT void addSeparator()
    { QAction *act = new QAction(this); act->setSeparator(true); addAction(act); }
    inline QT_COMPAT bool addTo(QWidget *w) { w->addActions(actions()); return true; }
#endif

public slots:
    void setEnabled(bool);
    inline void setDisabled(bool b) { setEnabled(!b); }
    void setVisible(bool);
    void setExclusive(bool);

protected:
    void childEvent(QChildEvent*);

signals:
    void triggered(QAction *);
    QT_MOC_COMPAT void selected(QAction *);
    void hovered(QAction *);

private:
    Q_DISABLE_COPY(QActionGroup)
    Q_PRIVATE_SLOT(d, void actionTriggered())
    Q_PRIVATE_SLOT(d, void actionChanged())
    Q_PRIVATE_SLOT(d, void actionHovered())
};

#endif
