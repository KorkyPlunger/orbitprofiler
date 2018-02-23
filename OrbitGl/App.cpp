//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "TextRenderer.h"
#include "GlCanvas.h"
#include "Capture.h"
#include "ImGuiOrbit.h"
#include "Params.h"
#include "Log.h"
#include "App.h"
#include "ProcessDataView.h"
#include "ModuleDataView.h"
#include "FunctionDataView.h"
#include "LiveFunctionDataView.h"
#include "CallStackDataView.h"
#include "TypeDataView.h"
#include "GlobalDataView.h"
#include "SessionsDataView.h"
#include "SamplingReport.h"
#include "ThreadView.h"
#include "ScopeTimer.h"
#include "OrbitSession.h"
#include "Serialization.h"
#include "CaptureWindow.h"
#include "LogDataView.h"
#include "DiaManager.h"
#include "MiniDump.h"
#include "CaptureSerializer.h"
#include "PluginManager.h"
#include "RuleEditor.h"

#include "Pdb.h"
#include "ModuleManager.h"
#include "TcpServer.h"
#include "ServerTimerManager.h"
#include "Injection.h"
#include "Utils.h"
#include "Tcp.h"
#include "PrintVar.h"
#include "Version.h"
#include "Debugger.h"
#include "Path.h"
#include "VariableTracing.h"

#include <thread>
#include <cmath>
#include <fstream>

#include <GL/freeglut.h>
//#include "EventTracer.h"

#ifdef _WIN32
#include "Disassembler.h"
#endif

using namespace std;

class OrbitApp* GOrbitApp;
ServerTimerManager* GTimerManager;
float GFontSize;

//-----------------------------------------------------------------------------
OrbitApp::OrbitApp() : m_FindFileCallback(nullptr)
                     , m_RuleEditor(nullptr)
                     , m_CallStackDataView(nullptr)
{
    m_Debugger = new Debugger();
}

//-----------------------------------------------------------------------------
OrbitApp::~OrbitApp()
{
#ifdef _WIN32
    oqpi_tk::stop_scheduler();
#endif
    delete m_Debugger;
    GOrbitApp = nullptr;
}

//-----------------------------------------------------------------------------
wstring OrbitApp::FindFile( const wstring & a_Caption, const wstring & a_Dir, const wstring & a_Filter )
{
    if( m_FindFileCallback )
    {
         return m_FindFileCallback( a_Caption, a_Dir, a_Filter );
    }
    
    return wstring();
}

//-----------------------------------------------------------------------------
void OrbitApp::SetCommandLineArguments(const vector< string > & a_Args)
{ 
    m_Arguments = a_Args;
    bool inject = false;

    for( string arg : a_Args )
    {
        if( Contains( arg, "host:" )  )
        { 
            vector< string > vec = Tokenize( arg, ":" );
            if( vec.size() > 1 )
            {
                string & host = vec[1];
                Capture::GCaptureHost = s2ws(host);
            }
        }
        else if( Contains( arg, "preset:" ) )
        {
            vector< string > vec = Tokenize( arg, ":" );
            if( vec.size() > 1 )
            {
                string & preset = vec[1];
                Capture::GPresetToLoad = s2ws( preset );
            }
        }
        else if( Contains( arg, "inject:" ) )
        {
            vector< string > vec = Tokenize( arg, ":" );
            if( vec.size() > 1 )
            {
                string & preset = vec[1];
                Capture::GProcessToInject = s2ws( preset );
            }
            inject = true;
        }
    }
}

// Get the horizontal and vertical screen sizes in pixel
//-----------------------------------------------------------------------------
void GetDesktopResolution(int& horizontal, int& vertical)
{
#ifdef _WIN32
    RECT desktop;
    // Get a handle to the desktop window
    const HWND hDesktop = GetDesktopWindow();
    // Get the size of screen to the variable desktop
    GetWindowRect(hDesktop, &desktop);
    // The top left corner will have coordinates (0,0)
    // and the bottom right corner will have coordinates
    // (horizontal, vertical)
    horizontal = desktop.right;
    vertical = desktop.bottom;
#endif
}

//-----------------------------------------------------------------------------
void GLoadPdbAsync( const vector<wstring> & a_Modules )
{
    GModuleManager.LoadPdbAsync( a_Modules, [](){ GOrbitApp->OnPdbLoaded(); } );
}

//-----------------------------------------------------------------------------
bool OrbitApp::Init()
{
    GOrbitApp = new OrbitApp();
    GCoreApp = GOrbitApp;
    GTimerManager = new ServerTimerManager();
    GTcpServer = new TcpServer();

    Path::Init();

    DiaManager::InitMsDiaDll();
    GModuleManager.Init();
    Capture::Init();
    Capture::GSamplingDoneCallback = &OrbitApp::AddSamplingReport;
    Capture::SetLoadPdbAsyncFunc( GLoadPdbAsync );

#ifdef _WIN32
    oqpi_tk::start_default_scheduler();
#endif

    GPluginManager.Initialize();

    int my_argc = 0;
    glutInit(&my_argc, NULL);
    glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
    GetDesktopResolution(GOrbitApp->m_ScreenRes[0], GOrbitApp->m_ScreenRes[1]);

    if( Capture::IsOtherInstanceRunning() )
    {
        ++Capture::GCapturePort;
    }

    GTcpServer->SetCallback( Msg_MiniDump, [=](const Message & a_Msg){ GOrbitApp->OnMiniDump(a_Msg); });
    GTcpServer->Start(Capture::GCapturePort);

    GParams.Load();
    GFontSize = GParams.m_FontSize;
    GOrbitApp->LoadFileMapping();
    GOrbitApp->LoadSymbolsFile();

    OrbitVersion::CheckForUpdate();

    return true;
}

//-----------------------------------------------------------------------------
void OrbitApp::PostInit()
{
    //GOrbitApp->CheckLicense();
    GOrbitApp->CallHome();
}

//-----------------------------------------------------------------------------
void OrbitApp::LoadFileMapping()
{
    m_FileMapping.clear();
    wstring fileName = Path::GetFileMappingFileName();
    if ( !Path::FileExists( fileName ) )
    {
        std::ofstream outfile( ws2s(fileName) );
        outfile << "//-------------------" << endl
                << "// Orbit File Mapping" << endl
                << "//-------------------" << endl
                << "// If the file path in the pdb is \"D:\\NoAccess\\File.cpp\"" << endl
                << "// and File.cpp is locally available in \"C:\\Available\\\""  << endl
                << "// then enter a file mapping on its own line like so:"        << endl
                << "// \"D:\\NoAccess\\File.cpp\" \"C:\\Available\\\"" << endl
                << endl
                << "\"D:\\NoAccess\" \"C:\\Avalaible\"" << endl;

        outfile.close();
    }

    std::wfstream infile( ws2s(fileName));
    if( !infile.fail())
    {
        wstring line;
        while( getline(infile, line) )
        {
            if( StartsWith( line, L"//") )
                continue;
            
            bool containsQuotes = Contains( line, L"\"" );
            
            vector< wstring > tokens = Tokenize( line );
            if( tokens.size() == 2 && !containsQuotes )
            {
                m_FileMapping[ ToLower( tokens[0]) ] = ToLower( tokens[1] );
            }
            else
            {
                vector< wstring > validTokens;
                for( wstring token : Tokenize( line, L"\"//") )
                {
                    if( !IsBlank( token ) )
                    {
                        validTokens.push_back( token );
                    }
                }

                if( validTokens.size() > 1 )
                {
                    m_FileMapping[ ToLower( validTokens[0]) ] = ToLower( validTokens[1] );
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------
void OrbitApp::LoadSymbolsFile()
{
    m_SymbolLocations.clear();

    wstring fileName = Path::GetSymbolsFileName();
    if( !Path::FileExists( fileName ) )
    {
        ofstream outfile( ws2s(fileName) );
        outfile << "//-------------------" << endl
            << "// Orbit Symbol Locations" << endl
            << "//-------------------" << endl
            << "// Orbit will scan the specified directories for pdb files." << endl
            << "// Enter one directory per line, like so:" << endl
            << "// \"D:\\MyApp\\Release\"" << endl
            << "// \"D:\\MySymbolServer\\" << endl
            << endl;

        outfile.close();
    }

    wfstream infile( ws2s(fileName) );
    if( !infile.fail() )
    {
        wstring line;
        while( getline( infile, line ) )
        {
            if( StartsWith( line, L"//" ) )
                continue;

            wstring dir = line;
            if( Path::DirExists( dir ) )
            {
                m_SymbolLocations.push_back( dir );
            }
        }
    }
}

//-----------------------------------------------------------------------------
void OrbitApp::ListSessions()
{
    vector< wstring > sessionFileNames = Path::ListFiles( Path::GetPresetPath(), L".opr" );
    vector< shared_ptr< Session > > sessions;
    for( wstring & fileName : sessionFileNames )
    {
        shared_ptr<Session> session = make_shared<Session>();

        ifstream file( ws2s(fileName).c_str(), ios::binary );
        if( !file.fail() )
        {
            cereal::BinaryInputArchive archive( file );
            archive( *session );
            file.close();
            session->m_FileName = fileName;
            sessions.push_back(session);
        }
    }

    m_SessionsDataView->SetSessions( sessions );
}

//-----------------------------------------------------------------------------
void OrbitApp::SetRemoteProcess( shared_ptr<Process> a_Process )
{
    m_ProcessesDataView->SetRemoteProcess( a_Process );
}

//-----------------------------------------------------------------------------
void OrbitApp::AddWatchedVariable( Variable * a_Variable )
{
    // Make sure type hierarchy has been generated
    if( Type* type = a_Variable->m_Pdb->GetTypePtrFromId( a_Variable->m_TypeIndex ) )
    {
        type->LoadDiaInfo();
    }

    for( WatchCallback & callback : m_AddToWatchCallbacks )
    {
        callback( a_Variable );
    }
}

//-----------------------------------------------------------------------------
void OrbitApp::UpdateVariable( Variable * a_Variable )
{
    for( WatchCallback & callback : m_UpdateWatchCallbacks )
    {
        callback( a_Variable );
    }
}

//-----------------------------------------------------------------------------
void OrbitApp::ClearWatchedVariables()
{
    if( Capture::GTargetProcess )
    {
        Capture::GTargetProcess->ClearWatchedVariables();
    }
}

//-----------------------------------------------------------------------------
void OrbitApp::RefreshWatch()
{
    Capture::GTargetProcess->RefreshWatchedVariables();
}

//-----------------------------------------------------------------------------
void OrbitApp::Disassemble( Function * a_Function, const char * a_MachineCode, int a_Size )
{
#ifdef _WIN32
    Disassembler disasm;
    disasm.LOGF( "asm: /* %s */\n", a_Function->PrettyNameStr().c_str() );
    const unsigned char* code = (const unsigned char*)a_MachineCode;
    disasm.Disassemble( code, a_Size, a_Function->GetVirtualAddress(), Capture::GTargetProcess->GetIs64Bit() );
    SendToUiAsync(disasm.GetResult());
#endif
}

//-----------------------------------------------------------------------------
const unordered_map<DWORD64, shared_ptr<class Rule> > * OrbitApp::GetRules()
{
    return &m_RuleEditor->GetRules();
}

//-----------------------------------------------------------------------------
void OrbitApp::CallHome()
{
    thread t( [&](){ CallHomeThread(); } );
    t.detach();
}

//-----------------------------------------------------------------------------
void OrbitApp::CallHomeThread()
{
    asio::ip::tcp::iostream stream;
    stream.expires_from_now( chrono::seconds( 60 ) );

    const bool isLocal = false;

    if( isLocal )
        stream.connect( "127.0.0.1", "58642" );
    else
        stream.connect( "www.telescopp.com", "http" );
    
    if( stream.fail() )
    {
        asio::error_code error = stream.error();
        //OutputDebugStringA( error.message().c_str() );
        return;
    }

    if( isLocal )
        stream << "POST http://localhost:58642/update HTTP/1.1\r\n";
    else
        stream << "POST http://www.telescopp.com/update HTTP/1.1\r\n";
    
    stream << "HAccept: text/html, application/xhtml+xml, image/jxr, */*\r\n";
    stream << "Referer: http://localhost:58642/\r\n";
    stream << "Accept-Language: en-US,en;q=0.8,fr-CA;q=0.5,fr;q=0.3\r\n";
    stream << "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/52.0.2743.116 Safari/537.36 Edge/15.14965\r\n";
    //stream << "Content-Type: application/x-www-form-urlencoded\r\n";
    stream << "Content-Type: multipart/form-data\r\n";
    stream << "Accept-Encoding: gzip, deflate\r\n";
    stream << "Host: localhost:60485\r\n";
    
    string content = ws2s(m_User) + "-" + OrbitVersion::GetVersion();
    stream << "Content-Length: " << content.length() << "\r\n\r\n";
    stream << XorString(content);

    stream.flush();
}

//-----------------------------------------------------------------------------
void OrbitApp::CheckLicense()
{
    m_License.clear();
    wifstream infile( ws2s( Path::GetLicenseName()) );
    
    if( !infile.fail() )
    {
        wstringstream buffer;
        buffer << infile.rdbuf();
        m_License = buffer.str();
        infile.close();
    }

    if( !OrbitVersion::CheckLicense( m_License ) )
    {
        do
        {
            SendToUiNow( L"license" );
        } while( !OrbitVersion::CheckLicense( m_License ) );

        wofstream outFile( ws2s(Path::GetLicenseName()) );
        if( !outFile.fail() )
        {
            outFile << m_License;
            outFile.close();
        }
    }

    m_User = Tokenize( m_License, L":" )[0];
    PRINT_VAR(m_User);
}

//-----------------------------------------------------------------------------
void OrbitApp::SetLicense( const wstring & a_License )
{
    m_License = a_License;
}

//-----------------------------------------------------------------------------
int OrbitApp::OnExit()
{
    GParams.Save();
    delete GOrbitApp;
	delete GTimerManager;
    GTcpServer->Stop();
    delete GTcpServer;
    Orbit_ImGui_Shutdown();
    return 0;
}

//-----------------------------------------------------------------------------
Timer GMainTimer;

bool DoZoom = false;

//-----------------------------------------------------------------------------
void OrbitApp::MainTick()
{
    TRACE_VAR( GMainTimer.QueryMillis() );

    GMainTimer.Reset();
    Capture::Update();
    GTcpServer->MainThreadTick();

    if( Capture::GPresetToLoad != L"" )
    {
        GOrbitApp->OnLoadSession( Capture::GPresetToLoad );
    }
    
    if( Capture::GProcessToInject != L"" )
    {
        cout << "Injecting into " << ws2s(Capture::GTargetProcess->GetFullName()) << endl;
        cout << "Orbit host: " << ws2s(Capture::GCaptureHost) << endl;
        GOrbitApp->SelectProcess(Capture::GProcessToInject);
        Capture::InjectRemote();
        exit(0);
    }

    GOrbitApp->m_Debugger->MainTick();
    GOrbitApp->CheckForUpdate();

    ++GOrbitApp->m_NumTicks;

    if( DoZoom )
    {
        GCurrentTimeGraph->UpdateThreadIds();
        GCurrentTimeGraph->m_Layout.CalculateOffsets();
        GOrbitApp->m_CaptureWindow->ZoomAll();
        GOrbitApp->NeedsRedraw();
        DoZoom = false;
    }
}

//-----------------------------------------------------------------------------
void OrbitApp::CheckForUpdate()
{
    if( !m_HasPromptedForUpdate && OrbitVersion::s_NeedsUpdate )
    {
        SendToUiNow( L"Update" );
        m_HasPromptedForUpdate = true;
    }
}

//-----------------------------------------------------------------------------
string OrbitApp::GetVersion()
{
    return OrbitVersion::GetVersion();
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterProcessesDataView( ProcessesDataView* a_Processes )
{
    assert(m_ProcessesDataView == nullptr);
    m_ProcessesDataView = a_Processes;
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterModulesDataView( ModulesDataView* a_Modules )
{
    assert(m_ModulesDataView == nullptr);
    assert(m_ProcessesDataView != nullptr);
    m_ModulesDataView = a_Modules;
    m_ProcessesDataView->SetModulesDataView( m_ModulesDataView );
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterFunctionsDataView( FunctionsDataView* a_Functions)
{
    m_FunctionsDataView = a_Functions;
    m_Panels.push_back( a_Functions );
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterLiveFunctionsDataView( LiveFunctionsDataView* a_Functions )
{
    m_LiveFunctionsDataView = a_Functions;
    m_Panels.push_back( a_Functions );
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterCallStackDataView( CallStackDataView* a_Callstack )
{
    assert(m_CallStackDataView == nullptr );
    m_CallStackDataView = a_Callstack;
    m_Panels.push_back(a_Callstack);
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterTypesDataView( TypesDataView* a_Types)
{
    m_TypesDataView = a_Types;
    m_Panels.push_back( a_Types );
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterGlobalsDataView( GlobalsDataView* a_Globals)
{
    m_GlobalsDataView = a_Globals;
    m_Panels.push_back( a_Globals );
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterSessionsDataView( SessionsDataView* a_Sessions )
{
    m_SessionsDataView = a_Sessions;
    m_Panels.push_back( a_Sessions );
    ListSessions();
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterCaptureWindow( CaptureWindow* a_Capture )
{
    assert(m_CaptureWindow == nullptr);
    m_CaptureWindow = a_Capture;
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterOutputLog( LogDataView * a_Log )
{
    assert( m_Log == nullptr );
    m_Log = a_Log;
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterRuleEditor( RuleEditor* a_RuleEditor )
{
    assert( m_RuleEditor == nullptr );
    m_RuleEditor = a_RuleEditor;
}

//-----------------------------------------------------------------------------
void OrbitApp::NeedsRedraw()
{
    m_CaptureWindow->NeedsUpdate();
}

//-----------------------------------------------------------------------------
void OrbitApp::AddSamplingReport(shared_ptr<SamplingProfiler> & a_SamplingProfiler)
{
    auto report = make_shared<SamplingReport>(a_SamplingProfiler);
    
    ThreadViewManager::s_CurrentSamplingProfiler = a_SamplingProfiler;

    for( SamplingReportCallback & callback : GOrbitApp->m_SamplingReportsCallbacks )
    {
        callback(report);
    }

    ThreadViewManager::s_CurrentSamplingProfiler = nullptr;
}

//-----------------------------------------------------------------------------
void OrbitApp::AddSelectionReport( shared_ptr<SamplingProfiler> & a_SamplingProfiler )
{
    auto report = make_shared<SamplingReport>( a_SamplingProfiler );

    for( SamplingReportCallback & callback : GOrbitApp->m_SelectionReportCallbacks )
    {
        callback( report );
    }
}

//-----------------------------------------------------------------------------
void OrbitApp::GoToCode( DWORD64 a_Address )
{
    m_CaptureWindow->FindCode( a_Address );
    SendToUiNow( L"gotocode" );
}

//-----------------------------------------------------------------------------
void OrbitApp::OnOpenPdb( const wstring a_FileName )
{
    Capture::GTargetProcess = make_shared<Process>();
    shared_ptr<Module> mod = make_shared<Module>();
    
    mod->m_FullName = a_FileName;
    mod->m_Name = Path::GetFileName( mod->m_FullName );
    mod->m_Directory = Path::GetDirectory( mod->m_FullName );
    mod->m_PdbName = a_FileName;
    mod->m_FoundPdb = true;
    mod->LoadDebugInfo();

    Capture::GTargetProcess->m_Name = Path::StripExtension( mod->m_Name );
    Capture::GTargetProcess->AddModule( mod );

    m_ModulesDataView->SetProcess( Capture::GTargetProcess );
    Capture::SetTargetProcess( Capture::GTargetProcess );
    GOrbitApp->FireRefreshCallbacks();

    EnqueueModuleToLoad( mod );
    LoadModules();
}

//-----------------------------------------------------------------------------
void OrbitApp::OnLaunchProcess( const wstring a_ProcessName, const wstring a_WorkingDir, const wstring a_Args )
{
    m_Debugger->LaunchProcess( a_ProcessName, a_WorkingDir, a_Args );
}

//-----------------------------------------------------------------------------
wstring OrbitApp::GetCaptureFileName()
{
    return Path::StripExtension( Capture::GTargetProcess->GetName() ) + L"_" + OrbitUtils::GetTimeStampW() + L".orbit";
}

//-----------------------------------------------------------------------------
wstring OrbitApp::GetSessionFileName()
{
    return Capture::GSessionPresets ? Capture::GSessionPresets->m_FileName : L"";
}

//-----------------------------------------------------------------------------
void OrbitApp::OnSaveSession( const wstring a_FileName )
{
    Capture::SaveSession( a_FileName );
    ListSessions();
    Refresh( DataViewType::SESSIONS );
}

//-----------------------------------------------------------------------------
void OrbitApp::OnLoadSession( const wstring a_FileName )
{
    shared_ptr<Session> session = make_shared<Session>();

    wstring fileName = Path::GetDirectory( a_FileName ) == L"" ? Path::GetPresetPath() + a_FileName : a_FileName; 

    ifstream file( ws2s(fileName.c_str()) );
    if (!file.fail())
    {
        cereal::BinaryInputArchive archive( file );
        archive(*session);
        if( SelectProcess( Path::GetFileName( session->m_ProcessFullPath ) ) )
        {
            session->m_FileName = fileName;
            Capture::LoadSession( session );
            Capture::GPresetToLoad = L"";
        }

        file.close();
    }
}

//-----------------------------------------------------------------------------
void OrbitApp::OnSaveCapture( const wstring a_FileName )
{
    CaptureSerializer ar;
    ar.m_TimeGraph = GCurrentTimeGraph;
    ar.Save( a_FileName );
}

//-----------------------------------------------------------------------------
void OrbitApp::OnLoadCapture( const wstring a_FileName )
{
    StopCapture();
    Capture::ClearCaptureData();
    GCurrentTimeGraph->Clear();
    if( Capture::GClearCaptureDataFunc )
    {
        Capture::GClearCaptureDataFunc();
    }

    CaptureSerializer ar;
    ar.m_TimeGraph = GCurrentTimeGraph;
    ar.Load( a_FileName );
    StopCapture();
    DoZoom = true; //TODO: remove global, review logic
}

//-----------------------------------------------------------------------------
void GLoadPdbAsync( const shared_ptr<Module> & a_Module )
{
    GModuleManager.LoadPdbAsync( a_Module, [](){ GOrbitApp->OnPdbLoaded(); } );
}

//-----------------------------------------------------------------------------
void OrbitApp::OnOpenCapture( const wstring a_FileName )
{
    Capture::OpenCapture( a_FileName );
    OnPdbLoaded();
}

//-----------------------------------------------------------------------------
void OrbitApp::OnDisconnect()
{
    GTcpServer->Send( Msg_Unload );
}

//-----------------------------------------------------------------------------
void OrbitApp::OnPdbLoaded()
{
    FireRefreshCallbacks();

    if( m_ModulesToLoad.size() == 0 )
    {
        SendToUiAsync( L"pdbloaded" );
    }
    else
    {
        LoadModules();
    }
}

//-----------------------------------------------------------------------------
void OrbitApp::LogMsg( const wstring & a_Msg )
{
    ORBIT_LOG( a_Msg );
}

//-----------------------------------------------------------------------------
void OrbitApp::FireRefreshCallbacks( DataViewType a_Type )
{
    for( DataViewModel* panel : m_Panels )
    {
        if( a_Type == DataViewType::ALL || a_Type == panel->GetType() )
        {
            panel->OnDataChanged();
        }
    }

    // UI callbacks
    for( RefreshCallback & callback : m_RefreshCallbacks )
    {
        callback( a_Type );
    }
}

//-----------------------------------------------------------------------------
void OrbitApp::AddUiMessageCallback( function< void( const wstring & ) > a_Callback )
{
    GTcpServer->SetUiCallback( a_Callback );
    m_UiCallback = a_Callback;
}

//-----------------------------------------------------------------------------
void OrbitApp::StartCapture()
{
    Capture::StartCapture();
    
    if( m_NeedsThawing )
    {
        m_Debugger->SendThawMessage();
        m_NeedsThawing = false;
    }
}

//-----------------------------------------------------------------------------
void OrbitApp::StopCapture()
{   
    GCurrentTimeGraph->m_MemTracker.DumpReport();
    Capture::StopCapture();
}

//-----------------------------------------------------------------------------
void OrbitApp::ToggleCapture()
{
    if( GTimerManager )
    {
        if( GTimerManager->m_IsRecording )
            StopCapture();
        else
            StartCapture();
    }
}

//-----------------------------------------------------------------------------
void OrbitApp::Unregister( DataViewModel * a_Model )
{
    for( int i = 0; i < m_Panels.size(); ++i )
    {
        if( m_Panels[i] == a_Model )
        {
            m_Panels.erase( m_Panels.begin()+i );
        }
    }
}

//-----------------------------------------------------------------------------
bool OrbitApp::SelectProcess( const wstring & a_Process )
{
    if( m_ProcessesDataView )
    {
        return m_ProcessesDataView->SelectProcess( a_Process );
    }

    return false;
}

//-----------------------------------------------------------------------------
bool OrbitApp::SelectProcess( unsigned long a_ProcessID )
{
    if( m_ProcessesDataView )
    {
        return m_ProcessesDataView->SelectProcess( a_ProcessID );
    }

    return false;
}

//-----------------------------------------------------------------------------
bool OrbitApp::Inject( unsigned long a_ProcessId )
{
    if( SelectProcess( a_ProcessId ) )
    {
        return Capture::Inject();
    }

    return false;
}

//-----------------------------------------------------------------------------
void OrbitApp::SetCallStack(shared_ptr<CallStack> a_CallStack)
{
    m_CallStackDataView->SetCallStack( a_CallStack );
    FireRefreshCallbacks( DataViewType::CALLSTACK );
}

//-----------------------------------------------------------------------------
void OrbitApp::SendToUiAsync( const wstring & a_Msg )
{
    GTcpServer->SendToUiAsync( a_Msg );
}

//-----------------------------------------------------------------------------
void OrbitApp::SendToUiNow( const wstring & a_Msg )
{
    m_UiCallback( a_Msg );
}

//-----------------------------------------------------------------------------
void OrbitApp::EnqueueModuleToLoad( const shared_ptr<Module> & a_Module )
{
    m_ModulesToLoad.push( a_Module );
}

//-----------------------------------------------------------------------------
void OrbitApp::LoadModules()
{
    if( m_ModulesToLoad.size() > 0 )
    {
        shared_ptr< Module > module = m_ModulesToLoad.front();
        m_ModulesToLoad.pop();
        GLoadPdbAsync( module );
    }
}

//-----------------------------------------------------------------------------
bool OrbitApp::IsLoading()
{
    return GPdbDbg->IsLoading();
}

//-----------------------------------------------------------------------------
void OrbitApp::SetTrackContextSwitches( bool a_Value )
{
    GParams.m_TrackContextSwitches = a_Value;
}

//-----------------------------------------------------------------------------
bool OrbitApp::GetTrackContextSwitches()
{
    return GParams.m_TrackContextSwitches;
}

//-----------------------------------------------------------------------------
void OrbitApp::EnableUnrealSupport( bool a_Value ) 
{ 
    GParams.m_UnrealSupport = a_Value; 
}

//-----------------------------------------------------------------------------
bool OrbitApp::GetUnrealSupportEnabled()
{ 
    return GParams.m_UnrealSupport; 
}

//-----------------------------------------------------------------------------
void OrbitApp::EnableUnsafeHooking( bool a_Value )
{
    GParams.m_AllowUnsafeHooking = a_Value;
}

//-----------------------------------------------------------------------------
bool OrbitApp::GetUnsafeHookingEnabled()
{
    return GParams.m_AllowUnsafeHooking;
}

//-----------------------------------------------------------------------------
bool OrbitApp::GetOutputDebugStringEnabled()
{
    return GParams.m_HookOutputDebugString;
}

//-----------------------------------------------------------------------------
void OrbitApp::EnableOutputDebugString( bool a_Value )
{
    GParams.m_HookOutputDebugString = a_Value;
}

//-----------------------------------------------------------------------------
void OrbitApp::EnableSampling( bool a_Value )
{
    GParams.m_TrackSamplingEvents = a_Value;
}

//-----------------------------------------------------------------------------
bool OrbitApp::GetSamplingEnabled()
{
    return GParams.m_TrackSamplingEvents;
}

//-----------------------------------------------------------------------------
void OrbitApp::OnMiniDump( const Message & a_Message )
{
    wstring dumpPath = Path::GetDumpPath();
    wstring o_File = dumpPath + L"a_received.dmp";
    ofstream out( ws2s(o_File), ios::binary );
    out.write( a_Message.m_Data, a_Message.m_Size );
    out.close();

    MiniDump miniDump(o_File);
    shared_ptr<Process> process = miniDump.ToOrbitProcess();
    process->SetID( (DWORD)a_Message.GetHeader().m_GenericHeader.m_Address );
    GOrbitApp->m_ProcessesDataView->SetRemoteProcess( process );
}

//-----------------------------------------------------------------------------
void OrbitApp::LaunchRuleEditor( Function * a_Function )
{
    m_RuleEditor->m_Window.Launch( a_Function );
    SendToUiNow(TEXT("RuleEditor"));
}
