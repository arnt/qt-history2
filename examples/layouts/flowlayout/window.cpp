#include <QtGui>

#include "flowlayout.h"
#include "window.h"

Window::Window()
{
    FlowLayout *flowLayout = new FlowLayout;

    flowLayout->addWidget(new QPushButton(tr("Short")));
    flowLayout->addWidget(new QPushButton(tr("Longer")));
    flowLayout->addWidget(new QPushButton(tr("Different text")));
    flowLayout->addWidget(new QPushButton(tr("More text")));
    flowLayout->addWidget(new QPushButton(tr("Even longer button text")));
    setLayout(flowLayout);

    setWindowTitle(tr("Flow Layout"));
}
