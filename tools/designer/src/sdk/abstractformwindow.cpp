#include "abstractformwindow.h"

#include <QMainWindow>

AbstractFormWindow::AbstractFormWindow(QWidget *parent, Qt::WFlags flags)
    : QWidget(parent, flags)
{
}

AbstractFormWindow::~AbstractFormWindow()
{
}

AbstractFormEditor *AbstractFormWindow::core() const
{
    return 0;
}

// This is very similar to the static FormWindow::findFormWindow(), please KEEP IN SYNC.
AbstractFormWindow *AbstractFormWindow::findFormWindow(QWidget *w)
{
    while (w) {
        if (AbstractFormWindow *fw = qt_cast<AbstractFormWindow*>(w)) {
            return fw;
        } else if (qt_cast<QMainWindow*>(w)) {
            /* skip */
        } else if (w->isTopLevel())
            break;
            
        w = w->parentWidget();
    }
    
    return 0;
}


