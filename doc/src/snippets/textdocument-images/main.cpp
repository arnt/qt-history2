#include <QtGui>

QString tr(const char *text)
{
    return QApplication::translate(text, text);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QTextEdit *editor = new QTextEdit;

    QTextDocument *document = new QTextDocument(editor);
    QTextCursor cursor(document);

    QTextImageFormat imageFormat;
    imageFormat.setName(":/images/advert.png");
    cursor.insertImage(imageFormat);

    cursor.insertBlock();
    cursor.insertText("Code less. Create more.");

    editor->setDocument(document);
    editor->setWindowTitle(tr("Text Document Images"));
    editor->resize(320, 480);
    editor->show();
    return app.exec();
}
