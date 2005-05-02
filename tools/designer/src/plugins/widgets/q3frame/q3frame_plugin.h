#ifndef Q3FRAMEPLUGIN_H
#define Q3FRAMEPLUGIN_H

#include <QtDesigner/QDesignerCustomWidgetInterface>

#include <Qt3Support/Q3Frame>

class Q3FramePlugin: public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)
public:
    Q3FramePlugin(QObject *parent = 0);
    virtual ~Q3FramePlugin();

    virtual QString name() const;
    virtual QString group() const;
    virtual QString toolTip() const;
    virtual QString whatsThis() const;
    virtual QString includeFile() const;
    virtual QIcon icon() const;

    virtual bool isContainer() const;
    virtual bool isForm() const;

    virtual QWidget *createWidget(QWidget *parent);

    virtual bool isInitialized() const;
    virtual void initialize(QDesignerFormEditorInterface *core);

    virtual QString domXml() const
    { return QLatin1String("\
        <widget class=\"Q3Frame\" name=\"frame\">\
            <property name=\"geometry\">\
                <rect>\
                    <x>0</x>\
                    <y>0</y>\
                    <width>100</width>\
                    <height>80</height>\
                </rect>\
            </property>\
        </widget>\
      "); }

private:
    bool m_initialized;
};

#endif
