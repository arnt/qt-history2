#ifndef QDOCKWINDOW_H
#define QDOCKWINDOW_H

#include <qframe.h>

class Q4MainWindow;
class Q4DockWindowPrivate;

class Q_GUI_EXPORT Q4DockWindow : public QFrame
{
    Q_OBJECT

    Q_PROPERTY(bool closable READ isClosable WRITE setClosable)
    Q_PROPERTY(bool movable READ isMovable WRITE setMovable)
    Q_PROPERTY(bool floatable READ isFloatable WRITE setFloatable)
    Q_PROPERTY(DockWindowAreaFlags allowedAreas READ allowedAreas WRITE setAllowedAreas)
    Q_PROPERTY(DockWindowArea currentArea READ currentArea WRITE setCurrentArea)

    Q_DECLARE_PRIVATE(Q4DockWindow)

public:
    Q4DockWindow(Q4MainWindow *parent, WFlags flags = 0);
    Q4DockWindow(Q4MainWindow *parent, DockWindowArea area, WFlags flags = 0);
    ~Q4DockWindow();

    void setParent(Q4MainWindow *parent);
    Q4MainWindow *mainWindow() const;

    void setClosable(bool closable = true);
    bool isClosable() const;

    void setMovable(bool movable = true);
    bool isMovable() const;

    void setFloatable(bool floatable = true);
    bool isFloatable() const;

    void setFloated(bool floated = true, const QPoint &pos = QPoint());
    bool isFloated() const;

    void setAllowedAreas(DockWindowAreaFlags areas);
    DockWindowAreaFlags allowedAreas() const;

    inline bool isDockable(DockWindowArea area)
    { return (allowedAreas() & area) == area; }

    DockWindowArea currentArea() const;

    void setCurrentArea(DockWindowArea area); // always extends
    void setCurrentArea(DockWindowArea area, Qt::Orientation direction, bool extend = false);

    void setCurrentArea(Q4DockWindow *after, Qt::Orientation direction); // always splits

protected:
    void changeEvent(QEvent *event);
    void childEvent(QChildEvent *event);
    bool event(QEvent *event);
};

#endif // QDOCKWINDOW_H
