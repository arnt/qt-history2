#include <projectsettingsiface.h>

class CppProjectSettings;

class ProjectSettingsInterfaceImpl : public ProjectSettingsInterface
{
public:
    ProjectSettingsInterfaceImpl();
    
    ProjectSettings *projectSetting();
    QStringList projectSettings() const;
    void connectTo( QUnknownInterface *appInterface );
    void deleteProjectSettingsObject( ProjectSettings * );
    QRESULT queryInterface( const QUuid &uuid, QUnknownInterface **iface );
    Q_REFCOUNT
    
private:
    CppProjectSettings *settingsTab;
    
};

