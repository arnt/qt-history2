/****************************************************************************
**
** Definition of QPushButton class.
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

#ifndef QPUSHBUTTON_H
#define QPUSHBUTTON_H

#ifndef QT_H
#include "qbutton.h"
#include "qiconset.h"
#endif // QT_H

#ifndef QT_NO_PUSHBUTTON
class QPushButtonPrivate;
class QPopupMenu;

class Q_GUI_EXPORT QPushButton : public QButton
{
    Q_OBJECT

    Q_PROPERTY(bool autoDefault READ autoDefault WRITE setAutoDefault)
    Q_PROPERTY(bool default READ isDefault WRITE setDefault)
    Q_PROPERTY(bool menuButton READ isMenuButton DESIGNABLE false)
    Q_PROPERTY(QIconSet iconSet READ iconSet WRITE setIconSet)
    Q_OVERRIDE(bool toggleButton WRITE setToggleButton)
    Q_OVERRIDE(bool on WRITE setOn)
    Q_PROPERTY(bool flat READ isFlat WRITE setFlat)
    Q_OVERRIDE(bool autoMask DESIGNABLE true SCRIPTABLE true)

public:
    QPushButton(QWidget *parent=0, const char* name=0);
    QPushButton(const QString &text, QWidget *parent=0, const char* name=0);
#ifndef QT_NO_ICONSET
    QPushButton(const QIconSet& icon, const QString &text, QWidget *parent=0, const char* name=0);
#endif
    ~QPushButton();

    QSize        sizeHint() const;

    void        move(int x, int y);
    void        move(const QPoint &p);
    void        resize(int w, int h);
    void        resize(const QSize &);
    void        setGeometry(int x, int y, int w, int h);

    void        setGeometry(const QRect &);

    void setToggleButton(bool);

    bool        autoDefault()        const        { return autoDefButton; }
    virtual void setAutoDefault(bool autoDef);
    bool        isDefault()        const        { return defButton; }
    virtual void setDefault(bool def);

    virtual void setIsMenuButton(bool enable) {  // obsolete functions
        if ((bool)hasMenuArrow == enable)
            return;
        hasMenuArrow = enable ? 1 : 0;
        update();
        updateGeometry();
    }
    bool        isMenuButton() const { return hasMenuArrow; }

#ifndef QT_NO_POPUPMENU
    void setPopup(QPopupMenu* popup);
    QPopupMenu* popup() const;
#endif
#ifndef QT_NO_ICONSET
    void setIconSet(const QIconSet&);
    QIconSet* iconSet() const;
#endif
    void setFlat(bool);
    bool isFlat() const;

public slots:
    virtual void setOn(bool);
    void openPopup();

protected:
    void        drawButton(QPainter *);
    void        drawButtonLabel(QPainter *);
    void        focusInEvent(QFocusEvent *);
    void        focusOutEvent(QFocusEvent *);
    void        resizeEvent(QResizeEvent *);
    void        updateMask();
private slots:
#ifndef QT_NO_POPUPMENU
    void popupPressed();
#endif
private:
    void        init();

    uint        autoDefButton        : 1;
    uint        defButton        : 1;
    uint        flt                : 1;
    uint        reserved                : 1; // UNUSED
    uint        lastEnabled        : 1; // UNUSED
    uint        hasMenuArrow        : 1;

    QPushButtonPrivate* d;

    friend class QDialog;

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QPushButton(const QPushButton &);
    QPushButton &operator=(const QPushButton &);
#endif
};


#endif // QT_NO_PUSHBUTTON

#endif // QPUSHBUTTON_H
