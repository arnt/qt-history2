using EnvDTE;
using System;
using System.IO;
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

	// Moc functions ------------------------------------------------
	public static void addMocStep() 
	{
	    try {
		Document doc = Connect.applicationObject.ActiveDocument;
		if ( doc == null ) {
		    Say ( "Cannot add Moc step... No file open" );
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

		Say( "Adding Moc step for " + vf.Name );
		// Moc'ing a CPP ------------------------------------
		if ( vf.Name.EndsWith(".cpp") ) {
		    if ( DialogResult.No == MessageBox.Show( "Normally one only Moc headerfiles.\n\r" +
							     "Are you sure you want to continue Moc'ing the sourcefile?",
							     "Moc sourcefile?", 
							     MessageBoxButtons.YesNo ) )
			return;

		    if ( !vp.CanAddFile( filename + ".moc" ) )
			throw new System.Exception( "*** Couldn't add moc file to project" );

		    addFileInFilter( vp, "Generated MOC Files", filename + ".moc" );
		    VCFile mf = (VCFile)((IVCCollection)vp.files).Item( filename + ".moc" );
		    addMStep( mf, filename, true );
		// Moc'ing a H --------------------------------------
		} else {
		    addMStep( vf, filename, false );
		    addFileInFilter( vp, "Generated MOC Files", "moc_" + filename + ".cpp" );
		}
	    }
	    catch( System.Exception e ) {
		Say( e, "Couldn't addMocStep()" );
	    }
	}
	
	private static bool addMStep( VCFile file, string filename, bool local ) 
	{
	    try {
		foreach ( VCFileConfiguration vfc in (IVCCollection)file.FileConfigurations ) {
		    VCCustomBuildTool cbt = (VCCustomBuildTool)vfc.Tool;
		    cbt.AdditionalDependencies  = "$(QTDIR)\\bin\\moc.exe";
		    if ( local ) {
			cbt.Description = "Moc'ing " + filename + ".cpp...";
			cbt.Outputs	=  filename + ".moc";
			cbt.CommandLine = cbt.AdditionalDependencies + " " + filename + ".cpp -o " + cbt.Outputs;
			
		    } else {
			cbt.Description = "Moc'ing " + file.Name + "...";
			cbt.Outputs	= "moc_" + filename + ".cpp";
			cbt.CommandLine = cbt.AdditionalDependencies + " " + file.RelativePath + " -o " + cbt.Outputs;
		    }
		}
		return true;
	    }
	    catch( System.Exception e ) {
		Say( e, "Couldn't addMStep()" );
	    }
	    return false;
	}

	private static void addFileInFilter( VCProject project, string filterName, string fileName ) 
	{
	    try {
		VCFilter vfilt = (VCFilter)((IVCCollection)project.Filters).Item( filterName );

		if ( vfilt == null ) {
		    if ( !project.CanAddFilter( filterName ) )
			throw new System.Exception( "Project can't add filter" + filterName );
		    project.AddFilter( filterName );
		}

		vfilt = (VCFilter)((IVCCollection)project.Filters).Item( filterName );
		if( vfilt.CanAddFile( fileName ) )
		    vfilt.AddFile( fileName );
	    }
	    catch( System.Exception e ) {
		Say( e, "Couldn't addFileInFilter()" );
	    }
	}

	// Project use Qt DLL functions ------------------------------------
	public static void makeQtProject() 
	{
	    try {
		Document doc = Connect.applicationObject.ActiveDocument;
		if ( doc == null ) {
		    Say( "Cannot add Qt to project... No project open" );
		    return;
		}
		ProjectItem itm = doc.ProjectItem;
		VCProject prj = (VCProject)((VCProjectItem)itm.Object).Project;
		foreach ( VCConfiguration pc in (IVCCollection)prj.Configurations ) {
		    Say( "Adding Qt settings for project configuration " + pc.Name );
		    VCLinkerTool lt = null;
		    try {
			lt = (VCLinkerTool)((IVCCollection)pc.Tools).Item("VCLinkerTool");
		    }
		    catch {
			Say( "Couldn't get Linker configuration! Does this configuration have a linker tool defined?" );
			return;
		    }
		    int index = lt.OutputFile.LastIndexOf( '.' );
		    string filename = lt.OutputFile.Substring( 0, index );
		    string qtlib = "qt";
		    if ( Resource.qtThreaded )
			qtlib += "-mt";
		    qtlib += Resource.qtVersion + ".lib";
		    lt.OutputFile = filename + ".dll";
		    lt.AdditionalLibraryDirectories += "$(QTDIR)\\lib";
		    lt.AdditionalDependencies += qtlib;
		    lt.AdditionalDependencies += "qtmain.lib";
		}
	    }
	    catch( System.Exception e ) {
		Say( e, "Couldn't makeQtProject()" );
	    }
	}
	// Project to DLL functions ----------------------------------------
	public static void makeProjectDll() {
	    try {
		Document doc = Connect.applicationObject.ActiveDocument;
		if ( doc == null ) {
		    Say( "Cannot convert static to dynamic project... No project open" );
		    return;
		}
		ProjectItem itm = doc.ProjectItem;
		VCProject prj = (VCProject)((VCProjectItem)itm.Object).Project;
		foreach ( VCConfiguration pc in (IVCCollection)prj.Configurations ) {
		    pc.ConfigurationType = ConfigurationTypes.typeDynamicLibrary;
		    VCLinkerTool lt = (VCLinkerTool)((IVCCollection)pc.Tools).Item("VCLinkerTool");

		    int index = lt.OutputFile.LastIndexOf( '.' );
		    string filename = lt.OutputFile.Substring( 0, index );
		    string qtlib = "qt";
		    if ( Resource.qtThreaded )
			qtlib += "-mt";
		    qtlib += Resource.qtVersion + ".lib";
		    lt.OutputFile = filename + ".dll";
		    lt.AdditionalLibraryDirectories += "$(QTDIR)\\lib";
		    lt.AdditionalDependencies += qtlib;
		    lt.BaseAddress = "0x39D00000";
		    lt.ImportLibrary = filename + ".lib";
		    lt.LinkIncremental = linkIncrementalType.linkIncrementalYes;
		    lt.ProgramDatabaseFile = filename + ".pdb";
		}
	    }
	    catch( System.Exception e ) {
		Say( e, "Couldn't makeProjectDll()" );
	    }
	}

	// Project generation functions ------------------------------------
	public static void newQtProject() 
	{
	    Say( "-- newQtProject " );
	    NewQtProject prj = new NewQtProject();
	    prj.ShowDialog();
	    if ( prj.DialogResult != DialogResult.OK )
		return;

	    FileInfo inf = new FileInfo( prj.txtProjectName.Text );
	    FileInfo resinf = new FileInfo( Connect.addinInstance.SatelliteDllPath );
	    string tempPath = resinf.DirectoryName + "\\baseTemplates";
	    string filename = inf.Name.EndsWith(".pro") ? inf.Name.Substring( 0, inf.Name.Length-4 ) : inf.Name;
	    
	    // Generate template project
	    if ( prj.optApplication.Checked ) {
		int opt = prj.optSDI.Checked ? 1 : prj.optMDI.Checked ? 2 : 3;
		Say( "Generating application " + (opt==1?"(SDI)":opt==2?"(MDI)":"(Dialog)") );
		newQtAppProject( inf.DirectoryName,
				 filename,
				 tempPath,
				 opt );
	    } else {
		Say( "Generating library " + (prj.optDynamic.Checked?"(dynamic - DLL)":"(static - LIB)") );
		newQtLibProject( inf.DirectoryName, 
				 filename, 
				 tempPath,
				 prj.optDynamic.Checked );
	    }

	    try {
		Say( "Generate .vcproj file from .pro" );
		if ( generateVCProjectFile( filename, inf.DirectoryName ) ) {
		    if ( prj.chkAddToSolution.Checked ) {
			// Add it to the current Solution
			try {
			    Connect.applicationObject.Solution.AddFromFile( 
				inf.DirectoryName + "\\" + filename + ".vcproj", false );
			    Say( "Added new project to current solution" );
			}
			catch {
			    MessageBox.Show( "*** Couldn't add project to Solution!\n\r" +
				"Does a project with the same name already exist in the Solution?" );
			    return;
			}
		    } else {
			// Close the current Solution, and add the new project
			Connect.applicationObject.Solution.Close( false ); /// ### Do we get a question here?
			Connect.applicationObject.Solution.AddFromFile( 
			    inf.DirectoryName + "\\" + filename + ".vcproj", false );
			Say( "Added new project to new solution" );
		    }
		    // If generating dynamic DLL, the files copied were
		    // for a static lib, so make project DLL
		    if ( prj.optDynamic.Checked )
			makeProjectDll();
		}
		// Start designer with the given .pro file
		Say( "Loading project in Designer" );
		Connect.extLoader.loadDesigner( inf.DirectoryName + "\\" + filename + ".pro", false );
	    }
	    catch ( System.Exception e ) {
		MessageBox.Show( "*** Couldn't start QMake!   " +
				 "Please verify that qmake.exe is in your path.\n\r\n\r" +
				 "Path = " + Environment.GetEnvironmentVariable("PATH"),
				 "Not starting QMake" );
		Say( e, "Couldn't run qmake [Path not correct?]" );
	    }
	    Say( "-- newQtProject - done " );
	}

	public static bool generateVCProjectFile( string filename, string workingDirectory ) 
	{
	    try {
		// Make qmake generate a vcproj for project
		System.Diagnostics.Process tmp = new System.Diagnostics.Process();
		tmp.StartInfo.FileName = "qmake";
		tmp.StartInfo.Arguments = "-tp vc " + filename + ".pro";
		tmp.StartInfo.WorkingDirectory = workingDirectory;
		tmp.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;
		tmp.Start();
		if ( tmp.WaitForExit(10000) )
		    return true;
		tmp.Kill();
		MessageBox.Show( "*** QMake never ended (10sec limit)\n\r" +
				 "Please verify that the correct qmake.exe is in your path.\n\r\n\r" +
				 "Path = " + Environment.GetEnvironmentVariable("PATH"),
				 "QMake didn't finish" );
	    }
	    catch ( System.Exception e ) {
		MessageBox.Show( "*** Couldn't start QMake!   " +
				 "Please verify that qmake.exe is in your path.\n\r\n\r" +
				 "Path = " + Environment.GetEnvironmentVariable("PATH"),
			 	 "Not starting QMake" );
		Say( e, "Couldn't run qmake [Path not correct?]" );
	    }
	    return false;
	}

	private static void newQtAppProject( string path, string filename, string res, int opt ) 
	{
	    try {
		if ( opt == 1 ) {
		    // Single Document Interface ----------
		    // copy identical files
		    File.Copy( res + "\\appSDI\\appSDI.pro", path + "\\" + filename + ".pro", true );
		    File.Copy( res + "\\appSDI\\main.cpp", path + "\\main.cpp", true );
		    File.Copy( res + "\\appSDI\\mainwindow.ui", path + "\\mainwindow.ui", true );
		    File.Copy( res + "\\appSDI\\sdiwindow.h", path + "\\sdiwindow.h", true );
		    // manipulate sdiwindow.cpp
		    StreamReader inF  = new StreamReader( res + "\\appSDI\\sdiwindow.cpp" );
		    StreamWriter outF = new StreamWriter( path + "\\sdiwindow.cpp", false );
		    string contents = inF.ReadToEnd();
		    contents = contents.Replace( "QMSNETPROJECTNAME", filename );
		    outF.Write( contents );
		    outF.Close();
		} else if ( opt == 2 ) {
		    // Multiple Documents Interface -------
		    // copy identical files
		    File.Copy( res + "\\appMDI\\appMDI.pro", path + "\\" + filename + ".pro", true );
		    File.Copy( res + "\\appMDI\\main.cpp", path + "\\main.cpp", true );
		    File.Copy( res + "\\appSDI\\mainwindow.ui", path + "\\mainwindow.ui", true );
		    File.Copy( res + "\\appMDI\\mdiwindow.h", path + "\\mdiwindow.h", true );
		    // manipulate mdiwindow.cpp
		    StreamReader inF  = new StreamReader( res + "\\appMDI\\mdiwindow.cpp" );
		    StreamWriter outF = new StreamWriter( path + "\\mdiwindow.cpp", false );
		    string contents = inF.ReadToEnd();
		    contents = contents.Replace( "QMSNETPROJECTNAME", filename );
		    outF.Write( contents );
		    outF.Close();
		} else {
		    // Dialog Interface -------------------
		    // copy identical files
		    File.Copy( res + "\\appDialog\\appDialog.pro", path + "\\" + filename + ".pro", true );
		    File.Copy( res + "\\appDialog\\main.cpp", path + "\\main.cpp", true );
		    File.Copy( res + "\\appDialog\\maindialog.ui", path + "\\maindialog.ui", true );
		}
	    }
	    catch( System.Exception e ) {
		Say( e, "Couldn't newQtAppProject() [Are templates available/accessible?]" );
	    }
	}

	private static void newQtLibProject( string path, string filename, string res, bool dyn ) 
	{
	    try{
		// copy identical files
		File.Copy( res + "\\libStatic\\libStatic.pro", path + "\\" + filename + ".pro", true );
		File.Copy( res + "\\libStatic\\libmain.h", path + "\\libmain.h", true );
		File.Copy( res + "\\libStatic\\libmain.cpp", path + "\\libmain.cpp", true );
	    }
	    catch( System.Exception e ) {
		Say( e, "Couldn't newQtAppProject() [Are templates available/accessible?]" );
	    }
	}

	// Output functions ------------------------------------------------
	public static void Say ( String str ) {
	    aPane().OutputString( str + "\r\n" );
	}

	public static void Say ( System.Exception e, String caption ) {
	    string str = "** QMsNet Exception: " + caption + "\r\n" + e.Message + "\r\n" + e.StackTrace.ToString() + "\r\n";
	    Debug.Write( str );
	    aPane().OutputString( str );
	}

	private static OutputWindowPane wndp = null;
	private static OutputWindowPane aPane () {
	    if ( wndp == null ) {
		OutputWindow wnd = (OutputWindow)Connect.applicationObject.Windows.Item( EnvDTE.Constants.vsWindowKindOutput ).Object;
		wndp = wnd.OutputWindowPanes.Add( "QMsNet - Trolltech .NET integration" );
	    }
	    return wndp;
	}
    }
}