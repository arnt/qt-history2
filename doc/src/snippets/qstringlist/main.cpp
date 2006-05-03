#include <QtGui>
#include <iostream>
using namespace std;

class Widget : public QWidget
{
public:
    Widget(QWidget *parent = 0);
};

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QStringList fonts;
    fonts << "Arial" << "Helvetica" << "Times" << "Courier";

    for (int i = 0; i < fonts.size(); ++i)
         cout << fonts.at(i).toLocal8Bit().constData() << endl;

    QStringListIterator javaStyleIterator(fonts);
    while (javaStyleIterator.hasNext())
         cout << javaStyleIterator.next().toLocal8Bit().constData() << endl;

    QStringList::const_iterator constIterator;
    for (constIterator = fonts.constBegin(); constIterator != fonts.constEnd();
           ++constIterator)
        cout << (*constIterator).toLocal8Bit().constData() << endl;

    QString str = fonts.join(",");
     // str == "Arial,Helvetica,Times,Courier"

    QStringList list;
    list = str.split(",");
     // list: ["Arial", "Helvetica", "Times", "Courier"]

    QStringList monospacedFonts = fonts.filter(QRegExp("Courier|Fixed"));

    QStringList files;
    files << "$QTDIR/src/moc/moc.y"
          << "$QTDIR/src/moc/moc.l"
          << "$QTDIR/include/qconfig.h";

    files.replaceInStrings("$QTDIR", "/usr/lib/qt");
    // files: [ "/usr/lib/qt/src/moc/moc.y", ...]

    QString str1, str2, str3;
    QStringList longerList = (QStringList() << str1 << str2 << str3);

    list.clear();
    list << "Bill Murray" << "John Doe" << "Bill Clinton";

    QStringList result;
    result = list.filter("Bill");
    // result: ["Bill Murray", "Bill Clinton"]

    result.clear();
    foreach (QString str, list) {
        if (str.contains("Bill"))
            result += str;
    }

    list.clear();
    list << "alpha" << "beta" << "gamma" << "epsilon";
    list.replaceInStrings("a", "o");
    // list == ["olpho", "beto", "gommo", "epsilon"]

    list.clear();
    list << "alpha" << "beta" << "gamma" << "epsilon";
    list.replaceInStrings(QRegExp("^a"), "o");
    // list == ["olpha", "beta", "gamma", "epsilon"]

    list.clear();
    list << "Bill Clinton" << "Murray, Bill";
    list.replaceInStrings(QRegExp("^(.*), (.*)$"), "\\2 \\1");
    // list == ["Bill Clinton", "Bill Murray"]
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Widget widget;
    widget.show();
    return app.exec();
}
