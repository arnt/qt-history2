/****************************************************************************
**
** Definition of QMenuBar class.
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

#ifndef QMENUBAR_H
#define QMENUBAR_H

#include "qmenu.h"

class Q4MenuBarPrivate;

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

    QRect actionGeometry(QAction *) const;
    QAction *actionAtPos(const QPoint &) const;

#ifdef QT_COMPAT
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
    inline QT_COMPAT bool isItemActive(int id) const { return findActionForId(id) == activeAction(); }
    inline QT_COMPAT bool isItemEnabled(int id) const { return findActionForId(id)->isEnabled(); }
    inline QT_COMPAT void setItemEnabled(int id, bool enable) { findActionForId(id)->setEnabled(enable); }
    inline QT_COMPAT bool isItemChecked(int id) const { return findActionForId(id)->isChecked(); }
    inline QT_COMPAT void setItemChecked(int id, bool check) { findActionForId(id)->setCheckable(check); }
    inline QT_COMPAT bool isItemVisible(int id) const { return findActionForId(id)->isVisible(); }
    inline QT_COMPAT void setItemVisible(int id, bool visible) { findActionForId(id)->setVisible(visible); }
    inline QT_COMPAT QRect itemRect(int index) {
        return actionGeometry(findActionForIndex(index));
    }
    inline QT_COMPAT int itemAtPos(const QPoint &p) {
        QAction *a = actionAtPos(p);
        return a ? a->id() : 0;
    }
    inline QT_COMPAT int indexOf(int id) const { return actions().indexOf(findActionForId(id)); }
    inline QT_COMPAT int idAt(int index) const {
        QAction *a = actions().value(index, 0);
        return a ? a->id() : 0;
    }
    inline QT_COMPAT void activateItemAt(int index) {
        if(QAction *ret = findActionForIndex(index))
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

protected:
    void setLeftWidget(QWidget *);
    void setRightWidget(QWidget *);

private:
    friend class QMenu;
    friend class QWorkspacePrivate;
    friend class QMenuPrivate;

    QAction *findActionForId(int id) const;
    inline QAction *findActionForIndex(int index) const {
        return actions().value(index, 0);
    }
    int insertAny(const QIconSet *icon, const QString *text, const QObject *receiver, const char *member,
                  const QKeySequence *accel, const QMenu *popup, int id, int index);

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




#ifdef QT_USE_NEW_MENU_SYSTEM

class Q_GUI_EXPORT QMenuBar : public Q4MenuBar
{
    Q_OBJECT
public:
    QMenuBar(QWidget* parent=0, const char* =0) : Q4MenuBar(parent) { }
};
typedef QAction QMenuItem;

#else
#include "q3menubar.h"
class Q_GUI_EXPORT QMenuBar : public Q3MenuBar
{
    Q_OBJECT
public:
    QMenuBar(QWidget* parent=0, const char* name=0) : Q3MenuBar(parent, name) { }
};
typedef Q3MenuItem QMenuItem;
#endif

#endif // QMENUBAR_H
