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

#ifndef QTOOLBAR_H
#define QTOOLBAR_H

#include <QtGui/qwidget.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_TOOLBAR

class QToolBarPrivate;

class QAction;
class QIcon;
class QMainWindow;

class Q_GUI_EXPORT QToolBar : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(bool movable READ isMovable WRITE setMovable
               DESIGNABLE (qobject_cast<QMainWindow *>(parentWidget()) != 0)
               NOTIFY movableChanged)
    Q_PROPERTY(Qt::ToolBarAreas allowedAreas READ allowedAreas WRITE setAllowedAreas
               DESIGNABLE (qobject_cast<QMainWindow *>(parentWidget()) != 0)
               NOTIFY allowedAreasChanged)
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation
               DESIGNABLE (qobject_cast<QMainWindow *>(parentWidget()) == 0)
               NOTIFY orientationChanged)
    Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize NOTIFY iconSizeChanged)
    Q_PROPERTY(Qt::ToolButtonStyle toolButtonStyle READ toolButtonStyle WRITE setToolButtonStyle
               NOTIFY toolButtonStyleChanged)

public:
    explicit QToolBar(const QString &title, QWidget *parent = 0);
    explicit QToolBar(QWidget *parent = 0);
    ~QToolBar();

    void setMovable(bool movable);
    bool isMovable() const;

    void setAllowedAreas(Qt::ToolBarAreas areas);
    Qt::ToolBarAreas allowedAreas() const;

    inline bool isAreaAllowed(Qt::ToolBarArea area) const
    { return (allowedAreas() & area) == area; }

    void setOrientation(Qt::Orientation orientation);
    Qt::Orientation orientation() const;

    void clear();

#ifdef Q_NO_USING_KEYWORD
    inline void addAction(QAction *action)
    { QWidget::addAction(action); }
#else
    using QWidget::addAction;
#endif

    QAction *addAction(const QString &text);
    QAction *addAction(const QIcon &icon, const QString &text);
    QAction *addAction(const QString &text, const QObject *receiver, const char* member);
    QAction *addAction(const QIcon &icon, const QString &text,
		       const QObject *receiver, const char* member);

    QAction *addSeparator();
    QAction *insertSeparator(QAction *before);

    QAction *addWidget(QWidget *widget);
    QAction *insertWidget(QAction *before, QWidget *widget);

    QRect actionGeometry(QAction *action) const;
    QAction *actionAt(const QPoint &p) const;
    inline QAction *actionAt(int x, int y) const;

    QAction *toggleViewAction() const;

    QSize iconSize() const;
    Qt::ToolButtonStyle toolButtonStyle() const;

    QWidget *widgetForAction(QAction *action) const;

public Q_SLOTS:
    void setIconSize(const QSize &iconSize);
    void setToolButtonStyle(Qt::ToolButtonStyle toolButtonStyle);

Q_SIGNALS:
    void actionTriggered(QAction *action);
    void movableChanged(bool movable);
    void allowedAreasChanged(Qt::ToolBarAreas allowedAreas);
    void orientationChanged(Qt::Orientation orientation);
    void iconSizeChanged(const QSize &iconSize);
    void toolButtonStyleChanged(Qt::ToolButtonStyle toolButtonStyle);

protected:
    void actionEvent(QActionEvent *event);
    void changeEvent(QEvent *event);
    void childEvent(QChildEvent *event);
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    bool event(QEvent *event);

#ifdef QT3_SUPPORT
public:
    QT3_SUPPORT_CONSTRUCTOR QToolBar(QWidget *parent, const char *name);
    inline QT3_SUPPORT void setLabel(const QString &label)
    { setWindowTitle(label); }
    inline QT3_SUPPORT QString label() const
    { return windowTitle(); }
#endif

private:
    Q_DECLARE_PRIVATE(QToolBar)
    Q_DISABLE_COPY(QToolBar)
    Q_PRIVATE_SLOT(d_func(), void _q_toggleView(bool))
    Q_PRIVATE_SLOT(d_func(), void _q_updateIconSize(const QSize &))
    Q_PRIVATE_SLOT(d_func(), void _q_updateToolButtonStyle(Qt::ToolButtonStyle))

    friend class QMainWindow;
};

inline QAction *QToolBar::actionAt(int ax, int ay) const
{ return actionAt(QPoint(ax, ay)); }

#endif // QT_NO_TOOLBAR

QT_END_HEADER

#endif // QTOOLBAR_H
