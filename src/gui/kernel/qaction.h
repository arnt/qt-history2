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

#ifndef QACTION_H
#define QACTION_H

#ifndef QT_H
#include "qwidget.h"
#include "qiconset.h"
#include "qstring.h"
#include "qkeysequence.h"
#endif // QT_H

class QMenu;
class QActionGroup;
class QActionPrivate;

class Q_GUI_EXPORT QAction : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QAction)

    Q_PROPERTY(bool checked READ isChecked WRITE setChecked)
    Q_PROPERTY(bool checkable READ isCheckable WRITE setCheckable)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled)
    Q_PROPERTY(QIconSet icon READ icon WRITE setIcon)
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QString iconText READ iconText WRITE setIconText)
    Q_PROPERTY(QString toolTip READ toolTip WRITE setToolTip)
    Q_PROPERTY(QString statusTip READ statusTip WRITE setStatusTip)
    Q_PROPERTY(QString whatsThis READ whatsThis WRITE setWhatsThis)
    Q_PROPERTY(QKeySequence shortcut READ shortcut WRITE setShortcut)
    Q_PROPERTY(QFont font READ font WRITE setFont)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible)

public:
    QAction(QObject* parent = 0);
    QAction(const QString &text, QObject* parent=0);
    QAction(const QIconSet &icon, const QString &text, QObject* parent=0);

#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QAction(QObject* parent, const char* name);
    QT_COMPAT_CONSTRUCTOR QAction(const QString &text, const QKeySequence &shortcut, QObject* parent, const char* name);
    QT_COMPAT_CONSTRUCTOR QAction(const QIconSet &icon, const QString &text, const QKeySequence &shortcut,
                                  QObject* parent, const char* name);
#endif
    ~QAction();

    void setActionGroup(QActionGroup *group);
    QActionGroup *actionGroup() const;
    void setIcon(const QIconSet&icon);
    QIconSet icon() const;

    void setText(const QString &text);
    QString text() const;

    void setIconText(const QString &text);
    QString iconText() const;

    void setToolTip(const QString &tip);
    QString toolTip() const;

    void setStatusTip(const QString &statusTip);
    QString statusTip() const;

    void setWhatsThis(const QString &what);
    QString whatsThis() const;

    QMenu *menu() const;

    void setSeparator(bool b);
    bool isSeparator() const;

    void setShortcut(const QKeySequence &shortcut);
    QKeySequence shortcut() const;

    void setFont(const QFont &font);
    QFont font() const;

    void setCheckable(bool);
    bool isCheckable() const;

    bool isChecked() const;

    bool isEnabled() const;

    bool isVisible() const;

    enum ActionEvent { Trigger, Hover };
    void activate(ActionEvent event);
    bool showStatusText(QWidget *widget=0);

#ifdef QT_COMPAT
    inline QT_COMPAT void setMenuText(const QString &text) { setText(text); }
    inline QT_COMPAT QString menuText() const { return text(); }
    inline QT_COMPAT bool isOn() const { return isChecked(); }
    inline QT_COMPAT void setOn(bool b) { setChecked(b); }
    inline QT_COMPAT bool isToggleAction() const { return isCheckable(); }
    inline QT_COMPAT void setToggleAction(bool b) { setCheckable(b); }
    inline QT_COMPAT void setIconSet(const QIconSet &i) { setIcon(i); }
    inline QT_COMPAT QIconSet iconSet() const { return icon(); }
    inline QT_COMPAT bool addTo(QWidget *w) { w->addAction(this); return true; }
    inline QT_COMPAT bool removeFrom(QWidget *w) { w->removeAction(this); return true; }
    inline QT_COMPAT void setAccel(const QKeySequence &shortcut) { setShortcut(shortcut); }
    inline QT_COMPAT QKeySequence accel() const { return shortcut(); }
#endif

    QWidget *parentWidget() const;

protected:
    bool event(QEvent *);

public slots:
    inline void toggle() { setChecked(!isChecked()); }
    void trigger() { activate(Trigger); }
    void hover() { activate(Hover); }
    void setChecked(bool);
    void setEnabled(bool);
    inline void setDisabled(bool b) { setEnabled(!b); }
    void setVisible(bool);

signals:
    void changed();
    void triggered();
    void hovered();
    void checked(bool);
#ifdef QT_COMPAT
    QT_MOC_COMPAT void toggled(bool);
    QT_MOC_COMPAT void activated();
#endif

private:
#ifdef QT_COMPAT
    friend class QMenuItem;
#endif
    friend class QWidget;
    friend class QActionGroup;
    friend class QMenu;
    friend class QMenuBar;
    friend class QShortcutMap;
#if defined(Q_DISABLE_COPY)  // Disabled copy constructor and operator=
    QAction(const QAction &);
    QAction &operator=(const QAction &);
#endif
};

#include "qactiongroup.h" // ### remove after tech-preview 1
#endif
