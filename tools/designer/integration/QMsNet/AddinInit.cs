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
	    catch ( System.Exception ){}
	}

	public void removeCommands()
	{
	    // Remove all commands we've registered...
	    foreach ( Command cmdDel in cmds ) {
		if ( (cmdDel.Name == Resource.NewQtProjectFullCommand) ||
		     (cmdDel.Name == Resource.MakeQtProjectFullCommand) ||
		     (cmdDel.Name == Resource.LoadDesignerFullCommand) ||
		     (cmdDel.Name == Resource.LoadQtProjectFullCommand) ||
		     (cmdDel.Name == Resource.SaveQtProjectFullCommand) ||
		     (cmdDel.Name == Resource.DLLQtProjectFullCommand) ||
		     (cmdDel.Name == Resource.AddMocStepFullCommand) ||
		     (cmdDel.Name == Resource.mntEventsFullCommand) ||
		     (cmdDel.Name == Resource.unmntEventsFullCommand) ) {
		    try { cmdDel.Delete(); }
		    catch ( System.Exception ){}
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

//		cmdBar = cmdBars.Add( Resource.CommandBarName, 
//					    MsoBarPosition.msoBarTop, 
//					    false, false );
//		cmdBar.Visible = true;
//		cmdBar.Protection = 0;
		// We may enable this to make users choose were to place the toolbar
		// QMsDevCmdBar.Position = MsoBarPosition.msoBarFloating;
	    }
	    catch( System.Exception e )
	    {
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
		cmd.AddControl( cmdBar, 1 );

		// DLL Project command
		cmd = cmds.AddNamedCommand( Connect.addinInstance,
					    Resource.DLLQtProject,
					    Resource.DLLQtProjectButtonText,
					    Resource.DLLQtProjectToolTip,
					    false,
					    Resource.DLLQtProjectBitmapID,
					    ref contextGUIDS,
					    disableFlags );
		cmd.AddControl( cmdBar, 1 );

		// Save Qt Project command
		cmd = cmds.AddNamedCommand( Connect.addinInstance,
					    Resource.SaveQtProject,
					    Resource.SaveQtProjectButtonText,
					    Resource.SaveQtProjectToolTip,
					    false,
					    Resource.SaveQtProjectBitmapID,
					    ref contextGUIDS,
					    disableFlags );
		cmd.AddControl( cmdBar, 1 );

		// Load Qt Project command
		cmd = cmds.AddNamedCommand( Connect.addinInstance,
					    Resource.LoadQtProject,
					    Resource.LoadQtProjectButtonText,
					    Resource.LoadQtProjectToolTip,
					    false,
					    Resource.LoadQtProjectBitmapID,
					    ref contextGUIDS,
					    disableFlags );
		cmd.AddControl( cmdBar, 1 );

		// Load Designer command
		cmd = cmds.AddNamedCommand( Connect.addinInstance,
					    Resource.LoadDesigner,
					    Resource.LoadDesignerButtonText,
					    Resource.LoadDesignerToolTip,
					    false,
					    Resource.LoadDesignerBitmapID,
					    ref contextGUIDS,
					    disableFlags );
		cmd.AddControl( cmdBar, 1 );

		// Make Qt Project command
		cmd = cmds.AddNamedCommand( Connect.addinInstance,
					    Resource.MakeQtProject,
					    Resource.MakeQtProjectButtonText,
					    Resource.MakeQtProjectToolTip,
					    false,
					    Resource.MakeQtProjectBitmapID,
					    ref contextGUIDS,
					    disableFlags );
		cmd.AddControl( cmdBar, 1 );

		// New Qt Project command
		cmd = cmds.AddNamedCommand( Connect.addinInstance,
					    Resource.NewQtProject,
					    Resource.NewQtProjectButtonText,
					    Resource.NewQtProjectToolTip,
					    false,
					    Resource.NewQtProjectBitmapID,
					    ref contextGUIDS,
					    disableFlags );
		cmd.AddControl( cmdBar, 1 );

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

//		_CommandBars commandBars;
//		CommandBar toolsCommandBar;
//		CommandBarControls commandBarControls;
//		CommandBarControl commandBarControl;
//		CommandBarEvents commandBarEvents;
//		String strCommandBarItem = "Tools";
//
//		commandBars = Connect.applicationObject.CommandBars;
//		toolsCommandBar = commandBars[strCommandBarItem];
//		commandBarControls = toolsCommandBar.Controls;
//
//		try {
////		    commandBarControl = commandBarControls.Add(MsoControlType.msoControlButton, 1, null, 1, false);
////		    commandBarControl.Visible = true;
////		    commandBarControl.Caption = "C# Test Button";
////		    commandBarEvents = (EnvDTE.CommandBarEvents)Connect.applicationObject.Events.get_CommandBarEvents(commandBarControl);
////		    commandBarEvents.Click += new EnvDTE._dispCommandBarControlEvents_ClickEventHandler(this.Click);
//		    commandBarControl = cmdBar.Controls.Add(MsoControlType.msoControlButton, 1, null, 1, false);
//		    commandBarControl.Visible = true;
//		    commandBarControl.Caption = "#4";
//		    commandBarEvents = (EnvDTE.CommandBarEvents)Connect.applicationObject.Events.get_CommandBarEvents(commandBarControl);
//		    commandBarEvents.Click += new EnvDTE._dispCommandBarControlEvents_ClickEventHandler(this.Click);
//		}
//		catch (System.Exception ex) {
//		    System.Windows.Forms.MessageBox.Show(ex.ToString());
//		}
	    }
	    catch( System.Exception e ) {
		Debug.Write( e.Message + "\r\n" + e.StackTrace.ToString(),
			     "Couldn't registerCommands()" );
	    }
	}
    }
}
