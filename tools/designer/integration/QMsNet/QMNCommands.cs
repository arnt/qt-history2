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

	public static void addMocStep() 
	{
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

		string[] fileparts = vf.Name.Split( new Char[] {'.'}, 2 );
		string filename = fileparts.Length > 0 ? fileparts[0] : "";
		string ext = fileparts.Length > 1 ? fileparts[1] : "";

		foreach ( VCFileConfiguration vfc in (IVCCollection)vf.FileConfigurations ) {
		    VCCustomBuildTool cbt = (VCCustomBuildTool)vfc.Tool;
		    cbt.Description		= "Moc'ing " + vf.Name + "...";
		    cbt.AdditionalDependencies  = "$(QTDIR)\\bin\\moc.exe";
		    cbt.Outputs			= "$(IntDir)\\moc\\moc_" + filename + ".cpp";
		    cbt.CommandLine		= cbt.AdditionalDependencies + " " +
						  vf.FullPath + " " +
						  "-o " +
						  cbt.Outputs;
		}

		Connect.applicationObject.StatusBar.Text = "Moc step added to file...";
	    }
	    catch( System.Exception e ) {
		Debug.Write( e.Message + "\r\n" + e.StackTrace.ToString(),
			     "Couldn't addMocStep()" );
	    }
	}

	public static void makeProjectDll() 
	{
	    ProjectItem itm = Connect.applicationObject
		.ActiveDocument.ProjectItem;
	    VCProject prj = (VCProject)((VCProjectItem)itm.Object).Project;
	    foreach ( VCConfiguration pc in (IVCCollection)prj.Configurations ) {
		pc.ConfigurationType = ConfigurationTypes.typeDynamicLibrary;
		VCLinkerTool lt = (VCLinkerTool)((IVCCollection)pc.Tools).Item("VCLinkerTool");

		int index = lt.OutputFile.LastIndexOf( '.' );
		string filename = lt.OutputFile.Substring( 0, index );
		lt.OutputFile = filename + ".dll";
	    }
	}
    }
}