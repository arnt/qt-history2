#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

struct PluginInterface {
    virtual ~PluginInterface() {}
    virtual QString pluginName() const = 0;
};
Q_DECLARE_INTERFACE(PluginInterface, "com.trolltect.autotests.plugininterface/1.0")

#endif // PLUGININTERFACE_H

