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
	public static _DTE applicationObject = null;
	public static AddIn addinInstance = null;
	public ExtLoader extLoader;

	// Functions ------------------------------------------------
	public Connect(){}
	public void OnConnection( object application, 
				  Extensibility.ext_ConnectMode connectMode, 
				  object addinInst, 
				  ref System.Array custom ) 
	{
	    applicationObject = (_DTE)application;
	    addinInstance = (AddIn)addinInst;
	    extLoader = new ExtLoader();

	    // General startup code
	    if ( ( ext_ConnectMode.ext_cm_AfterStartup == connectMode ) ||
		( ext_ConnectMode.ext_cm_Startup      == connectMode ) ) {
		extLoader.startUp();
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
		    AddinInit aii = new AddinInit( applicationObject, addinInstance );
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
	    extLoader.shutDown();
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
			(commandName == Resource.AddMocStepFullCommand) ||
			(commandName == Resource.mntEventsFullCommand) ||
			(commandName == Resource.unmntEventsFullCommand) ) {
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
		    if ( commandName == Resource.NewQtProjectFullCommand ) {
			handled = true;
			MessageBox.Show( "New Qt Project", "Command" );
		    } else if ( commandName == Resource.MakeQtProjectFullCommand ) {
			handled = true;
			MessageBox.Show( "Make Qt Project", "Command" );
		    } else if ( commandName == Resource.LoadDesignerFullCommand ) {
			handled = true;
			extLoader.loadDesigner( "", true );
		    } else if ( commandName == Resource.LoadQtProjectFullCommand ) {
			handled = true;
			MessageBox.Show( "Load Qt Project", "Command" );
			extLoader.loadProject();
		    } else if ( commandName == Resource.SaveQtProjectFullCommand ) {
			handled = true;
			MessageBox.Show( "Save Qt Project", "Command" );
			extLoader.saveProject();
		    } else if ( commandName == Resource.DLLQtProjectFullCommand ) {
			handled = true;
			MessageBox.Show( "DLL Project", "Command" );
		    } else if ( commandName == Resource.AddMocStepFullCommand ) {
			handled = true;
			QMNCommands.addMocStep();
		    } else if ( commandName == Resource.mntEventsFullCommand ) {
			MonitorEvents();
		    } else if ( commandName == Resource.unmntEventsFullCommand ) {
			StopMonitoringEvents();
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

	// ----------------------------------------------------------
	// Event monitoring functions
	// ----------------------------------------------------------

	private EnvDTE.WindowEvents windowsEvents;
	private EnvDTE.TextEditorEvents textEditorEvents;
	private EnvDTE.TaskListEvents taskListEvents;
	private EnvDTE.SolutionEvents solutionEvents;
	private EnvDTE.SelectionEvents selectionEvents;
	private EnvDTE.OutputWindowEvents outputWindowEvents;
	private EnvDTE.FindEvents findEvents;
	private EnvDTE.DTEEvents dteEvents;
	private EnvDTE.DocumentEvents documentEvents;
	private EnvDTE.DebuggerEvents debuggerEvents;
	private EnvDTE.CommandEvents commandEvents;
	private EnvDTE.BuildEvents buildEvents;
	private EnvDTE.ProjectItemsEvents miscFilesEvents;
	private EnvDTE.ProjectItemsEvents solutionItemsEvents;
	private OutputWindowPane outputWindowPane;

	public void MonitorEvents() {
	    EnvDTE.Events events = applicationObject.Events;
	    OutputWindow outputWindow = (OutputWindow)applicationObject.Windows.Item(Constants.vsWindowKindOutput).Object;
	    outputWindowPane = outputWindow.OutputWindowPanes.Add("DTE Event Information");

	    //Retrieve the event objects from the automation model
	    windowsEvents = (EnvDTE.WindowEvents)events.get_WindowEvents(null);
	    textEditorEvents = (EnvDTE.TextEditorEvents)events.get_TextEditorEvents(null);
	    taskListEvents = (EnvDTE.TaskListEvents)events.get_TaskListEvents("");
	    solutionEvents = (EnvDTE.SolutionEvents)events.SolutionEvents;
	    selectionEvents = (EnvDTE.SelectionEvents)events.SelectionEvents;
	    outputWindowEvents = (EnvDTE.OutputWindowEvents)events.get_OutputWindowEvents("");
	    findEvents = (EnvDTE.FindEvents)events.FindEvents;
	    dteEvents = (EnvDTE.DTEEvents)events.DTEEvents;
	    documentEvents = (EnvDTE.DocumentEvents)events.get_DocumentEvents(null);
	    debuggerEvents = (EnvDTE.DebuggerEvents)events.DebuggerEvents;
	    commandEvents = (EnvDTE.CommandEvents)events.get_CommandEvents("{00000000-0000-0000-0000-000000000000}", 0);
	    buildEvents = (EnvDTE.BuildEvents)events.BuildEvents;
	    miscFilesEvents = (EnvDTE.ProjectItemsEvents)events.MiscFilesEvents;
	    solutionItemsEvents = (EnvDTE.ProjectItemsEvents)events.SolutionItemsEvents;

	    //Connect to each delegate exposed from each object retrieved above
	    windowsEvents.WindowActivated += new _dispWindowEvents_WindowActivatedEventHandler(this.WindowActivated);
	    windowsEvents.WindowClosing += new _dispWindowEvents_WindowClosingEventHandler(this.WindowClosing);
	    windowsEvents.WindowCreated += new _dispWindowEvents_WindowCreatedEventHandler(this.WindowCreated);
	    windowsEvents.WindowMoved += new _dispWindowEvents_WindowMovedEventHandler(this.WindowMoved);
			
	    textEditorEvents.LineChanged += new _dispTextEditorEvents_LineChangedEventHandler(this.LineChanged);

	    taskListEvents.TaskAdded += new _dispTaskListEvents_TaskAddedEventHandler(this.TaskAdded);
	    taskListEvents.TaskModified += new _dispTaskListEvents_TaskModifiedEventHandler(this.TaskModified);
	    taskListEvents.TaskNavigated += new _dispTaskListEvents_TaskNavigatedEventHandler(this.TaskNavigated);
	    taskListEvents.TaskRemoved += new _dispTaskListEvents_TaskRemovedEventHandler(this.TaskRemoved);

	    solutionEvents.AfterClosing += new _dispSolutionEvents_AfterClosingEventHandler(this.AfterClosing);
	    solutionEvents.BeforeClosing += new _dispSolutionEvents_BeforeClosingEventHandler(this.BeforeClosing);
	    solutionEvents.Opened += new _dispSolutionEvents_OpenedEventHandler(this.Opened);
	    solutionEvents.ProjectAdded += new _dispSolutionEvents_ProjectAddedEventHandler(this.ProjectAdded);
	    solutionEvents.ProjectRemoved += new _dispSolutionEvents_ProjectRemovedEventHandler(this.ProjectRemoved);
	    solutionEvents.ProjectRenamed += new _dispSolutionEvents_ProjectRenamedEventHandler(this.ProjectRenamed);
	    solutionEvents.QueryCloseSolution += new _dispSolutionEvents_QueryCloseSolutionEventHandler(this.QueryCloseSolution);
	    solutionEvents.Renamed += new _dispSolutionEvents_RenamedEventHandler(this.Renamed);

	    selectionEvents.OnChange += new _dispSelectionEvents_OnChangeEventHandler(this.OnChange);

	    outputWindowEvents.PaneAdded += new _dispOutputWindowEvents_PaneAddedEventHandler(this.PaneAdded);
	    outputWindowEvents.PaneClearing += new _dispOutputWindowEvents_PaneClearingEventHandler(this.PaneClearing);
	    outputWindowEvents.PaneUpdated += new _dispOutputWindowEvents_PaneUpdatedEventHandler(this.PaneUpdated);

	    findEvents.FindDone += new _dispFindEvents_FindDoneEventHandler(this.FindDone);

	    dteEvents.ModeChanged += new _dispDTEEvents_ModeChangedEventHandler(this.ModeChanged);
	    dteEvents.OnBeginShutdown += new _dispDTEEvents_OnBeginShutdownEventHandler(this.OnBeginShutdown);
	    dteEvents.OnMacrosRuntimeReset += new _dispDTEEvents_OnMacrosRuntimeResetEventHandler(this.OnMacrosRuntimeReset);
	    dteEvents.OnStartupComplete += new _dispDTEEvents_OnStartupCompleteEventHandler(this.OnStartupComplete);

	    documentEvents.DocumentClosing += new _dispDocumentEvents_DocumentClosingEventHandler(this.DocumentClosing);
	    documentEvents.DocumentOpened += new _dispDocumentEvents_DocumentOpenedEventHandler(this.DocumentOpened);
	    documentEvents.DocumentOpening += new _dispDocumentEvents_DocumentOpeningEventHandler(this.DocumentOpening);
	    documentEvents.DocumentSaved += new _dispDocumentEvents_DocumentSavedEventHandler(this.DocumentSaved);

	    debuggerEvents.OnContextChanged += new _dispDebuggerEvents_OnContextChangedEventHandler(this.OnContextChanged);
	    debuggerEvents.OnEnterBreakMode += new _dispDebuggerEvents_OnEnterBreakModeEventHandler(this.OnEnterBreakMode);
	    debuggerEvents.OnEnterDesignMode += new _dispDebuggerEvents_OnEnterDesignModeEventHandler(this.OnEnterDesignMode);
	    debuggerEvents.OnEnterRunMode += new _dispDebuggerEvents_OnEnterRunModeEventHandler(this.OnEnterRunMode);
	    debuggerEvents.OnExceptionNotHandled += new _dispDebuggerEvents_OnExceptionNotHandledEventHandler(this.OnExceptionNotHandled);
	    debuggerEvents.OnExceptionThrown += new _dispDebuggerEvents_OnExceptionThrownEventHandler(this.OnExceptionThrown);

	    commandEvents.AfterExecute += new _dispCommandEvents_AfterExecuteEventHandler(this.AfterExecute);
	    commandEvents.BeforeExecute += new _dispCommandEvents_BeforeExecuteEventHandler(this.BeforeExecute);

	    buildEvents.OnBuildBegin += new _dispBuildEvents_OnBuildBeginEventHandler(this.OnBuildBegin);
	    buildEvents.OnBuildDone += new _dispBuildEvents_OnBuildDoneEventHandler(this.OnBuildDone);
	    buildEvents.OnBuildProjConfigBegin += new _dispBuildEvents_OnBuildProjConfigBeginEventHandler(this.OnBuildProjConfigBegin);
	    buildEvents.OnBuildProjConfigDone += new _dispBuildEvents_OnBuildProjConfigDoneEventHandler(this.OnBuildProjConfigDone);

	    miscFilesEvents.ItemAdded += new _dispProjectItemsEvents_ItemAddedEventHandler(this.MiscFilesEvents_ItemAdded);
	    miscFilesEvents.ItemRemoved += new _dispProjectItemsEvents_ItemRemovedEventHandler(this.MiscFilesEvents_ItemRemoved);
	    miscFilesEvents.ItemRenamed += new _dispProjectItemsEvents_ItemRenamedEventHandler(this.MiscFilesEvents_ItemRenamed);

	    solutionItemsEvents.ItemAdded += new _dispProjectItemsEvents_ItemAddedEventHandler(this.SolutionItemsEvents_ItemAdded);
	    solutionItemsEvents.ItemRemoved += new _dispProjectItemsEvents_ItemRemovedEventHandler(this.SolutionItemsEvents_ItemRemoved);
	    solutionItemsEvents.ItemRenamed += new _dispProjectItemsEvents_ItemRenamedEventHandler(this.SolutionItemsEvents_ItemRenamed);
	}

	public void StopMonitoringEvents() 
	{
	    if(windowsEvents != null) {
		windowsEvents.WindowActivated -= new _dispWindowEvents_WindowActivatedEventHandler(this.WindowActivated);
		windowsEvents.WindowClosing -= new _dispWindowEvents_WindowClosingEventHandler(this.WindowClosing);
		windowsEvents.WindowCreated -= new _dispWindowEvents_WindowCreatedEventHandler(this.WindowCreated);
		windowsEvents.WindowMoved -= new _dispWindowEvents_WindowMovedEventHandler(this.WindowMoved);
	    }
			
	    if(textEditorEvents != null)
		textEditorEvents.LineChanged -= new _dispTextEditorEvents_LineChangedEventHandler(this.LineChanged);

	    if(taskListEvents != null) {
		taskListEvents.TaskAdded -= new _dispTaskListEvents_TaskAddedEventHandler(this.TaskAdded);
		taskListEvents.TaskModified -= new _dispTaskListEvents_TaskModifiedEventHandler(this.TaskModified);
		taskListEvents.TaskNavigated -= new _dispTaskListEvents_TaskNavigatedEventHandler(this.TaskNavigated);
		taskListEvents.TaskRemoved -= new _dispTaskListEvents_TaskRemovedEventHandler(this.TaskRemoved);
	    }

	    if(solutionEvents != null) {
		solutionEvents.AfterClosing -= new _dispSolutionEvents_AfterClosingEventHandler(this.AfterClosing);
		solutionEvents.BeforeClosing -= new _dispSolutionEvents_BeforeClosingEventHandler(this.BeforeClosing);
		solutionEvents.Opened -= new _dispSolutionEvents_OpenedEventHandler(this.Opened);
		solutionEvents.ProjectAdded -= new _dispSolutionEvents_ProjectAddedEventHandler(this.ProjectAdded);
		solutionEvents.ProjectRemoved -= new _dispSolutionEvents_ProjectRemovedEventHandler(this.ProjectRemoved);
		solutionEvents.ProjectRenamed -= new _dispSolutionEvents_ProjectRenamedEventHandler(this.ProjectRenamed);
		solutionEvents.QueryCloseSolution -= new _dispSolutionEvents_QueryCloseSolutionEventHandler(this.QueryCloseSolution);
		solutionEvents.Renamed -= new _dispSolutionEvents_RenamedEventHandler(this.Renamed);
	    }

	    if(selectionEvents != null)
		selectionEvents.OnChange -= new _dispSelectionEvents_OnChangeEventHandler(this.OnChange);

	    if(outputWindowEvents != null) {
		outputWindowEvents.PaneAdded -= new _dispOutputWindowEvents_PaneAddedEventHandler(this.PaneAdded);
		outputWindowEvents.PaneClearing -= new _dispOutputWindowEvents_PaneClearingEventHandler(this.PaneClearing);
		outputWindowEvents.PaneUpdated -= new _dispOutputWindowEvents_PaneUpdatedEventHandler(this.PaneUpdated);
	    }

	    if(findEvents != null)
		findEvents.FindDone -= new _dispFindEvents_FindDoneEventHandler(this.FindDone);
  
	    if(dteEvents != null) {
		dteEvents.ModeChanged -= new _dispDTEEvents_ModeChangedEventHandler(this.ModeChanged);
		dteEvents.OnBeginShutdown -= new _dispDTEEvents_OnBeginShutdownEventHandler(this.OnBeginShutdown);
		dteEvents.OnMacrosRuntimeReset -= new _dispDTEEvents_OnMacrosRuntimeResetEventHandler(this.OnMacrosRuntimeReset);
		dteEvents.OnStartupComplete -= new _dispDTEEvents_OnStartupCompleteEventHandler(this.OnStartupComplete);
	    }

	    if(documentEvents != null) {
		documentEvents.DocumentClosing -= new _dispDocumentEvents_DocumentClosingEventHandler(this.DocumentClosing);
		documentEvents.DocumentOpened -= new _dispDocumentEvents_DocumentOpenedEventHandler(this.DocumentOpened);
		documentEvents.DocumentOpening -= new _dispDocumentEvents_DocumentOpeningEventHandler(this.DocumentOpening);
		documentEvents.DocumentSaved -= new _dispDocumentEvents_DocumentSavedEventHandler(this.DocumentSaved);
	    }

	    if(debuggerEvents != null) {
		debuggerEvents.OnContextChanged -= new _dispDebuggerEvents_OnContextChangedEventHandler(this.OnContextChanged);
		debuggerEvents.OnEnterBreakMode -= new _dispDebuggerEvents_OnEnterBreakModeEventHandler(this.OnEnterBreakMode);
		debuggerEvents.OnEnterDesignMode -= new _dispDebuggerEvents_OnEnterDesignModeEventHandler(this.OnEnterDesignMode);
		debuggerEvents.OnEnterRunMode -= new _dispDebuggerEvents_OnEnterRunModeEventHandler(this.OnEnterRunMode);
		debuggerEvents.OnExceptionNotHandled -= new _dispDebuggerEvents_OnExceptionNotHandledEventHandler(this.OnExceptionNotHandled);
		debuggerEvents.OnExceptionThrown -= new _dispDebuggerEvents_OnExceptionThrownEventHandler(this.OnExceptionThrown);
	    }

	    if(commandEvents != null) {
		commandEvents.AfterExecute -= new _dispCommandEvents_AfterExecuteEventHandler(this.AfterExecute);
		commandEvents.BeforeExecute -= new _dispCommandEvents_BeforeExecuteEventHandler(this.BeforeExecute);
	    }

	    if(buildEvents != null) {
		buildEvents.OnBuildBegin -= new _dispBuildEvents_OnBuildBeginEventHandler(this.OnBuildBegin);
		buildEvents.OnBuildDone -= new _dispBuildEvents_OnBuildDoneEventHandler(this.OnBuildDone);
		buildEvents.OnBuildProjConfigBegin -= new _dispBuildEvents_OnBuildProjConfigBeginEventHandler(this.OnBuildProjConfigBegin);
		buildEvents.OnBuildProjConfigDone -= new _dispBuildEvents_OnBuildProjConfigDoneEventHandler(this.OnBuildProjConfigDone);
	    }

	    if(miscFilesEvents != null) {
		miscFilesEvents.ItemAdded -= new _dispProjectItemsEvents_ItemAddedEventHandler(this.MiscFilesEvents_ItemAdded);
		miscFilesEvents.ItemRemoved -= new _dispProjectItemsEvents_ItemRemovedEventHandler(this.MiscFilesEvents_ItemRemoved);
		miscFilesEvents.ItemRenamed -= new _dispProjectItemsEvents_ItemRenamedEventHandler(this.MiscFilesEvents_ItemRenamed);
	    }

	    if(solutionItemsEvents != null) {
		solutionItemsEvents.ItemAdded -= new _dispProjectItemsEvents_ItemAddedEventHandler(this.SolutionItemsEvents_ItemAdded);
		solutionItemsEvents.ItemRemoved -= new _dispProjectItemsEvents_ItemRemovedEventHandler(this.SolutionItemsEvents_ItemRemoved);
		solutionItemsEvents.ItemRenamed -= new _dispProjectItemsEvents_ItemRenamedEventHandler(this.SolutionItemsEvents_ItemRenamed);
	    }
	}

	//WindowEvents
	public void WindowClosing(EnvDTE.Window closingWindow) {
	    outputWindowPane.OutputString("WindowEvents::WindowClosing\n");
	    outputWindowPane.OutputString("\tWindow: " + closingWindow.Caption + "\n");
	}

	public void WindowActivated(EnvDTE.Window gotFocus, EnvDTE.Window lostFocus) {
	    outputWindowPane.OutputString("WindowEvents::WindowActivated\n");
	    outputWindowPane.OutputString("\tWindow receiving focus: " + gotFocus.Caption + "\n");
	    outputWindowPane.OutputString("\tWindow that lost focus: " + lostFocus.Caption + "\n");
	}

	public void WindowCreated(EnvDTE.Window window) {
	    outputWindowPane.OutputString("WindowEvents::WindowCreated\n");
	    outputWindowPane.OutputString("\tWindow: " + window.Caption + "\n");
	}

	public void WindowMoved(EnvDTE.Window window, int top, int left, int width, int height) {
	    outputWindowPane.OutputString("WindowEvents::WindowMoved\n");
	    outputWindowPane.OutputString("\tWindow: " + window.Caption + "\n");
	    outputWindowPane.OutputString("\tLocation: (" + top.ToString() + " , " + left.ToString() + " , " + width.ToString() + " , " + height.ToString() + ")\n");
	}

	//TextEditorEvents
	public void LineChanged(EnvDTE.TextPoint startPoint, EnvDTE.TextPoint endPoint, int hint) {
	    vsTextChanged textChangedHint = (vsTextChanged)hint;
	    outputWindowPane.OutputString("TextEditorEvents::LineChanged\n");
	    outputWindowPane.OutputString("\tDocument: " + startPoint.Parent.Parent.Name + "\n");
	    outputWindowPane.OutputString("\tChange hint: " + textChangedHint.ToString() + "\n");
	}

	//TaskListEvents
	public void TaskAdded(EnvDTE.TaskItem taskItem) {
	    outputWindowPane.OutputString("TaskListEvents::TaskAdded\n");
	    outputWindowPane.OutputString("\tTask description: " + taskItem.Description + "\n");
	}

	public void TaskModified(EnvDTE.TaskItem taskItem, EnvDTE.vsTaskListColumn columnModified) {
	    outputWindowPane.OutputString("TaskListEvents::TaskModified\n");
	    outputWindowPane.OutputString("\tTask description: " + taskItem.Description + "\n");
	}

	public void TaskNavigated(EnvDTE.TaskItem taskItem, ref bool navigateHandled) {
	    outputWindowPane.OutputString("TaskListEvents::TaskNavigated\n");
	    outputWindowPane.OutputString("\tTask description: " + taskItem.Description + "\n");
	}

	public void TaskRemoved(EnvDTE.TaskItem taskItem) {
	    outputWindowPane.OutputString("TaskListEvents::TaskRemoved\n");
	    outputWindowPane.OutputString("\tTask description: " + taskItem.Description + "\n");
	}

	//SolutionEvents
	public void AfterClosing() {
	    outputWindowPane.OutputString("SolutionEvents::AfterClosing\n");
	}

	public void BeforeClosing() {
	    outputWindowPane.OutputString("SolutionEvents::BeforeClosing\n");
	}

	public void Opened() {
	    outputWindowPane.OutputString("SolutionEvents::Opened\n");
	}

	public void ProjectAdded(EnvDTE.Project project) {
	    outputWindowPane.OutputString("SolutionEvents::ProjectAdded\n");
	    outputWindowPane.OutputString("\tProject: " + project.UniqueName + "\n");
	}

	public void ProjectRemoved(EnvDTE.Project project) {
	    outputWindowPane.OutputString("SolutionEvents::ProjectRemoved\n");
	    outputWindowPane.OutputString("\tProject: " + project.UniqueName + "\n");
	}

	public void ProjectRenamed(EnvDTE.Project project, string oldName) {
	    outputWindowPane.OutputString("SolutionEvents::ProjectRenamed\n");
	    outputWindowPane.OutputString("\tProject: " + project.UniqueName + "\n");
	}

	public void QueryCloseSolution(ref bool cancel) {
	    outputWindowPane.OutputString("SolutionEvents::QueryCloseSolution\n");
	}

	public void Renamed(string oldName) {
	    outputWindowPane.OutputString("SolutionEvents::Renamed\n");
	}

	//SelectionEvents
	public void OnChange() {
	    outputWindowPane.OutputString("SelectionEvents::OnChange\n");
	    int count = applicationObject.SelectedItems.Count;
	    for (int i = 1 ; i <= applicationObject.SelectedItems.Count ; i++) {
		outputWindowPane.OutputString("Item name: " + applicationObject.SelectedItems.Item(i).Name + "\n");
	    }
	}

	//OutputWindowEvents
	public void PaneAdded(EnvDTE.OutputWindowPane pane) {
	    outputWindowPane.OutputString("OutputWindowEvents::PaneAdded\n");
	    outputWindowPane.OutputString("\tPane: " + pane.Name + "\n");
	}

	public void PaneClearing(EnvDTE.OutputWindowPane pane) {
	    outputWindowPane.OutputString("OutputWindowEvents::PaneClearing\n");
	    outputWindowPane.OutputString("\tPane: " + pane.Name + "\n");
	}

	public void PaneUpdated(EnvDTE.OutputWindowPane pane) {
	    //Dont want to do this one, or we will end up in a recursive call:
	    //outputWindowPane.OutputString("OutputWindowEvents::PaneUpdated\n");
	    //outputWindowPane.OutputString("\tPane: " + pane.Name + "\n");
	}

	//FindEvents
	public void FindDone(EnvDTE.vsFindResult result, bool cancelled) {
	    outputWindowPane.OutputString("FindEvents::FindDone\n");
	}

	//DTEEvents
	public void ModeChanged(EnvDTE.vsIDEMode LastMode) {
	    outputWindowPane.OutputString("DTEEvents::ModeChanged\n");
	}

	public void OnBeginShutdown() {
	    outputWindowPane.OutputString("DTEEvents::OnBeginShutdown\n");
	}

	public void OnMacrosRuntimeReset() {
	    outputWindowPane.OutputString("DTEEvents::OnMacrosRuntimeReset\n");
	}

	public void OnStartupComplete() {
	    outputWindowPane.OutputString("DTEEvents::OnStartupComplete\n");
	}

	//DocumentEvents
	public void DocumentClosing(EnvDTE.Document document) {
	    outputWindowPane.OutputString("DocumentEvents::DocumentClosing\n");
	    outputWindowPane.OutputString("\tDocument: " + document.Name + "\n");
	}

	public void DocumentOpened(EnvDTE.Document document) {
	    outputWindowPane.OutputString("DocumentEvents::DocumentOpened\n");
	    outputWindowPane.OutputString("\tDocument: " + document.Name + "\n");
	}

	public void DocumentOpening(string documentPath, bool ReadOnly) {
	    outputWindowPane.OutputString("DocumentEvents::DocumentOpening\n");
	    outputWindowPane.OutputString("\tPath: " + documentPath + "\n");
	}

	public void DocumentSaved(EnvDTE.Document document) {
	    outputWindowPane.OutputString("DocumentEvents::DocumentSaved\n");
	    outputWindowPane.OutputString("\tDocument: " + document.Name + "\n");
	}

	//DebuggerEvents
	public void OnContextChanged(EnvDTE.Process NewProcess, EnvDTE.Program NewProgram, EnvDTE.Thread NewThread, EnvDTE.StackFrame NewStackFrame) {
	    outputWindowPane.OutputString("DebuggerEvents::OnContextChanged\n");
	}

	public void OnEnterBreakMode(EnvDTE.dbgEventReason reason, ref EnvDTE.dbgExecutionAction executionAction) {
	    executionAction = EnvDTE.dbgExecutionAction.dbgExecutionActionDefault;
	    outputWindowPane.OutputString("DebuggerEvents::OnEnterBreakMode\n");
	}

	public void OnEnterDesignMode(EnvDTE.dbgEventReason Reason) {
	    outputWindowPane.OutputString("DebuggerEvents::OnEnterDesignMode\n");
	}

	public void OnEnterRunMode(EnvDTE.dbgEventReason Reason) {
	    outputWindowPane.OutputString("DebuggerEvents::OnEnterRunMode\n");
	}

	public void OnExceptionNotHandled(string exceptionType, string name, int code, string description, ref EnvDTE.dbgExceptionAction exceptionAction) {
	    exceptionAction = EnvDTE.dbgExceptionAction.dbgExceptionActionDefault;
	    outputWindowPane.OutputString("DebuggerEvents::OnExceptionNotHandled\n");
	}

	public void OnExceptionThrown(string exceptionType, string name, int code, string description, ref EnvDTE.dbgExceptionAction exceptionAction) {
	    exceptionAction = EnvDTE.dbgExceptionAction.dbgExceptionActionDefault;
	    outputWindowPane.OutputString("DebuggerEvents::OnExceptionThrown\n");
	}

	//CommandEvents
	public void AfterExecute(string Guid, int ID, object CustomIn, object CustomOut) {
	    string commandName = "";
	    try {
		commandName = applicationObject.Commands.Item(Guid, ID).Name;
	    }
	    catch (System.Exception ) {
	    }
	    outputWindowPane.OutputString("CommandEvents::AfterExecute\n");
	    if(commandName != "")
		outputWindowPane.OutputString("\tCommand name: " + commandName+ "\n");
	    outputWindowPane.OutputString("\tCommand GUID/ID: " + Guid + ", " + ID.ToString() + "\n");
	}

	public void BeforeExecute(string Guid, int ID, object CustomIn, object CustomOut, ref bool CancelDefault) {
	    string commandName = "";
	    try {
		commandName = applicationObject.Commands.Item(Guid, ID).Name;
	    }
	    catch (System.Exception ) {
	    }
	    outputWindowPane.OutputString("CommandEvents::BeforeExecute\n");
	    if(commandName != "")
		outputWindowPane.OutputString("\tCommand name: " + commandName+ "\n");
	    outputWindowPane.OutputString("\tCommand GUID/ID: " + Guid + ", " + ID.ToString() + "\n");
	}

	//BuildEvents
	public void OnBuildBegin(EnvDTE.vsBuildScope Scope, EnvDTE.vsBuildAction Action) {
	    outputWindowPane.OutputString("BuildEvents::OnBuildBegin\n");
	}

	public void OnBuildDone(EnvDTE.vsBuildScope Scope, EnvDTE.vsBuildAction Action) {
	    outputWindowPane.OutputString("BuildEvents::OnBuildDone\n");
	}

	public void OnBuildProjConfigBegin(string project, string projectConfig, string platform, string solutionConfig) {
	    outputWindowPane.OutputString("BuildEvents::OnBuildProjConfigBegin\n");
	    outputWindowPane.OutputString("\tProject: " + project + "\n");
	    outputWindowPane.OutputString("\tProject Configuration: " + projectConfig + "\n");
	    outputWindowPane.OutputString("\tPlatform: " + platform + "\n");
	    outputWindowPane.OutputString("\tSolution Configuration: " + solutionConfig + "\n");
	}

	public void OnBuildProjConfigDone(string project, string projectConfig, string platform, string solutionConfig, bool success) {
	    outputWindowPane.OutputString("BuildEvents::OnBuildProjConfigDone\n");
	    outputWindowPane.OutputString("\tProject: " + project + "\n");
	    outputWindowPane.OutputString("\tProject Configuration: " + projectConfig + "\n");
	    outputWindowPane.OutputString("\tPlatform: " + platform + "\n");
	    outputWindowPane.OutputString("\tSolution Configuration: " + solutionConfig + "\n");
	    outputWindowPane.OutputString("\tBuild success: " + success.ToString() + "\n");
	}

	//MiscFilesEvents
	public void MiscFilesEvents_ItemAdded(EnvDTE.ProjectItem projectItem) {
	    outputWindowPane.OutputString("MiscFilesEvents::ItemAdded\n");
	    outputWindowPane.OutputString("\tProject Item: " + projectItem.Name + "\n");
	}

	public void MiscFilesEvents_ItemRemoved(EnvDTE.ProjectItem projectItem) {
	    outputWindowPane.OutputString("MiscFilesEvents::ItemRemoved\n");
	    outputWindowPane.OutputString("\tProject Item: " + projectItem.Name + "\n");
	}

	public void MiscFilesEvents_ItemRenamed(EnvDTE.ProjectItem projectItem, string OldName) {
	    outputWindowPane.OutputString("MiscFilesEvents::ItemRenamed\n");
	    outputWindowPane.OutputString("\tProject Item: " + projectItem.Name + "\n");
	}

	//SolutionItemsEvents
	public void SolutionItemsEvents_ItemAdded(EnvDTE.ProjectItem projectItem) {
	    outputWindowPane.OutputString("SolutionItemsEvents::ItemAdded\n");
	    outputWindowPane.OutputString("\tProject Item: " + projectItem.Name + "\n");
	}

	public void SolutionItemsEvents_ItemRemoved(EnvDTE.ProjectItem projectItem) {
	    outputWindowPane.OutputString("SolutionItemsEvents::ItemRemoved\n");
	    outputWindowPane.OutputString("\tProject Item: " + projectItem.Name + "\n");
	}

	public void SolutionItemsEvents_ItemRenamed(EnvDTE.ProjectItem projectItem, string OldName) {
	    outputWindowPane.OutputString("SolutionItemsEvents::ItemRenamed\n");
	    outputWindowPane.OutputString("\tProject Item: " + projectItem.Name + "\n");
	}
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