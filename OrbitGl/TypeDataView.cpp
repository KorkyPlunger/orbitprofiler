//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------


#include "TypeDataView.h"
#include "Capture.h"
#include "OrbitType.h"
#include "App.h"
#include "OrbitProcess.h"
#include "Pdb.h"
#include <algorithm>

#ifdef _WIN32
#include "OrbitDia.h"
#endif

using namespace std;

//-----------------------------------------------------------------------------
TypesDataView::TypesDataView()
{
    m_SortingToggles.resize(Type::NUM_EXPOSED_MEMBERS, false);
    m_SortingToggles[Type::SELECTED] = true;
    OnDataChanged();

    GOrbitApp->RegisterTypesDataView(this);
}

//-----------------------------------------------------------------------------
void TypesDataView::OnDataChanged()
{
    int numTypes = (int)Capture::GTargetProcess->GetTypes().size();
    m_Indices.resize(numTypes);
    for( int i = 0; i < numTypes; ++i )
    {
        m_Indices[i] = i;
    }
}

//-----------------------------------------------------------------------------
vector<int> TypesDataView::s_HeaderMap;
vector<float> TypesDataView::s_HeaderRatios;

//-----------------------------------------------------------------------------
const vector<wstring>& TypesDataView::GetColumnHeaders()
{
    static vector<wstring> Columns;

    if (s_HeaderMap.size() == 0)
    {
        Columns.push_back(L"Index");          s_HeaderMap.push_back(Type::INDEX);                s_HeaderRatios.push_back(0);
        Columns.push_back(L"Type");           s_HeaderMap.push_back(Type::NAME);                 s_HeaderRatios.push_back(0.5f);
        Columns.push_back(L"Length");         s_HeaderMap.push_back(Type::LENGTH);               s_HeaderRatios.push_back(0);
        Columns.push_back(L"TypeId");         s_HeaderMap.push_back(Type::TYPE_ID);              s_HeaderRatios.push_back(0);
        Columns.push_back(L"UnModifiedId");   s_HeaderMap.push_back(Type::TYPE_ID_UNMODIFIED );  s_HeaderRatios.push_back(0);
        Columns.push_back(L"NumVariables");   s_HeaderMap.push_back(Type::NUM_VARIABLES);        s_HeaderRatios.push_back(0);
        Columns.push_back(L"NumFunctions");   s_HeaderMap.push_back(Type::NUM_FUNCTIONS);        s_HeaderRatios.push_back(0);
        Columns.push_back(L"NumBaseClasses"); s_HeaderMap.push_back(Type::NUM_BASE_CLASSES);     s_HeaderRatios.push_back(0);
        Columns.push_back(L"BaseOffset");     s_HeaderMap.push_back(Type::BASE_OFFSET);          s_HeaderRatios.push_back(0);
        Columns.push_back(L"Module");         s_HeaderMap.push_back(Type::MODULE);               s_HeaderRatios.push_back(0);
    }

    return Columns;
}

//-----------------------------------------------------------------------------
const vector<float>& TypesDataView::GetColumnHeadersRatios()
{
    return s_HeaderRatios;
}

//-----------------------------------------------------------------------------
wstring TypesDataView::GetValue( int a_Row, int a_Column )
{
    ScopeLock lock( Capture::GTargetProcess->GetDataMutex() );

    Type & type = GetType(a_Row);

    wstring value;

    switch ( s_HeaderMap[a_Column] )
    {
    case Type::INDEX:
        value = Format(L"%d", a_Row);                      break;
    case Type::SELECTED:                                   
        value = type.m_Selected;                           break;
    case Type::NAME:                                       
        value = type.GetName();                            break;
    case Type::LENGTH:                                     
        value = Format(L"%d", (int)type.m_Length);         break;
    case Type::TYPE_ID:                                    
        value = Format(L"%lu", (int)type.m_Id);            break;
    case Type::TYPE_ID_UNMODIFIED:
        value = Format( L"%lu", (int)type.m_UnmodifiedId );break;
    case Type::NUM_VARIABLES:
        value = Format(L"%d", type.m_NumVariables);        break;
    case Type::NUM_FUNCTIONS:                              
        value = Format(L"%d", type.m_NumFunctions);        break;
    case Type::NUM_BASE_CLASSES:                           
        value = Format(L"%d", type.m_NumBaseClasses);      break;
    case Type::BASE_OFFSET:                                
        value = Format(L"%d", type.m_BaseOffset);          break;
    case Type::MODULE:                                     
        value = type.m_Pdb->GetName();                     break;
    default:                                               break;
    }

    return value;
}

//-----------------------------------------------------------------------------
void TypesDataView::OnFilter( const wstring & a_Filter )
{
    ParallelFilter( a_Filter );

    if( m_LastSortedColumn != -1 )
    {
        OnSort( m_LastSortedColumn, false );
    }
}

//-----------------------------------------------------------------------------
void TypesDataView::ParallelFilter( const wstring & a_Filter )
{
#ifdef _WIN32
    m_FilterTokens = Tokenize( ToLower( a_Filter ) );
    vector<Type*> & types = Capture::GTargetProcess->GetTypes();
    const auto prio = oqpi::task_priority::normal;
    auto numWorkers = oqpi_tk::scheduler().workersCount( prio );
    vector< vector<int> > indicesArray;
    indicesArray.resize( numWorkers );

    oqpi_tk::parallel_for( "TypesDataViewParallelFor", (int)types.size(), [&]( int32_t a_BlockIndex, int32_t a_ElementIndex )
    {
        vector<int> & result = indicesArray[a_BlockIndex];
        const wstring & name = types[a_ElementIndex]->GetNameLower();

        for( wstring & filterToken : m_FilterTokens )
        {
            if( name.find( filterToken ) == wstring::npos )
            {
                return;
            }
        }

        result.push_back( a_ElementIndex );
    } );

    set< int > indicesSet;
    for( vector<int> & results : indicesArray )
    {
        for( int index : results )
        {
            indicesSet.insert( index );
        }
    }

    m_Indices.clear();
    for( int i : indicesSet )
    {
        m_Indices.push_back( i );
    }
#endif
}

//-----------------------------------------------------------------------------
#define ORBIT_TYPE_SORT( Member ) [&](int a, int b) { return OrbitUtils::Compare(types[a]->Member, types[b]->Member, ascending); }

//-----------------------------------------------------------------------------
void TypesDataView::OnSort( int a_Column, bool a_Toggle )
{
    const vector<Type*> & types = Capture::GTargetProcess->GetTypes();
    auto MemberID = Type::MemberID( s_HeaderMap[a_Column] );

    if (a_Toggle)
    {
        m_SortingToggles[MemberID] = !m_SortingToggles[MemberID];
    }

    bool ascending = m_SortingToggles[MemberID];
    
    function<bool(int a, int b)> sorter = nullptr;

    switch (MemberID)
    {
    case Type::NAME:               sorter   = ORBIT_TYPE_SORT(m_Name);           break;
    case Type::LENGTH:             sorter   = ORBIT_TYPE_SORT(m_Length);         break;
    case Type::TYPE_ID:            sorter   = ORBIT_TYPE_SORT(m_Id);             break;
    case Type::TYPE_ID_UNMODIFIED: sorter   = ORBIT_TYPE_SORT(m_UnmodifiedId);   break;
    case Type::NUM_VARIABLES:      sorter   = ORBIT_TYPE_SORT(m_NumVariables);   break;
    case Type::NUM_FUNCTIONS:      sorter   = ORBIT_TYPE_SORT(m_NumFunctions);   break;
    case Type::NUM_BASE_CLASSES:   sorter   = ORBIT_TYPE_SORT(m_NumBaseClasses); break;
    case Type::BASE_OFFSET:        sorter   = ORBIT_TYPE_SORT(m_BaseOffset);     break;
    case Type::MODULE:             sorter   = ORBIT_TYPE_SORT(m_Pdb->GetName()); break;
    case Type::SELECTED:           sorter   = ORBIT_TYPE_SORT(m_Selected);       break;
    }

    if( sorter )
    {
        sort(m_Indices.begin(), m_Indices.end(), sorter);
    }

    m_LastSortedColumn = a_Column;
}

//-----------------------------------------------------------------------------
enum TypesContextMenu
{
    TYPES_MENU_PROP,
    TYPES_MENU_VIEW,
    TYPES_MENU_CLIP
};

//-----------------------------------------------------------------------------
const vector<wstring>& TypesDataView::GetContextMenu(int a_Index)
{
    static vector<wstring> Menu = { L"Summary", L"Details" /*, L"Copy"*/ };
    return Menu;
}

//-----------------------------------------------------------------------------
void TypesDataView::OnProp( vector<int> & a_Items )
{
    for(auto & item : a_Items) 
    { 
        Type & type = GetType(item);
        shared_ptr<Variable> var = type.GetTemplateVariable();
        var->Print();
        GOrbitApp->SendToUiNow( L"output" );
    }
}

//-----------------------------------------------------------------------------
void TypesDataView::OnView( vector<int> & a_Items )
{
#ifdef _WIN32
    for( auto & item : a_Items )
    {
        Type & type = GetType( item );
        shared_ptr<Variable> var = type.GetTemplateVariable();
        var->PrintDetails();
        OrbitDia::DiaDump( type.GetDiaSymbol() );
        GOrbitApp->SendToUiNow( L"output" );
    }
#endif
}

//-----------------------------------------------------------------------------
void TypesDataView::OnClip( vector<int> & a_Items )
{
    GOrbitApp->SendToUiAsync( L"output" );
    //for(auto & item : a_Items) {}
}

//-----------------------------------------------------------------------------
void TypesDataView::OnContextMenu( int a_MenuIndex, vector<int> & a_ItemIndices )
{
    switch (a_MenuIndex)
    {
        case TYPES_MENU_PROP: OnProp(a_ItemIndices); break;
        case TYPES_MENU_VIEW: OnView(a_ItemIndices); break;
        case TYPES_MENU_CLIP: OnClip(a_ItemIndices); break;
        default: break;
    }
}

//-----------------------------------------------------------------------------
Type & TypesDataView::GetType(unsigned int a_Row) const
{
    vector<Type*> & types = Capture::GTargetProcess->GetTypes();
    return *types[m_Indices[a_Row]];
}
