#ifndef PREFERENCES_H
#define PREFERENCES_H
#include "preferences.h"

class EDITOR_EXPORT Preferences : public PreferencesBase
{ 
    Q_OBJECT

public:
    Preferences( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~Preferences();

protected slots:
    void boldChanged( bool );
    void elementChanged( const QString & );
    void familyChanged( const QString & );
    void italicChanged( bool );
    void colorClicked();
    void sizeChanged( int );
    void underlineChanged( bool );
    void init();

};

#endif // PREFERENCES_H
