#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>

#include <QIcon>

#include "tictactoe.h"
#include "tictactoeplugin.h"
#include "tictactoetaskmenu.h"
#include "qplugin.h"

TicTacToePlugin::TicTacToePlugin(QObject *parent)
    :QObject(parent)
{
    initialized = false;
}

QString TicTacToePlugin::name() const
{
    return QLatin1String("TicTacToe");
}

QString TicTacToePlugin::group() const
{
    return QLatin1String("Display Widgets [Examples]");
}

QString TicTacToePlugin::toolTip() const
{
    return QString();
}

QString TicTacToePlugin::whatsThis() const
{
    return QString();
}

QString TicTacToePlugin::includeFile() const
{
    return QLatin1String("tictactoe.h");
}

QIcon TicTacToePlugin::icon() const
{
    return QIcon();
}

bool TicTacToePlugin::isContainer() const
{
    return false;
}

QWidget *TicTacToePlugin::createWidget(QWidget *parent)
{
    TicTacToe *ticTacToe = new TicTacToe(parent);
    ticTacToe->setState("-X-XO----");
    return ticTacToe;
}

bool TicTacToePlugin::isInitialized() const
{
    return initialized;
}

void TicTacToePlugin::initialize(QDesignerFormEditorInterface *formEditor)
{
    if (initialized)
        return;

    QExtensionManager *manager = formEditor->extensionManager();
    Q_ASSERT(manager != 0);

    manager->registerExtensions(new TicTacToeTaskMenuFactory(manager),
                                Q_TYPEID(QDesignerTaskMenuExtension));

    initialized = true;
}

Q_EXPORT_PLUGIN(TicTacToePlugin)



