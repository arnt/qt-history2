/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/

#include "qskin.h"

void Skinable::init()
{
    QSkinLayout *s = new QSkinLayout(this, "usit");
    
    s->add(Back);
    s->add(Forward); 
    s->add(Play);
    s->add(Stop);
    s->add(QuitButton);
    s->add(Volume);
}