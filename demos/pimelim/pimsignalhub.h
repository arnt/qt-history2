#ifndef PIMSIGNALHUB_H
#define PIMSIGNALHUB_H

#include <qobject.h>

class QStackedWidget;
class QAbstractItemView;
class PimModel;
class PimEditor;
class PimViewer;

class PimSignalHub : public QObject
{
    Q_OBJECT
public:
    PimSignalHub(QObject *parent = 0);
    ~PimSignalHub();

    void setStack(QStackedWidget *stack);
    QStackedWidget *stack() const;

    void setView(QAbstractItemView *view);
    QAbstractItemView *view() const;

    void setModel(PimModel *model);
    PimModel *model() const;

    void setEditor(PimEditor *editor);
    PimEditor *editor() const;

    void setViewer(PimViewer *viewer);
    PimViewer *viewer() const;

public slots:
    void createNew();
    void viewCurrent();
    void editCurrent();
    void deleteCurrent();
    void setDefault();

private:
    void setCurrent(QWidget *w);
    QStackedWidget *s;
    QAbstractItemView *a;
    PimModel *m;
    PimEditor *e;
    PimViewer *v;
};

#endif //PIMSIGNALHUB_H
