#include <QtGui>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QDirModel *model = new QDirModel(QDir());

    QTreeView *tree = new QTreeView;
    tree->setModel(model);
    tree->setEditTriggers(QAbstractItemView::NoEditTriggers);

    tree->setWindowTitle(QObject::tr("Dir View"));
    app.setMainWidget(tree);
    tree->show();

    return app.exec();
}
