using System;
using System.IO;
using System.Collections;
using System.Diagnostics;
using System.Windows.Forms;

namespace QMsNet
{
    public class Template
    {
	public string path;
	// Template.inf fields
	public string format = "1.0";
	public string name = "<give template a name>";
	public string description = "";
	public string version = "1.0";
	public string language = "C++";
	public string templateType = "app";
	public string configuration = "qt warn_on";
	public string ignoreConfiguration;
	public ArrayList headers = new ArrayList();
	public ArrayList sources = new ArrayList();
	public ArrayList forms = new ArrayList();
	public ArrayList translations = new ArrayList();
	public ArrayList images = new ArrayList();
	public ArrayList other = new ArrayList();
	public ArrayList extra = new ArrayList();
	public ArrayList unknown = new ArrayList();

	private enum SectionType {
	    Unknown,
	    Format,
	    Name,
	    Description,
	    Version,
	    Language,
	    TemplateType,
	    Configuration,
	    IgnoreConfiguration,
	    Headers,
	    Sources,
	    Forms,
	    Translations,
	    Images,
	    Other,
	    Extra
	}

	public Template()
	{
	}
	
	public bool loadTemplate( string loadPath )
	{
	    StreamReader file;
	    string contents = "";

	    if ( !loadPath.EndsWith( ".inf" ) ) {
		path = loadPath;
		loadPath += "\\Template.inf";
	    } else {
		int index = loadPath.LastIndexOf( "\\" );
                path = loadPath.Substring( 0, index );
	    }

	    try {
		file = new StreamReader( loadPath, true );
	    }
	    catch {
		QMNCommands.Say( "** ERROR: Couldn't load the template '" + loadPath + "'" );
		return false;
	    }

	    SectionType currentSection = SectionType.Unknown;
	    while( (contents = file.ReadLine()) != null ) {
		// Remove line comments
		int commentIndex = contents.IndexOf( "#" );
		if ( commentIndex != -1 )
		    contents = contents.Remove( commentIndex, contents.Length - commentIndex );
		
		// Remove whitespaces
		contents = contents.Trim();

		// Comment only & empty line shortcut
		if ( contents.Length == 0 )
		    continue;
		
		// New section block
		if ( contents.StartsWith( "[" ) ) {
		    switch ( contents.ToLower() ) {
			case "[format]":
			    currentSection = SectionType.Format;
			    break;
			case "[name]":
			    currentSection = SectionType.Name;
			    break;
			case "[description]":
			    currentSection = SectionType.Description;
			    break;
			case "[version]":
			    currentSection = SectionType.Version;
			    break;
			case "[language]":
			    currentSection = SectionType.Language;
			    break;
			case "[templatetype]":
			    currentSection = SectionType.TemplateType;
			    break;
			case "[configuration]":
			    currentSection = SectionType.Configuration;
			    break;
			case "[ignoreconfiguration]":
			    currentSection = SectionType.IgnoreConfiguration;
			    break;
			case "[headers]":
			    currentSection = SectionType.Headers;
			    break;
			case "[sources]":
			    currentSection = SectionType.Sources;
			    break;
			case "[forms]":
			    currentSection = SectionType.Forms;
			    break;
			case "[translations]":
			    currentSection = SectionType.Translations;
			    break;
			case "[images]":
			    currentSection = SectionType.Images;
			    break;
			case "[other]":
			    currentSection = SectionType.Other;
			    break;
			case "[extra]":
			    currentSection = SectionType.Extra;
			    break;
			default:
			    currentSection = SectionType.Unknown;
			    QMNCommands.Say( "** WARN: Unknown section block in template " + path );
			    break;
		    } // switch
		    continue;
		} // if

		// Add line to template variable
		switch ( currentSection ) {
		    case SectionType.Format:
			format = contents;
			break;
		    case SectionType.Name:
			name = contents;
			break;
		    case SectionType.Description:
			description += contents;
			break;
		    case SectionType.Version:
			version = contents;
			break;
		    case SectionType.Language:
			language = contents;
			break;
		    case SectionType.TemplateType:
			templateType = contents;
			break;
		    case SectionType.Configuration:
			configuration += contents;
			break;
		    case SectionType.IgnoreConfiguration:
			ignoreConfiguration += contents;
			break;
		    case SectionType.Headers:
			headers.Add( contents );
			break;
		    case SectionType.Sources:
			sources.Add( contents );
			break;
		    case SectionType.Forms:
			forms.Add( contents );
			break;
		    case SectionType.Translations:
			translations.Add( contents );
			break;
		    case SectionType.Images:
			images.Add( contents );
			break;
		    case SectionType.Other:
			other.Add( contents );
			break;
		    case SectionType.Extra:
			extra.Add( contents );
			break;
		    default:
			unknown.Add( contents );
			QMNCommands.Say( "** WARN: Unknown section block variable " + contents );
			break;
		} // switch
	    } // while
	    return true;
	}

	public bool createProject( string projectName, string destinationProject )
	{
	    try {
		FileInfo inf = new FileInfo( destinationProject );
		DirectoryInfo dirInf = Directory.CreateDirectory( inf.DirectoryName );

		copyTemplateFiles( inf.DirectoryName );
		generateQtProFile( inf.FullName, projectName );
		return generateVCProjectFile( inf.Name, inf.DirectoryName, this );
	    }
	    catch ( Exception e ) {
		QMNCommands.Say( "Couldn't create project from template\r\n" + e.ToString() );
		return false;
	    }
	}

	private bool copyTemplateFiles( string destPath )
	{
	    try {
		foreach( string header in headers )
		    File.Copy( path + "\\" + header, destPath + "\\" + header, true );

		foreach( string source in sources )
		    File.Copy( path + "\\" + source, destPath + "\\" + source, true );

		foreach( string form in forms )
		    File.Copy( path + "\\" + form, destPath + "\\" + form, true );

		foreach( string translation in translations )
		    File.Copy( path + "\\" + translation, destPath + "\\" + translation, true );

		foreach( string image in images )
		    File.Copy( path + "\\" + image, destPath + "\\" + image, true );

		foreach( string otr in other )
		    File.Copy( path + "\\" + otr, destPath + "\\" + otr, true );

		foreach( string xtra in extra )
		    File.Copy( path + "\\" + xtra, destPath + "\\" + xtra, true );
	    }
	    catch ( Exception e ) {
		QMNCommands.Say( "** ERROR: Couldn't copy all template files\r\n" + e.ToString() );
		return false;
	    }
	    return true;
	}

	private bool generateQtProFile( string fullPath, string projectName ) 
	{
	    // This should really be made more generic, but is not
	    // needed when we only have two templates.
	    StreamWriter of = new StreamWriter( fullPath );
	    of.WriteLine( "# ----------------------------------------------------------" );
	    of.WriteLine( "# Project generated by QMsNet " + Res.Version );
	    of.WriteLine( "#    Template used:    " + name );
	    of.WriteLine( "#    Template version: " + version );
	    of.WriteLine( "# ----------------------------------------------------------\r\n" );
	    
	    // Project type
	    of.WriteLine( "TEMPLATE       = " + templateType );
	    of.WriteLine( "TARGET         = " + projectName );
	    of.WriteLine( "LANGUAGE       = " + language );
	    of.WriteLine( "CONFIG        += " + configuration );
	    if ( ignoreConfiguration != null )
		of.WriteLine( "CONFIG        -= " + ignoreConfiguration );
	    of.WriteLine( "\r\nunix {" );
	    of.WriteLine( "   UI_DIR      = .ui" );
	    of.WriteLine( "   MOC_DIR     = .moc" );
	    of.WriteLine( "   OBJECTS_DIR = .obj" );
	    of.WriteLine( "}" );

	    foreach( string uk in unknown )
		of.WriteLine( "# Unknown template variable: " + uk );

	    if ( headers.Count > 0 ) {
		of.Write( "\r\n\r\nHEADERS        = " );
		foreach( string header in headers )
		    of.Write( " \\\r\n\t" + header );
	    }

	    if ( sources.Count > 0 ) {
		of.Write( "\r\n\r\nSOURCES        = " );
		foreach( string source in sources )
		    of.Write( " \\\r\n\t" + source );
	    }

	    if ( forms.Count > 0 ) {
		of.Write( "\r\n\r\nFORMS          = " );
		foreach( string form in forms )
		    of.Write( " \\\r\n\t" + form );
	    }
	    
	    if ( translations.Count > 0 ) {
		of.Write( "\r\n\r\nTRANSLATIONS   = " );
		foreach( string translation in translations )
		    of.Write( " \\\r\n\t" + translation );
	    }
	    
	    if ( images.Count > 0 ) {
		of.Write( "\r\n\r\nIMAGES         = " );
		foreach( string image in images )
		    of.Write( " \\\r\n\t" + image );
	    }
	    
	    of.Flush();
	    of.Close();
	    return true;
	}

	public static bool generateVCProjectFile( string filename, string workingDirectory, Template inst ) {
	    try {
		string vcproj = filename.Substring( 0, filename.LastIndexOf(".") ) + ".vcproj";
		// Make qmake generate a vcproj for project
		System.Diagnostics.Process tmp = new System.Diagnostics.Process();
		tmp.StartInfo.FileName = Res.QtDir + "\\bin\\qmake";
		tmp.StartInfo.Arguments = "-tp vc " + filename + " -o " + vcproj;
		tmp.StartInfo.WorkingDirectory = workingDirectory;
		tmp.StartInfo.WindowStyle = ProcessWindowStyle.Hidden;
		tmp.Start();
		if ( tmp.WaitForExit(10000) ) {
		    //### Check if ints != null
		    //### and add others to Others folder in project
		    return true;
		}
		tmp.Kill();
		MessageBox.Show( "*** QMake never ended (10sec limit)\r\n" +
		    "Please verify that the correct qmake.exe is in your path.\r\n\r\n" +
		    "Path = " + Environment.GetEnvironmentVariable("PATH"),
		    "QMake didn't finish" );
	    }
	    catch ( System.Exception e ) {
		MessageBox.Show( "*** Couldn't start QMake!   " +
		    "Please verify that qmake.exe is in your path.\r\n\r\n" +
		    "Path = " + Environment.GetEnvironmentVariable("PATH"),
		    "Not starting QMake" );
		QMNCommands.Say( e, "Couldn't run qmake [Path not correct?]" );
	    }
	    return false;
	}
    
    }
}
