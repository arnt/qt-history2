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

public:
    QToolBar(QMainWindow *parent);
    ~QToolBar();

    inline void addAction(QAction *action)
    { QWidget::addAction(action); }
    inline void insertAction(QAction *before, QAction *action)
    { QWidget::insertAction(before, action); }

    QAction *addAction(const QString &text);
    QAction *addAction(const QIconSet &icon, const QString &text);
    QAction *addAction(const QString &text, const QObject *receiver, const char* member);
    QAction *addAction(const QIconSet &icon, const QString &text,
		       const QObject *receiver, const char* member);
    QAction *insertAction(QAction *before, const QString &text);
    QAction *insertAction(QAction *before, const QIconSet &icon, const QString &text);
    QAction *insertAction(QAction *before, const QString &text,
			  const QObject *receiver, const char* member);
    QAction *insertAction(QAction *before, const QIconSet &icon, const QString &text,
			  const QObject *receiver, const char* member);

    QAction *addSeparator();
    QAction *insertSeparator(QAction *before);

    ToolBarArea currentArea() const;
    void setCurrentArea(ToolBarArea area, bool linebreak = false);

signals:
    void actionTriggered(QAction *action);

protected:
    void childEvent(QChildEvent *event);
    void actionEvent(QActionEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    Q_PRIVATE_SLOT(void actionTriggered());
};

#endif // QTOOLBAR_H
