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

#ifndef QDOCKWINDOW_H
#define QDOCKWINDOW_H

#include <qframe.h>

class QMainWindow;
class QDockWindowPrivate;

class Q_GUI_EXPORT QDockWindow : public QFrame
{
    Q_DECLARE_PRIVATE(QDockWindow)
    Q_OBJECT

    Q_FLAGS(DockWindowFeature)
    Q_PROPERTY(DockWindowFeatures features READ features WRITE setFeatures)
    Q_PROPERTY(Qt::DockWindowAreas allowedAreas READ allowedAreas WRITE setAllowedAreas)
    Q_PROPERTY(Qt::DockWindowArea area READ area WRITE setArea)

public:
    enum DockWindowFeature {
        DockWindowClosable    = 0x01,
        DockWindowMovable     = 0x02,
        DockWindowFloatable   = 0x04,

        DockWindowFeatureMask = 0x07,
        AllDockWindowFeatures = DockWindowFeatureMask,
        NoDockWindowFeatures  = 0x00
    };
    Q_DECLARE_FLAGS(DockWindowFeatures, DockWindowFeature)

    QDockWindow(QMainWindow *parent, Qt::WFlags flags = 0);
    QDockWindow(QMainWindow *parent, Qt::DockWindowArea area, Qt::WFlags flags = 0);
    ~QDockWindow();

    QMainWindow *mainWindow() const;
    void setParent(QMainWindow *parent);

    QWidget *widget() const;
    void setWidget(QWidget *widget);

    void setFeatures(DockWindowFeatures features);
    void setFeature(DockWindowFeature features, bool on = true);
    DockWindowFeatures features() const;
    bool hasFeature(DockWindowFeature feature) const;

    void setTopLevel(bool topLevel = true, const QPoint &pos = QPoint());

    void setAllowedAreas(Qt::DockWindowAreas areas);
    Qt::DockWindowAreas allowedAreas() const;

    inline bool isDockable(Qt::DockWindowArea area)
    { return (allowedAreas() & area) == area; }

    Qt::DockWindowArea area() const;

    void setArea(Qt::DockWindowArea area); // always extends
    void setArea(Qt::DockWindowArea area, Qt::Orientation direction, bool extend = false);
    void setArea(QDockWindow *after, Qt::Orientation direction); // always splits

protected:
    void changeEvent(QEvent *event);
    void closeEvent(QCloseEvent *event);
    bool event(QEvent *event);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QDockWindow::DockWindowFeatures)

#endif // QDOCKWINDOW_H
