#ifndef QTOOLBAR_H
#define QTOOLBAR_H

#include <qframe.h>

class QToolBarPrivate;

class QAction;
class QIconSet;
class QMainWindow;

class Q_GUI_EXPORT QToolBar : public QFrame
{
    Q_DECLARE_PRIVATE(QToolBar);
    Q_OBJECT

    Q_PROPERTY(bool movable READ isMovable WRITE setMovable)
    Q_PROPERTY(Qt::ToolBarAreas allowedAreas READ allowedAreas WRITE setAllowedAreas)
    Q_PROPERTY(Qt::ToolBarArea area READ area WRITE setArea)

public:
    QToolBar(QMainWindow *parent);
    ~QToolBar();

    void setParent(QMainWindow *parent);
    QMainWindow *mainWindow() const;

    void setMovable(bool movable = true);
    bool isMovable() const;

    void setAllowedAreas(Qt::ToolBarAreas areas);
    Qt::ToolBarAreas allowedAreas() const;

    inline bool isDockable(Qt::ToolBarArea area)
    { return (allowedAreas() & area) == area; }

    void setArea(Qt::ToolBarArea area, bool linebreak = false);
    Qt::ToolBarArea area() const;

    void addAction(QAction *action);
    QAction *addAction(const QString &text);
    QAction *addAction(const QIconSet &icon, const QString &text);
    QAction *addAction(const QString &text, const QObject *receiver, const char* member);
    QAction *addAction(const QIconSet &icon, const QString &text,
		       const QObject *receiver, const char* member);

    void insertAction(int index, QAction *action);
    QAction *insertAction(int index, const QString &text);
    QAction *insertAction(int index, const QIconSet &icon, const QString &text);
    QAction *insertAction(int index, const QString &text,
			  const QObject *receiver, const char* member);
    QAction *insertAction(int index, const QIconSet &icon, const QString &text,
			  const QObject *receiver, const char* member);

    QAction *addSeparator();
    QAction *insertSeparator(int index);

    QAction *action(int index) const;
    int indexOf(QAction *action) const;

    QAction *actionAt(int x, int y) const;
    inline QAction *actionAt(const QPoint &p) const
    { return actionAt(p.x(), p.y()); }

    void addWidget(QWidget *widget);
    void insertWidget(int index, QWidget *widget);
    void removeWidget(QWidget *widget);

    QWidget *widget(int index) const;
    int indexOf(QWidget *widget) const;

#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QToolBar(QMainWindow *parent, const char *name);
    inline QT_COMPAT void setLabel(const QString &label)
    { setWindowTitle(label); }
    inline QT_COMPAT QString label() const
    { return windowTitle(); }
#endif

signals:
    void actionTriggered(QAction *action);

protected:
    void actionEvent(QActionEvent *event);
    void childEvent(QChildEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    Q_PRIVATE_SLOT(void actionTriggered());
};

#endif // QTOOLBAR_H
