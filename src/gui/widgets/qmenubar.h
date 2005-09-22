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

#ifndef QMENUBAR_H
#define QMENUBAR_H

#include <QtGui/qmenu.h>

QT_MODULE(Gui)

#ifndef QT_NO_MENUBAR

class QMenuBarPrivate;
#ifdef QT3_SUPPORT
class QMenuItem;
#endif

class Q_GUI_EXPORT QMenuBar : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(bool defaultUp READ isDefaultUp WRITE setDefaultUp)

public:
    explicit QMenuBar(QWidget *parent = 0);
    ~QMenuBar();

#ifdef Q_NO_USING_KEYWORD
    void addAction(QAction *action) { QWidget::addAction(action); }
#else
    using QWidget::addAction;
#endif
    QAction *addAction(const QString &text);
    QAction *addAction(const QString &text, const QObject *receiver, const char* member);

    QAction *addMenu(QMenu *menu);
    QMenu *addMenu(const QString &title);
    QMenu *addMenu(const QIcon &icon, const QString &title);


    QAction *addSeparator();

    QAction *insertMenu(QAction *before, QMenu *menu);

    void clear();

    QAction *activeAction() const;
    void setActiveAction(QAction *action);

    void setDefaultUp(bool);
    bool isDefaultUp() const;

    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    int heightForWidth(int) const;

    QRect actionGeometry(QAction *) const;
    QAction *actionAt(const QPoint &) const;

    void setCornerWidget(QWidget *w, Qt::Corner corner = Qt::TopRightCorner);
    QWidget *cornerWidget(Qt::Corner corner = Qt::TopRightCorner) const;

#ifdef Q_WS_MAC
    MenuRef macMenu();
#endif

Q_SIGNALS:
    void triggered(QAction *action);
    void hovered(QAction *action);

protected:
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

#ifdef QT3_SUPPORT
public:
    QT3_SUPPORT_CONSTRUCTOR QMenuBar(QWidget *parent, const char *name);
    inline QT3_SUPPORT uint count() const { return actions().count(); }
    inline QT3_SUPPORT int insertItem(const QString &text, const QObject *receiver, const char* member,
                                    const QKeySequence& shortcut = 0, int id = -1, int index = -1) {
        return insertAny(0, &text, receiver, member, &shortcut, 0, id, index);
    }
    inline QT3_SUPPORT int insertItem(const QIcon& icon, const QString &text,
                                    const QObject *receiver, const char* member,
                                    const QKeySequence& shortcut = 0, int id = -1, int index = -1) {
        return insertAny(&icon, &text, receiver, member, &shortcut, 0, id, index);
    }
    inline QT3_SUPPORT int insertItem(const QPixmap &pixmap, const QObject *receiver, const char* member,
                                    const QKeySequence& shortcut = 0, int id = -1, int index = -1) {
        QIcon icon(pixmap);
        return insertAny(&icon, 0, receiver, member, &shortcut, 0, id, index);
    }
    inline QT3_SUPPORT int insertItem(const QString &text, int id=-1, int index=-1) {
        return insertAny(0, &text, 0, 0, 0, 0, id, index);
    }
    inline QT3_SUPPORT int insertItem(const QIcon& icon, const QString &text, int id=-1, int index=-1) {
        return insertAny(&icon, &text, 0, 0, 0, 0, id, index);
    }
    inline QT3_SUPPORT int insertItem(const QString &text, QMenu *popup, int id=-1, int index=-1) {
        return insertAny(0, &text, 0, 0, 0, popup, id, index);
    }
    inline QT3_SUPPORT int insertItem(const QIcon& icon, const QString &text, QMenu *popup, int id=-1, int index=-1) {
        return insertAny(&icon, &text, 0, 0, 0, popup, id, index);
    }
    inline QT3_SUPPORT int insertItem(const QPixmap &pixmap, int id=-1, int index=-1) {
        QIcon icon(pixmap);
        return insertAny(&icon, 0, 0, 0, 0, 0, id, index);
    }
    inline QT3_SUPPORT int insertItem(const QPixmap &pixmap, QMenu *popup, int id=-1, int index=-1) {
        QIcon icon(pixmap);
        return insertAny(&icon, 0, 0, 0, 0, popup, id, index);
    }
    QT3_SUPPORT int insertSeparator(int index=-1);
    inline QT3_SUPPORT void removeItem(int id) {
        if(QAction *act = findActionForId(id))
            removeAction(act); }
    inline QT3_SUPPORT void removeItemAt(int index) {
        if(QAction *act = actions().value(index))
            removeAction(act); }
#ifndef QT_NO_SHORTCUT
    inline QT3_SUPPORT QKeySequence accel(int id) const {
        if(QAction *act = findActionForId(id))
            return act->shortcut();
        return QKeySequence(); }
    inline QT3_SUPPORT void setAccel(const QKeySequence& key, int id) {
        if(QAction *act = findActionForId(id))
            act->setShortcut(key);
    }
#endif
    inline QT3_SUPPORT QIcon iconSet(int id) const {
        if(QAction *act = findActionForId(id))
            return act->icon();
        return QIcon(); }
    inline QT3_SUPPORT QString text(int id) const {
        if(QAction *act = findActionForId(id))
            return act->text();
        return QString(); }
    inline QT3_SUPPORT QPixmap pixmap(int id) const {
        if(QAction *act = findActionForId(id))
            return act->icon().pixmap(QSize(22,22));
        return QPixmap(); }
    inline QT3_SUPPORT void setWhatsThis(int id, const QString &w) {
        if(QAction *act = findActionForId(id))
            act->setWhatsThis(w); }
    inline QT3_SUPPORT QString whatsThis(int id) const {
        if(QAction *act = findActionForId(id))
            return act->whatsThis();
        return QString(); }

    inline QT3_SUPPORT void changeItem(int id, const QString &text) {
        if(QAction *act = findActionForId(id))
            act->setText(text); }
    inline QT3_SUPPORT void changeItem(int id, const QPixmap &pixmap) {
        if(QAction *act = findActionForId(id))
            act->setIcon(QIcon(pixmap)); }
    inline QT3_SUPPORT void changeItem(int id, const QIcon &icon, const QString &text) {
        if(QAction *act = findActionForId(id)) {
            act->setIcon(icon);
            act->setText(text);
        }
    }
    inline QT3_SUPPORT bool isItemActive(int id) const { return findActionForId(id) == activeAction(); }
    inline QT3_SUPPORT bool isItemEnabled(int id) const {
        if(QAction *act = findActionForId(id))
            return act->isEnabled();
        return false; }
    inline QT3_SUPPORT void setItemEnabled(int id, bool enable) {
        if(QAction *act = findActionForId(id))
            act->setEnabled(enable); }
    inline QT3_SUPPORT bool isItemChecked(int id) const {
        if(QAction *act = findActionForId(id))
            return act->isChecked();
        return false; }
    inline QT3_SUPPORT void setItemChecked(int id, bool check) {
        if(QAction *act = findActionForId(id))
            act->setChecked(check); }
    inline QT3_SUPPORT bool isItemVisible(int id) const {
        if(QAction *act = findActionForId(id))
            return act->isVisible();
        return false; }
    inline QT3_SUPPORT void setItemVisible(int id, bool visible) {
        if(QAction *act = findActionForId(id))
            act->setVisible(visible); }
    inline QT3_SUPPORT int indexOf(int id) const { return actions().indexOf(findActionForId(id)); }
    inline QT3_SUPPORT int idAt(int index) const {
        return index >= 0 && index < actions().size()
                        ? findIdForAction(actions().at(index))
                        : -1;
    }
    inline QT3_SUPPORT void activateItemAt(int index) {
        if(QAction *ret = actions().value(index))
            ret->activate(QAction::Trigger);
    }
    inline QT3_SUPPORT bool connectItem(int id, const QObject *receiver, const char* member) {
        if(QAction *act = findActionForId(id)) {
            QObject::connect(act, SIGNAL(triggered()), receiver, member);
            return true;
        }
        return false;
    }
    inline QT3_SUPPORT bool disconnectItem(int id,const QObject *receiver, const char* member) {
        if(QAction *act = findActionForId(id)) {
            QObject::disconnect(act, SIGNAL(triggered()), receiver, member);
            return true;
        }
        return false;
    }
    inline QT3_SUPPORT QMenuItem *findItem(int id) const {
        return (QMenuItem*)findActionForId(id);
    }
    QT3_SUPPORT bool setItemParameter(int id, int param);
    QT3_SUPPORT int itemParameter(int id) const;

    //frame
    QT3_SUPPORT int frameWidth() const;

    //menubar
    enum Separator { Never=0, InWindowsStyle=1 };
    inline QT3_SUPPORT Separator separator() const { return InWindowsStyle; }
    inline QT3_SUPPORT void setSeparator(Separator) { }

    QT3_SUPPORT void setAutoGeometry(bool);
    QT3_SUPPORT bool autoGeometry() const;

Q_SIGNALS:
    QT_MOC_COMPAT void activated(int itemId);
    QT_MOC_COMPAT void highlighted(int itemId);

protected:
    inline QT3_SUPPORT QRect itemRect(int index) {
        if(QAction *act = actions().value(index))
            return actionGeometry(act);
        return QRect();
    }
    inline QT3_SUPPORT int itemAtPos(const QPoint &p) {
        return findIdForAction(actionAt(p));
    }

private:
    QAction *findActionForId(int id) const;
    int insertAny(const QIcon *icon, const QString *text, const QObject *receiver, const char *member,
                  const QKeySequence *shorcut, const QMenu *popup, int id, int index);
    int findIdForAction(QAction*) const;
#endif

private:
    Q_DECLARE_PRIVATE(QMenuBar)
    Q_DISABLE_COPY(QMenuBar)
    Q_PRIVATE_SLOT(d_func(), void actionTriggered())
    Q_PRIVATE_SLOT(d_func(), void actionHovered())
    Q_PRIVATE_SLOT(d_func(), void internalShortcutActivated(int))
    Q_PRIVATE_SLOT(d_func(), void updateLayout())

    friend class QMenu;
    friend class QMenuPrivate;

#ifdef Q_WS_MAC
    friend class QApplicationPrivate;
    static bool macUpdateMenuBar();
    friend bool qt_mac_activate_action(MenuRef, uint, QAction::ActionEvent, bool);
#endif
};

#endif // QT_NO_MENUBAR

#endif // QMENUBAR_H
