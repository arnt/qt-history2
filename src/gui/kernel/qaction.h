/****************************************************************************
**
** Definition of QAction class.
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

#ifndef QACTION_H
#define QACTION_H

#ifndef QT_H
#include "qwidget.h"
#include "qiconset.h"
#include "qstring.h"
#include "qkeysequence.h"
#endif // QT_H

#ifndef QT_NO_ACTION

class QMenu;
class QActionGroup;
class QActionPrivate;
class QActionGroupPrivate;

class Q_GUI_EXPORT QAction : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QAction);

    Q_PROPERTY(bool checked READ isChecked WRITE setChecked)
    Q_PROPERTY(bool checkable READ isCheckable WRITE setCheckable)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled)
    Q_PROPERTY(QIconSet icon READ icon WRITE setIcon)
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QString toolTip READ toolTip WRITE setToolTip)
    Q_PROPERTY(QString statusTip READ statusTip WRITE setStatusTip)
    Q_PROPERTY(QString whatsThis READ whatsThis WRITE setWhatsThis)
#ifndef QT_NO_ACCEL
    Q_PROPERTY(QKeySequence accel READ accel WRITE setAccel)
#endif
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible)

public:
    QAction(QActionGroup* parent);
    QAction(QWidget* parent=0);
    QAction(const QString& text, QMenu *menu, QWidget* parent=0);
    QAction(const QString& text, QWidget* parent=0);
    QAction(const QIconSet& icon, const QString& text, QWidget* parent=0);
    QAction(const QString& text, QMenu *menu, QActionGroup* parent);
    QAction(const QString& text, QActionGroup* parent);
    QAction(const QIconSet& icon, const QString& text, QActionGroup* parent);
#ifndef QT_NO_ACCEL
    QAction(const QString& text, QKeySequence accel, QWidget* parent=0);
    QAction(const QIconSet& icon, const QString& text, QKeySequence accel,
              QWidget* parent=0);
    QAction(const QString& text, QKeySequence accel, QActionGroup* parent);
    QAction(const QIconSet& icon, const QString& text, QKeySequence accel,
              QActionGroup* parent);
#endif
    ~QAction();

    void setActionGroup(QActionGroup *group);
    QActionGroup *actionGroup() const;
    void setIcon(const QIconSet&);
    QIconSet icon() const;
    void setText(const QString&);
    QString text() const;
    void setToolTip(const QString&);
    QString toolTip() const;
    void setStatusTip(const QString&);
    QString statusTip() const;
    void setWhatsThis(const QString&);
    QString whatsThis() const;
    void setMenu(QMenu *);
    QMenu *menu() const;
    void setSeparator(bool b);
    bool isSeparator() const;
#ifndef QT_NO_ACCEL
    void setAccel(const QKeySequence& key);
    QKeySequence accel() const;
#endif
    void setCheckable(bool);
    bool isCheckable() const;
    bool isChecked() const;
    bool isEnabled() const;
    bool isVisible() const;

    enum ActionEvent { Trigger, Hover };
    void activate(ActionEvent event);

#ifdef QT_COMPAT
    inline QT_COMPAT bool isOn() const { return isChecked(); }
    inline QT_COMPAT void setOn(bool b) { setChecked(b); }
    inline QT_COMPAT bool isToggleAction() const { return isCheckable(); }
    inline QT_COMPAT void setToggleAction(bool b) { setCheckable(b); }
    inline QT_COMPAT void setIconSet(const QIconSet &i) { setIcon(i); }
    inline QT_COMPAT QIconSet iconSet() const { return icon(); }
    inline QT_COMPAT bool addTo(QWidget *w) { w->addAction(this); return true; }
    inline QT_COMPAT bool removeFrom(QWidget *w) { w->removeAction(this); return true; }
#endif

protected:
    virtual void addedTo(QWidget *) {};

public slots:
    void setChecked(bool);
    void setEnabled(bool);
    inline void setDisabled(bool b) { setEnabled(!b); }
    void setVisible(bool);

signals:
    void dataChanged();
    void triggered();
    void hovered();

private slots:
    void sendAccelActivated();

private:
    friend class QWidget;
    friend class QActionGroup;
#if defined(Q_DISABLE_COPY)  // Disabled copy constructor and operator=
    QAction(const QAction &);
    QAction &operator=(const QAction &);
#endif
};

class Q_GUI_EXPORT QActionGroup : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QActionGroup);

    Q_PROPERTY(bool exclusive READ isExclusive WRITE setExclusive)

public:
    QActionGroup(QObject* parent);
    ~QActionGroup();

    QAction *addAction(QAction* a);
#ifndef QT_NO_ACCEL
    QAction *addAction(const QString& text, QKeySequence accel);
    QAction *addAction(const QIconSet& icon, const QString& text, QKeySequence accel);
#endif
    void removeAction(QAction *a);
    QList<QAction*> actionList() const;

    QAction *checked() const;
    bool isExclusive() const;
    bool isEnabled() const;
    bool isVisible() const;

#ifdef QT_COMPAT
    inline QT_COMPAT void add(QAction* a) { addAction(a); }
    inline QT_COMPAT void addSeparator() { QAction *act = new QAction(this); act->setSeparator(true); addAction(act); }
    inline QT_COMPAT bool addTo(QWidget *w) {
        QList<QAction*> acts = actionList();
        for(int i = 0; i < acts.size(); i++)
            w->addAction(acts.at(i));
        return true;
    }
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
    void hovered(QAction *);

private slots:
    void internalTriggered();
    void internalDataChanged();
    void internalHovered();

private:
#if defined(Q_DISABLE_COPY)  // Disabled copy constructor and operator=
    QActionGroup(const QActionGroup &);
    QActionGroup &operator=(const QActionGroup &);
#endif
};

#endif

#endif
