/****************************************************************************
**
** Definition of QGroupBox widget class.
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

#ifndef Q3GROUPBOX_H
#define Q3GROUPBOX_H

#ifndef QT_H
#include "qframe.h"
#endif // QT_H

class Q3GroupBoxPrivate;

class Q_COMPAT_EXPORT Q3GroupBox : public QWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Q3GroupBox)

    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation DESIGNABLE false)
    Q_PROPERTY(int columns READ columns WRITE setColumns DESIGNABLE false)
    Q_PROPERTY(bool flat READ isFlat WRITE setFlat)
    Q_PROPERTY(bool checkable READ isCheckable WRITE setCheckable)
    Q_PROPERTY(bool checked READ isChecked WRITE setChecked)
public:
    Q3GroupBox(QWidget* parent=0, const char* name=0);
    Q3GroupBox(const QString &title,
               QWidget* parent=0, const char* name=0);
    Q3GroupBox(int strips, Qt::Orientation o,
               QWidget* parent=0, const char* name=0);
    Q3GroupBox(int strips, Qt::Orientation o, const QString &title,
               QWidget* parent=0, const char* name=0);
    ~Q3GroupBox();

    virtual void setColumnLayout(int strips, Qt::Orientation o);

    QString title() const;
    virtual void setTitle(const QString &);

    int alignment() const;
    virtual void setAlignment(int);

    int columns() const;
    void setColumns(int);

    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation);

    int insideMargin() const;
    int insideSpacing() const;
    void setInsideMargin(int m);
    void setInsideSpacing(int s);

    void addSpace(int);
    QSize sizeHint() const;

    bool isFlat() const;
    void setFlat(bool b);
    bool isCheckable() const;
    void setCheckable(bool b);
    bool isChecked() const;

#ifdef QT_COMPAT
    inline QT_COMPAT int margin() const { return insideMargin(); }
    inline QT_COMPAT void setMargin(int m) { setInsideMargin(m); }
#endif

public slots:
    void setChecked(bool b);

signals:
    void toggled(bool);

protected:
    bool event(QEvent *);
    void childEvent(QChildEvent *);
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);
    void focusInEvent(QFocusEvent *);
    void changeEvent(QEvent *);

private:

    Q_PRIVATE_SLOT(void fixFocus())
    Q_PRIVATE_SLOT(void setChildrenEnabled(bool b))

private:

#if defined(Q_DISABLE_COPY)
    Q3GroupBox(const Q3GroupBox &);
    Q3GroupBox &operator=(const Q3GroupBox &);
#endif
};


#endif // Q3GROUPBOX_H
