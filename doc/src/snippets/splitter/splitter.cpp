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

    {
    // SAVE STATE
    QSettings settings;
    settings.setValue("splitterSizes", splitter->saveState());
    }

    {
    // RESTORE STATE
    QSettings settings;
    splitter->restoreState(settings.value("splitterSizes").toByteArray());
    }

    QListIterator<int> it(splitter->sizes());
    while (it.hasNext())
        processSize(it.next());

    return 0;
}
