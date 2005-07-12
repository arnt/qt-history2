#include "config.h"

#include <qvariant.h>
#include "gammaview.h"
/*
 *  Constructs a Config as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
Config::Config(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
    : QDialog(parent, name, modal, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(buttonOk, SIGNAL(clicked()), this, SLOT(accept()));
    connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(size_width, SIGNAL(valueChanged(int)), size_custom, SLOT(animateClick()));
    connect(size_height, SIGNAL(valueChanged(int)), size_custom, SLOT(animateClick()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
Config::~Config()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void Config::languageChange()
{
    retranslateUi(this);
}

