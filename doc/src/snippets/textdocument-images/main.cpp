#include <QtGui>

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
    editor->setWindowTitle(QObject::tr("Text Document Images"));
    editor->resize(320, 480);
    editor->show();
    return app.exec();
}
