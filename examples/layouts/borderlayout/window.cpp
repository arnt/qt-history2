#include <QtGui>

#include "borderlayout.h"
#include "window.h"

Window::Window()
{
    QTextBrowser *centralWidget = new QTextBrowser(this);
    centralWidget->setPlainText(tr("Central widget"));

    BorderLayout *layout = new BorderLayout(this);
    layout->addWidget(centralWidget, BorderLayout::Center);
    layout->addWidget(createLabel("North"), BorderLayout::North);
    layout->addWidget(createLabel("West"), BorderLayout::West);
    layout->addWidget(createLabel("East 1"), BorderLayout::East);
    layout->addWidget(createLabel("East 2") , BorderLayout::East);
    layout->addWidget(createLabel("South"), BorderLayout::South);

    setWindowTitle(tr("Border Layout"));
}

QLabel *Window::createLabel(const QString &text)
{
    QLabel *label = new QLabel(text, this);
    label->setFrameStyle(QFrame::Box | QFrame::Raised);
    return label;
}
