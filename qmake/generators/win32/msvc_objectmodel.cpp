#include "msvc_objectmodel.h"

const char* _AdditionalIncludeDirectories 	= "AdditionalIncludeDirectories=\"";
const char* _AdditionalOptions 			= "AdditionalOptions=\"";
const char* _AdditionalUsingDirectories 	= "AdditionalUsingDirectories=\"";
const char* _AssemblerListingLocation 		= "AssemblerListingLocation=\"";
const char* _AssemblerOutput 			= "AssemblerOutput=\"";
const char* _BasicRuntimeChecks 		= "BasicRuntimeChecks=\"";
const char* _BrowseInformation 			= "BrowseInformation=\"";
const char* _BrowseInformationFile 		= "BrowseInformationFile=\"";
const char* _BufferSecurityCheck 		= "BufferSecurityCheck=\"";
const char* _CallingConvention 			= "CallingConvention=\"";
const char* _CompileAs 				= "CompileAs=\"";
const char* _CompileAsManaged 			= "CompileAsManaged=\"";
const char* _CompileOnly 			= "CompileOnly=\"";
const char* _Culture				= "Culture=\"";
const char* _DebugInformationFormat 		= "DebugInformationFormat=\"";
const char* _DefaultCharIsUnsigned 		= "DefaultCharIsUnsigned=\"";
const char* _DelayLoadDLLs	 		= "DelayLoadDLLs=\"";
const char* _Detect64BitPortabilityProblems 	= "Detect64BitPortabilityProblems=\"";
const char* _DisableLanguageExtensions 		= "DisableLanguageExtensions=\"";
const char* _DisableSpecificWarnings 		= "DisableSpecificWarnings=\"";
const char* _EnableFiberSafeOptimizations 	= "EnableFiberSafeOptimizations=\"";
const char* _EnableFunctionLevelLinking 	= "EnableFunctionLevelLinking=\"";
const char* _EnableIntrinsicFunctions 		= "EnableIntrinsicFunctions=\"";
const char* _ExceptionHandling 			= "ExceptionHandling=\"";
const char* _ExpandAttributedSource 		= "ExpandAttributedSource=\"";
const char* _FavorSizeOrSpeed 			= "FavorSizeOrSpeed=\"";
const char* _ForceConformanceInForLoopScope 	= "ForceConformanceInForLoopScope=\"";
const char* _ForcedIncludeFiles 		= "ForcedIncludeFiles=\"";
const char* _ForcedUsingFiles 			= "ForcedUsingFiles=\"";
const char* _GeneratePreprocessedFile 		= "GeneratePreprocessedFile=\"";
const char* _GlobalOptimizations 		= "GlobalOptimizations=\"";
const char* _IgnoreStandardIncludePath 		= "IgnoreStandardIncludePath=\"";
const char* _ImproveFloatingPointConsistency 	= "ImproveFloatingPointConsistency=\"";
const char* _InlineFunctionExpansion 		= "InlineFunctionExpansion=\"";
const char* _KeepComments 			= "KeepComments=\"";
const char* _MinimalRebuild 			= "MinimalRebuild=\"";
const char* _ObjectFile 			= "ObjectFile=\"";
const char* _OmitFramePointers 			= "Optimization=\"";
const char* _Optimization 			= "OptimizeForProcessor =\"";
const char* _OptimizeForProcessor 		= "OptimizeForProcessor=\"";
const char* _OptimizeForWindowsApplication 	= "OptimizeForWindowsApplication=\"";
const char* _OutputFile 			= "OutputFile=\"";
const char* _PrecompiledHeaderFile 		= "PrecompiledHeaderFile=\"";
const char* _PrecompiledHeaderThrough 		= "PrecompiledHeaderThrough=\"";
const char* _PreprocessorDefinitions 		= "PreprocessorDefinitions=\"";
const char* _ProgramDataBaseFileName 		= "ProgramDataBaseFileName=\"";
const char* _RuntimeLibrary 			= "RuntimeLibrary=\"";
const char* _RuntimeTypeInfo 			= "RuntimeTypeInfo=\"";
const char* _ShowIncludes 			= "ShowIncludes=\"";
const char* _SmallerTypeCheck 			= "SmallerTypeCheck=\"";
const char* _StringPooling 			= "StringPooling=\"";
const char* _StructMemberAlignment 		= "StructMemberAlignment=\"";
const char* _SuppressStartupBanner 		= "SuppressStartupBanner=\"";
const char* _ToolName				= "Name=\"";
const char* _TreatWChar_tAsBuiltInType		= "TreatWChar_tAsBuiltInType=\"";
const char* _TurnOffAssemblyGeneration 		= "TurnOffAssemblyGeneration=\"";
const char* _UndefineAllPreprocessorDefinitions = "UndefineAllPreprocessorDefinitions=\"";
const char* _UndefinePreprocessorDefinitions 	= "UndefinePreprocessorDefinitions=\"";
const char* _UsePrecompiledHeader 		= "UsePrecompiledHeader=\"";
const char* _WarnAsError 			= "WarnAsError=\"";
const char* _WarnLevel 				= "WarnLevel=\"";
const char* _WarningLevel 			= "WarningLevel=\"";
const char* _WholeProgramOptimization		= "WholeProgramOptimization=\"";

// VCCLCompilerTool -------------------------------------------------

VCCLCompilerTool::VCCLCompilerTool()
    :	BufferSecurityCheck( unset ), 
	CompileOnly( unset ), 
	DefaultCharIsUnsigned( unset ),
	Detect64BitPortabilityProblems( unset ), 
	DisableLanguageExtensions( unset ),
	EnableFiberSafeOptimizations( unset ), 
	EnableFunctionLevelLinking( unset ),
	EnableIntrinsicFunctions( unset ), 
	ExceptionHandling( unset ), 
	ExpandAttributedSource( unset ), 
	ForceConformanceInForLoopScope( unset ),
	GlobalOptimizations( unset ), 
	IgnoreStandardIncludePath( unset ),
	ImproveFloatingPointConsistency( unset ), 
	KeepComments( unset ), 
	MinimalRebuild( unset ), 
	OmitFramePointers( unset ), 
	OptimizeForWindowsApplication( unset ), 
	RuntimeTypeInfo( unset ), 
	ShowIncludes( unset ),
	SmallerTypeCheck( unset ), 
	StringPooling( unset ), 
	SuppressStartupBanner( unset ),
	TreatWChar_tAsBuiltInType( unset ), 
	TurnOffAssemblyGeneration( unset ),
	UndefineAllPreprocessorDefinitions( unset ), 
	WarnAsError( unset ),
	WholeProgramOptimization( unset )
{
}

// VCLinkerTool -----------------------------------------------------

VCLinkerTool::VCLinkerTool()
    :	GenerateDebugInformation( unset ),
	GenerateMapFile( unset ),
	IgnoreAllDefaultLibraries( unset ),
	IgnoreEmbeddedIDL( unset ),
	IgnoreImportLibrary( unset ),
	LinkDLL( unset ),
	LinkTimeCodeGeneration( unset ),
	MapExports( unset ),
	MapLines( unset ),
	RegisterOutput( unset ),
	ResourceOnlyDLL( unset ),
	SetChecksum( unset ),
	StackCommitSize( 0 ),
	StackReserveSize( 0 ),
	SupportUnloadOfDelayLoadedDLL( unset ),
	SuppressStartupBanner( unset ),
	SwapRunFromCD( unset ),
	SwapRunFromNet( unset ),
	TurnOffAssemblyGeneration( unset ),
	TypeLibraryResourceID( 0 )
{
}

// VCCustomBuildTool ------------------------------------------------
VCCustomBuildTool::VCCustomBuildTool()
{
}

// VCResourceCompilerTool -------------------------------------------
VCResourceCompilerTool::VCResourceCompilerTool()
    :   Culture( rcUseDefault ),
	IgnoreStandardIncludePath( unset ),
	ShowProgress( linkProgressNotSet )

{
    PreprocessorDefinitions = "NDEBUG";
}

// VCPostBuildEventTool ---------------------------------------------
VCPostBuildEventTool::VCPostBuildEventTool()
{
    ToolName = "VCPostBuildEventTool";
}

// VCPreBuildEventTool ----------------------------------------------
VCPreBuildEventTool::VCPreBuildEventTool()
{
    ToolName = "VCPreBuildEventTool";
}

// VCPreLinkEventTool -----------------------------------------------
VCPreLinkEventTool::VCPreLinkEventTool()
{
    ToolName = "VCPreLinkEventTool";
}
