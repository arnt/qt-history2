#include <QtScript>

int main(int argc, char *argv[])
{
    QScriptEngine engine;
    QScriptValue val(&engine, 123);
    engine.globalObject().setProperty("foo", val);
    qDebug() << "foo times two is:" << engine.evaluate("foo * 2").toNumber();
    return 0;
}

