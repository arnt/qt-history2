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

#ifndef QT_H
#include "qabstractbutton.h"
#endif // QT_H

#ifndef QT_NO_TOOLBUTTON

class QToolButtonPrivate;
class QMenu;

class Q_GUI_EXPORT QToolButton : public QAbstractButton
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QToolButton)
    Q_ENUMS(TextPosition)

    Q_PROPERTY(bool usesBigPixmap READ usesBigPixmap WRITE setUsesBigPixmap)
    Q_PROPERTY(bool usesTextLabel READ usesTextLabel WRITE setUsesTextLabel)
    Q_PROPERTY(int popupDelay READ popupDelay WRITE setPopupDelay)
    Q_PROPERTY(bool autoRaise READ autoRaise WRITE setAutoRaise)
    Q_PROPERTY(TextPosition textPosition READ textPosition WRITE setTextPosition)
    Q_OVERRIDE(Qt::BackgroundMode backgroundMode DESIGNABLE true)

public:
    enum TextPosition {
        BesideIcon,
        BelowIcon,
        Right = BesideIcon, // obsolete
        Under = BelowIcon // obsolete
    };
    QToolButton(QWidget * parent=0);

    QToolButton(Qt::ArrowType type, QWidget *parent=0);
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
    void setUsesBigPixmap(bool enable);
    void setUsesTextLabel(bool enable);
    void setTextPosition(TextPosition pos);

protected:
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
    QT_COMPAT_CONSTRUCTOR QToolButton(QWidget * parent, const char* name);
    QT_COMPAT_CONSTRUCTOR QToolButton(Qt::ArrowType type, QWidget *parent, const char* name);
    QT_COMPAT_CONSTRUCTOR QToolButton( const QIconSet& s, const QString &textLabel,
                                       const QString& grouptext,
                                       QObject * receiver, const char* slot,
                                       QWidget * parent, const char* name=0 );
    inline QT_COMPAT void setPixmap(const QPixmap &pixmap) { setIcon(static_cast<QIconSet>(pixmap)); }
    QT_COMPAT void setOnIconSet(const QIconSet&);
    QT_COMPAT void setOffIconSet(const QIconSet&);
    inline QT_COMPAT void setIconSet(const QIconSet &icon){setIcon(icon);}
    QT_COMPAT void setIconSet(const QIconSet &, bool on);
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
};

#endif // QT_NO_TOOLBUTTON

#endif // QTOOLBUTTON_H
