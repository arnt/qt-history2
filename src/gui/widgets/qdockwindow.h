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

#include <QtGui/qframe.h>

class QDockWindowLayout;
class QDockWindowPrivate;
class QMainWindow;

class Q_GUI_EXPORT QDockWindow : public QFrame
{
    Q_OBJECT

    Q_FLAGS(DockWindowFeatures)
    Q_PROPERTY(DockWindowFeatures features READ features WRITE setFeatures)
    Q_PROPERTY(Qt::DockWindowAreas allowedAreas READ allowedAreas WRITE setAllowedAreas)

public:
    QDockWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~QDockWindow();

    QWidget *widget() const;
    void setWidget(QWidget *widget);

    enum DockWindowFeature {
        DockWindowClosable    = 0x01,
        DockWindowMovable     = 0x02,
        DockWindowFloatable   = 0x04,

        DockWindowFeatureMask = 0x07,
        AllDockWindowFeatures = DockWindowFeatureMask,
        NoDockWindowFeatures  = 0x00
    };
    Q_DECLARE_FLAGS(DockWindowFeatures, DockWindowFeature)

    void setFeatures(DockWindowFeatures features);
    void setFeature(DockWindowFeature features, bool on = true);
    DockWindowFeatures features() const;
    bool hasFeature(DockWindowFeature feature) const;

    void setTopLevel(bool topLevel = true, const QPoint &pos = QPoint());

    void setAllowedAreas(Qt::DockWindowAreas areas);
    Qt::DockWindowAreas allowedAreas() const;

    inline bool isDockable(Qt::DockWindowArea area)
    { return (allowedAreas() & area) == area; }

    QAction *toggleViewAction() const;

protected:
    void changeEvent(QEvent *event);
    void closeEvent(QCloseEvent *event);
    bool event(QEvent *event);

private:
    Q_DECLARE_PRIVATE(QDockWindow)
    Q_DISABLE_COPY(QDockWindow)
    Q_PRIVATE_SLOT(d, void toggleView(bool))
    friend class QDockWindowLayout;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QDockWindow::DockWindowFeatures)

#endif // QDOCKWINDOW_H
