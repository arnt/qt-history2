/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include <qtranslator.h>
#include <qfiledialog.h>
#include <qapplication.h>

void MainWindow::loadTranslation()
{
    QString file = QFileDialog::getOpenFileName();
    
    if ( file.isEmpty() )
	return;
    
    static QTranslator *translator = 0;
    if ( !translator ) {
	translator = new QTranslator( qApp );
    	qApp->installTranslator( translator );
    }
    translator->load( file );
}
