namespace QMsNet
{
    using EnvDTE;
    using System;
    using System.Drawing;
    using System.Windows.Forms;
    using System.Diagnostics;
    using System.Runtime.InteropServices;
    using Microsoft.Office.Core;
    using Extensibility;

    public class AddinInit
    {
	// Variabels ------------------------------------------------------
	private Commands cmds;
	private _CommandBars cmdBars;
	private CommandBar cmdBar;

	// Functions ------------------------------------------------------
	public AddinInit( _DTE a, AddIn b )
	{
	    cmdBars = a.CommandBars;
	    cmds    = a.Commands;
	    try {
		cmdBar  = (CommandBar)cmdBars[ Resource.CommandBarName ];
	    }
	    catch ( System.Exception e ){
		MessageBox.Show( e.Message );
	    }
	}

	public void removeCommands()
	{
	    // Remove all commands we've registered...
	    foreach ( Command cmdDel in cmds ) {
		if ( (cmdDel.Name == Resource.NewQtProjectFullCommand) ||
		     (cmdDel.Name == Resource.DialogQtProjectFullCommand) ||
		     (cmdDel.Name == Resource.LoadDesignerFullCommand) ||
		     (cmdDel.Name == Resource.LoadQtProjectFullCommand) ||
		     (cmdDel.Name == Resource.SaveQtProjectFullCommand) ||
		     (cmdDel.Name == Resource.MakeQtProjectFullCommand) ||
		     (cmdDel.Name == Resource.AddMocStepFullCommand) ||
		     (cmdDel.Name == Resource.mntEventsFullCommand) ||
		     (cmdDel.Name == Resource.unmntEventsFullCommand) ) {
		    try { cmdDel.Delete(); }
		    catch {}
		}                                  
	    }
	}
	public void removeCommandBars() 
	{
	    // Remove all command bars we've registered...
	    try { cmds.RemoveCommandBar( cmdBar ) ; }
	    catch ( System.Exception ) {}
	}
	
	public void registerCommandBars()
	{
	    try {
		cmdBar = (CommandBar)
			cmds.AddCommandBar( Resource.CommandBarName,
					    vsCommandBarType.vsCommandBarTypeToolbar,
		                            null,
					    1 );
		cmdBar.Position = MsoBarPosition.msoBarTop;
		cmdBar.Visible = true;
		cmdBar.Protection = MsoBarProtection.msoBarNoCustomize;
	    }
	    catch( System.Exception e )
	    {
		MessageBox.Show( e.Message );
		Debug.Write( e.Message + "\r\n" + e.StackTrace.ToString(),
			     "Couldn't registerCommandBars()" );
	    }
	}

	public void registerCommands()
	{
	    object []contextGUIDS = new object[]{};
	    try {
		if ( cmdBar == null ) throw new System.Exception();
		int disableFlags = (int)vsCommandStatus.vsCommandStatusSupported + 
				   (int)vsCommandStatus.vsCommandStatusEnabled;
		Command cmd = null;

		// Add Moc Step for current file command
		cmd = cmds.AddNamedCommand( Connect.addinInstance,
					    Resource.AddMocStep,
					    Resource.AddMocStepButtonText,
					    Resource.AddMocStepToolTip,
					    false,
					    Resource.AddMocStepBitmapID,
					    ref contextGUIDS,
					    disableFlags );
		CommandBarControl ctrl = cmd.AddControl( cmdBar, 1 );
		ctrl.Caption = " ";

		// Make Project command
		cmd = cmds.AddNamedCommand( Connect.addinInstance,
					    Resource.MakeQtProject,
					    Resource.MakeQtProjectButtonText,
					    Resource.MakeQtProjectToolTip,
					    false,
					    Resource.MakeQtProjectBitmapID,
					    ref contextGUIDS,
					    disableFlags );
		ctrl = cmd.AddControl( cmdBar, 1 );
		ctrl.Caption = " ";

		// Save Qt Project command
		cmd = cmds.AddNamedCommand( Connect.addinInstance,
					    Resource.SaveQtProject,
					    Resource.SaveQtProjectButtonText,
					    Resource.SaveQtProjectToolTip,
					    false,
					    Resource.SaveQtProjectBitmapID,
					    ref contextGUIDS,
					    disableFlags );
		ctrl = cmd.AddControl( cmdBar, 1 );
		ctrl.Caption = " ";

		// Load Qt Project command
		cmd = cmds.AddNamedCommand( Connect.addinInstance,
					    Resource.LoadQtProject,
					    Resource.LoadQtProjectButtonText,
					    Resource.LoadQtProjectToolTip,
					    false,
					    Resource.LoadQtProjectBitmapID,
					    ref contextGUIDS,
					    disableFlags );
		ctrl = cmd.AddControl( cmdBar, 1 );
		ctrl.Caption = " ";

		// Load Designer command
		cmd = cmds.AddNamedCommand( Connect.addinInstance,
					    Resource.LoadDesigner,
					    Resource.LoadDesignerButtonText,
					    Resource.LoadDesignerToolTip,
					    false,
					    Resource.LoadDesignerBitmapID,
					    ref contextGUIDS,
					    disableFlags );
		ctrl = cmd.AddControl( cmdBar, 1 );
		ctrl.Caption = " ";

		// Dialog Qt Project command
//		cmd = cmds.AddNamedCommand( Connect.addinInstance,
//					    Resource.DialogQtProject,
//					    Resource.DialogQtProjectButtonText,
//					    Resource.DialogQtProjectToolTip,
//					    false,
//					    Resource.DialogQtProjectBitmapID,
//					    ref contextGUIDS,
//					    disableFlags );
//		ctrl = cmd.AddControl( cmdBar, 1 );
//		ctrl.Caption = " ";

		// New Qt Project command
		cmd = cmds.AddNamedCommand( Connect.addinInstance,
					    Resource.NewQtProject,
					    Resource.NewQtProjectButtonText,
					    Resource.NewQtProjectToolTip,
					    false,
					    Resource.NewQtProjectBitmapID,
					    ref contextGUIDS,
					    disableFlags );
		ctrl = cmd.AddControl( cmdBar, 1 );
		ctrl.Caption = " ";

		// Monitor Events command
		cmds.AddNamedCommand( Connect.addinInstance,
				      Resource.mntEvents,
				      Resource.mntEventsButtonText,
				      Resource.mntEventsToolTip,
				      true, 0, ref contextGUIDS,
				      disableFlags );

		// Stop Monitoring Events command
		cmds.AddNamedCommand( Connect.addinInstance,
				      Resource.unmntEvents,
				      Resource.unmntEventsButtonText,
				      Resource.unmntEventsToolTip,
				      true, 0, ref contextGUIDS,
				      disableFlags );
	    }
	    catch( System.Exception e ) {
		MessageBox.Show( e.Message );
		Debug.Write( e.Message + "\r\n" + e.StackTrace.ToString(),
			     "Couldn't registerCommands()" );
	    }
	}
    }
}
