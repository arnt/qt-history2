#include <QtGui>

#include "optionbutton.h"
#include "pages.h"
#include "window.h"

Window::Window()
{
    contentsWidget = new QWidget;
    QWidgetView *view = new QWidgetView(this);
    view->setWidget(contentsWidget);

    // ### Change QStackedBox to QStackedWidget before TP2
    pagesWidget = new QStackedBox(this);
    pagesWidget->addWidget(new ConfigurationPage);
    pagesWidget->addWidget(new UpdatePage);
    pagesWidget->addWidget(new QueryPage);

    QPushButton *closeButton = new QPushButton(tr("Close"), this);

    createIcons();

    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

    QHBoxLayout *horizontalLayout = new QHBoxLayout;
    horizontalLayout->addWidget(view);
    horizontalLayout->addWidget(pagesWidget, 1);

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(closeButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(horizontalLayout);
    mainLayout->addStretch(1);
    mainLayout->addSpacing(12);
    mainLayout->addLayout(buttonsLayout);

    setWindowTitle(tr("Stacked widget"));
}

void Window::createIcons()
{
    QPixmap pixmap;

    contentsWidget->setBackgroundRole(QPalette::Base);

    pixmap.load("Resources/config.png");
    OptionButton *configButton = new OptionButton(104, 104, 0, contentsWidget);
    configButton->setIcon(QIcon(pixmap));
    configButton->setText(tr("Configuration"));
    configButton->setCheckable(true);
    configButton->setAutoExclusive(true);
    configButton->setChecked(true);
    configButton->move(0, 0);

    pixmap.load("Resources/update.png");
    OptionButton *updateButton = new OptionButton(104, 104, 1, contentsWidget);
    updateButton->setIcon(QIcon(pixmap));
    updateButton->setText(tr("Update"));
    updateButton->setCheckable(true);
    updateButton->setAutoExclusive(true);
    updateButton->move(0, 104);

    pixmap.load("Resources/query.png");
    OptionButton *queryButton = new OptionButton(104, 104, 2, contentsWidget);
    queryButton->setIcon(QIcon(pixmap));
    queryButton->setText(tr("Query"));
    queryButton->setCheckable(true);
    queryButton->setAutoExclusive(true);
    queryButton->move(0, 208);

    connect(configButton, SIGNAL(clicked()), this, SLOT(changePage()));
    connect(updateButton, SIGNAL(clicked()), this, SLOT(changePage()));
    connect(queryButton, SIGNAL(clicked()), this, SLOT(changePage()));

    contentsWidget->resize(104, 312);
}

void Window::changePage()
{
    pagesWidget->setCurrentIndex(static_cast<OptionButton *>(sender())->page());
}
