using System;

namespace QMsNet
{
    using EnvDTE;
    using System;
    using System.IO;
    using System.Windows.Forms;
    using System.Diagnostics;
    using System.Runtime.InteropServices;
    using Microsoft.VisualStudio.VCProjectEngine;
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
	    FileDialog toOpen = new OpenFileDialog();
	    toOpen.Filter = "Qt Project files (*.pro)|*.pro|All files (*.*)|*.*";
	    toOpen.FilterIndex = 1;
	    toOpen.Title = "Select a Qt Project to add to the Solution";

	    if ( DialogResult.OK != toOpen.ShowDialog() )
		return;

	    FileInfo mainInfo = new FileInfo( toOpen.FileName );
	    string name = mainInfo.Name;
	    int lio = name.LastIndexOf( "." );
	    if ( lio != -1 )
		name = name.Remove( lio, name.Length - lio );

	    FileInfo VCInfo = new FileInfo( mainInfo.DirectoryName + "\\" + name + ".vcproj" );

	    if ( !VCInfo.Exists || 
		 (VCInfo.Exists && 
		  DialogResult.Yes == MessageBox.Show( "A .vcproj files already exists for this project!\n\r" + 
						       "Select 'Yes' to regenerate the project file, and 'No' to use the existing one", 
						       "VCProj file exists!", MessageBoxButtons.YesNo )) )
		if ( !QMNCommands.generateVCProjectFile( mainInfo.FullName, mainInfo.DirectoryName ) ) {
		    MessageBox.Show( "*** Couldn't generate project" );
		    return;
		}

	    try {
		Connect.applicationObject.Solution.AddFromFile( VCInfo.FullName, false );
	    }
	    catch {
		MessageBox.Show( "*** Couldn't add project to Solution!\n\r" +
				 "Does a project with the same name already exist in the Solution?" );
	    }
	}

	public void saveProject()
	{
	    try {
		Document doc = Connect.applicationObject.ActiveDocument;
		if ( doc == null ) {
		    Connect.applicationObject.StatusBar.Text = "Cannot save Qt project... No file open";
		    return;
		}

		ProjectItem itm = doc.ProjectItem;
		VCProjectItem vitm = (VCProjectItem)itm.Object;
		VCProject vp = (VCProject)vitm.Project;

		FileDialog toSave = new SaveFileDialog();
		toSave.FileName = vp.Name + ".pro";
		toSave.Filter = "Qt Project files (*.pro)|*.pro|All files (*.*)|*.*";
		toSave.FilterIndex = 1;
		toSave.Title = "Select a location and filename to save the current project";

		if ( DialogResult.OK != toSave.ShowDialog() )
		    return;

		VCConfiguration       cfg = (VCConfiguration)((IVCCollection)vp.Configurations).Item(1);
		VCLinkerTool       linker = (VCLinkerTool)((IVCCollection)cfg.Tools).Item("VCLinkerTool");
		VCLibrarianTool librarian = (VCLibrarianTool)((IVCCollection)cfg.Tools).Item("VCLibrarianTool");

		string prjType = "app";
		switch ( ((VCConfiguration)((IVCCollection)vp.Configurations).Item(1)).ConfigurationType ) {
		    case ConfigurationTypes.typeApplication:
			// already assigned
			break;
		    case ConfigurationTypes.typeDynamicLibrary:
		    case ConfigurationTypes.typeStaticLibrary:
			prjType = "lib";
			break;
		}
		bool   prjDebug  = (linker==null) ? true : linker.GenerateDebugInformation;
		string prjConfig = "qt warn_on" + (prjDebug ? " debug" : "");

		string prjHeaders = "";
		string prjSources = "";
		string prjForms = "";
		string prjImages = "";
		string prjTranslations = "";

		foreach( VCFile vcf in (IVCCollection)vp.files ) {
		    if ( vcf.Name.StartsWith( "moc_" ) ||
			 vcf.Name.EndsWith( ".moc" ) )
			continue; // qmake figure these out
		    if ( vcf.Name.EndsWith( ".cpp" ) ||
			vcf.Name.EndsWith( ".ui.h" ) ) {
			prjSources += " \\\r\n\t" + vcf.RelativePath;
			continue;
		    }
		    if ( vcf.Name.EndsWith( ".h" ) ) {
			prjHeaders += " \\\r\n\t" + vcf.RelativePath;
			continue;
		    }
		    if ( vcf.Name.EndsWith( ".ui" ) ) {
			prjForms += " \\\r\n\t" + vcf.RelativePath;
			continue;
		    }
		    if ( vcf.Name.EndsWith( ".png" ) ||
			 vcf.Name.EndsWith( ".gif" ) ||
			 vcf.Name.EndsWith( ".bmp" ) ) {
			prjImages += " \\\r\n\t" + vcf.RelativePath;
			continue;
		    }
		    if ( vcf.Name.EndsWith( ".ts" ) )
			prjTranslations += " \\\r\n\t" + vcf.RelativePath;
		}

		StreamWriter outF = new StreamWriter( toSave.FileName, false );
		outF.WriteLine( "# ----------------------------------------------------------" );
		outF.WriteLine( "# " + prjType + " project generated by QMsNet (save function)" );
		outF.WriteLine( "# ----------------------------------------------------------" );
		outF.WriteLine( "TEMPLATE  = " + prjType );
		outF.WriteLine( "LANGUAGE  = C++\r\n" );
		outF.WriteLine( "CONFIG	 += " + prjConfig );
		outF.WriteLine( "\r\nunix {" );
		outF.WriteLine( "   UI_DIR      = .ui" );
		outF.WriteLine( "   MOC_DIR     = .moc" );
		outF.WriteLine( "   OBJECTS_DIR = .obj" );
		outF.WriteLine( "}\r\n" );
		if ( prjHeaders.Length > 0 )
		    outF.WriteLine( "HEADERS   = " + prjHeaders );
		if ( prjSources.Length > 0 )
		    outF.WriteLine( "SOURCES   = " + prjSources );
		if ( prjForms.Length > 0 )
		    outF.WriteLine( "FORMS     = " + prjForms );
		if ( prjTranslations.Length > 0 )
		    outF.WriteLine( "TRANSLATIONS = " + prjTranslations );
		if ( prjImages.Length > 0 )
		    outF.WriteLine( "IMAGES    = " + prjImages );
		outF.Close();
	    }
	    catch ( System.Exception e ) {
		MessageBox.Show( "*** SaveProject(): Something is wrong!!\n\r" + e.StackTrace );
	    }
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
		    if ( file.Length > 0 )
			file = "-client \"" + file +"\"";
		}
		catch {}

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
	    catch {
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

	    // Find .qmake.cache
	    Resource.QtDir = Environment.GetEnvironmentVariable( "QTDIR" );
	    FileInfo qmakeCache = new FileInfo( Resource.QtDir + "\\.qmake.cache" );
	    if ( !qmakeCache.Exists ) {
		MessageBox.Show( "Couldn't find .qmake.cache\n\r" + 
				 "Make sure the environment variable QTDIR is set to you Qt directory", 
				 ".qmake.cache not found!" );
		return;
	    }

	    // Find the Qt version number
	    StreamReader inF  = new StreamReader( qmakeCache.FullName );
	    bool readMore = true;
	    string contents;
	    while ( readMore ) {
		contents = inF.ReadLine();
		readMore = !contents.StartsWith("QMAKE_QT_VERSION_OVERRIDE=");
		if ( !readMore ) {
		    int idx = contents.LastIndexOf( "=" ) + 1;
		    Resource.qtVersion = contents.Substring( idx, contents.Length - idx );
		}
	    }

	    // Figure out if Qt is configured as shared && threaded
	    contents = inF.ReadToEnd();
	    if ( contents.IndexOf( "shared" ) > 0 ) Resource.qtShared = true;
	    if ( contents.IndexOf( "thread" ) > 0 ) Resource.qtThreaded = true;
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
	private void OnDocumentOpening( string path, bool readOnly ) 
	{
	    if( path.EndsWith(".ui") )
		loadDesigner( path, false );
	}

	private void OnDocumentOpened( Document doc ) 
	{
	    if( doc.Name.EndsWith(".ui") ) {
		doc.Close( vsSaveChanges.vsSaveChangesNo );
		tmUIKiller.Start();
	    }
	}

	private void OnTimer( Object source, System.Timers.ElapsedEventArgs e ) 
	{
	    tmUIKiller.Stop();
	    foreach( Window wnd in Connect.applicationObject.Windows )
		if ( wnd.Caption == "" )
		    wnd.Close( vsSaveChanges.vsSaveChangesNo );
	}
    }
}
