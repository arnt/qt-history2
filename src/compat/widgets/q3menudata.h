/****************************************************************************
**
** Definition of Q3MenuData class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt Compat Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef Q3MENUDATA_H
#define Q3MENUDATA_H

#ifndef QT_H
#include "qglobal.h"
#include "qiconset.h" // conversion QPixmap->QIconset
#include "qkeysequence.h"
#include "qstring.h"
#include "qfont.h"
#endif // QT_H

#ifndef QT_NO_MENUDATA

class Q3PopupMenu;
class Q3MenuDataData;
class QObject;
class QSignalEmitter;

class Q3CustomMenuItem;
class Q3MenuItemData;

class Q_COMPAT_EXPORT Q3MenuItem                        // internal menu item class
{
friend class Q3MenuData;
public:
    Q3MenuItem();
   ~Q3MenuItem();

    int                id()                const        { return ident; }
    QIconSet   *iconSet()        const        { return iconset_data; }
    QString        text()                const        { return text_data; }
    QString        whatsThis()        const        { return whatsthis_data; }
    QPixmap    *pixmap()        const        { return pixmap_data; }
    Q3PopupMenu *popup()                const        { return popup_menu; }
    QWidget *widget()                const        { return widget_item; }
    Q3CustomMenuItem *custom()        const;
#ifndef QT_NO_ACCEL
    QKeySequence key()                const        { return accel_key; }
#endif
    QSignalEmitter    *signal()        const        { return signal_data; }
    int signalValue()        const        { return signal_value; }
    bool        isSeparator()        const        { return is_separator; }
    bool        isEnabled()        const        { return is_enabled; }
    bool        isChecked()        const        { return is_checked; }
    bool        isDirty()        const        { return is_dirty; }
    bool        isVisible()        const        { return is_visible; }
    bool        isEnabledAndVisible() const { return is_enabled && is_visible; }

    void        setText(const QString &text) { text_data = text; }
    void        setDirty(bool dirty)               { is_dirty = dirty; }
    void        setVisible(bool visible)               { is_visible = visible; }
    void        setWhatsThis(const QString &text) { whatsthis_data = text; }

private:
    int                ident;                                // item identifier
    QIconSet   *iconset_data;                        // icons
    QString        text_data;                        // item text
    QString        whatsthis_data;                        // item Whats This help text
    QPixmap    *pixmap_data;                        // item pixmap
    Q3PopupMenu *popup_menu;                        // item popup menu
    QWidget    *widget_item;                        // widget menu item
#ifndef QT_NO_ACCEL
    QKeySequence        accel_key;                // accelerator key (state|ascii)
#endif
    QSignalEmitter    *signal_data;                        // connection
    int signal_value;                        // connection value
    uint        is_separator : 1;                // separator flag
    uint        is_enabled   : 1;                // disabled flag
    uint        is_checked   : 1;                // checked flag
    uint        is_dirty     : 1;                // dirty (update) flag
    uint        is_visible     : 1;                // visibility flag
    Q3MenuItemData* d;

    Q3MenuItemData* extra();

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    Q3MenuItem(const Q3MenuItem &);
    Q3MenuItem &operator=(const Q3MenuItem &);
#endif
};


typedef QList<Q3MenuItem*> Q3MenuItemList;


class Q_COMPAT_EXPORT Q3CustomMenuItem : public Qt
{
public:
    Q3CustomMenuItem();
    virtual ~Q3CustomMenuItem();
    virtual bool fullSpan() const;
    virtual bool isSeparator() const;
    virtual void setFont(const QFont& font);
    virtual void paint(QPainter* p, const QPalette& pal, bool act,
                        bool enabled, int x, int y, int w, int h) = 0;
    virtual QSize sizeHint() = 0;
};


class Q_COMPAT_EXPORT Q3MenuData                        // menu data class
{
friend class Q3MenuBar;
friend class Q3PopupMenu;
public:
    Q3MenuData();
    virtual ~Q3MenuData();

    uint        count() const;


    int                insertItem(const QString &text,
                            const QObject *receiver, const char* member,
                            const QKeySequence& accel = 0, int id = -1, int index = -1);
    int                insertItem(const QIconSet& icon,
                            const QString &text,
                            const QObject *receiver, const char* member,
                            const QKeySequence& accel = 0, int id = -1, int index = -1);
    int                insertItem(const QPixmap &pixmap,
                            const QObject *receiver, const char* member,
                            const QKeySequence& accel = 0, int id = -1, int index = -1);
    int                insertItem(const QIconSet& icon,
                            const QPixmap &pixmap,
                            const QObject *receiver, const char* member,
                            const QKeySequence& accel = 0, int id = -1, int index = -1);

    int                insertItem(const QString &text, int id=-1, int index=-1);
    int                insertItem(const QIconSet& icon,
                            const QString &text, int id=-1, int index=-1);

    int                insertItem(const QString &text, Q3PopupMenu *popup,
                            int id=-1, int index=-1);
    int                insertItem(const QIconSet& icon,
                            const QString &text, Q3PopupMenu *popup,
                            int id=-1, int index=-1);


    int                insertItem(const QPixmap &pixmap, int id=-1, int index=-1);
    int                insertItem(const QIconSet& icon,
                            const QPixmap &pixmap, int id=-1, int index=-1);
    int                insertItem(const QPixmap &pixmap, Q3PopupMenu *popup,
                            int id=-1, int index=-1);
    int                insertItem(const QIconSet& icon,
                            const QPixmap &pixmap, Q3PopupMenu *popup,
                            int id=-1, int index=-1);

    int                insertItem(QWidget* widget, int id=-1, int index=-1);

    int                insertItem(const QIconSet& icon, Q3CustomMenuItem* custom, int id=-1, int index=-1);
    int                insertItem(Q3CustomMenuItem* custom, int id=-1, int index=-1);


    int                insertSeparator(int index=-1);

    void        removeItem(int id);
    void        removeItemAt(int index);
    void        clear();

#ifndef QT_NO_ACCEL
    QKeySequence accel(int id)        const;
    void        setAccel(const QKeySequence& key, int id);
#endif

    QIconSet    *iconSet(int id)        const;
    QString text(int id)                const;
    QPixmap    *pixmap(int id)        const;

    void setWhatsThis(int id, const QString&);
    QString whatsThis(int id) const;


    void        changeItem(int id, const QString &text);
    void        changeItem(int id, const QPixmap &pixmap);
    void        changeItem(int id, const QIconSet &icon, const QString &text);
    void        changeItem(int id, const QIconSet &icon, const QPixmap &pixmap);

    void        changeItem(const QString &text, int id) { changeItem(id, text); } // obsolete
    void        changeItem(const QPixmap &pixmap, int id) { changeItem(id, pixmap); } // obsolete
    void        changeItem(const QIconSet &icon, const QString &text, int id) {        // obsolete
        changeItem(id, icon, text);
    }

    bool        isItemActive(int id) const;

    bool        isItemEnabled(int id) const;
    void        setItemEnabled(int id, bool enable);

    bool        isItemChecked(int id) const;
    void        setItemChecked(int id, bool check);

    bool        isItemVisible(int id) const;
    void        setItemVisible(int id, bool visible);

    virtual void updateItem(int id);

    int                indexOf(int id)        const;
    int                idAt(int index)        const;
    virtual void        setId(int index, int id);

    bool        connectItem(int id,
                             const QObject *receiver, const char* member);
    bool        disconnectItem(int id,
                                const QObject *receiver, const char* member);

    bool        setItemParameter(int id, int param);
    int        itemParameter(int id) const;

    Q3MenuItem  *findItem(int id)        const;
    Q3MenuItem  *findItem(int id, Q3MenuData ** parent)        const;
    Q3MenuItem * findPopup(Q3PopupMenu *, int *index = 0);

    virtual void activateItemAt(int index);

protected:
    int                   actItem;
    Q3MenuItemList *mitems;
    Q3MenuData          *parentMenu;
    uint           mitemsAutoDelete : 1;
    uint           isPopupMenu : 1;
    uint           isMenuBar : 1;
    uint           badSize : 1;
    uint           mouseBtDn : 1;
    uint        avoid_circularity : 1;
    uint        actItemDown : 1;
    virtual void   menuContentsChanged();
    virtual void   menuStateChanged();
    virtual void   menuInsPopup(Q3PopupMenu *);
    virtual void   menuDelPopup(Q3PopupMenu *);

private:
    int                insertAny(const QString *, const QPixmap *, Q3PopupMenu *,
                           const QIconSet*, int, int, QWidget* = 0, Q3CustomMenuItem* = 0);
    void        removePopup(Q3PopupMenu *);
    void        changeItemIconSet(int id, const QIconSet &icon);

    Q3MenuDataData *d;

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    Q3MenuData(const Q3MenuData &);
    Q3MenuData &operator=(const Q3MenuData &);
#endif
};


#endif // QT_NO_MENUDATA

#endif // Q3MENUDATA_H
