#include <QtGui>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QTextEdit *editor = new QTextEdit();

    QTextDocument *document = editor->document();
    QTextCursor cursor(document);

    QTextImageFormat imageFormat;
    imageFormat.setName(":/images/advert.png");
    cursor.insertImage(imageFormat);

    editor->setWindowTitle(QObject::tr("Text Document Images"));
    editor->resize(320, 480);
    editor->show();
    app.setMainWidget(editor);
    
    return app.exec();
}
