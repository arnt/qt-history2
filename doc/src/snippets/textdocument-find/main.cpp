#include <QtGui>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QTextEdit *editor = new QTextEdit();

    QTextCursor cursor(editor->textCursor());
    cursor.movePosition(QTextCursor::Start); 

    QTextCharFormat plainFormat(cursor.charFormat());
    QTextCharFormat colorFormat = plainFormat;
    colorFormat.setTextColor(Qt::red);

    cursor.insertText(QObject::tr("Text can be displayed in a variety of "
                                  "different character "
                                  "formats. "), plainFormat);
    cursor.insertText(QObject::tr("We can emphasize text by making it "));
    cursor.insertText(QObject::tr("italic, give it a different color "));
    cursor.insertText(QObject::tr("to the default text color, underline it, "));
    cursor.insertText(QObject::tr("and use many other effects."));

    QString searchString = QObject::tr("text");

    QTextDocument *document = editor->document();
    QTextCursor modifyCursor(document);

    while (!modifyCursor.isNull() && !modifyCursor.atEnd()) {
        modifyCursor = document->find(searchString, modifyCursor);

        if (!modifyCursor.isNull()) {
            modifyCursor.movePosition(QTextCursor::WordRight,
                                      QTextCursor::KeepAnchor);

            QString text = modifyCursor.selectedText();
            modifyCursor.removeSelectedText();
            modifyCursor.insertText(text, colorFormat);
        }
    }

    editor->setWindowTitle(QObject::tr("Text Document Find"));
    editor->resize(320, 480);
    editor->show();
    app.setMainWidget(editor);
    
    return app.exec();
}
