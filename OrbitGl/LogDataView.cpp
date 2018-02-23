#include "LogDataView.h"
#include "App.h"
#include "TcpServer.h"
#include "Capture.h"
#include "App.h"
#include "Callstack.h"
#include "SamplingProfiler.h"
#include <chrono>

using namespace std;

vector<float> LogDataView::s_HeaderRatios;

//-----------------------------------------------------------------------------
LogDataView::LogDataView()
{
    GOrbitApp->RegisterOutputLog( this );
    GTcpServer->SetCallback( Msg_OrbitLog, [=]( const Message & a_Msg ){ this->OnReceiveMessage(a_Msg); } );
    m_UpdatePeriodMs = 100;
}

//-----------------------------------------------------------------------------
const vector<wstring>& LogDataView::GetColumnHeaders()
{
    static vector<wstring> Columns;
    if( Columns.size() == 0 )
    {
        Columns.push_back( L"Log" );      s_HeaderRatios.push_back(0.7f);
        Columns.push_back( L"Time" );     s_HeaderRatios.push_back(0.15f);
        Columns.push_back( L"ThreadId" ); s_HeaderRatios.push_back(0.15f);
    }
    return Columns;
}

//-----------------------------------------------------------------------------
const vector<float>& LogDataView::GetColumnHeadersRatios()
{
    return s_HeaderRatios;
}

//-----------------------------------------------------------------------------
wstring LogDataView::GetValue( int a_Row, int a_Column )
{
    const OrbitLogEntry & entry = GetEntry( a_Row );
    wstring value;

    switch( a_Column )
    {
    case LDV_Time:
    {
#ifdef _WIN32
        auto micros = PerfCounter::get_microseconds(Capture::GCaptureTimer.m_PerfCounter.get_start(), entry.m_Time);
        chrono::system_clock::time_point sysTime = Capture::GCaptureTimePoint + chrono::microseconds(micros);
        time_t now_c = chrono::system_clock::to_time_t(sysTime);
        tm now_tm;
        localtime_s(&now_tm, &now_c);
        TCHAR buffer[256];
        wcsftime( buffer, sizeof( buffer ), L"%H:%M:%S", &now_tm );

        value = buffer;
#endif
        break;
    }
    case LDV_Message:
        value = s2ws(entry.m_Text); break;
    case LDV_ThreadId:
        value = Format(L"%u", entry.m_ThreadId); break;
    default: break;
    }

    return value;
}

//-----------------------------------------------------------------------------
wstring LogDataView::GetToolTip( int a_Row, int a_Column )
{
    return wstring();
}

//-----------------------------------------------------------------------------
bool LogDataView::ScrollToBottom()
{
    return true;
}

//-----------------------------------------------------------------------------
bool LogDataView::SkipTimer()
{
    return !Capture::IsCapturing();
}

//-----------------------------------------------------------------------------
void LogDataView::OnDataChanged()
{
    ScopeLock lock( m_Mutex );
    m_Indices.resize( m_Entries.size() );
    for( int i = 0; i < m_Entries.size(); ++i )
    {
        m_Indices[i] = i;
    }
}

//-----------------------------------------------------------------------------
void LogDataView::OnFilter( const wstring & a_Filter )
{
    vector< string > tokens = Tokenize( ToLower( ws2s( a_Filter ) ) );
    vector<int> indices;

    for( int i = 0; i < (int)m_Entries.size(); ++i )
    {
        const OrbitLogEntry & entry = m_Entries[i];
        string text = ToLower( entry.m_Text );

        bool match = true;

        for( string & filterToken : tokens )
        {
            if( !( text.find( filterToken ) != wstring::npos ) )
            {
                match = false;
                break;
            }
        }

        if( match )
        {
            indices.push_back(i);
        }
    }

    m_Indices = indices;
}

//-----------------------------------------------------------------------------
enum LDV_ContextMenu
{
    GO_TO_CODE,
    GO_TO_CALLSTACK,
    COPY
};

//-----------------------------------------------------------------------------
const vector<wstring>& LogDataView::GetContextMenu( int a_Index )
{
    const OrbitLogEntry & entry = LogDataView::GetEntry( a_Index );
    m_SelectedCallstack = Capture::GetCallstack( entry.m_CallstackHash );
    static vector<wstring> menu;
    menu.clear();
    if( m_SelectedCallstack )
    {
        for( int i = 0; i < m_SelectedCallstack->m_Depth; ++i )
        {
            DWORD64 addr = m_SelectedCallstack->m_Data[i];
            menu.push_back( Capture::GSamplingProfiler->GetSymbolFromAddress(addr) );
        }
    }
    return menu;
}

//-----------------------------------------------------------------------------
void LogDataView::OnContextMenu( int a_Index, vector<int>& a_ItemIndices )
{
    if( m_SelectedCallstack && m_SelectedCallstack->m_Depth > a_Index )
    {
        GOrbitApp->GoToCode( m_SelectedCallstack->m_Data[a_Index] );
    }
}

//-----------------------------------------------------------------------------
void LogDataView::Add( const OrbitLogEntry & a_Msg )
{
    ScopeLock lock( m_Mutex );
    m_Entries.push_back( a_Msg );
    OnDataChanged();
}

//-----------------------------------------------------------------------------
const OrbitLogEntry & LogDataView::GetEntry( unsigned int a_Row ) const
{
    return m_Entries[m_Indices[a_Row]];
}

//-----------------------------------------------------------------------------
void LogDataView::OnReceiveMessage( const Message & a_Msg )
{
    bool isLog = a_Msg.GetType() == Msg_OrbitLog;
    assert(isLog);
    if( isLog )
    {
        OrbitLogEntry entry;
        memcpy( &entry, a_Msg.GetData(), OrbitLogEntry::GetSizeWithoutString() );
        const char* log = a_Msg.GetData() + OrbitLogEntry::GetSizeWithoutString();
        entry.m_Text = log;
        RemoveTrailingNewLine(entry.m_Text);
        Add(entry);
    }
}
