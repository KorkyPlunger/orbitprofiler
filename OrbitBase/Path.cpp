//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Path.h"
#include "Utils.h"
//#include <direct.h>
#include <fstream>
//#include "Shlobj.h"
#include "OrbitTypes.h"
using namespace std;

wstring Path::m_BasePath;
bool         Path::m_IsPackaged;

//-----------------------------------------------------------------------------
void Path::Init()
{
}

//-----------------------------------------------------------------------------
wstring Path::GetExecutableName()
{
#ifdef _WIN32
    WCHAR  cwBuffer[2048] = { 0 };
    LPWSTR pszBuffer = cwBuffer;
    DWORD  dwMaxChars = _countof(cwBuffer);
    DWORD  dwLength = 0;

    dwLength = ::GetModuleFileNameW(NULL, pszBuffer, dwMaxChars);
    wstring exeFullName = wstring(pszBuffer);

    // Clean up "../" inside full path
    wchar_t buffer[MAX_PATH];
    GetFullPathName( exeFullName.c_str(), MAX_PATH, buffer, nullptr );
    exeFullName = buffer;

    replace(exeFullName.begin(), exeFullName.end(), '\\', '/');
    return exeFullName;
#else
    return L"OrbitTodo";
#endif
}

//-----------------------------------------------------------------------------
wstring Path::GetExecutablePath()
{
    wstring fullPath = GetExecutableName();
    wstring path = fullPath.substr(0, fullPath.find_last_of(L"/")) + L"/";
    return path;
}

//-----------------------------------------------------------------------------
bool Path::FileExists(const wstring & a_File)
{
    std::ifstream f( ws2s(a_File).c_str() );
    return f.good();
}

//-----------------------------------------------------------------------------
bool Path::DirExists( const wstring & a_Dir )
{
#ifdef _WIN32
    DWORD ftyp = GetFileAttributesA( ws2s(a_Dir).c_str() );
    if( ftyp == INVALID_FILE_ATTRIBUTES )
        return false;

    if( ftyp & FILE_ATTRIBUTE_DIRECTORY )
        return true;
#endif
    return false;
}

//-----------------------------------------------------------------------------
wstring Path::GetBasePath()
{
    if( m_BasePath.size() > 0 )
        return m_BasePath;

    wstring exePath = GetExecutablePath();
    m_BasePath = exePath.substr(0, exePath.find(L"bin/"));
    m_IsPackaged = DirExists( GetBasePath() + L"text" );

    return m_BasePath;
}
 
//-----------------------------------------------------------------------------
wstring Path::GetOrbitAppPdb()
{
    return GetBasePath() + wstring( L"bin/Win32/Debug/OrbitApp.pdb" );
}

//-----------------------------------------------------------------------------
wstring Path::GetDllPath( bool a_Is64Bit )
{
    wstring basePath = GetBasePath();
    
#ifdef _DEBUG
    basePath += m_IsPackaged ? L"" : ( a_Is64Bit ? L"bin/x64/Debug/" : L"bin/Win32/Debug/" );
#else
    basePath += m_IsPackaged ? L"" : ( a_Is64Bit ? L"bin/x64/Release/" : L"bin/Win32/Release/" );
#endif

    return basePath + GetDllName( a_Is64Bit );
}

//-----------------------------------------------------------------------------
wstring Path::GetDllName( bool a_Is64Bit )
{
    return a_Is64Bit ? L"Orbit64.dll" : L"Orbit32.dll";
}

//-----------------------------------------------------------------------------
wstring Path::GetParamsFileName()
{
    wstring paramsDir = Path::GetAppDataPath() + L"config/";
    _mkdir( ws2s( paramsDir ).c_str() );
    return paramsDir + L"config.xml";
}

//-----------------------------------------------------------------------------
wstring Path::GetFileMappingFileName()
{
    wstring paramsDir = Path::GetAppDataPath() + L"config/";
    _mkdir( ws2s( paramsDir ).c_str());
    return paramsDir + wstring( L"FileMapping.txt" );
}

//-----------------------------------------------------------------------------
wstring Path::GetSymbolsFileName()
{
    wstring paramsDir = Path::GetAppDataPath() + L"config/";
    _mkdir( ws2s( paramsDir ).c_str() );
    return paramsDir + wstring( L"Symbols.txt" );
}

//-----------------------------------------------------------------------------
wstring Path::GetLicenseName()
{
    wstring appDataDir = Path::GetAppDataPath();
    _mkdir( ws2s( appDataDir ).c_str() );
    return  appDataDir + wstring( L"user.txt" );
}

//-----------------------------------------------------------------------------
wstring Path::GetCachePath()
{
    wstring cacheDir = Path::GetAppDataPath() + L"cache/";
    _mkdir( ws2s( cacheDir ).c_str() );
    return cacheDir;
}

//-----------------------------------------------------------------------------
wstring Path::GetPresetPath()
{
    wstring presetDir = Path::GetAppDataPath() + L"presets/";
    _mkdir( ws2s( presetDir ).c_str() );
    return presetDir;
}

//-----------------------------------------------------------------------------
wstring Path::GetPluginPath()
{
    wstring presetDir = Path::GetAppDataPath() + L"plugins/";
    _mkdir( ws2s( presetDir ).c_str() );
    return presetDir;
}

//-----------------------------------------------------------------------------
wstring Path::GetCapturePath()
{
    wstring captureDir = Path::GetAppDataPath() + L"output/";
    _mkdir( ws2s( captureDir ).c_str() );
    return captureDir;
}

//-----------------------------------------------------------------------------
wstring Path::GetDumpPath()
{
    wstring captureDir = Path::GetAppDataPath() + L"dumps/";
    _mkdir( ws2s( captureDir ).c_str() );
    return captureDir;
}

//-----------------------------------------------------------------------------
wstring Path::GetTmpPath()
{
    wstring captureDir = Path::GetAppDataPath() + L"temp/";
    _mkdir( ws2s( captureDir ).c_str() );
    return captureDir;
}

//-----------------------------------------------------------------------------
wstring Path::GetFileName( const wstring & a_FullName )
{
    wstring FullName = a_FullName;
    replace( FullName.begin(), FullName.end(), '\\', '/' );
    auto index = FullName.find_last_of( L"/" );
    if( index != wstring::npos )
    {
        wstring FileName = FullName.substr( FullName.find_last_of( L"/" ) + 1 );
        return FileName;
    }
    
    return a_FullName;
}

//-----------------------------------------------------------------------------
wstring Path::GetFileNameNoExt( const wstring & a_FullName )
{
    return StripExtension( GetFileName( a_FullName ) );
}

//-----------------------------------------------------------------------------
wstring Path::StripExtension( const wstring & a_FullName )
{
    size_t index = a_FullName.find_last_of( L"." );
    if( index != wstring::npos )
        return a_FullName.substr( 0, index );
    return a_FullName;
}

//-----------------------------------------------------------------------------
wstring Path::GetExtension( const wstring & a_FullName )
{
    // returns ".ext" (includes point)
    size_t index = a_FullName.find_last_of( L"." );
    if( index != wstring::npos )
        return a_FullName.substr( index, a_FullName.length() );
    return L"";
}

//-----------------------------------------------------------------------------
wstring Path::GetDirectory( const wstring & a_FullName )
{
    wstring FullName = a_FullName;
    replace(FullName.begin(), FullName.end(), '\\', '/');
    auto index = FullName.find_last_of( L"/" );
    if (index != string::npos)
    {
        wstring FileName = FullName.substr(0, FullName.find_last_of( L"/" ) + 1);
        return FileName;
    }

    return L"";
}

//-----------------------------------------------------------------------------
wstring Path::GetProgramFilesPath()
{
#ifdef WIN32
    TCHAR pf[MAX_PATH] = {0};
    SHGetSpecialFolderPath(
        0,
        pf,
        CSIDL_PROGRAM_FILES,
        FALSE );
    return wstring(pf) + L"\\OrbitProfiler\\";
#else
    return L"TodoLinux";
#endif
}

//-----------------------------------------------------------------------------
wstring Path::GetAppDataPath()
{
    string appData = GetEnvVar( "APPDATA" );
    string path = appData + "\\OrbitProfiler\\";
    _mkdir( path.c_str() );
    return s2ws( path );
}

//-----------------------------------------------------------------------------
wstring Path::GetMainDrive()
{
    return s2ws( GetEnvVar("SystemDrive") );
}

//-----------------------------------------------------------------------------
bool Path::IsSourceFile( const wstring & a_File )
{
    wstring ext = Path::GetExtension( a_File );
    return ext == L".c" || ext == L".cpp" || ext == L".h" || ext == L".hpp" || ext == L".inl" || ext == L".cxx";
}

//-----------------------------------------------------------------------------
vector< wstring > Path::ListFiles( const wstring & a_Dir, function< bool(const wstring &)> a_Filter )
{
    vector< wstring > files;

#ifdef WIN32
    for( auto it = tr2::sys::recursive_directory_iterator( a_Dir );
        it != tr2::sys::recursive_directory_iterator(); ++it )
    {
        const auto& file = it->path();

        if( !is_directory( file ) && a_Filter( file.wstring() ) )
        {
            files.push_back( file.wstring() );
        }
    }
#endif

    return files;
}

//-----------------------------------------------------------------------------
vector< wstring > Path::ListFiles( const wstring & a_Dir, const wstring & a_Filter )
{
    return ListFiles( a_Dir, [&]( const wstring & a_Name ){ return Contains( a_Name, a_Filter ); } );
}
