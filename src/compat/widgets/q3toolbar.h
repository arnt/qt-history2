/****************************************************************************
**
** Definition of Q3ToolBar class.
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

#ifndef QTOOLBAR_H
#define QTOOLBAR_H

#ifndef QT_H
#include "q3dockwindow.h"
#endif // QT_H

#ifndef QT_NO_TOOLBAR

class Q3MainWindow;
class Q3ToolBarPrivate;

class Q_COMPAT_EXPORT Q3ToolBar: public Q3DockWindow
{
    Q_OBJECT
    Q_PROPERTY(QString label READ label WRITE setLabel)

public:
    Q3ToolBar(const QString &label,
              Q3MainWindow *, ToolBarDock = DockTop,
              bool newLine = false, const char* name=0);
    Q3ToolBar(const QString &label, Q3MainWindow *, QWidget *,
              bool newLine = false, const char* name=0, WFlags f = 0);
    Q3ToolBar(Q3MainWindow* parent=0, const char* name=0);
    ~Q3ToolBar();

    void addSeparator();

    void show();
    void hide();

    Q3MainWindow * mainWindow() const;

    virtual void setStretchableWidget(QWidget *);

    bool event(QEvent * e);

    virtual void setLabel(const QString &);
    QString label() const;

    virtual void clear();

    QSize minimumSize() const;
    QSize minimumSizeHint() const;

    void setOrientation(Orientation o);
    void setMinimumSize(int minw, int minh);

protected:
    void resizeEvent(QResizeEvent *e);
    void styleChange(QStyle &);
    void actionEvent(QActionEvent *);

private slots:
    void createPopup();

private:
    void init();
    void checkForExtension(const QSize &sz);
    Q3ToolBarPrivate * d;
    Q3MainWindow * mw;
    QWidget * sw;
    QString l;

    friend class Q3MainWindow;

private:        // Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    Q3ToolBar(const Q3ToolBar &);
    Q3ToolBar& operator=(const Q3ToolBar &);
#endif
};

#endif // QT_NO_TOOLBAR

#endif // QTOOLBAR_H
