#include <QtScript>

int main(int argc, char *argv[])
{
    QScriptEngine engine;
    qDebug() << "the magic number is:" << engine.evaluate("1 + 2").toNumber();
    return 0;
}
