#include "preferencesimpl.h"

/* 
 *  Constructs a Preferences which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
Preferences::Preferences( QWidget* parent,  const char* name, WFlags fl )
    : PreferencesBase( parent, name, fl )
{
}

/*  
 *  Destroys the object and frees any allocated resources
 */
Preferences::~Preferences()
{
    // no need to delete child widgets, Qt does it all for us
}

/* 
 * protected slot
 */
void Preferences::boldChanged( bool )
{
    qWarning( "Preferences::boldChanged( bool ) not yet implemented!" ); 
}
/* 
 * protected slot
 */
void Preferences::elementChanged( const QString & )
{
    qWarning( "Preferences::elementChanged( const QString & ) not yet implemented!" ); 
}
/* 
 * protected slot
 */
void Preferences::familyChanged( const QString & )
{
    qWarning( "Preferences::familyChanged( const QString & ) not yet implemented!" ); 
}
/* 
 * protected slot
 */
void Preferences::italicChanged( bool )
{
    qWarning( "Preferences::italicChanged( bool ) not yet implemented!" ); 
}
/* 
 * protected slot
 */
void Preferences::colorClicked()
{
    qWarning( "Preferences::colorClicked() not yet implemented!" ); 
}
/* 
 * protected slot
 */
void Preferences::sizeChanged( int )
{
    qWarning( "Preferences::sizeChanged( int ) not yet implemented!" ); 
}
/* 
 * protected slot
 */
void Preferences::underlineChanged( bool )
{
    qWarning( "Preferences::underlineChanged( bool ) not yet implemented!" ); 
}
/* 
 * protected slot
 */
void Preferences::init()
{
    qWarning( "Preferences::init() not yet implemented!" ); 
}

