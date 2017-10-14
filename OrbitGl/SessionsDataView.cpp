//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Core.h"
#include "SessionsDataView.h"
#include "ModuleDataView.h"
#include "Pdb.h"
#include "OrbitType.h"
#include "Capture.h"
#include "App.h"
#include "Callstack.h"
#include "OrbitSession.h"

using namespace std;

//-----------------------------------------------------------------------------
SessionsDataView::SessionsDataView()
{
    m_SortingToggles.resize(SDV_NumColumns, false);
    GOrbitApp->RegisterSessionsDataView(this);
}

//-----------------------------------------------------------------------------
vector<float> SessionsDataView::s_HeaderRatios;

//-----------------------------------------------------------------------------
const vector<wstring>& SessionsDataView::GetColumnHeaders()
{
	static vector<wstring> Columns;
    if( Columns.size() == 0 )
    { 
        Columns.push_back( L"Session" );  s_HeaderRatios.push_back( 0.5 );
        Columns.push_back( L"Process" );  s_HeaderRatios.push_back( 0 );
        //Columns.push_back( L"LastUsed" ); s_HeaderRatios.push_back( 0 );
    };

	return Columns;
}

//-----------------------------------------------------------------------------
enum SessionsContextMenuIDs
{
    SESSIONS_LOAD
};

//-----------------------------------------------------------------------------
const vector<wstring>& SessionsDataView::GetContextMenu( int a_Index )
{
    static vector<wstring> Menu = { L"Load Session" };
    return Menu;
}

//-----------------------------------------------------------------------------
const vector<float>& SessionsDataView::GetColumnHeadersRatios()
{
    return s_HeaderRatios;
}

//-----------------------------------------------------------------------------
wstring SessionsDataView::GetValue( int row, int col )
{
	wstring value;

    const shared_ptr<Session> & session = GetSession(row);

    switch (col)
    {
    case SDV_SessionName:
        value = Path::GetFileName( session->m_FileName ); break;
    case SDV_ProcessName:
        value = Path::GetFileName( session->m_ProcessFullPath ); break;
    /*case SDV_LastUsed:
        value = "LastUsed"; break;*/
        break;
    default: break;
    }

	return value;
}

//-----------------------------------------------------------------------------
wstring SessionsDataView::GetToolTip( int a_Row, int a_Column )
{
    const Session & session = *GetSession(a_Row);
    return session.m_FileName;
}

//-----------------------------------------------------------------------------
#define ORBIT_SESSION_SORT( Member ) [&](int a, int b) { return OrbitUtils::Compare(m_Sessions[a]->##Member, m_Sessions[b]->##Member, ascending); }

//-----------------------------------------------------------------------------
void SessionsDataView::OnSort(int a_Column, bool a_Toggle)
{
    SdvColumn pdvColumn = SdvColumn(a_Column);
    
    if (a_Toggle)
    {
        m_SortingToggles[pdvColumn] = !m_SortingToggles[pdvColumn];
    }

    bool ascending = m_SortingToggles[pdvColumn];
    function<bool(int a, int b)> sorter = nullptr;

    switch (pdvColumn)
    {
    case SDV_SessionName: sorter = ORBIT_SESSION_SORT(m_FileName); break;
    case SDV_ProcessName: sorter = ORBIT_SESSION_SORT(m_ProcessFullPath); break;
    }

    if (sorter)
    {
        sort(m_Indices.begin(), m_Indices.end(), sorter);
    }

    m_LastSortedColumn = a_Column;
}

//-----------------------------------------------------------------------------
void SessionsDataView::OnContextMenu( int a_Index, vector<int> & a_ItemIndices )
{
    switch( a_Index )
    {
    case SESSIONS_LOAD:
    {
        for( int index : a_ItemIndices )
        {
            const shared_ptr<Session> & session = GetSession( index );
            if( GOrbitApp->SelectProcess( Path::GetFileName( session->m_ProcessFullPath ) ) )
            {
                Capture::LoadSession( session );
            }
        }

        GOrbitApp->LoadModules();
        break;
    }
    default: break;
    }
}

//-----------------------------------------------------------------------------
void SessionsDataView::OnFilter( const wstring & a_Filter )
{
    vector<int> indices;

    vector< wstring > tokens = Tokenize( ToLower( a_Filter ) );

    for (int i = 0; i < (int)m_Sessions.size(); ++i)
    {
        const Session & session = *m_Sessions[i];
        wstring name = Path::GetFileName( ToLower( session.m_FileName ) );
        wstring path = ToLower( session.m_ProcessFullPath );

        bool match = true;

        for( wstring & filterToken : tokens )
        {
            if (!(name.find(filterToken) != wstring::npos ||
                path.find(filterToken) != wstring::npos))
            {
                match = false;
                break;
            }
        }

        if (match)
        {
            indices.push_back(i);
        }
    }

    m_Indices = indices;

    if (m_LastSortedColumn != -1)
    {
        OnSort(m_LastSortedColumn, false);
    }
}

//-----------------------------------------------------------------------------
void SessionsDataView::OnDataChanged()
{
    m_Indices.resize( m_Sessions.size() );
    for( int i = 0; i < m_Sessions.size(); ++i )
    {
        m_Indices[i] = i;
    }

    if( m_LastSortedColumn != -1 )
    {
        OnSort( m_LastSortedColumn, false );
    }
}

//-----------------------------------------------------------------------------
void SessionsDataView::SetSessions( const vector< shared_ptr< Session > > & a_Sessions )
{
    m_Sessions = a_Sessions;
    OnDataChanged();
}

//-----------------------------------------------------------------------------
const shared_ptr<Session> & SessionsDataView::GetSession( unsigned int a_Row ) const
{
    return m_Sessions[m_Indices[a_Row]];
}
