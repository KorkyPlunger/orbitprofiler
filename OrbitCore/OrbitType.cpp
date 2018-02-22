//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------


#include "OrbitType.h"
#include "BaseTypes.h"
#include "Pdb.h"
#include "Log.h"
#include "Capture.h"

#include <xxhash.h> // xxHash-r42

#include "SamplingProfiler.h"
#include "PrintVar.h"
#include "Params.h"
#include "Serialization.h"
#include "TcpServer.h"

#ifdef _WIN32
#include "OrbitDia.h"
#include "DiaManager.h"
#include "SymbolUtils.h"
#include "DiaParser.h"
#include <dia2.h>
#endif

using namespace std;

//-----------------------------------------------------------------------------
void Type::LoadDiaInfo()
{
#ifdef _WIN32
    if( !m_DiaInfoLoaded )
    {
        if( IDiaSymbol* diaSymbol = GetDiaSymbol() )
        {
            m_DiaInfoLoaded = true;
            GenerateDiaHierarchy();
            DiaParser parser;
            parser.GetTypeInformation( this, SymTagData );
            parser.GetTypeInformation( this, SymTagFunction );
            GenerateDataLayout();
        }
    }
#endif
}

//-----------------------------------------------------------------------------
void Type::GenerateDiaHierarchy()
{
#ifdef _WIN32
    if( m_HierarchyGenerated )
        return;

    LoadDiaInfo();
    IDiaSymbol* diaSymbol = GetDiaSymbol();
    GenerateDiaHierarchy( diaSymbol );
    diaSymbol->Release();

    for( auto & pair : m_ParentTypes )
    {
        ULONG parentId = pair.second.m_TypeId;
        Type & parentType = m_Pdb->GetTypeFromId( parentId );
        
        if( parentType.m_Id == parentId )
        {
            DiaParser parser;
            parser.GetTypeInformation( &parentType, SymTagData );
            parentType.GenerateDiaHierarchy();
        }
    }

    m_HierarchyGenerated = true;
#endif
}

//-----------------------------------------------------------------------------
void Type::AddParent( IDiaSymbol* a_Parent )
{
#ifdef _WIN32
    LONG offset;
    if( a_Parent->get_offset( &offset ) == S_OK )
    {
        IDiaSymbol* typeSym;
        if( a_Parent->get_type( &typeSym ) == S_OK )
        {
            DWORD typeId;
            if( typeSym->get_symIndexId( &typeId ) == S_OK )
            {
                if( m_ParentTypes.find( typeId ) != m_ParentTypes.end() )
                {
                    // We have already treated this parent, perhaps a recursive hierarchy...
                    return;
                }

                Parent parent;
                parent.m_BaseOffset = offset;
                parent.m_Name = VAR_TO_STR( typeId );
                parent.m_TypeId = typeId;
                m_ParentTypes[typeId] = parent;
            }
        }
    }
#endif
}

//-----------------------------------------------------------------------------
void Type::GenerateDiaHierarchy( IDiaSymbol* a_DiaSymbol )
{
#ifdef _WIN32
    IDiaEnumSymbols *pEnumChildren;
    IDiaSymbol *pChild;
    DWORD dwSymTag;
    ULONG celt = 0;

    if( a_DiaSymbol->get_symTag( &dwSymTag ) != S_OK )
    {
        return;
    }

    if( dwSymTag  == SymTagUDT )
    {
        if( SUCCEEDED( a_DiaSymbol->findChildren( SymTagBaseClass, NULL, nsNone, &pEnumChildren ) ) )
        {
            while( SUCCEEDED( pEnumChildren->Next( 1, &pChild, &celt ) ) && ( celt == 1 ) )
            {
                AddParent( pChild );
                pChild->Release();
            }

            pEnumChildren->Release();
        }
    }

    for( auto & pair : m_ParentTypes )
    {
        Parent & parent = pair.second;
        Type & type = m_Pdb->GetTypeFromId( parent.m_TypeId );
        type.GenerateDiaHierarchy();
    }
#endif
}

//-----------------------------------------------------------------------------
void Type::GenerateDataLayout() const
{
    if( m_Hierarchy.size() == 0 )
    {
        GenerateHierarchy( m_Hierarchy );

        for( const auto & it : m_Hierarchy )
        {
            const Parent & parent = it.second;
            LONG parentOffset = parent.m_BaseOffset;
            const Type & type = m_Pdb->GetTypeFromId(parent.m_TypeId);
            type.ListDataMembers( parentOffset, m_DataMembersFull );
        }

        ListDataMembers( m_BaseOffset, m_DataMembersFull );
    }
}

//-----------------------------------------------------------------------------
void Type::ListDataMembers( ULONG a_BaseOffset, map<ULONG, Variable> & o_DataMembersFull ) const
{
    for( auto & pair : m_DataMembers )
    {
        ULONG offset = pair.first + a_BaseOffset;
        const Variable & member = pair.second;
        //const Type & memberType = m_Pdb->GetTypeFromId( member.m_TypeIndex );
       
        if( o_DataMembersFull.find(offset) != o_DataMembersFull.end() )
        {
            ORBIT_LOG( "Error in print type" );
        }

        o_DataMembersFull[offset] = member;
    }
}

//-----------------------------------------------------------------------------
const map<ULONG, Variable > & Type::GetFullVariableMap() const
{   
    GenerateDataLayout();
    return m_DataMembersFull;
}

//-----------------------------------------------------------------------------
IDiaSymbol* Type::GetDiaSymbol()
{
    if( !m_Pdb )
    {
        return nullptr;
    }

    IDiaSymbol* sym = m_Pdb->GetDiaSymbolFromId( m_Id );
    if( sym == nullptr )
    {
        sym = m_Pdb->GetDiaSymbolFromId( m_UnmodifiedId );
    }
    return sym;
}

//-----------------------------------------------------------------------------
bool Type::IsA( const wstring & a_TypeName )
{
    GenerateDiaHierarchy();

    if( m_Name == a_TypeName )
    {
        return true;
    }

    for( auto & pair : m_ParentTypes )
    {
        Parent & parent = pair.second;
        Type & parentType = m_Pdb->GetTypeFromId( parent.m_TypeId );
        if( parentType.IsA( a_TypeName ) )
        {
            return true;
        }
    }

    return false;
}

//-----------------------------------------------------------------------------
int Type::GetOffset( const wstring & a_Member )
{
    LoadDiaInfo();
    
    for( auto & pair : m_DataMembersFull )
    {
        Variable & var = pair.second;
        if( var.m_Name == a_Member )
        {
            return pair.first;
        }
    }

    return -1;
}

//-----------------------------------------------------------------------------
Variable* Type::FindImmediateChild( const wstring & a_Name )
{
    LoadDiaInfo();

    for( auto & pair : m_DataMembers )
    {
        Variable & var = pair.second;
        if( var.m_Name == a_Name )
        {
            Type* type = var.GetType();
            type->LoadDiaInfo();
            return &var;
        }
    }

    return nullptr;
}

//-----------------------------------------------------------------------------
void Type::OutputPadding() const
{
    ULONG64 nextOffset = 0;
    ULONG64 idealNextOffset = 0;

    for (auto & pair : m_DataMembersFull)
    {
        ULONG64 offset = pair.first;
        const Variable& member = pair.second;
        const Type & memberType = m_Pdb->GetTypeFromId(member.m_TypeIndex);

        const auto & nextIt = m_DataMembersFull.upper_bound( pair.first );
        nextOffset = nextIt != m_DataMembersFull.end() ? nextIt->first : m_Length;
        
        idealNextOffset = offset + memberType.m_Length;
        if( memberType.m_Length > 0 && nextOffset > idealNextOffset )
        {
            Variable paddingMember;
            paddingMember.m_Name = L"padding";
            paddingMember.m_TypeIndex = 0xFFFFFFFF;
            paddingMember.m_Size = ULONG(nextOffset - idealNextOffset);
            m_DataMembers[(ULONG)idealNextOffset] = paddingMember;
            m_DataMembersFull[(ULONG)idealNextOffset] = m_DataMembers[(ULONG)idealNextOffset];
        }
    }
}

//-----------------------------------------------------------------------------
void Type::GenerateHierarchy( map<ULONG, Parent> & a_Hierarchy, int a_Offset ) const
{
    for( auto it : m_ParentTypes )
    {
        Parent & parent = it.second;
        ULONG parentOffset = parent.m_BaseOffset;

        const Type & parentType = m_Pdb->GetTypeFromId( parent.m_TypeId );

        parentType.GenerateHierarchy( a_Hierarchy, a_Offset + parentOffset );

        if( parentType.m_DataMembers.size() > 0 )
        {
            ULONG firstVarOffset = a_Offset + parentOffset + parentType.m_DataMembers.begin()->first;

            if( a_Hierarchy.find(firstVarOffset) != a_Hierarchy.end() )
            {
                ORBIT_LOG("Error in GenerateHierarchy");
            }

            parent.m_BaseOffset += a_Offset;
            parent.m_Name = parentType.m_Name;
            a_Hierarchy[firstVarOffset] = parent;
        }
    }
}

//-----------------------------------------------------------------------------
unsigned long long Type::Hash()
{
    if( m_Hash == 0 )
    {
        XXH64_state_t xxHashState;
        XXH64_reset( &xxHashState, 0x123456789ABCDEFF );

        XXH64_update( &xxHashState, &m_Pdb, sizeof(m_Pdb) );
        XXH64_update( &xxHashState, m_Name.data(), m_Name.size() );
        XXH64_update( &xxHashState, &m_Length, sizeof( m_Length ) );
        XXH64_update( &xxHashState, &m_NumVariables, sizeof( m_NumVariables ) );
        XXH64_update( &xxHashState, &m_NumFunctions, sizeof( m_NumFunctions) );
        XXH64_update( &xxHashState, &m_Id, sizeof( m_Id ) );
        XXH64_update( &xxHashState, & m_NumBaseClasses, sizeof( m_NumBaseClasses) );
    
        m_Hash = XXH64_digest( &xxHashState );
    }

    return m_Hash;
}

//-----------------------------------------------------------------------------
shared_ptr<Variable> Type::GetTemplateVariable()
{
    if( m_TemplateVariable == nullptr )
    {
        m_TemplateVariable = GenerateVariable( 0 );
    }

    return m_TemplateVariable;
}

//-----------------------------------------------------------------------------
shared_ptr<Variable> Type::GenerateVariable( DWORD64 a_Address, const wstring* a_Name )
{
    LoadDiaInfo();

    shared_ptr<Variable> var = make_shared<Variable>();
    var->m_Pdb = this->m_Pdb;
    var->m_Address = a_Address;
    var->m_TypeIndex = m_Id;
    var->m_Name = a_Name ? *a_Name : this->m_Name;
    var->m_Size = (ULONG)this->m_Length;

    // Parents
    for( auto & pair : m_ParentTypes )
    {
        Parent & parent = pair.second;
        ULONG baseOffset = parent.m_BaseOffset;
        
        if( Type* type = m_Pdb->GetTypePtrFromId( parent.m_TypeId ) )
        {
            shared_ptr<Variable> parent = type->GenerateVariable( a_Address + baseOffset );
            parent->m_IsParent = true;
            parent->m_BaseOffset = baseOffset;
            var->AddChild( parent );
        }
    }
    
    // Members
    for( auto & pair : m_DataMembers )
    {
        ULONG memberOffset = pair.first;
        Variable & member = pair.second;
        Type* type = m_Pdb->GetTypePtrFromId( member.m_TypeIndex );
        type->LoadDiaInfo();
        if( type && type->HasMembers() )
        {
            var->AddChild( type->GenerateVariable( a_Address + memberOffset, &member.m_Name ) );
        }
        else
        {
            shared_ptr<Variable> newMember = make_shared<Variable>(member);
            newMember->m_Address = a_Address + memberOffset;
            newMember->m_Name = member.m_Name;
            var->AddChild( newMember );
        }
    }

    return var;
}
