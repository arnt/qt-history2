#include <QtGui>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QTextEdit *editor = new QTextEdit;

    QTextDocument *document = editor->document();
    QTextCursor redCursor(document);
    QTextCursor blueCursor(document);

    QTextCharFormat redFormat(redCursor.charFormat());
    redFormat.setForeground(Qt::red);
    QTextCharFormat blueFormat(blueCursor.charFormat());
    blueFormat.setForeground(Qt::blue);

    redCursor.setCharFormat(redFormat);
    blueCursor.setCharFormat(blueFormat);

    for (int i = 0; i < 20; ++i) {
        if (i % 2 == 0)
            redCursor.insertText(QObject::tr("%1 ").arg(i), redFormat);
        if (i % 5 == 0)
            blueCursor.insertText(QObject::tr("%1 ").arg(i), blueFormat);
    }

    editor->setWindowTitle(QObject::tr("Text Document Cursors"));
    editor->resize(320, 480);
    editor->show();
    return app.exec();
}
