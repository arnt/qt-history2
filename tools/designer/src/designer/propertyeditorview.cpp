
#include "propertyeditorview.h"

// components
#include <propertyeditor.h>

PropertyEditorView::PropertyEditorView(AbstractFormEditor *core, QWidget *parent)
: QMainWindow(parent, Qt::WStyle_Tool),
      m_core(core)
{
    setWindowTitle(tr("Property Editor"));

    PropertyEditor *editor = new PropertyEditor(core, this);
    setCentralWidget(editor);

    core->setPropertyEditor(editor);

    (void) statusBar();
}

PropertyEditorView::~PropertyEditorView()
{
}

void PropertyEditorView::showEvent(QShowEvent *ev)
{
    emit visibilityChanged(true);
    QMainWindow::showEvent(ev);
}

void PropertyEditorView::hideEvent(QHideEvent *ev)
{
    emit visibilityChanged(false);
    QMainWindow::hideEvent(ev);
}
