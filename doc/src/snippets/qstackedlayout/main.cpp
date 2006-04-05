#include <QtGui>
#include <QApplication>

QString tr(const char * text)
{
    return QObject::tr(text);
}

bool connect(const QObject * sender, const char * signal,
                    const QObject * receiver, const char * method,
                    Qt::ConnectionType type = Qt::AutoCompatConnection)
{
    return QObject::connect(sender, signal, receiver, method, type);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget *mainWidget = new QWidget;

    QWidget *firstPageWidget = new QWidget;
    QWidget *secondPageWidget = new QWidget;
    QWidget *thirdPageWidget = new QWidget;

    QStackedLayout *layout = new QStackedLayout;
    layout->addWidget(firstPageWidget);
    layout->addWidget(secondPageWidget);
    layout->addWidget(thirdPageWidget);

    QComboBox *pageComboBox = new QComboBox;
    pageComboBox->addItem(tr("Page 1"));
    pageComboBox->addItem(tr("Page 2"));
    pageComboBox->addItem(tr("Page 3"));
    connect(pageComboBox, SIGNAL(activated(int)),
             layout, SLOT(setCurrentIndex(int)));

    mainWidget->setLayout(layout);
    mainWidget->show();
    return app.exec();
}
