/****************************************************************************
**
** Definition of Q3Action class.
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

#ifndef Q3ACTION_H
#define Q3ACTION_H

#ifndef QT_H
#include "qobject.h"
#include "qiconset.h"
#include "qstring.h"
#include "qkeysequence.h"
#endif // QT_H

#ifndef QT_NO_ACTION

class Q3ActionPrivate;
class Q3ActionGroupPrivate;
class QStatusBar;
class QPopupMenu;
class QToolTipGroup;
class QWidget;

class Q_COMPAT_EXPORT Q3Action : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool toggleAction READ isToggleAction WRITE setToggleAction)
    Q_PROPERTY(bool on READ isOn WRITE setOn)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled)
    Q_PROPERTY(QIconSet iconSet READ iconSet WRITE setIconSet)
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QString menuText READ menuText WRITE setMenuText)
    Q_PROPERTY(QString toolTip READ toolTip WRITE setToolTip)
    Q_PROPERTY(QString statusTip READ statusTip WRITE setStatusTip)
    Q_PROPERTY(QString whatsThis READ whatsThis WRITE setWhatsThis)
#ifndef QT_NO_ACCEL
    Q_PROPERTY(QKeySequence accel READ accel WRITE setAccel)
#endif
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible)

public:
    Q3Action(QObject* parent, const char* name = 0);
#ifndef QT_NO_ACCEL
    Q3Action(const QString& menuText, QKeySequence accel,
             QObject* parent, const char* name = 0);
    Q3Action(const QIconSet& icon, const QString& menuText, QKeySequence accel,
             QObject* parent, const char* name = 0);

    Q3Action(const QString& text, const QIconSet& icon, const QString& menuText, QKeySequence accel,
             QObject* parent, const char* name = 0, bool toggle = false); // obsolete
    Q3Action(const QString& text, const QString& menuText, QKeySequence accel, QObject* parent,
             const char* name = 0, bool toggle = false); // obsolete
#endif
    Q3Action(QObject* parent, const char* name , bool toggle); // obsolete
    ~Q3Action();

    virtual void setIconSet(const QIconSet&);
    QIconSet iconSet() const;
    virtual void setText(const QString&);
    QString text() const;
    virtual void setMenuText(const QString&);
    QString menuText() const;
    virtual void setToolTip(const QString&);
    QString toolTip() const;
    virtual void setStatusTip(const QString&);
    QString statusTip() const;
    virtual void setWhatsThis(const QString&);
    QString whatsThis() const;
#ifndef QT_NO_ACCEL
    virtual void setAccel(const QKeySequence& key);
    QKeySequence accel() const;
#endif
    virtual void setToggleAction(bool);

    bool isToggleAction() const;
    bool isOn() const;
    bool isEnabled() const;
    bool isVisible() const;
    virtual bool addTo(QWidget*);
    virtual bool removeFrom(QWidget*);

protected:
    virtual void addedTo(QWidget *actionWidget, QWidget *container);
    virtual void addedTo(int index, QPopupMenu *menu);

public slots:
    void activate();
    void toggle();
    virtual void setOn(bool);
    virtual void setEnabled(bool);
    void setDisabled(bool);
    virtual void setVisible(bool);

signals:
    void activated();
    void toggled(bool);

private slots:
    void internalActivation();
    void toolButtonToggled(bool);
    void objectDestroyed();
    void menuStatusText(int id);
    void showStatusText(const QString&);
    void clearStatusText();

private:
    void init();

    friend class Q3ActionPrivate;
    friend class Q3ActionGroup;
    friend class Q3ActionGroupPrivate;
    Q3ActionPrivate* d;

#if defined(Q_DISABLE_COPY)  // Disabled copy constructor and operator=
    Q3Action(const Q3Action &);
    Q3Action &operator=(const Q3Action &);
#endif
};

class Q_COMPAT_EXPORT Q3ActionGroup : public Q3Action
{
    Q_OBJECT
    Q_PROPERTY(bool exclusive READ isExclusive WRITE setExclusive)
    Q_PROPERTY(bool usesDropDown READ usesDropDown WRITE setUsesDropDown)

public:
    Q3ActionGroup(QObject* parent, const char* name = 0);
    Q3ActionGroup(QObject* parent, const char* name , bool exclusive ); // obsolete
    ~Q3ActionGroup();
    void setExclusive(bool);
    bool isExclusive() const;
    void add(Q3Action* a);
    void addSeparator();
    bool addTo(QWidget*);
    bool removeFrom(QWidget*);
    void setEnabled(bool);
    void setToggleAction(bool toggle);
    void setOn(bool on);
    void setVisible(bool);

    void setUsesDropDown(bool enable);
    bool usesDropDown() const;

    void setIconSet(const QIconSet&);
    void setText(const QString&);
    void setMenuText(const QString&);
    void setToolTip(const QString&);
    void setWhatsThis(const QString&);

protected:
    void childEvent(QChildEvent*);
    virtual void addedTo(QWidget *actionWidget, QWidget *container, Q3Action *a);
    virtual void addedTo(int index, QPopupMenu *menu, Q3Action *a);
    virtual void addedTo(QWidget *actionWidget, QWidget *container);
    virtual void addedTo(int index, QPopupMenu *menu);

signals:
    void selected(Q3Action*);
    void activated(Q3Action *);

private slots:
    void childToggled(bool);
    void childActivated();
    void childDestroyed();
    void internalComboBoxActivated(int);
    void internalComboBoxHighlighted(int);
    void internalToggle(Q3Action*);
    void objectDestroyed();

private:
    Q3ActionGroupPrivate* d;

#ifdef QT_COMPAT
public:
    QT_COMPAT void insert(Q3Action* a) { add(a); }
#endif

private:
#if defined(Q_DISABLE_COPY)  // Disabled copy constructor and operator=
    Q3ActionGroup(const Q3ActionGroup &);
    Q3ActionGroup &operator=(const Q3ActionGroup &);
#endif
};

#endif

#endif
