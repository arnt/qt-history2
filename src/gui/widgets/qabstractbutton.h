/****************************************************************************
**
** Definition of QAbstractButton widget class.
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

#ifndef QABSTRACTBUTTON_H
#define QABSTRACTBUTTON_H

#ifndef QT_H
#include "qwidget.h"
#include "qkeysequence.h"
#endif // QT_H


class Q4ButtonGroup;
class QAbstractButtonPrivate;

class Q_GUI_EXPORT QAbstractButton : public QWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QAbstractButton);
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(QPixmap pixmap READ pixmap WRITE setPixmap)
    Q_PROPERTY(QKeySequence mnemonic READ accel WRITE setMnemonic)
    Q_PROPERTY(bool checkable READ isCheckable WRITE setCheckable)
    Q_PROPERTY(bool checked READ isChecked WRITE setChecked)
    Q_PROPERTY(bool autoRepeat READ autoRepeat WRITE setAutoRepeat)
    Q_PROPERTY(bool autoExclusive READ autoExclusive WRITE setAutoExclusive DESIGNABLE false)

    QDOC_PROPERTY(bool down READ isDown WRITE setDown)

public:
    QAbstractButton(QWidget* parent=0);
    ~QAbstractButton();

    void setText(const QString &);
    QString text() const;

    void setPixmap(const QPixmap &);
    QPixmap pixmap() const;

    void setMnemonic(const QKeySequence&);
    QKeySequence mnemonic() const;

    void setCheckable(bool);
    bool isCheckable() const;

    bool isChecked() const;

    void setDown(bool);
    bool isDown() const;

    void setAutoRepeat(bool);
    bool autoRepeat() const;

    void setAutoExclusive(bool);
    bool autoExclusive() const;

    Q4ButtonGroup *group() const;

    QSize sizeHint() const;

public slots:
    void animateClick(int msec = 100);
    void click();
    void toggle();
    void setChecked(bool);

signals:
    void pressed();
    void released();
    void clicked();
    void toggled(bool);

protected:
    QAbstractButton(QAbstractButtonPrivate &, QWidget* parent);
    virtual bool hitButton(const QPoint &pos) const;

    virtual void checkStateSet();
    virtual void nextCheckState();

    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    void changeEvent(QEvent *);
    void timerEvent(QTimerEvent *);

public:
#ifdef QT_COMPAT
    QAbstractButton(QWidget *parent, const char *name, WFlags f=0);
    inline QT_COMPAT bool isOn() const { return isChecked(); }
public slots:
    inline QT_COMPAT void setOn(bool b) { setChecked(b); }
public:
    inline QT_COMPAT bool isToggleButton() const { return isCheckable(); }
    inline QT_COMPAT void setToggleButton(bool b) { setCheckable(b); }
    inline QT_COMPAT void setAccel(const QKeySequence &key) { setMnemonic(key); }
    inline QT_COMPAT QKeySequence accel() const { return mnemonic(); }
#endif

private:
    friend class Q4ButtonGroup;
};


#endif // QABSTRACTBUTTON_H
