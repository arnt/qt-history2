#include <QtGui>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QObject *parent = &app;

    QStringList numbers;
    numbers << "One" << "Two" << "Three" << "Four" << "Five";

    QAbstractItemModel *stringListModel = new QStringListModel(numbers, parent);

    QStringFilterModel *filterModel = new QStringFilterModel(parent);
    filterModel->setSourceModel(stringListModel);

    QWidget *window = new QWidget;

    QListView *filteredView = new QListView;
    filteredView->setModel(filterModel);
    filteredView->setWindowTitle("Filtered view onto a string list model");

    QLineEdit *patternEditor = new QLineEdit;
    QObject::
    connect(patternEditor, SIGNAL(textChanged(const QString &)),
            filterModel, SLOT(setPattern(const QString &)));

    QVBoxLayout *layout = new QVBoxLayout(window);
    layout->addWidget(filteredView);
    layout->addWidget(patternEditor);

    window->show();
    return app.exec();
}
