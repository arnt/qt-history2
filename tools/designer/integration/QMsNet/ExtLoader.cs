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
	private static System.Diagnostics.Process prc = null;
	private EnvDTE.DocumentEvents docEvents;

	// Functions ------------------------------------------------------
	public ExtLoader()
	{
	}
	public void loadProject()
	{
	}
	public void saveProject()
	{
	}

	public void loadDesigner( string file, bool onlyOne ) 
	{
	    try {
		if( prc != null && !onlyOne ) {
		    if( prc.HasExited != true ) {
			file = "-client \"" + file +"\"";
			SetForegroundWindow( prc.MainWindowHandle );
		    }
		}

		prc = new System.Diagnostics.Process();
		prc.StartInfo.FileName = "Designer";
		prc.StartInfo.Arguments = file;
		prc.StartInfo.WindowStyle = ProcessWindowStyle.Normal;
		prc.Start();
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
	}

	public void shutDown()
	{
	    // Unregister the event handler
	    if ( docEvents != null )
		docEvents.DocumentOpening -= new EnvDTE._dispDocumentEvents_DocumentOpeningEventHandler( OnDocumentOpening );
	}

	// Event handlers -------------------------------------------
	private void OnDocumentOpening( string path, bool readOnly ) {
	    if( path.EndsWith(".ui") )
		loadDesigner( path, false );
	}
    }
}
