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
#ifdef QT_COMPAT
class QMenuItem;
#endif

class Q_GUI_EXPORT QMenu : public QWidget
{
private:
    Q_OBJECT
    Q_DECLARE_PRIVATE(QMenu);

    Q_PROPERTY(bool tearOffEnabled READ isTearOffEnabled WRITE setTearOffEnabled)
    Q_PROPERTY(bool checkable READ isCheckable WRITE setCheckable)

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

    void contextMenuEvent(QContextMenuEvent *);
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
    inline QT_COMPAT void removeItem(int id) {
        if(QAction *act = findActionForId(id)) 
            removeAction(act); }
    inline QT_COMPAT void removeItemAt(int index) { 
        if(QAction *act = actions().value(index))
            removeAction(act); }
#ifndef QT_NO_ACCEL
    inline QT_COMPAT QKeySequence accel(int id) const { 
        if(QAction *act = findActionForId(id))
            return act->accel();
        return QKeySequence(); }
    inline QT_COMPAT void setAccel(const QKeySequence& key, int id) { 
        if(QAction *act = findActionForId(id))
            act->setAccel(key);
    }
#endif
    inline QT_COMPAT QIconSet iconSet(int id) const { 
        if(QAction *act = findActionForId(id))
            return act->icon();
        return QIconSet(); }
    inline QT_COMPAT QString text(int id) const { 
        if(QAction *act = findActionForId(id))
            return act->text();
        return QString(); }
    inline QT_COMPAT QPixmap pixmap(int id) const { 
        if(QAction *act = findActionForId(id))
            return act->icon().pixmap();
        return QString(); }
    inline QT_COMPAT void setWhatsThis(int id, const QString &w) { 
        if(QAction *act = findActionForId(id))
            act->setWhatsThis(w); }
    inline QT_COMPAT QString whatsThis(int id) const { 
        if(QAction *act = findActionForId(id))
            return act->whatsThis(); 
        return QString(); }

    inline QT_COMPAT void changeItem(int id, const QString &text) { 
        if(QAction *act = findActionForId(id))
            return act->setText(text); }
    inline QT_COMPAT void changeItem(int id, const QPixmap &pixmap) { 
        if(QAction *act = findActionForId(id))
            act->setIcon(QIconSet(pixmap)); }
    inline QT_COMPAT void changeItem(int id, const QIconSet &icon, const QString &text) {
        if(QAction *act = findActionForId(id)) {
            act->setIcon(icon);
            act->setText(text);
        }
    }
    inline QT_COMPAT bool isItemActive(int id) const { return findActionForId(id) == activeAction(); }
    inline QT_COMPAT bool isItemEnabled(int id) const { 
        if(QAction *act = findActionForId(id)) 
            return act->isEnabled(); 
        return false; }
    inline QT_COMPAT void setItemEnabled(int id, bool enable) { 
        if(QAction *act = findActionForId(id)) 
            return act->setEnabled(enable); }
    inline QT_COMPAT bool isItemChecked(int id) const { 
        if(QAction *act = findActionForId(id)) 
            return act->isChecked(); 
        return false; }
    inline QT_COMPAT void setItemChecked(int id, bool check) { 
        if(QAction *act = findActionForId(id)) 
            act->setChecked(check); }
    inline QT_COMPAT bool isItemVisible(int id) const { 
        if(QAction *act = findActionForId(id)) 
            return act->isVisible();
        return false; }
    inline QT_COMPAT void setItemVisible(int id, bool visible) { 
        if(QAction *act = findActionForId(id)) 
            return act->setVisible(visible); }
    inline QT_COMPAT QRect itemGeometry(int index) {
        if(QAction *act = actions().value(index)) 
            return actionGeometry(act);
        return QRect();
    }
    inline QT_COMPAT int indexOf(int id) const { return actions().indexOf(findActionForId(id)); }
    inline QT_COMPAT int idAt(int index) const {
        return findIdForAction(actions().value(index));
    }
    inline QT_COMPAT void activateItemAt(int index) {
        if(QAction *ret = actions().value(index))
            ret->activate(QAction::Trigger);
    }
    inline QT_COMPAT bool connectItem(int id, const QObject *receiver, const char* member) {
        if(QAction *act = findActionForId(id)) {
            QObject::connect(act, SIGNAL(triggered()), receiver, member);
            return true;
        }
        return false;
    }
    inline QT_COMPAT bool disconnectItem(int id,const QObject *receiver, const char* member) {
        if(QAction *act = findActionForId(id)) {
            QObject::disconnect(act, SIGNAL(triggered()), receiver, member);
            return true;
        } 
        return false;
    }
    inline QT_COMPAT QMenuItem *findItem(int id) const {
        return (QMenuItem*)findActionForId(id);
    }
    QT_COMPAT bool setItemParameter(int id, int param);
    QT_COMPAT int itemParameter(int id) const;

    //frame
    QT_COMPAT int frameWidth() const;

    //popupmenu
    inline QT_COMPAT void popup(const QPoint & pos, int indexAtPoint) { popup(pos, actions().value(indexAtPoint)); }
    inline QT_COMPAT int exec(const QPoint & pos, int indexAtPoint) {
        return findIdForAction(exec(pos, actions().value(indexAtPoint)));
    }
    inline QT_COMPAT int insertTearOffHandle(int = 0, int = 0) {
        setTearOffEnabled(true);
        return -1;
    }

protected:
    inline QT_COMPAT int itemAtPos(const QPoint &p, bool ignoreSeparator = true) {
        return findIdForAction(actionAtPos(p, ignoreSeparator));
    }
    inline QT_COMPAT int columns() const { return columnCount(); }
    inline QT_COMPAT int itemHeight(int index) {
        return actionGeometry(actions().value(index)).height();
    }
    inline QT_COMPAT int itemHeight(QMenuItem *mi) {
        return actionGeometry((QAction*)mi).height();
    }

signals:
    QT_COMPAT void aboutToHide();
    QT_COMPAT void aboutToShow();
    QT_COMPAT void activated(int itemId);
    QT_COMPAT void highlighted(int itemId);

private slots:
    void compatActivated(QAction *);
    void compatHighlighted(QAction *);

private:
    int insertAny(const QIconSet *icon, const QString *text, const QObject *receiver, const char *member,
                  const QKeySequence *accel, const QMenu *popup, int id, int index);
    QAction *findActionForId(int id) const;
    int findIdForAction(QAction*) const;
#endif

private:
    friend class QMenuBar;
    friend class QMenuBarPrivate;
    friend class QTornOffMenu;

#ifdef Q_WS_MAC
    friend OSStatus qt_mac_menu_event(EventHandlerCallRef, EventRef, void *);
    friend bool qt_mac_activate_action(MenuRef, uint, QAction::ActionEvent, bool);
#endif
#if defined(Q_DISABLE_COPY)  // Disabled copy constructor and operator=
    QMenu(const QMenu &);
    QMenu &operator=(const QMenu &);
#endif
};

#endif /* __QMENU_H__ */
