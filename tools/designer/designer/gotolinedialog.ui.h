void GotoLineDialog::init()
{
    editor = 0;
}

void GotoLineDialog::destroy()
{
    if ( editor )
	editor->release();
}

void GotoLineDialog::gotoLine()
{
    if ( editor )
	editor->gotoLine( spinLine->value() );
    accept();
}

void GotoLineDialog::setEditor( EditorInterface *e )
{
    editor = e;
    editor->addRef();
}

