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
class Q4MenuBarPrivate;

#ifndef QT_USE_NEW_MENU_SYSTEM
//# define QT_USE_NEW_MENU_SYSTEM
#endif

class Q_GUI_EXPORT QMenu : public QWidget
{
private:
    Q_OBJECT
    Q_DECLARE_PRIVATE(QMenu);

    Q_PROPERTY(bool tearOffEnabled READ isTearOffEnabled WRITE setTearOffEnabled)

public:
    QMenu(QWidget *parent = 0);
    ~QMenu();

    void     addAction(QAction *action) { QWidget::addAction(action); }
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
    QRect itemGeometry(QAction *);

#ifdef Q_WS_MAC
    MenuRef macMenu(MenuRef merge=0);
#endif

#ifdef QT_COMPAT
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
    inline QT_COMPAT int insertItem(const QIconSet& icon, const QPixmap &, const QObject *receiver, 
                                    const char* member, const QKeySequence& accel = 0, int id = -1, int index = -1) {
        qWarning("QMenu: There is no Icon & Pixmap!");
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
    inline QT_COMPAT int insertItem(const QIconSet& icon, const QPixmap &, int id=-1, int index=-1) {
        qWarning("QMenu: There is no Icon & Pixmap!");
        return insertAny(&icon, 0, 0, 0, 0, 0, id, index);
    }
    inline QT_COMPAT int insertItem(const QPixmap &pixmap, QMenu *popup, int id=-1, int index=-1) {
        QIconSet icon(pixmap);
        return insertAny(&icon, 0, 0, 0, 0, popup, id, index);
    }
    inline QT_COMPAT int insertItem(const QIconSet& icon, const QPixmap &, QMenu *popup,
                                    int id=-1, int index=-1) {
        qWarning("QMenu: There is no Icon & Pixmap!");
        return insertAny(&icon, 0, 0, 0, 0, popup, id, index);
    }
    inline int insertAny(const QIconSet *icon, const QString *text, const QObject *receiver, const char *member,
                                   const QKeySequence *accel, const QMenu *popup, int id, int index) {
        verifyPlatformCanCastPointerToInt();
        if(id != -1)
            qWarning("QMenu: id cannot be passed into insertItem!");
        QAction *act = new QAction;
        if(icon)
            act->setIcon(*icon);
        if(text)
            act->setText(*text);
        if(popup)
            act->setMenu(const_cast<QMenu*>(popup));
        if(accel)
            act->setAccel(*accel);
        if(receiver && member)
            QObject::connect(act, SIGNAL(triggered()), receiver, member);
        if(index == -1)
            addAction(act);
        else
            insertAction(act, findActionForIndex(index+1));
        return (int)act;
    }
    inline QT_COMPAT int insertSeparator(int index=-1) {
        verifyPlatformCanCastPointerToInt();
        QAction *act = new QAction;
        act->setSeparator(true);
        if(index == -1)
            addAction(act);
        else
            insertAction(act, findActionForIndex(index+1));
        return (int)act;
    }

    inline QT_COMPAT void removeItem(int id) { removeAction(findActionForId(id)); }
    inline QT_COMPAT void removeItemAt(int index) { removeAction(findActionForIndex(index)); }
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
    inline QT_COMPAT void changeItem(int id, const QIconSet &icon, const QPixmap &) {
        qWarning("QMenu: There is no Icon & Pixmap!");
        findActionForId(id)->setIcon(icon);
    }
    inline QT_COMPAT bool isItemActive(int id) const { return findActionForId(id) == activeAction(); }
    inline QT_COMPAT bool isItemEnabled(int id) const { return findActionForId(id)->isEnabled(); }
    inline QT_COMPAT void setItemEnabled(int id, bool enable) { findActionForId(id)->setEnabled(enable); }
    inline QT_COMPAT bool isItemChecked(int id) const { return findActionForId(id)->isChecked(); }
    inline QT_COMPAT void setItemChecked(int id, bool check) { findActionForId(id)->setCheckable(check); }
    inline QT_COMPAT bool isItemVisible(int id) const { return findActionForId(id)->isVisible(); }
    inline QT_COMPAT void setItemVisible(int id, bool visible) { findActionForId(id)->setVisible(visible); }
    inline QT_COMPAT int indexOf(int id) const { return actions().indexOf(findActionForId(id)); }
    inline QT_COMPAT QRect itemGeometry(int index) { 
        verifyPlatformCanCastPointerToInt();
        return itemGeometry(findActionForIndex(index)); 
    }
    inline QT_COMPAT int idAt(int index) const { 
        verifyPlatformCanCastPointerToInt();
        return (int)findActionForIndex(index);
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
        verifyPlatformCanCastPointerToInt();
        return (QAction*)id;
    }

    //popupmenu
    inline QT_COMPAT void popup(const QPoint & pos, int indexAtPoint) { popup(pos, findActionForIndex(indexAtPoint)); }
    inline QT_COMPAT int exec(const QPoint & pos, int indexAtPoint) { 
        verifyPlatformCanCastPointerToInt();
        return (int)exec(pos, findActionForIndex(indexAtPoint)); 
    }
    inline QT_COMPAT int insertTearOffHandle(int id=-1, int index=-1) {
        if(id != -1)
            qWarning("QMenu: id cannot be passed into insertTearOffHandle!");
        if(index != -1)
            qFatal("QMenu: index cannot be passed into insertTearOffHandle!");
        setTearOffEnabled(true);
        return 0;
    }
#endif

signals:
    void activated(QAction *action);
    void highlighted(QAction *action);

protected:
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

private:
    friend class Q4MenuBar;
    friend class Q4MenuBarPrivate;
    friend class QTornOffMenu;

#ifdef QT_COMPAT
    inline void verifyPlatformCanCastPointerToInt() const {
        if(sizeof(QAction*) > sizeof(int)) 
            qFatal("QMenu: This platform cannot safely handle the compat functions"
                   "due to the size of a pointer. Please rewrite to use the new QMenu code.");
    }
    inline QAction *findActionForId(int id) const {
        verifyPlatformCanCastPointerToInt();
        return (QAction*)id;
    }
    inline QAction *findActionForIndex(int index) const {
        if(index == -1)
            return 0;
        else if(QAction *ret = actions()[index])
            return ret;
        qFatal("QMenu: %d is bigger than the allowed (%d)", index, actions().size());
        return 0;
    }
#endif

#ifdef Q_WS_MAC
    friend bool qt_mac_activate_action(MenuRef, uint, QAction::ActionEvent, bool);
#endif
#if defined(Q_DISABLE_COPY)  // Disabled copy constructor and operator=
    QMenu(const QMenu &);
    QMenu &operator=(const QMenu &);
#endif
};

class Q_GUI_EXPORT Q4MenuBar : public QWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Q4MenuBar);
    Q_PROPERTY(bool defaultUp READ isDefaultUp WRITE setDefaultUp)

public:
    Q4MenuBar(QWidget *parent = 0);
    ~Q4MenuBar();

    QAction *addMenu(const QString &title, QMenu *menu);
    QAction *insertMenu(QAction *before, const QString &title, QMenu *menu);

    void clear();

    QAction *activeAction() const;

    void setDefaultUp(bool);
    bool isDefaultUp() const;

    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    int heightForWidth(int) const;

#ifdef QT_COMPAT
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
    inline QT_COMPAT int insertItem(const QIconSet& icon, const QPixmap &, const QObject *receiver, 
                                    const char* member, const QKeySequence& accel = 0, int id = -1, int index = -1) {
        qWarning("QMenuBar: There is no Icon & Pixmap!");
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
    inline QT_COMPAT int insertItem(const QIconSet& icon, const QPixmap &, int id=-1, int index=-1) {
        qWarning("QMenuBar: There is no Icon & Pixmap!");
        return insertAny(&icon, 0, 0, 0, 0, 0, id, index);
    }
    inline QT_COMPAT int insertItem(const QPixmap &pixmap, QMenu *popup, int id=-1, int index=-1) {
        QIconSet icon(pixmap);
        return insertAny(&icon, 0, 0, 0, 0, popup, id, index);
    }
    inline QT_COMPAT int insertItem(const QIconSet& icon, const QPixmap &, QMenu *popup,
                                    int id=-1, int index=-1) {
        qWarning("QMenuBar: There is no Icon & Pixmap!");
        return insertAny(&icon, 0, 0, 0, 0, popup, id, index);
    }
    inline int insertAny(const QIconSet *icon, const QString *text, const QObject *receiver, const char *member,
                                   const QKeySequence *accel, const QMenu *popup, int id, int index) {
        verifyPlatformCanCastPointerToInt();
        if(id != -1)
            qWarning("QMenuBar: id cannot be passed into insertItem!");
        QAction *act = new QAction;
        if(icon)
            act->setIcon(*icon);
        if(text)
            act->setText(*text);
        if(popup)
            act->setMenu(const_cast<QMenu*>(popup));
        if(receiver && member)
            QObject::connect(act, SIGNAL(triggered()), receiver, member);
        if(index == -1)
            addAction(act);
        else
            insertAction(act, findActionForIndex(index+1));
        return (int)act;
    }
    inline QT_COMPAT int insertSeparator(int index=-1) {
        verifyPlatformCanCastPointerToInt();
        QAction *act = new QAction;
        act->setSeparator(true);
        if(index == -1)
            addAction(act);
        else
            insertAction(act, findActionForIndex(index+1));
        return (int)act;
    }

    inline QT_COMPAT void removeItem(int id) { removeAction(findActionForId(id)); }
    inline QT_COMPAT void removeItemAt(int index) { removeAction(findActionForIndex(index)); }
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
    inline QT_COMPAT void changeItem(int id, const QIconSet &icon, const QPixmap &) {
        qWarning("QMenuBar: There is no Icon & Pixmap!");
        findActionForId(id)->setIcon(icon);
    }
    inline QT_COMPAT bool isItemActive(int id) const { return findActionForId(id) == activeAction(); }
    inline QT_COMPAT bool isItemEnabled(int id) const { return findActionForId(id)->isEnabled(); }
    inline QT_COMPAT void setItemEnabled(int id, bool enable) { findActionForId(id)->setEnabled(enable); }
    inline QT_COMPAT bool isItemChecked(int id) const { return findActionForId(id)->isChecked(); }
    inline QT_COMPAT void setItemChecked(int id, bool check) { findActionForId(id)->setCheckable(check); }
    inline QT_COMPAT bool isItemVisible(int id) const { return findActionForId(id)->isVisible(); }
    inline QT_COMPAT void setItemVisible(int id, bool visible) { findActionForId(id)->setVisible(visible); }
    inline QT_COMPAT int indexOf(int id) const { return actions().indexOf(findActionForId(id)); }
    inline QT_COMPAT int idAt(int index) const { 
        verifyPlatformCanCastPointerToInt();
        return (int)actions()[index];
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
        verifyPlatformCanCastPointerToInt();
        return (QAction*)id;
    }
#endif

#ifdef Q_WS_MAC
    MenuRef macMenu();
#endif

signals:
    void activated(QAction *action);
    void highlighted(QAction *action);

protected:
    void contextMenuEvent(QContextMenuEvent *);
    void changeEvent(QEvent *);
    void keyPressEvent(QKeyEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void leaveEvent(QEvent *);
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
    void actionEvent(QActionEvent *);
    void focusOutEvent(QFocusEvent *);
    void focusInEvent(QFocusEvent *);
    bool eventFilter(QObject *, QEvent *);
    bool event(QEvent *);

private slots:
    void internalShortcutActivated(int);

private:
    friend class QMenu;
    friend class QMenuPrivate;

#ifdef QT_COMPAT
    inline void verifyPlatformCanCastPointerToInt() const {
        if(sizeof(QAction*) > sizeof(int)) 
            qFatal("QMenuBar: This platform cannot safely handle the compat functions"
                   "due to the size of a pointer. Please rewrite to use the new QMenu/QMenuBar code.");
    }
    inline QAction *findActionForId(int id) const {
        verifyPlatformCanCastPointerToInt();
        return (QAction*)id;
    }
    inline QAction *findActionForIndex(int index) const {
        if(index == -1)
            return 0;
        else if(QAction *ret = actions()[index])
            return ret;
        qFatal("QMenuBar: %d is bigger than the allowed (%d)", index, actions().size());
        return 0;
    }
#endif

#ifdef Q_WS_MAC
    friend class QApplication;
    static bool macUpdateMenuBar();
    friend bool qt_mac_activate_action(MenuRef, uint, QAction::ActionEvent, bool);
#endif
#if defined(Q_DISABLE_COPY)  // Disabled copy constructor and operator=
    Q4MenuBar(const Q4MenuBar &);
    Q4MenuBar &operator=(const Q4MenuBar &);
#endif
};

#endif /* __QMENU_H__ */
