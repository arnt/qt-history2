#include <QtGui>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QTextBrowser browser;
    QColor linkColor(Qt::red);
    QString sheet = QString::fromLatin1("a { text-decoration: underline; color: %1 }").arg(linkColor.name());
    browser.document()->setDefaultStyleSheet(sheet);
    browser.setSource(QUrl("../../../html/index.html"));
    browser.resize(800, 600);
    browser.show();

    return app.exec();
}

