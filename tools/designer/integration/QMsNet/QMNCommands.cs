using EnvDTE;
using System;
using System.Drawing;
using System.Windows.Forms;
using System.Diagnostics;
using System.Runtime.InteropServices;
using Microsoft.Office.Core;
using Microsoft.VisualStudio.VCProjectEngine;
using Extensibility;

namespace QMsNet
{
    public class QMNCommands
    {
	public QMNCommands()
	{
	}

	public static void addMocStep() {
	    try {
		Document doc = Connect.applicationObject.ActiveDocument;
		if ( doc == null ) {
		    Connect.applicationObject.StatusBar.Text = "Cannot add Moc step... No file open";
		    return;
		}

		ProjectItem itm = doc.ProjectItem;
		VCProjectItem vitm = (VCProjectItem)itm.Object;
		VCProject vp = (VCProject)vitm.Project;
		IVCCollection myFiles = (IVCCollection)vp.files;
		VCFile vf = (VCFile)myFiles.Item( doc.FullName );
		foreach ( VCFileConfiguration vfc in (IVCCollection)vf.FileConfigurations ) {
		    VCCustomBuildTool cbt = (VCCustomBuildTool)vfc.Tool;
		    // Modify custom build step...
		}

		Connect.applicationObject.StatusBar.Text = "Moc step added to file...";
	    }
	    catch( System.Exception e ) {
		Debug.Write( e.Message + "\r\n" + e.StackTrace.ToString(),
			     "Couldn't addMocStep()" );
	    }
	}
    }
}