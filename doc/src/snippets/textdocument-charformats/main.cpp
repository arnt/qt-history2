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
    qtFormat.setTextColor(QColor("#990000"));

    QTextCharFormat underlineFormat = plainFormat;
    underlineFormat.setFontUnderline(true);

    cursor.insertText(app.tr("Character formats"),
                      headingFormat);

    cursor.insertBlock();

    cursor.insertText(app.tr("Text can be displayed in a variety of different "
                             "character formats. "), plainFormat);
    cursor.insertText(app.tr("We can emphasize text by "));
    cursor.insertText(app.tr("making it italic"), emphasisFormat);
    cursor.insertText(app.tr(", give it a "), plainFormat);
    cursor.insertText(app.tr("different color "), qtFormat);
    cursor.insertText(app.tr("to the default text color, "), plainFormat);
    cursor.insertText(app.tr("underline it"), underlineFormat);
    cursor.insertText(app.tr(", and use many other effects."), plainFormat);

    editor->setWindowTitle(app.tr("Text Document Character Formats"));
    editor->resize(320, 480);
    editor->show();
    app.setMainWidget(editor);
    
    return app.exec();
}

