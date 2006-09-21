#include <QtGui>
#include <QApplication>
#include <stdio.h>

class Widget : public QWidget
{
public:
    Widget(QWidget *parent = 0);

    void constCharPointer();
    void constCharArray();
    void characterReference();
    void atFunction();
    void stringLiteral();
    void modify();
    void index();
    QString boolToString(bool b);
    void nullVsEmpty();

    void appendFunction();
    void argFunction();
    void chopFunction();
    void compareFunction();
    void compareSensitiveFunction();
    void containsFunction();
    void countFunction();
    void dataFunction();
    void endsWithFunction();
    void fillFunction();
    void fromRawDataFunction();

    void indexOfFunction();
    void firstIndexOfFunction();
    void insertFunction();
    void isNullFunction();
    void isEmptyFunction();
    void lastIndexOfFunction();
    void leftFunction();
    void leftJustifiedFunction();
    void midFunction();
    void numberFunction();

    void prependFunction();
    void removeFunction();
    void replaceFunction();
    void reserveFunction();
    void resizeFunction();
    void rightFunction();
    void rightJustifiedFunction();
    void sectionFunction();
    void setNumFunction();
    void simplifiedFunction();

    void sizeFunction();
    void splitFunction();
    void splitCaseSensitiveFunction();
    void sprintfFunction();
    void startsWithFunction();
    void toDoubleFunction();
    void toFloatFunction();
    void toIntFunction();
    void toLongFunction();
    void toLongLongFunction();

    void toLowerFunction();
    void toShortFunction();
    void toUIntFunction();
    void toULongFunction();
    void toULongLongFunction();
    void toUShortFunction();
    void toUpperFunction();
    void trimmedFunction();
    void truncateFunction();

    void plusEqualOperator();
    void arrayOperator();
};

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
}

void Widget::constCharPointer()
{
    QString str = "Hello";
}

void Widget::constCharArray()
{
    static const QChar data[4] = { 0x0055, 0x006e, 0x10e3, 0x03a3 };
    QString str(data, 4);
}

void Widget::characterReference()
{
    QString str;
    str.resize(4);

    str[0] = QChar('U');
    str[1] = QChar('n');
    str[2] = QChar(0x10e3);
    str[3] = QChar(0x03a3);
}

void Widget::atFunction()
{
    QString str;

    for (int i = 0; i < str.size(); ++i) {
        if (str.at(i) >= QChar('a') && str.at(i) <= QChar('f'))
            qDebug() << "Found character in range [a-f]";
    }
}

void Widget::stringLiteral()
{
    QString str;

    if (str == "auto" || str == "extern"
            || str == "static" || str == "register") {
        // ...
    }
}

void Widget::modify()
{
    QString str = "and";
    str.prepend("rock ");     // str == "rock and"
    str.append(" roll");        // str == "rock and roll"
    str.replace(5, 3, "&");   // str == "rock & roll"
}

void Widget::index()
{
    QString str = "We must be <b>bold</b>, very <b>bold</b>";
    int j = 0;

    while ((j = str.indexOf("<b>", j)) != -1) {
        qDebug() << "Found <b> tag at index position" << j;
        ++j;
    }
}

    QString Widget::boolToString(bool b)
    {
        QString result;
        if (b)
            result = "True";
        else
            result = "False";
        return result;
    }


void Widget::nullVsEmpty()
{
    QString().isNull();               // returns true
    QString().isEmpty();              // returns true

    QString("").isNull();             // returns false
    QString("").isEmpty();            // returns true

    QString("abc").isNull();          // returns false
    QString("abc").isEmpty();         // returns false
}

void Widget::appendFunction()
{
    QString x = "free";
    QString y = "dom";

    x.append(y);
    // x == "freedom"

    x.insert(x.size(), y);
}

void Widget::argFunction()
{
    QString i;           // current file's number
    QString total;       // number of files to process
    QString fileName;    // current file's name

    QString status = QString("Processing file %1 of %2: %3")
                    .arg(i).arg(total).arg(fileName);

    QString str;
    str = "%1 %2";

    str.arg("%1f", "Hello");        // returns "%1f Hello"
    str.arg("%1f").arg("Hello");    // returns "Hellof"

    str = QString("Decimal 63 is %1 in hexadecimal")
            .arg(63, 0, 16);
    // str == "Decimal 63 is 3f in hexadecimal"

    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
    str = QString("%1 %L2 %L3")
            .arg(12345)
            .arg(12345)
            .arg(12345, 0, 16);
    // str == "12345 12,345 3039"
}

void Widget::chopFunction()
{
    QString str("LOGOUT\r\n");
    str.chop(2);
    // str == "LOGOUT"
}

void Widget::compareFunction()
{
    int x = QString::compare("auto", "auto");   // x == 0
    int y = QString::compare("auto", "car");    // y < 0
    int z = QString::compare("car", "auto");    // z > 0
}

void Widget::compareSensitiveFunction()
{
    int x = QString::compare("aUtO", "AuTo", Qt::CaseInsensitive);  // x == 0
    int y = QString::compare("auto", "Car", Qt::CaseSensitive);     // y > 0
    int z = QString::compare("auto", "Car", Qt::CaseInsensitive);   // z < 0
}

void Widget::containsFunction()
{
    QString str = "Peter Pan";
    str.contains("peter", Qt::CaseInsensitive);    // returns true
}

void Widget::countFunction()
{
    QString str = "banana and panama";
    str.count(QRegExp("a[nm]a"));    // returns 4
}

void Widget::dataFunction()
{
    QString str = "Hello world";
    QChar *data = str.data();
    while (data) {
        qDebug() << data->unicode();
        ++data;
    }
}

void Widget::endsWithFunction()
{
    QString str = "Bananas";
    str.endsWith("anas");         // returns true
    str.endsWith("pple");         // returns false
}

void Widget::fillFunction()
{
    QString str = "Berlin";
    str.fill('z');
    // str == "zzzzzz"

    str.fill('A', 2);
    // str == "AA"
}

void Widget::fromRawDataFunction()
{
     QRegExp pattern;
     static const QChar unicode[] = {
             0x005A, 0x007F, 0x00A4, 0x0060,
             0x1009, 0x0020, 0x0020};
     int size = sizeof(unicode) / sizeof(QChar);

     QString str = QString::fromRawData(unicode, size);
     if (str.contains(QRegExp(pattern))) {
         // ...
     }
}

void Widget::indexOfFunction()
{
    QString x = "sticky question";
    QString y = "sti";
    x.indexOf(y);               // returns 0
    x.indexOf(y, 1);            // returns 10
    x.indexOf(y, 10);           // returns 10
    x.indexOf(y, 11);           // returns -1
}

void Widget::firstIndexOfFunction()
{
    QString str = "the minimum";
    str.indexOf(QRegExp("m[aeiou]"), 0);       // returns 4
}

void Widget::insertFunction()
{
    QString str = "Meal";
    str.insert(1, QString("ontr"));
    // str == "Montreal"
}

void Widget::isEmptyFunction()
{
    QString().isEmpty();            // returns true
    QString("").isEmpty();          // returns true
    QString("x").isEmpty();         // returns false
    QString("abc").isEmpty();       // returns false
}

void Widget::isNullFunction()
{
    QString().isNull();             // returns true
    QString("").isNull();           // returns false
    QString("abc").isNull();        // returns false
}

void Widget::lastIndexOfFunction()
{
    QString x = "crazy azimuths";
    QString y = "az";
    x.lastIndexOf(y);           // returns 6
    x.lastIndexOf(y, 6);        // returns 6
    x.lastIndexOf(y, 5);        // returns 2
    x.lastIndexOf(y, 1);        // returns -1

    QString str = "the minimum";
    str.lastIndexOf(QRegExp("m[aeiou]"));      // returns 8
}

void Widget::leftFunction()
{
    QString x = "Pineapple";
    QString y = x.left(4);      // y == "Pine"
}

void Widget::leftJustifiedFunction()
{
    QString s = "apple";
    QString t = s.leftJustified(8, '.');    // t == "apple..."

    QString str = "Pineapple";
    str = str.leftJustified(5, '.', true);    // str == "Pinea"
}

void Widget::midFunction()
{
    QString x = "Nine pineapples";
    QString y = x.mid(5, 4);            // y == "pine"
    QString z = x.mid(5);               // z == "pineapples"
}

void Widget::numberFunction()
{
    long a = 63;
    QString s = QString::number(a, 16);             // s == "3f"
    QString t = QString::number(a, 16).toUpper();     // t == "3F"
}

void Widget::prependFunction()
{
    QString x = "ship";
    QString y = "air";
    x.prepend(y);
    // x == "airship"
}

void Widget::removeFunction()
{
    QString s = "Montreal";
    s.remove(1, 4);
    // s == "Meal"

    QString t = "Ali Baba";
    t.remove(QChar('a'), Qt::CaseInsensitive);
    // t == "li Bb"

    QString r = "Telephone";
    r.remove(QRegExp("[aeiou]."));
    // r == "The"
}

void Widget::replaceFunction()
{
    QString x = "Say yes!";
    QString y = "no";
    x.replace(4, 3, y);
    // x == "Say no!"

    QString str = "colour behaviour flavour neighbour";
    str.replace(QString("ou"), QString("o"));
    // str == "color behavior flavor neighbor"

    QString s = "Banana";
    s.replace(QRegExp("a[mn]"), "ox");
    // s == "Boxoxa"

    QString t = "A <i>bon mot</i>.";
    t.replace(QRegExp("<i>([^<]*)</i>"), "\\emph{\\1}");
    // t == "A \\emph{bon mot}."
}

void Widget::reserveFunction()
{
    QString result;
    int maxSize;
    bool condition;
    QChar nextChar;

    result.reserve(maxSize);

    while (condition)
        result.append(nextChar);

    result.squeeze();
}

void Widget::resizeFunction()
{
    QString s = "Hello world";
    s.resize(5);
    // s == "Hello"

    s.resize(8);
    // s == "Hello???" (where ? stands for any character)

    QString t = "Hello";
    t += QString(10, 'X');
    // t == "HelloXXXXXXXXXX"

    QString r = "Hello";
    r = r.leftJustified(10, ' ');
    // r == "Hello     "
}

void Widget::rightFunction()
{
    QString x = "Pineapple";
    QString y = x.right(5);      // y == "apple"
}

void Widget::rightJustifiedFunction()
{
    QString s = "apple";
    QString t = s.rightJustified(8, '.');    // t == "...apple"

    QString str = "Pineapple";
    str = str.rightJustified(5, '.', true);    // str == "Pinea"
}

void Widget::sectionFunction()
{
    QString str;
    QString csv = "forename,middlename,surname,phone";
    QString path = "/usr/local/bin/myapp"; // First field is empty
    QString::SectionFlag flag = QString::SectionSkipEmpty;


    str = csv.section(',', 2, 2);   // str == "surname"
    str = path.section('/', 3, 4);  // str == "bin/myapp"
    str = path.section('/', 3, 3, flag); // str == "myapp"

    str = csv.section(',', -3, -2);  // str == "middlename,surname"
    str = path.section('/', -1); // str == "myapp"

    QString data = "forename**middlename**surname**phone";

    str = data.section("**", 2, 2); // str == "surname"
    str = data.section("**", -3, -2); // str == "middlename**surname"

    QString line = "forename\tmiddlename  surname \t \t phone";
    QRegExp sep("\\s+");
    str = line.section(sep, 2, 2); // s == "surname"
    str = line.section(sep, -3, -2); // s == "middlename  surname"
}

void Widget::setNumFunction()
{
    QString str;
    str.setNum(1234);       // str == "1234"
}

void Widget::simplifiedFunction()
{
    QString str = "  lots\t of\nwhitespace\r\n ";
    str = str.simplified();
    // str == "lots of whitespace";
}

void Widget::sizeFunction()
{
    QString str = "World";
    int n = str.size();         // n == 5
    str.data()[0];              // returns 'W'
    str.data()[4];              // returns 'd'
    str.data()[5];              // returns '\0'
}

void Widget::splitFunction()
{
    QString str;
    QStringList list;

    str = "Some  text\n\twith  strange whitespace.";
    list = str.split(QRegExp("\\s+"));
    // list: [ "Some", "text", "with", "strange", "whitespace." ]

    str = "This time, a normal English sentence.";
    list = str.split(QRegExp("\\W+"), QString::SkipEmptyParts);
    // list: [ "This", "time", "a", "normal", "English", "sentence" ]

    str = "Now: this sentence fragment.";
    list = str.split(QRegExp("\\b"));
    // list: [ "", "Now", ": ", "this", " ", "sentence", " ", "fragment", "." ]
}

void Widget::splitCaseSensitiveFunction()
{
    QString str = "a,,b,c";

    QStringList list1 = str.split(",");
    // list1: [ "a", "", "b", "c" ]

    QStringList list2 = str.split(",", QString::SkipEmptyParts);
    // list2: [ "a", "b", "c" ]
}

void Widget::sprintfFunction()
{
    size_t BufSize;
    char buf[BufSize];

    ::snprintf(buf, BufSize, "%lld", 123456789LL);
    QString str = QString::fromAscii(buf);

    QString result;
    QTextStream(&result) << "pi = " << 3.14;
    // result == "pi = 3.14"
}

void Widget::startsWithFunction()
{
    QString str = "Bananas";
    str.startsWith("Ban");     // returns true
    str.startsWith("Car");     // returns false
}

void Widget::toDoubleFunction()
{
    QString str = "1234.56";
    double val = str.toDouble();   // val == 1234.56

    bool ok;
    double d;

    d = QString( "1234.56e-02" ).toDouble(&ok); // ok == true, d == 12.3456

    QLocale::setDefault(QLocale::C);
    d = QString( "1234,56" ).toDouble(&ok); // ok == false
    d = QString( "1234.56" ).toDouble(&ok); // ok == true, d == 1234.56

    QLocale::setDefault(QLocale::German);
    d = QString( "1234,56" ).toDouble(&ok); // ok == true, d == 1234.56
    d = QString( "1234.56" ).toDouble(&ok); // ok == true, d == 1234.56

    QLocale::setDefault(QLocale::C);
    d = QString( "1,234,567.89" ).toDouble(&ok); // ok == false
}

void Widget::toFloatFunction()
{
    QString str1 = "1234.56";
    str1.toFloat();             // returns 1234.56

    bool ok;
    QString str2 = "R2D2";
    str2.toFloat(&ok);          // returns 0.0, sets ok to false
}

void Widget::toIntFunction()
{
    QString str = "FF";
    bool ok;
    int hex = str.toInt(&ok, 16);       // hex == 255, ok == true
    int dec = str.toInt(&ok, 10);       // dec == 0, ok == false
}

void Widget::toLongFunction()
{
    QString str = "FF";
    bool ok;

    long hex = str.toLong(&ok, 16);     // hex == 255, ok == true
    long dec = str.toLong(&ok, 10);     // dec == 0, ok == false
}

void Widget::toLongLongFunction()
{
    QString str = "FF";
    bool ok;

    qint64 hex = str.toLongLong(&ok, 16);      // hex == 255, ok == true
    qint64 dec = str.toLongLong(&ok, 10);      // dec == 0, ok == false
}

void Widget::toLowerFunction()
{
    QString str = "TROlltECH";
    str = str.toLower();        // str == "trolltech"
}

void Widget::toShortFunction()
{
    QString str = "FF";
    bool ok;

    short hex = str.toShort(&ok, 16);   // hex == 255, ok == true
    short dec = str.toShort(&ok, 10);   // dec == 0, ok == false
}

void Widget::toUIntFunction()
{
    QString str = "FF";
    bool ok;

    uint hex = str.toUInt(&ok, 16);     // hex == 255, ok == true
    uint dec = str.toUInt(&ok, 10);     // dec == 0, ok == false
}

void Widget::toULongFunction()
{
    QString str = "FF";
    bool ok;

    ulong hex = str.toULong(&ok, 16);   // hex == 255, ok == true
    ulong dec = str.toULong(&ok, 10);   // dec == 0, ok == false
}

void Widget::toULongLongFunction()
{
    QString str = "FF";
    bool ok;

    quint64 hex = str.toULongLong(&ok, 16);    // hex == 255, ok == true
    quint64 dec = str.toULongLong(&ok, 10);    // dec == 0, ok == false
}

void Widget::toUShortFunction()
{
    QString str = "FF";
    bool ok;

    ushort hex = str.toUShort(&ok, 16);     // hex == 255, ok == true
    ushort dec = str.toUShort(&ok, 10);     // dec == 0, ok == false
}

void Widget::toUpperFunction()
{
    QString str = "TeXt";
    str = str.toUpper();        // str == "TEXT"
}

void Widget::trimmedFunction()
{
    QString str = "  lots\t of\nwhitespace\r\n ";
    str = str.trimmed();
    // str == "lots\t of\nwhitespace"
}

void Widget::truncateFunction()
{
    QString str = "Vladivostok";
    str.truncate(4);
    // str == "Vlad"
}

void Widget::plusEqualOperator()
{
    QString x = "free";
    QString y = "dom";
    x += y;
    // x == "freedom"
}

void Widget::arrayOperator()
{
    QString str;

    if (str[0] == QChar('?'))
        str[0] = QChar('_');
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Widget widget;
    widget.show();
    return app.exec();
}
