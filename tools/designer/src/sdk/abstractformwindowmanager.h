#ifndef ABSTRACTFORMWINDOWMANAGER_H
#define ABSTRACTFORMWINDOWMANAGER_H

#include "sdk_global.h"
#include "abstractformwindow.h"

#include <QObject>

class QAction;
class AbstractFormEditor;
class DomUI;
class QWidget;

class QT_SDK_EXPORT AbstractDnDItem : public QObject
{
    Q_OBJECT
public:
    virtual DomUI *domUi() const = 0;
    virtual QWidget *decoration() const = 0;
    virtual QPoint hotSpot() const = 0;
    inline virtual ~AbstractDnDItem() {}
};

class QT_SDK_EXPORT AbstractFormWindowManager: public QObject
{
    Q_OBJECT
public:
    AbstractFormWindowManager(QObject *parent = 0);
    virtual ~AbstractFormWindowManager();

    virtual QAction *actionCut() const;
    virtual QAction *actionCopy() const;
    virtual QAction *actionPaste() const;
    virtual QAction *actionDelete() const;
    virtual QAction *actionSelectAll() const;
    virtual QAction *actionLower() const;
    virtual QAction *actionRaise() const;
    virtual QAction *actionUndo() const;
    virtual QAction *actionRedo() const;

    virtual QAction *actionHorizontalLayout() const;
    virtual QAction *actionVerticalLayout() const;
    virtual QAction *actionSplitHorizontal() const;
    virtual QAction *actionSplitVertical() const;
    virtual QAction *actionGridLayout() const;
    virtual QAction *actionBreakLayout() const;
    virtual QAction *actionAdjustSize() const;

    virtual AbstractFormWindow *activeFormWindow() const;

    virtual int formWindowCount() const;
    virtual AbstractFormWindow *formWindow(int index) const;

    virtual AbstractFormWindow *createFormWindow(QWidget *parentWidget = 0, Qt::WFlags flags = 0);

    virtual AbstractFormEditor *core() const;

    virtual void dragItems(const QList<AbstractDnDItem*> &item_list, AbstractFormWindow *source_form) = 0;

signals:
    void formWindowAdded(AbstractFormWindow *formWindow);
    void formWindowRemoved(AbstractFormWindow *formWindow);
    void activeFormWindowChanged(AbstractFormWindow *formWindow);
    void formWindowClosing(AbstractFormWindow *formWindow, bool *accept);

public slots:
    virtual void addFormWindow(AbstractFormWindow *formWindow);
    virtual void removeFormWindow(AbstractFormWindow *formWindow);
    virtual void setActiveFormWindow(AbstractFormWindow *formWindow);
};

#endif // ABSTRACTFORMWINDOWMANAGER_H

