#include "objectinspectorview.h"

// components
#include <objectinspector.h>

// sdk
#include <abstractformeditor.h>

ObjectInspectorView::ObjectInspectorView(AbstractFormEditor *core, QWidget *parent)
: QMainWindow(parent, Qt::WStyle_Tool),
      m_core(core)
{
    setWindowTitle(tr("Object Inspector"));

    ObjectInspector *editor = new ObjectInspector(core, this);
    setCentralWidget(editor);

    core->setObjectInspector(editor);

    (void) statusBar();
}

ObjectInspectorView::~ObjectInspectorView()
{
}

void ObjectInspectorView::hideEvent(QHideEvent *ev)
{
    emit visibilityChanged(false);
    QMainWindow::hideEvent(ev);
}

void ObjectInspectorView::showEvent(QShowEvent *ev)
{
    emit visibilityChanged(true);
    QMainWindow::showEvent(ev);
}
