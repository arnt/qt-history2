using System;

namespace QMsNet
{
    using EnvDTE;
    using System;
    using System.IO;
    using System.Windows.Forms;
    using System.Diagnostics;
    using Extensibility;

    public class ExtLoader
    {
	// Variabels ------------------------------------------------------
	private static System.Diagnostics.Process prc = null;

	// Functions ------------------------------------------------------
	public ExtLoader()
	{
	}
	public static void loadProject()
	{
	}
	public static void saveProject()
	{
	}
	public static void loadDesigner() 
	{
	    try {
		if( prc != null ) {
		    if( prc.HasExited == true ) {
			prc = null;
		    } else {
			Connect.applicationObject.StatusBar.Text = "Designer is already running...";
			return;
		    }
		}

		prc = new System.Diagnostics.Process();
		prc.StartInfo.FileName = "Designer";
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
	public static void startUp()
	{
	}
	public static void shutDown()
	{
	}
    }
}
