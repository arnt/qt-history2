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

#ifndef QABSTRACTBUTTON_H
#define QABSTRACTBUTTON_H

#ifndef QT_H
#include "qwidget.h"
#include "qiconset.h"
#include "qkeysequence.h"
#endif // QT_H


class QButtonGroup;
class QAbstractButtonPrivate;

class Q_GUI_EXPORT QAbstractButton : public QWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QAbstractButton)
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QIconSet icon READ icon WRITE setIcon)
    Q_PROPERTY(QKeySequence shortcut READ shortcut WRITE setShortcut)
    Q_PROPERTY(bool checkable READ isCheckable WRITE setCheckable)
    Q_PROPERTY(bool checked READ isChecked WRITE setChecked)
    Q_PROPERTY(bool autoRepeat READ autoRepeat WRITE setAutoRepeat)
    Q_PROPERTY(bool autoExclusive READ autoExclusive WRITE setAutoExclusive DESIGNABLE false)
    QDOC_PROPERTY(bool down READ isDown WRITE setDown)

public:
    QAbstractButton(QWidget* parent=0);
    ~QAbstractButton();

    void setText(const QString &text);
    QString text() const;

    void setIcon(const QIconSet &icon);
    QIconSet icon() const;

    void setShortcut(const QKeySequence &key);
    QKeySequence shortcut() const;

    void setCheckable(bool);
    bool isCheckable() const;

    bool isChecked() const;

    void setDown(bool);
    bool isDown() const;

    void setAutoRepeat(bool);
    bool autoRepeat() const;

    void setAutoExclusive(bool);
    bool autoExclusive() const;

    QButtonGroup *group() const;

public slots:
    void animateClick(int msec = 100);
    void click();
    void toggle();
    void setChecked(bool);

signals:
    void pressed();
    void released();
    void clicked();
    void toggled(bool checked);

protected:
    QAbstractButton(QAbstractButtonPrivate &, QWidget* parent);

    virtual void paintEvent(QPaintEvent *e) = 0;
    virtual bool hitButton(const QPoint &pos) const;
    virtual void checkStateSet();
    virtual void nextCheckState();

    bool event(QEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);
    void changeEvent(QEvent *e);
    void timerEvent(QTimerEvent *e);

public:
#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QAbstractButton(QWidget *parent, const char *name, Qt::WFlags f=0);
    inline QT_COMPAT bool isOn() const { return isChecked(); }
    inline QT_COMPAT QPixmap *pixmap() const { return 0; } // help styles compile
    inline QT_COMPAT void setPixmap( const QPixmap &p ) { setIcon(QIconSet(p)); }
    QT_COMPAT QIconSet *iconSet() const;
    inline QT_COMPAT void setIconSet(const QIconSet &icon){ setIcon(icon); }
public slots:
    inline QT_MOC_COMPAT void setOn(bool b) { setChecked(b); }
public:
    inline QT_COMPAT bool isToggleButton() const { return isCheckable(); }
    inline QT_COMPAT void setToggleButton(bool b) { setCheckable(b); }
    inline QT_COMPAT void setAccel(const QKeySequence &key) { setShortcut(key); }
    inline QT_COMPAT QKeySequence accel() const { return shortcut(); }
#endif

private:
    friend class QButtonGroup;
};


#endif // QABSTRACTBUTTON_H
