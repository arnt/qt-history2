#ifndef QDOCKWINDOW_H
#define QDOCKWINDOW_H

#include <qframe.h>

class QMainWindow;
class QDockWindowPrivate;

class Q_GUI_EXPORT QDockWindow : public QFrame
{
    Q_OBJECT

    Q_PROPERTY(bool closable READ isClosable WRITE setClosable)
    Q_PROPERTY(bool movable READ isMovable WRITE setMovable)
    Q_PROPERTY(bool floatable READ isFloatable WRITE setFloatable)
    Q_PROPERTY(Qt::DockWindowAreaFlags allowedAreas READ allowedAreas WRITE setAllowedAreas)
    Q_PROPERTY(Qt::DockWindowArea area READ area WRITE setArea)

    Q_DECLARE_PRIVATE(QDockWindow)

public:
    QDockWindow(QMainWindow *parent, Qt::WFlags flags = 0);
    QDockWindow(QMainWindow *parent, Qt::DockWindowArea area, Qt::WFlags flags = 0);
    ~QDockWindow();

    void setParent(QMainWindow *parent);
    QMainWindow *mainWindow() const;

    void setClosable(bool closable = true);
    bool isClosable() const;

    void setMovable(bool movable = true);
    bool isMovable() const;

    void setFloatable(bool floatable = true);
    bool isFloatable() const;

    void setFloated(bool floated = true, const QPoint &pos = QPoint());
    bool isFloated() const;

    void setAllowedAreas(Qt::DockWindowAreaFlags areas);
    Qt::DockWindowAreaFlags allowedAreas() const;

    inline bool isDockable(Qt::DockWindowArea area)
    { return (allowedAreas() & area) == area; }

    Qt::DockWindowArea area() const;

    void setArea(Qt::DockWindowArea area); // always extends
    void setArea(Qt::DockWindowArea area, Qt::Orientation direction, bool extend = false);
    void setArea(QDockWindow *after, Qt::Orientation direction); // always splits

protected:
    void changeEvent(QEvent *event);
    void childEvent(QChildEvent *event);
    void closeEvent(QCloseEvent *event);
    bool event(QEvent *event);
};

#endif // QDOCKWINDOW_H
