using System;

namespace QMsNet
{
    using EnvDTE;
    using System;
    using System.IO;
    using System.Windows.Forms;
    using System.Diagnostics;
    using System.Runtime.InteropServices;
    using Extensibility;

    public class ExtLoader
    {
	[DllImport("user32.dll")] private static extern bool SetForegroundWindow(IntPtr hWnd);

	// Variabels ------------------------------------------------------
	private static System.Diagnostics.Process prcDesigner = null;
	private EnvDTE.DocumentEvents docEvents;
	private System.Timers.Timer tmUIKiller = null;

	// Functions ------------------------------------------------------
	public ExtLoader()
	{
	    tmUIKiller = new System.Timers.Timer( 1.0 );
	    tmUIKiller.Elapsed += new System.Timers.ElapsedEventHandler(OnTimer);
	    tmUIKiller.Stop();
	}
	public void loadProject()
	{
	}
	public void saveProject()
	{
	}

	public void loadDesigner( string file, bool locateProject ) 
	{
	    try {
		try {
		    if ( locateProject ) {
			System.Array prjs = (System.Array)Connect.applicationObject.ActiveSolutionProjects;
			EnvDTE.Project prj = (EnvDTE.Project)prjs.GetValue( 0 );
			string temp = prj.FileName;
			if( temp.Length != 0 ) {
			    file = temp.Substring( 0, temp.LastIndexOf(".") );
			    file += ".pro";
			}
		    }
		    file = "-client \"" + file +"\"";
		}
		catch( System.Exception ) {}

		System.Diagnostics.Process tmp = new System.Diagnostics.Process();
		tmp.StartInfo.FileName = "Designer";
		tmp.StartInfo.Arguments = file;
		tmp.StartInfo.WindowStyle = ProcessWindowStyle.Normal;
		tmp.Start();

		if( prcDesigner == null || 
		   (prcDesigner != null && prcDesigner.HasExited == true) )
		    prcDesigner = tmp;

		SetForegroundWindow( prcDesigner.MainWindowHandle );
	    }
	    catch ( System.Exception ) {
		MessageBox.Show( "*** Couldn't start Designer!   " +
				 "Please verify that Designer.exe is in your path.\n\r\n\r" +
				 "Path = " + Environment.GetEnvironmentVariable("PATH"),
				 "Not starting Designer" );
	    }
	}
	
	public void startUp()
	{
	    // Monitor document events to look out for opening 
	    // of .ui files, so we may start Designer...
	    docEvents = Connect.applicationObject.Events.get_DocumentEvents( null );
	    docEvents.DocumentOpening += new EnvDTE._dispDocumentEvents_DocumentOpeningEventHandler( OnDocumentOpening );
	    docEvents.DocumentOpened += new EnvDTE._dispDocumentEvents_DocumentOpenedEventHandler( OnDocumentOpened );
	}

	public void shutDown()
	{
	    // Unregister the event handler
	    if ( docEvents != null ) {
		docEvents.DocumentOpening -= new EnvDTE._dispDocumentEvents_DocumentOpeningEventHandler( OnDocumentOpening );
		docEvents.DocumentOpened -= new EnvDTE._dispDocumentEvents_DocumentOpenedEventHandler( OnDocumentOpened );
	    }
	}

	// Event handlers -------------------------------------------
	private void OnDocumentOpening( string path, bool readOnly ) {
	    if( path.EndsWith(".ui") )
		loadDesigner( path, false );
	}

	private void OnDocumentOpened( Document doc ) {
	    if( doc.Name.EndsWith(".ui") ) {
		doc.Close( vsSaveChanges.vsSaveChangesNo );
		tmUIKiller.Start();
	    }
	}

	private void OnTimer( Object source, System.Timers.ElapsedEventArgs e ) {
	    tmUIKiller.Stop();
	    foreach( Window wnd in Connect.applicationObject.Windows )
		if ( wnd.Caption == "" )
		    wnd.Close( vsSaveChanges.vsSaveChangesNo );
	
	}
    }
}
