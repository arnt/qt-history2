namespace QMsNet
{
    using EnvDTE;
    using System;
    using System.IO;
    using System.Drawing;
    using System.Windows.Forms;
    using System.Diagnostics;
    using System.Runtime.InteropServices;
    using Microsoft.Office.Core;
    using Extensibility;
    using Microsoft.Win32;
    [GuidAttribute( "C174ACCD-D856-4B60-9887-0FF9E841E0EC" ), 
    ProgId( "QMsNet" )]

    public class Connect : Object, Extensibility.IDTExtensibility2, IDTCommandTarget
    {
	// Variabels ------------------------------------------------
	private _DTE applicationObject;
	private AddIn addInInstance;

	// Functions ------------------------------------------------
	public Connect(){}
	public void OnConnection( object application, 
				  Extensibility.ext_ConnectMode connectMode, 
				  object addInInst, 
				  ref System.Array custom )
	{
	    applicationObject = (_DTE)application;
	    addInInstance = (AddIn)addInInst;
	
	    // General startup code
	    if ( ( ext_ConnectMode.ext_cm_AfterStartup == connectMode ) ||
		( ext_ConnectMode.ext_cm_Startup      == connectMode ) ) {
		if ( null == ExtLoader.applicationObject ) {
		    ExtLoader.applicationObject = applicationObject;
		    ExtLoader.startUp();
		}
	    }
	    // ######################################################
	    // # The following UISetup signal is only sent once, 
	    // # the first time the AddIn is executed. To trigger
	    // # a reinit of the AddId set the following reg value:
	    // # [HKEY_CURRENT_USER\SOFTWARE\Microsoft\VisualStudio\
	    // # 7.0\PreloadAddinState]
	    // # "QMsNet"=dword:1
	    else if( ext_ConnectMode.ext_cm_UISetup == connectMode ) {
		// To ease the job of creating a AddIn, we here try 
		// to remove all commands before we register them,
		// since trying to register a command that already
		// exists will cause an exception...
		// --> Tip from John Robbins
		try {
    		    MessageBox.Show( "Setting up QMsNet..." );
		    AddinInit aii = new AddinInit( applicationObject, addInInstance );
		    aii.removeCommands();
		    aii.removeCommandBars();
		    aii.registerCommandBars();
		    aii.registerCommands();
		}
		catch ( System.Exception e ) {
		    Debug.Write( e.Message + "\r\n" + e.StackTrace.ToString(),
			"Couldn't registerCommands()" );
		}
	    }// #####################################################
	}

	public void OnDisconnection( Extensibility.ext_DisconnectMode disconnectMode, 
								    ref System.Array custom )
	{
	    ExtLoader.shutDown();
	}

	public void QueryStatus( string commandName, 
				 EnvDTE.vsCommandStatusTextWanted neededText, 
				 ref EnvDTE.vsCommandStatus status, 
				 ref object commandText )
	{
	    try {
		if( neededText == EnvDTE.vsCommandStatusTextWanted.vsCommandStatusTextWantedNone ) {
		    if( (commandName == Resource.NewQtProjectFullCommand) ||
			(commandName == Resource.MakeQtProjectFullCommand) ||
			(commandName == Resource.LoadDesignerFullCommand) ||
			(commandName == Resource.LoadQtProjectFullCommand) ||
			(commandName == Resource.SaveQtProjectFullCommand) ||
			(commandName == Resource.DLLQtProjectFullCommand) ||
			(commandName == Resource.AddMocStepFullCommand) ) {
			status = (vsCommandStatus)vsCommandStatus.vsCommandStatusSupported 
			    | vsCommandStatus.vsCommandStatusEnabled;
		    }
		}
	    }
	    catch ( System.Exception e ) {
		Debug.Write( e.Message + "\r\n" + e.StackTrace.ToString(),
		    "Couldn't registerCommands()" );
	    }
	}

	public void Exec( string commandName, 
			  EnvDTE.vsCommandExecOption executeOption, 
			  ref object varIn, 
			  ref object varOut, 
			  ref bool handled )
	{

	    try {
		handled = false;
		if( executeOption == EnvDTE.vsCommandExecOption.vsCommandExecOptionDoDefault ) {
		    if( commandName == Resource.LoadQtProjectFullCommand ) {
			handled = true;
			ExtLoader.LoadProject();
		    } else if ( commandName == Resource.SaveQtProjectFullCommand ) {
			handled = true;
			ExtLoader.SaveProject();
		    }
		}
	    }
	    catch( System.Exception e ) {
		Debug.Write( e.Message + "\r\n" + e.StackTrace.ToString(),
		    "Couldn't registerCommands()" );
	    }
	}

	// Unused functions...
	public void OnAddInsUpdate( ref System.Array custom ){}
	public void OnStartupComplete( ref System.Array custom ){}
	public void OnBeginShutdown( ref System.Array custom ){}
    }
}



#region Read me for Add-in installation and setup information.
// When run, the Add-in wizard prepared the registry for the Add-in.
// At a later time, the Add-in or its commands may become unavailable for reasons such as:
//   1) You moved this project to a computer other than which is was originally created on.
//   2) You chose 'Yes' when presented with a message asking if you wish to remove the Add-in.
//   3) You add new commands or modify commands already defined.
// You will need to re-register the Add-in by building the QMsNetSetup project,
// right-clicking the project in the Solution Explorer, and then choosing install.
// Alternatively, you could execute the ReCreateCommands.reg file the Add-in Wizard generated in
// the project directory, or run 'devenv /setup' from a command prompt.
#endregion
#region Read me for function descriptions
// onConnection()
/// <summary>
///      Implements the OnConnection method of the IDTExtensibility2 interface.
///      Receives notification that the Add-in is being loaded.
/// </summary>
/// <param term='application'>
///      Root object of the host application.
/// </param>
/// <param term='connectMode'>
///      Describes how the Add-in is being loaded.
/// </param>
/// <param term='addInInst'>
///      Object representing this Add-in.
/// </param>
/// <seealso class='IDTExtensibility2' />

// onDisconnect()
/// <summary>
///     Implements the OnDisconnection method of the IDTExtensibility2 interface.
///     Receives notification that the Add-in is being unloaded.
/// </summary>
/// <param term='disconnectMode'>
///      Describes how the Add-in is being unloaded.
/// </param>
/// <param term='custom'>
///      Array of parameters that are host application specific.
/// </param>
/// <seealso class='IDTExtensibility2' />

// public void OnAddInsUpdate( ref System.Array custom )
/// <summary>
///      Implements the OnAddInsUpdate method of the IDTExtensibility2 interface.
///      Receives notification that the collection of Add-ins has changed.
/// </summary>
/// <param term='custom'>
///      Array of parameters that are host application specific.
/// </param>
/// <seealso class='IDTExtensibility2' />

// public void OnStartupComplete( ref System.Array custom )
/// <summary>
///      Implements the OnStartupComplete method of the IDTExtensibility2 interface.
///      Receives notification that the host application has completed loading.
/// </summary>
/// <param term='custom'>
///      Array of parameters that are host application specific.
/// </param>
/// <seealso class='IDTExtensibility2' />

// public void OnBeginShutdown( ref System.Array custom )
/// <summary>
///      Implements the OnBeginShutdown method of the IDTExtensibility2 interface.
///      Receives notification that the host application is being unloaded.
/// </summary>
/// <param term='custom'>
///      Array of parameters that are host application specific.
/// </param>
/// <seealso class='IDTExtensibility2' />

// public void QueryStatus( string commandName, EnvDTE.vsCommandStatusTextWanted neededText, ref EnvDTE.vsCommandStatus status, ref object commandText )
/// <summary>
///      Implements the QueryStatus method of the IDTCommandTarget interface.
///      This is called when the command's availability is updated
/// </summary>
/// <param term='commandName'>
///		The name of the command to determine state for.
/// </param>
/// <param term='neededText'>
///		Text that is needed for the command.
/// </param>
/// <param term='status'>
///		The state of the command in the user interface.
/// </param>
/// <param term='commandText'>
///		Text requested by the neededText parameter.
/// </param>
/// <seealso class='Exec' />

// public void Exec( string commandName, EnvDTE.vsCommandExecOption executeOption, ref object varIn, ref object varOut, ref bool handled )
/// <summary>
///      Implements the Exec method of the IDTCommandTarget interface.
///      This is called when the command is invoked.
/// </summary>
/// <param term='commandName'>
///		The name of the command to execute.
/// </param>
/// <param term='executeOption'>
///		Describes how the command should be run.
/// </param>
/// <param term='varIn'>
///		Parameters passed from the caller to the command handler.
/// </param>
/// <param term='varOut'>
///		Parameters passed from the command handler to the caller.
/// </param>
/// <param term='handled'>
///		Informs the caller if the command was handled or not.
/// </param>
/// <seealso class='Exec' />
#endregion