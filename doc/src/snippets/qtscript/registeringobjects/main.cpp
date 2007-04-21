#include <QObject>
#include <QtScript>
#include "myobject.h"

int main(int argc, char *argv[])
{
    QScriptEngine engine;
    QObject *someObject = new MyObject;
    QScriptValue objectValue = engine.newQObject(someObject);
    engine.globalObject().setProperty("myObject", objectValue);
    qDebug() << "myObject's calculate() function returns"
             << engine.evaluate("myObject.calculate(10)").toNumber();
    return 0;
}
