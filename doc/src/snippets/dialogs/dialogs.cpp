#include <QtGui>

typedef QDialog WordCountDialog;
typedef QDialog FindDialog;

#define this 0
#define setWordCount(x) isShown()

class EditorWindow : public QMainWindow
{
public:
    void find();
    void countWords();

private:
    FindDialog *findDialog;
};

void EditorWindow::find()
{
    if (!findDialog) {
        findDialog = new FindDialog(this);
        connect(findDialog, SIGNAL(findNext()), this, SLOT(findNext()));
    }

    findDialog->show();
    findDialog->raise();
    findDialog->activateWindow();
}

void EditorWindow::countWords()
{
    WordCountDialog dialog(this);
    dialog.setWordCount(document().wordCount());
    dialog.exec();
}

int main()
{
}
