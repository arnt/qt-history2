#include <QtGui>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QDirModel *model = new QDirModel;

    QTreeView *tree = new QTreeView;
    tree->setModel(model);

    tree->setWindowTitle(QObject::tr("Dir View"));
    tree->resize(640, 480);
    tree->show();

    return app.exec();
}
