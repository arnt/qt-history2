/****************************************************************************
**
** Definition of QAction class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
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

class Q4Menu;
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
    QAction(const QString& text, Q4Menu *menu, QWidget* parent=0);
    QAction(const QString& text, QWidget* parent=0);
    QAction(const QIconSet& icon, const QString& text, QWidget* parent=0);
    QAction(const QString& text, Q4Menu *menu, QActionGroup* parent);
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
    void setMenu(Q4Menu *);
    Q4Menu *menu() const;
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

protected:
    void sendDataChanged();
    virtual void addedTo(QWidget *) {} // can this go away? 

public slots:
    void setChecked(bool);
    void setEnabled(bool);
    inline void setDisabled(bool b) { setEnabled(!b); }
    void setVisible(bool);

private slots:
    void sendAccelActivated(); 

signals:
    void dataChanged();
    void triggered();
    void hovered();

private:
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
     void internalHovered();

private:
#if defined(Q_DISABLE_COPY)  // Disabled copy constructor and operator=
    QActionGroup(const QActionGroup &);
    QActionGroup &operator=(const QActionGroup &);
#endif
};

#endif

#endif
