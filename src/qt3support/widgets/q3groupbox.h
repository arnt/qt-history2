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

#ifndef Q3GROUPBOX_H
#define Q3GROUPBOX_H

#include "Qt3Support/q3frame.h"

class Q3Accel;
class Q3GroupBoxPrivate;
class QVBoxLayout;
class QGridLayout;
class QSpacerItem;

class Q_COMPAT_EXPORT Q3GroupBox : public Q3Frame
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation DESIGNABLE false)
    Q_PROPERTY(int columns READ columns WRITE setColumns DESIGNABLE false)
    Q_PROPERTY(bool flat READ isFlat WRITE setFlat)
    Q_PROPERTY(bool checkable READ isCheckable WRITE setCheckable)
    Q_PROPERTY(bool checked READ isChecked WRITE setChecked)
    Q_FLAGS(Qt::Alignment)
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

    QString title() const { return str; }
    virtual void setTitle(const QString &);

    int alignment() const { return align; }
    virtual void setAlignment(int);

    int columns() const;
    void setColumns(int);

    Qt::Orientation orientation() const { return dir; }
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
    void setEnabled(bool on);

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
    void fontChange(const QFont &);

private slots:
    void fixFocus();
    void setChildrenEnabled(bool b);

private:
    void skip();
    void init();
    void calculateFrame();
    void insertWid(QWidget*);
    void setTextSpacer();
#ifndef QT_NO_CHECKBOX
    void updateCheckBoxGeometry();
#endif
    QString str;
    int align;
    int lenvisible;
#ifndef QT_NO_ACCEL
    Q3Accel * accel;
#endif
    Q3GroupBoxPrivate * d;

    QVBoxLayout *vbox;
    QGridLayout *grid;
    int row;
    signed int col : 30;
    uint bFlat : 1;
    int nRows, nCols;
    Qt::Orientation dir;
    int spac, marg;

    Q_DISABLE_COPY(Q3GroupBox)
};

#endif // Q3GROUPBOX_H
