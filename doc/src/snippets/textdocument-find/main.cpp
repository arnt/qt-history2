#include <QtGui>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QTextEdit *editor = new QTextEdit();

    QTextCursor cursor(editor->textCursor());
    cursor.movePosition(QTextCursor::Start);

    QTextCharFormat plainFormat(cursor.charFormat());
    QTextCharFormat colorFormat = plainFormat;
    colorFormat.setForeground(Qt::red);

    cursor.insertText(QObject::tr("Text can be displayed in a variety of "
                                  "different character "
                                  "formats. "), plainFormat);
    cursor.insertText(QObject::tr("We can emphasize text by making it "));
    cursor.insertText(QObject::tr("italic, give it a different color "));
    cursor.insertText(QObject::tr("to the default text color, underline it, "));
    cursor.insertText(QObject::tr("and use many other effects."));

    QString searchString = QObject::tr("text");

    QTextDocument *document = editor->document();
    QTextCursor newCursor(document);

    while (!newCursor.isNull() && !newCursor.atEnd()) {
        newCursor = document->find(searchString, newCursor);

        if (!newCursor.isNull()) {
            newCursor.movePosition(QTextCursor::WordRight,
                                   QTextCursor::KeepAnchor);

            newCursor.mergeCharFormat(colorFormat);
        }
    }

    editor->setWindowTitle(QObject::tr("Text Document Find"));
    editor->resize(320, 480);
    editor->show();
    return app.exec();
}
