#include <QtGui>

#include "flowlayout.h"
#include "window.h"

Window::Window()
{
    FlowLayout *flowLayout = new FlowLayout(this);

    flowLayout->addWidget(new QPushButton(tr("Short"), this));
    flowLayout->addWidget(new QPushButton(tr("Longer"), this));
    flowLayout->addWidget(new QPushButton(tr("Different text"), this));
    flowLayout->addWidget(new QPushButton(tr("More text"), this));
    flowLayout->addWidget(new QPushButton(tr("Even longer button text"), this));

    setWindowTitle(tr("Flow Layout"));
}
