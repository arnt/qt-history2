#include <QtGui>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QTextEdit *editor = new QTextEdit();

    QTextCursor cursor(editor->textCursor());
    cursor.movePosition(QTextCursor::Start); 

    QTextCharFormat plainFormat(cursor.charFormat());

    QTextCharFormat headingFormat = plainFormat;
    headingFormat.setFontWeight(QFont::Bold);
    headingFormat.setFontPointSize(16);

    QTextCharFormat emphasisFormat = plainFormat;
    emphasisFormat.setFontItalic(true);

    QTextCharFormat qtFormat = plainFormat;
    qtFormat.setForeground(QColor("#990000"));

    QTextCharFormat underlineFormat = plainFormat;
    underlineFormat.setFontUnderline(true);

    cursor.insertText(QObject::tr("Character formats"),
                      headingFormat);

    cursor.insertBlock();

    cursor.insertText(QObject::tr("Text can be displayed in a variety of "
                                  "different character formats. "), plainFormat);
    cursor.insertText(QObject::tr("We can emphasize text by "));
    cursor.insertText(QObject::tr("making it italic"), emphasisFormat);
    cursor.insertText(QObject::tr(", give it a "), plainFormat);
    cursor.insertText(QObject::tr("different color "), qtFormat);
    cursor.insertText(QObject::tr("to the default text color, "), plainFormat);
    cursor.insertText(QObject::tr("underline it"), underlineFormat);
    cursor.insertText(QObject::tr(", and use many other effects."), plainFormat);

    editor->setWindowTitle(QObject::tr("Text Document Character Formats"));
    editor->resize(320, 480);
    editor->show();
    return app.exec();
}
