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

#include "QtGui/qicon.h"
#include "QtGui/qkeysequence.h"
#include "QtCore/qstring.h"
#include "QtGui/qwidget.h"

class QMenu;
class QActionGroup;
class QActionPrivate;

class Q_GUI_EXPORT QAction : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QAction)

    Q_PROPERTY(bool checkable READ isCheckable WRITE setCheckable)
    Q_PROPERTY(bool checked READ isChecked WRITE setChecked)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled)
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon)
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QString iconText READ iconText WRITE setIconText)
    Q_PROPERTY(QString toolTip READ toolTip WRITE setToolTip)
    Q_PROPERTY(QString statusTip READ statusTip WRITE setStatusTip)
    Q_PROPERTY(QString whatsThis READ whatsThis WRITE setWhatsThis)
    Q_PROPERTY(QKeySequence shortcut READ shortcut WRITE setShortcut)
    Q_PROPERTY(QFont font READ font WRITE setFont)
    Q_PROPERTY(Qt::ShortcutContext shortcutContext READ shortcutContext WRITE setShortcutContext)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible)

public:
    explicit QAction(QObject* parent = 0);
    QAction(const QString &text, QObject* parent=0);
    QAction(const QIcon &icon, const QString &text, QObject* parent=0);

#ifdef QT3_SUPPORT
    QT3_SUPPORT_CONSTRUCTOR QAction(QObject* parent, const char* name);
    QT3_SUPPORT_CONSTRUCTOR QAction(const QString &text, const QKeySequence &shortcut, QObject* parent, const char* name);
    QT3_SUPPORT_CONSTRUCTOR QAction(const QIcon &icon, const QString &text, const QKeySequence &shortcut,
                                  QObject* parent, const char* name);
#endif
    ~QAction();

    void setActionGroup(QActionGroup *group);
    QActionGroup *actionGroup() const;
    void setIcon(const QIcon &icon);
    QIcon icon() const;

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

    void setShortcutContext(Qt::ShortcutContext context);
    Qt::ShortcutContext shortcutContext() const;

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

#ifdef QT3_SUPPORT
    inline QT3_SUPPORT void setMenuText(const QString &text) { setText(text); }
    inline QT3_SUPPORT QString menuText() const { return text(); }
    inline QT3_SUPPORT bool isOn() const { return isChecked(); }
    inline QT3_SUPPORT void setOn(bool b) { setChecked(b); }
    inline QT3_SUPPORT bool isToggleAction() const { return isCheckable(); }
    inline QT3_SUPPORT void setToggleAction(bool b) { setCheckable(b); }
    inline QT3_SUPPORT void setIconSet(const QIcon &i) { setIcon(i); }
    inline QT3_SUPPORT QIcon iconSet() const { return icon(); }
    inline QT3_SUPPORT bool addTo(QWidget *w) { w->addAction(this); return true; }
    inline QT3_SUPPORT bool removeFrom(QWidget *w) { w->removeAction(this); return true; }
    inline QT3_SUPPORT void setAccel(const QKeySequence &shortcut) { setShortcut(shortcut); }
    inline QT3_SUPPORT QKeySequence accel() const { return shortcut(); }
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
#ifdef QT3_SUPPORT
    QT_MOC_COMPAT void toggled(bool);
    QT_MOC_COMPAT void activated(int = 0);
#endif

private:
    Q_DISABLE_COPY(QAction)

#ifdef QT3_SUPPORT
    friend class QMenuItem;
#endif
    friend class QWidget;
    friend class QActionGroup;
    friend class QMenu;
    friend class QMenuBar;
    friend class QShortcutMap;
};

#include "QtGui/qactiongroup.h" // ### remove after tech-preview 1

#endif // QACTION_H
