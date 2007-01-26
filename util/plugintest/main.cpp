#include <QtCore/QtCore>

#include <stdio.h>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    const QStringList args = app.arguments();
    if (args.count() != 2) {
        printf("Usage: ./plugintest libplugin.so\nThis tool loads a plugin and displays whether QPluginLoader could load it or not.\nIf the plugin could not be loaded, it'll display the error string.\n");
        return 1;
    }

    QPluginLoader loader(args.at(1));
    if (loader.load())
        printf("success!\n");
    else
        printf("failure: %s\n", qPrintable(loader.errorString()));

    return 0;
}

