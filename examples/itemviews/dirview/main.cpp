#include <QtGui>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QDirModel *model = new QDirModel(QDir::root());

    QTreeView *tree = new QTreeView;
    tree->setModel(model);

    tree->setWindowTitle(QObject::tr("Dir View"));
    app.setMainWidget(tree);
    tree->resize(640, 480);
    tree->show();

    return app.exec();
}
