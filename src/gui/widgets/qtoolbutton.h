/****************************************************************************
**
** Definition of QToolButton class.
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

#ifndef QTOOLBUTTON_H
#define QTOOLBUTTON_H

#ifndef QT_H
#include "qabstractbutton.h"
#endif // QT_H

#ifndef QT_NO_TOOLBUTTON

class QToolButtonPrivate;
class QToolBar;
class QMenu;

class Q_GUI_EXPORT QToolButton : public QAbstractButton
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QToolButton);
    Q_ENUMS(TextPosition)

    Q_PROPERTY(bool usesBigPixmap READ usesBigPixmap WRITE setUsesBigPixmap)
    Q_PROPERTY(bool usesTextLabel READ usesTextLabel WRITE setUsesTextLabel)
    Q_PROPERTY(int popupDelay READ popupDelay WRITE setPopupDelay)
    Q_PROPERTY(bool autoRaise READ autoRaise WRITE setAutoRaise)
    Q_PROPERTY(TextPosition textPosition READ textPosition WRITE setTextPosition)
    Q_OVERRIDE(BackgroundMode backgroundMode DESIGNABLE true)

public:
    enum TextPosition {
        BesideIcon,
        BelowIcon,
        Right = BesideIcon, // obsolete
        Under = BelowIcon // obsolete
    };
    QToolButton(QWidget * parent=0);

#ifndef QT_NO_TOOLBAR
    QToolButton(const QIconSet& s, const QString &textLabel,
                 const QString& statusTip,
                 QObject * receiver, const char* slot,
                 QToolBar * parent=0, const char* name=0);
#endif
    QToolButton(ArrowType type, QWidget *parent=0, const char* name=0);
    ~QToolButton();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;


    bool usesBigPixmap() const;
    bool usesTextLabel() const;

    void setMenu(QMenu* menu);
    QMenu* menu() const;
    void showMenu();

    void setPopupDelay(int delay);
    int popupDelay() const;

    void setAutoRaise(bool enable);
    bool autoRaise() const;
    TextPosition textPosition() const;

public slots:
    virtual void setUsesBigPixmap(bool enable);
    virtual void setUsesTextLabel(bool enable);
    void setTextPosition(TextPosition pos);

protected:
    void mousePressEvent(QMouseEvent *);
    void drawBevel(QPainter *);
    void drawLabel(QPainter *);
    void paintEvent(QPaintEvent *);

    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void timerEvent(QTimerEvent *);

    bool uses3D() const;

    bool eventFilter(QObject *o, QEvent *e);

private:
    Q_PRIVATE_SLOT(void popupPressed())
    void popupTimerDone();


#ifdef QT_COMPAT
public:
    QToolButton(QWidget * parent, const char* name);
    inline QT_COMPAT void setPixmap(const QPixmap &pixmap) { setIcon(pixmap); }
    QT_COMPAT void setOnIconSet(const QIconSet&);
    QT_COMPAT void setOffIconSet(const QIconSet&);
    QT_COMPAT void setIconSet(const QIconSet &, bool on = true);
    inline QT_COMPAT void setTextLabel(const QString &text, bool tooltip = true) { setText(text); if (tooltip)setToolTip(text);}
    inline QT_COMPAT QString textLabel() const { return text(); }
    QT_COMPAT QIconSet onIconSet() const;
    QT_COMPAT QIconSet offIconSet() const;
    QT_COMPAT QIconSet iconSet(bool on) const;
    inline QT_COMPAT QIconSet iconSet() const { return icon(); }
    inline QT_COMPAT void openPopup()  { showMenu(); }
    inline QT_COMPAT void setPopup(QMenu* popup) {setMenu(popup); }
    inline QT_COMPAT QMenu* popup() const { return menu(); }
#endif

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QToolButton(const QToolButton &);
    QToolButton& operator=(const QToolButton &);
#endif
};

#endif // QT_NO_TOOLBUTTON

#endif // QTOOLBUTTON_H
