void OptionsDialog::apply()
{
    WFlags f = WDestructiveClose | WType_TopLevel | WStyle_Customize;

    if ( bgBorder->isChecked() ) {
	if ( rbBorderNormal->isChecked() )
	    f |= WStyle_NormalBorder;
	else if ( rbBorderDialog->isChecked() )
	    f |= WStyle_DialogBorder;

	if ( bgTitle->isChecked() ) {
	    f |= WStyle_Title;
	    if ( cbTitleSystem->isChecked() )
		f |= WStyle_SysMenu;
	    if ( cbTitleMinimize->isChecked() )
		f |= WStyle_Minimize;
	    if ( cbTitleMaximize->isChecked() )
		f |= WStyle_Maximize;
	    if ( cbTitleContext->isChecked() )
		f |= WStyle_ContextHelp;
	}
    } else {
	f |= WStyle_NoBorder;
    }
   
    QWidget *parent = this;
    if ( cbBehaviorTaskbar->isChecked() ) {
	parent = 0;
	f |= WGroupLeader;
    }
    if ( cbBehaviorStays->isChecked() )
	f |= WStyle_StaysOnTop /*| WX11BypassWM*/;
    if ( cbBehaviorPopup->isChecked() )
	f |= WType_Popup;
    if ( cbBehaviorModal->isChecked() )
	f |= WShowModal;
    if ( cbBehaviorTool->isChecked() )
	f |= WStyle_Tool;

    if ( !widget ) {
	widget = new QVBox( parent, 0, f );
	widget->setMargin( 20 );
	QPushButton *okButton = new QPushButton( "Close", widget );
	connect( okButton, SIGNAL(clicked()), widget, SLOT(close()) );
	widget->move( pos() );
	widget->show();
    } else {
	widget->reparent( parent, f, widget->geometry().topLeft(), TRUE );
    }
    
    widget->setCaption( leCaption->text() );
    widget->setIcon( leIcon->text() );
}

void OptionsDialog::pickIcon()
{
    QString filename = QFileDialog::getOpenFileName( QString::null, QString::null, this );
    leIcon->setText( filename );
}
