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

#ifndef QTOOLBUTTON_H
#define QTOOLBUTTON_H

#include "qabstractbutton.h"

#ifndef QT_NO_TOOLBUTTON

class QToolButtonPrivate;
class QMenu;

class Q_GUI_EXPORT QToolButton : public QAbstractButton
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QToolButton)
    Q_ENUMS(Qt::ToolButtonStyle)
    Q_ENUMS(Qt::IconSize)

    Q_PROPERTY(Qt::ToolButtonStyle toolButtonStyle READ toolButtonStyle WRITE setToolButtonStyle)
    Q_PROPERTY(Qt::IconSize iconSize READ iconSize WRITE setIconSize)
    Q_PROPERTY(int popupDelay READ popupDelay WRITE setPopupDelay)
    Q_PROPERTY(bool autoRaise READ autoRaise WRITE setAutoRaise)
    Q_OVERRIDE(Qt::BackgroundMode backgroundMode DESIGNABLE true)

public:
    enum ToolButtonPopupMode {
        InstantPopupMode,
        MenuButtonPopupMode,
        DelayedPopupMode
    };

    QToolButton(QWidget * parent=0);

    QToolButton(Qt::ArrowType type, QWidget *parent=0);
    ~QToolButton();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    Qt::IconSize iconSize() const;
    Qt::ToolButtonStyle toolButtonStyle() const;

    void setMenu(QMenu* menu);
    QMenu* menu() const;
    void showMenu();

    void setPopupMode(QToolButton::ToolButtonPopupMode mode);
    QToolButton::ToolButtonPopupMode popupMode();

    void setPopupDelay(int delay);
    int popupDelay() const;

    void setAutoRaise(bool enable);
    bool autoRaise() const;

public slots:
    void setIconSize(Qt::IconSize size);
    void setToolButtonStyle(Qt::ToolButtonStyle style);

protected:
    QToolButton(QToolButtonPrivate &, QWidget* parent);
    void mousePressEvent(QMouseEvent *);
    void drawBevel(QPainter *);
    void drawLabel(QPainter *);
    void paintEvent(QPaintEvent *);
    void actionEvent(QActionEvent *);

    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void timerEvent(QTimerEvent *);

    bool uses3D() const;

private:
    Q_DISABLE_COPY(QToolButton)
    Q_PRIVATE_SLOT(d, void popupPressed())

#ifdef QT_COMPAT
public:
    enum TextPosition {
        BesideIcon,
        BelowIcon
        , Right = BesideIcon,
        Under = BelowIcon
    };

    QT_COMPAT_CONSTRUCTOR QToolButton(QWidget * parent, const char* name);
    QT_COMPAT_CONSTRUCTOR QToolButton(Qt::ArrowType type, QWidget *parent, const char* name);
    QT_COMPAT_CONSTRUCTOR QToolButton( const QIcon& s, const QString &textLabel,
                                       const QString& grouptext,
                                       QObject * receiver, const char* slot,
                                       QWidget * parent, const char* name=0 );
    inline QT_COMPAT void setPixmap(const QPixmap &pixmap) { setIcon(static_cast<QIcon>(pixmap)); }
    QT_COMPAT void setOnIconSet(const QIcon&);
    QT_COMPAT void setOffIconSet(const QIcon&);
    inline QT_COMPAT void setIconSet(const QIcon &icon){setIcon(icon);}
    QT_COMPAT void setIconSet(const QIcon &, bool on);
    inline QT_COMPAT void setTextLabel(const QString &text, bool tooltip = true) { setText(text); if (tooltip)setToolTip(text);}
    inline QT_COMPAT QString textLabel() const { return text(); }
    QT_COMPAT QIcon onIconSet() const;
    QT_COMPAT QIcon offIconSet() const;
    QT_COMPAT QIcon iconSet(bool on) const;
    inline QT_COMPAT QIcon iconSet() const { return icon(); }
    inline QT_COMPAT void openPopup()  { showMenu(); }
    inline QT_COMPAT void setPopup(QMenu* popup) {setMenu(popup); }
    inline QT_COMPAT QMenu* popup() const { return menu(); }
    inline QT_COMPAT bool usesBigPixmap() const { return iconSize() == Qt::LargeIconSize; }
    inline QT_COMPAT bool usesTextLabel() const { return toolButtonStyle() != Qt::ToolButtonIconOnly; }
    inline QT_COMPAT TextPosition textPosition() const
    { return toolButtonStyle() == Qt::ToolButtonTextUnderIcon ? BelowIcon : BesideIcon; }

public slots:
    QT_MOC_COMPAT void setUsesBigPixmap(bool enable) { enable ? setIconSize(Qt::LargeIconSize) : setIconSize(Qt::SmallIconSize); }
    QT_MOC_COMPAT void setUsesTextLabel(bool enable) { enable ? setToolButtonStyle(Qt::ToolButtonTextUnderIcon)
                                                              : setToolButtonStyle(Qt::ToolButtonIconOnly); }
    QT_MOC_COMPAT void setTextPosition(TextPosition pos) { pos == BesideIcon ? setToolButtonStyle(Qt::ToolButtonTextBesideIcon)
                                                                             : setToolButtonStyle(Qt::ToolButtonTextUnderIcon); }

#endif
};

#endif // QT_NO_TOOLBUTTON

#endif // QTOOLBUTTON_H
