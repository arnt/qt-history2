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
#include "qabstractbutton.h"
#endif // QT_H

#ifndef QT_NO_PUSHBUTTON
class QPushButtonPrivate;
class QMenu;

class Q_GUI_EXPORT QPushButton : public QAbstractButton
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QPushButton);
    Q_PROPERTY(bool autoDefault READ autoDefault WRITE setAutoDefault)
    Q_PROPERTY(bool default READ isDefault WRITE setDefault)
    Q_PROPERTY(bool flat READ isFlat WRITE setFlat)
    Q_OVERRIDE(bool autoMask DESIGNABLE true SCRIPTABLE true)

public:
    QPushButton(QWidget *parent=0);
    QPushButton(const QString &text, QWidget *parent=0);
    QPushButton(const QIconSet& icon, const QString &text, QWidget *parent=0);
    ~QPushButton();

    QSize sizeHint() const;

    bool autoDefault() const;
    void setAutoDefault(bool);
    bool isDefault() const;
    void setDefault(bool);

    void setMenu(QMenu* menu);
    QMenu* menu() const;

    void setFlat(bool);
    bool isFlat() const;

public slots:
    void popupMenu();

protected:
    void drawBevel(QPainter *);
    void drawLabel(QPainter *);
    void paintEvent(QPaintEvent *);
    void keyPressEvent(QKeyEvent *);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    void updateMask();
private:
    Q_PRIVATE_SLOT(void popupPressed())
    friend class QDialog;

public:
#ifdef QT_COMPAT
    QPushButton(QWidget *parent, const char* name);
    QPushButton(const QString &text, QWidget *parent, const char* name);
    QPushButton(const QIconSet& icon, const QString &text, QWidget *parent, const char* name);
    inline QT_COMPAT void openPopup()  { popupMenu(); }
    inline QT_COMPAT bool isMenuButton() const { return popup() !=  0; }
    inline QT_COMPAT void setPopup(QMenu* popup) {setMenu(popup); }
    inline QT_COMPAT QMenu* popup() const { return menu(); }
#endif
private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QPushButton(const QPushButton &);
    QPushButton &operator=(const QPushButton &);
#endif
};


#endif // QT_NO_PUSHBUTTON

#endif // QPUSHBUTTON_H
