#include <QtGui/QtGui>

void processSize(int)
{
}

int main()
{
    QWidget *parent = 0;

    QSplitter *splitter = new QSplitter(parent);
    QListView *listview = new QListView;
    QTreeView *treeview = new QTreeView;
    QTextEdit *textedit = new QTextEdit;
    splitter->addWidget(listview);
    splitter->addWidget(treeview);
    splitter->addWidget(textedit);

    QSettings settings;
    settings.setValue("splitterSizes", splitter->saveState());

    QSettings restoreSettings;
    splitter->restoreState(restoreSettings.value("splitterSizes").toByteArray());

    QListIterator<int> it(splitter->sizes());
    while (it.hasNext())
        processSize(it.next());

    return 0;
}
