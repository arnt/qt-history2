/****************************************************************************
**
** Definition of QMenu and QMenuBar classes.
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef __QMENU_H__
#define __QMENU_H__

#include <qwidget.h>
#include <qstring.h>
#include <qiconset.h>
#include <qaction.h>

class QMenuPrivate;

class Q_GUI_EXPORT QMenu : public QWidget
{
private:
    Q_OBJECT
    Q_DECLARE_PRIVATE(QMenu);

    Q_PROPERTY(bool tearOffEnabled READ isTearOffEnabled WRITE setTearOffEnabled)

public:
    QMenu(QWidget *parent = 0);
    ~QMenu();

#ifdef Q_NO_USING_KEYWORD
    void addAction(QAction *action) { QWidget::addAction(action); }
#else
    using QWidget::addAction;
#endif
    QAction *addAction(const QString &text);
    QAction *addAction(const QIconSet &icon, const QString &text);
    QAction *addAction(const QString &text, const QObject *receiver, const char* member);
    QAction *addAction(const QIconSet &icon, const QString &text, const QObject *receiver, const char* member);
    QAction *addMenu(const QString &text, QMenu *menu);
    QAction *addSeparator();

    QAction *insertMenu(QAction *before, const QString &text, QMenu *menu);
    QAction *insertSeparator(QAction *before);

    void clear();

    void setTearOffEnabled(bool);
    bool isTearOffEnabled() const;

    void setCheckable(bool);
    bool isCheckable() const;

    QAction *activeAction() const;

    void popup(const QPoint &pos, QAction *at=0);
    QAction *exec();
    QAction *exec(const QPoint &pos, QAction *at=0);
    static QAction *exec(QList<QAction*> actions, const QPoint &pos, QAction *at=0);

    QSize sizeHint() const;

    QRect actionGeometry(QAction *) const;
    QAction *actionAtPos(const QPoint &, bool ignoreSeparator = true) const;

#ifdef Q_WS_MAC
    MenuRef macMenu(MenuRef merge=0);
#endif

signals:
    void activated(QAction *action);
    void highlighted(QAction *action);

protected:
    int columnCount() const;
    void changeEvent(QEvent *);
    void keyPressEvent(QKeyEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void leaveEvent(QEvent *);
    void hideEvent(QHideEvent *);
    void paintEvent(QPaintEvent *);
    void actionEvent(QActionEvent *);
    void timerEvent(QTimerEvent *);
    bool event(QEvent *);

private slots:
    void internalSetSloppyAction();
    void internalDelayedPopup();

#ifdef QT_COMPAT
public:
    //menudata
    inline QT_COMPAT uint count() const { return actions().count(); }
    inline QT_COMPAT int insertItem(const QString &text, const QObject *receiver, const char* member,
                                    const QKeySequence& accel = 0, int id = -1, int index = -1) {
        return insertAny(0, &text, receiver, member, &accel, 0, id, index);
    }
    inline QT_COMPAT int insertItem(const QIconSet& icon, const QString &text,
                                    const QObject *receiver, const char* member,
                                    const QKeySequence& accel = 0, int id = -1, int index = -1) {
        return insertAny(&icon, &text, receiver, member, &accel, 0, id, index);
    }
    inline QT_COMPAT int insertItem(const QPixmap &pixmap, const QObject *receiver, const char* member,
                                    const QKeySequence& accel = 0, int id = -1, int index = -1) {
        QIconSet icon(pixmap);
        return insertAny(&icon, 0, receiver, member, &accel, 0, id, index);
    }
    inline QT_COMPAT int insertItem(const QString &text, int id=-1, int index=-1) {
        return insertAny(0, &text, 0, 0, 0, 0, id, index);
    }
    inline QT_COMPAT int insertItem(const QIconSet& icon, const QString &text, int id=-1, int index=-1) {
        return insertAny(&icon, &text, 0, 0, 0, 0, id, index);
    }
    inline QT_COMPAT int insertItem(const QString &text, QMenu *popup, int id=-1, int index=-1) {
        return insertAny(0, &text, 0, 0, 0, popup, id, index);
    }
    inline QT_COMPAT int insertItem(const QIconSet& icon, const QString &text, QMenu *popup, int id=-1, int index=-1) {
        return insertAny(&icon, &text, 0, 0, 0, popup, id, index);
    }
    inline QT_COMPAT int insertItem(const QPixmap &pixmap, int id=-1, int index=-1) {
        QIconSet icon(pixmap);
        return insertAny(&icon, 0, 0, 0, 0, 0, id, index);
    }
    inline QT_COMPAT int insertItem(const QPixmap &pixmap, QMenu *popup, int id=-1, int index=-1) {
        QIconSet icon(pixmap);
        return insertAny(&icon, 0, 0, 0, 0, popup, id, index);
    }
    QT_COMPAT int insertSeparator(int index=-1);
    inline QT_COMPAT void removeItem(int id) { removeAction(findActionForId(id)); }
    inline QT_COMPAT void removeItemAt(int index) { removeAction(actions().value(index)); }
#ifndef QT_NO_ACCEL
    inline QT_COMPAT QKeySequence accel(int id) const { return findActionForId(id)->accel(); }
    inline QT_COMPAT void setAccel(const QKeySequence& key, int id) { findActionForId(id)->setAccel(key); }
#endif
    inline QT_COMPAT QIconSet iconSet(int id) const { return findActionForId(id)->icon(); }
    inline QT_COMPAT QString text(int id) const { return findActionForId(id)->text(); }
    inline QT_COMPAT QPixmap pixmap(int id) const { return findActionForId(id)->icon().pixmap(); }
    inline QT_COMPAT void setWhatsThis(int id, const QString &w) { findActionForId(id)->setWhatsThis(w); }
    inline QT_COMPAT QString whatsThis(int id) const { return findActionForId(id)->whatsThis(); }

    inline QT_COMPAT void changeItem(int id, const QString &text) { findActionForId(id)->setText(text); }
    inline QT_COMPAT void changeItem(int id, const QPixmap &pixmap) { findActionForId(id)->setIcon(QIconSet(pixmap)); }
    inline QT_COMPAT void changeItem(int id, const QIconSet &icon, const QString &text) {
        QAction *act = findActionForId(id);
        act->setIcon(icon);
        act->setText(text);
    }
    inline QT_COMPAT bool isItemActive(int id) const { return findActionForId(id) == activeAction(); }
    inline QT_COMPAT bool isItemEnabled(int id) const { return findActionForId(id)->isEnabled(); }
    inline QT_COMPAT void setItemEnabled(int id, bool enable) { findActionForId(id)->setEnabled(enable); }
    inline QT_COMPAT bool isItemChecked(int id) const { return findActionForId(id)->isChecked(); }
    inline QT_COMPAT void setItemChecked(int id, bool check) { findActionForId(id)->setCheckable(check); }
    inline QT_COMPAT bool isItemVisible(int id) const { return findActionForId(id)->isVisible(); }
    inline QT_COMPAT void setItemVisible(int id, bool visible) { findActionForId(id)->setVisible(visible); }
    inline QT_COMPAT QRect itemGeometry(int index) {
        return actionGeometry(actions().value(index));
    }
    inline QT_COMPAT int itemAtPos(const QPoint &p, bool ignoreSeparator = true) {
        return actionAtPos(p, ignoreSeparator)->id();
    }

    inline QT_COMPAT int indexOf(int id) const { return actions().indexOf(findActionForId(id)); }
    inline QT_COMPAT int idAt(int index) const {
        QAction * a = actions().value(index);
        return a ? a->id() : 0;
    }
    inline QT_COMPAT void activateItemAt(int index) {
        if(QAction *ret = actions().value(index))
            ret->activate(QAction::Trigger);
    }
    inline QT_COMPAT bool connectItem(int id, const QObject *receiver, const char* member) {
        QObject::connect(findActionForId(id), SIGNAL(triggered()), receiver, member);
        return true;
    }
    inline QT_COMPAT bool disconnectItem(int id,const QObject *receiver, const char* member) {
        QObject::disconnect(findActionForId(id), SIGNAL(triggered()), receiver, member);
        return true;
    }
    inline QT_COMPAT QAction *findItem(int id) const {
        return findActionForId(id);
    }

    //frame
    QT_COMPAT int frameWidth() const;

    //popupmenu
    inline QT_COMPAT void popup(const QPoint & pos, int indexAtPoint) { popup(pos, actions().value(indexAtPoint)); }
    inline QT_COMPAT int exec(const QPoint & pos, int indexAtPoint) {
        QAction *a = exec(pos, actions().value(indexAtPoint));
        return a ? a->id() : 0;
    }
    inline QT_COMPAT int insertTearOffHandle(int = 0, int = 0) {
        setTearOffEnabled(true);
        return -1;
    }

protected:
    inline QT_COMPAT int columns() const { return columnCount(); }
    inline QT_COMPAT int itemHeight(int index) {
        return actionGeometry(actions().value(index)).height();
    }
    inline QT_COMPAT int itemHeight(QAction *act) {
        return actionGeometry(act).height();
    }

private:
    int insertAny(const QIconSet *icon, const QString *text, const QObject *receiver, const char *member,
                  const QKeySequence *accel, const QMenu *popup, int id, int index);
    QAction *findActionForId(int id) const;
#endif

private:
    friend class QMenuBar;
    friend class QMenuBarPrivate;
    friend class QTornOffMenu;

#ifdef Q_WS_MAC
    friend bool qt_mac_activate_action(MenuRef, uint, QAction::ActionEvent, bool);
#endif
#if defined(Q_DISABLE_COPY)  // Disabled copy constructor and operator=
    QMenu(const QMenu &);
    QMenu &operator=(const QMenu &);
#endif
};

#endif /* __QMENU_H__ */
