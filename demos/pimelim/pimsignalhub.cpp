#include "pimsignalhub.h"
#include "pimeditor.h"
#include "pimviewer.h"
#include "pimmodel.h"
#include <qabstractitemview.h>
#include <qstackedwidget.h>

PimSignalHub::PimSignalHub(QObject *parent)
    : QObject(parent), s(0), a(0), m(0), e(0), v(0)
{

}

PimSignalHub::~PimSignalHub()
{

}

void PimSignalHub::setStack(QStackedWidget *stack)
{
    s = stack;
}

QStackedWidget *PimSignalHub::stack() const
{
    return s;
}

void PimSignalHub::setView(QAbstractItemView *view)
{
    a = view;
}

QAbstractItemView *PimSignalHub::view() const
{
    return a;
}

void PimSignalHub::setModel(PimModel *model)
{
    m = model;
}

PimModel *PimSignalHub::model() const
{
    return m;
}

void PimSignalHub::setEditor(PimEditor *editor)
{
    e = editor;
}

PimEditor *PimSignalHub::editor() const
{
    return e;
}

void PimSignalHub::setViewer(PimViewer *viewer)
{
    v = viewer;
}

PimViewer *PimSignalHub::viewer() const
{
    return v;
}

void PimSignalHub::createNew()
{
    if (s && e) {
        setCurrent(e);
        e->create();
    }
}

void PimSignalHub::viewCurrent()
{
    QModelIndex index = a ? a->currentIndex() : QModelIndex();
    if (s && v && index.isValid()) {
        setCurrent(v);
        v->view(index);
    }
}

void PimSignalHub::editCurrent()
{
    QModelIndex index = a ? a->currentIndex() : QModelIndex();
    if (s && e &&  index.isValid()) {
        setCurrent(e);
        e->edit(index);
    }
}

void PimSignalHub::deleteCurrent()
{
    QModelIndex index = a ? a->currentIndex() : QModelIndex();
    if (s && m && index.isValid())
        m->removeEntry(index.row());
}

void PimSignalHub::setDefault()
{
    if (s && a)
        setCurrent(a);
}

void PimSignalHub::setCurrent(QWidget *w)
{
    int idx = s->indexOf(w);
    s->setCurrentIndex(idx);
}
