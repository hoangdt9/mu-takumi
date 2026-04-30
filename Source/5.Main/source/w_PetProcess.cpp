// w_PetProcess.cpp: implementation of the PetProcess class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_PetActionStand.h"
#include "w_PetActionRound.h"
#include "w_PetActionDemon.h"
#include "w_PetActionCollecter.h"
#include "w_PetActionCollecter_Add.h"
#include "w_PetActionUnicorn.h"
#include "w_PetProcess.h"
#include "ReadScript.h"
#include "./Utilities/Log/ErrorReport.h"


static BYTE bBuxCode[3] = {0xfc,0xcf,0xab};
static void BuxConvert(BYTE *Buffer,int Size)
{
	for(int i=0;i<Size;i++)
		Buffer[i] ^= bBuxCode[i%3];
}

PetInfoPtr PetInfo::Make()
{
	PetInfoPtr petInfo( new PetInfo );
	return petInfo;
}

PetInfo::PetInfo():
m_scale(0.0f),
m_actions(NULL),
m_speeds(NULL),
m_count(0)
{
}

PetInfo::~PetInfo()
{
	Destroy();
}

void PetInfo::Destroy()
{
	if(NULL != m_actions)
	{
		delete [] m_actions;
	}
	m_actions = NULL;

	if(NULL != m_speeds)
	{
		delete [] m_speeds;
	}
	m_speeds = NULL;
}

void PetInfo::SetActions(int count, int *actions, float *speeds) 
{
	if(NULL == actions || 0 >= count || NULL == speeds) return;

	m_count = count;
	m_actions = new int[count]();
	memcpy(m_actions, actions, sizeof(int)*m_count);

	m_speeds = new float[count];
	memcpy(m_speeds, speeds, sizeof(float)*m_count);
}

PetProcessPtr g_petProcess;

PetProcess& ThePetProcess()
{
	assert( g_petProcess );
	return *g_petProcess;
}

PetProcessPtr PetProcess::Make()
{
	PetProcessPtr petprocess( new PetProcess );
	petprocess->Init();
	return petprocess;
}

PetProcess::PetProcess()
{
}

PetProcess::~PetProcess()
{
	Destroy();
}

void PetProcess::Init()
{
	PetActionStandPtr actionStand = PetActionStand::Make();
	m_petsAction.insert( make_pair( PC4_ELF, actionStand ) );
	//m_petsAction.insert( make_pair( 4, actionStand ) );

	PetActionRoundPtr actionRound = PetActionRound::Make();
	m_petsAction.insert( make_pair( PC4_TEST, actionRound ) );

	PetActionDemonPtr actionTest = PetActionDemon::Make();
	m_petsAction.insert( make_pair( PC4_SATAN, actionTest ) );

	PetActionCollecterPtr actionCollecter = PetActionCollecter::Make();
	m_petsAction.insert( make_pair( XMAS_RUDOLPH, actionCollecter ) );

#ifdef PJH_ADD_PANDA_PET
	PetActionCollecterAddPtr actionCollecter_Add = PetActionCollecterAdd::Make();
	m_petsAction.insert( make_pair( PANDA, actionCollecter_Add ) );
#endif //#ifdef PJH_ADD_PANDA_PET

 	PetActionUnicornPtr actionUnicorn = PetActionUnicorn::Make();
 	m_petsAction.insert( make_pair( UNICORN, actionUnicorn) );
	
	PetActionCollecterSkeletonPtr actionCollecter_Skeleton = PetActionCollecterSkeleton::Make();
	m_petsAction.insert( make_pair( SKELETON, actionCollecter_Skeleton ) );

	LoadData();
}

void PetProcess::Destroy()
{
	for( PetList::iterator iter = m_petsList.begin(); iter != m_petsList.end(); )
	{
		PetList::iterator tempiter = iter;
		++iter;
		BoostWeak_Ptr(PetObject) basepet = *tempiter;
		
		if( basepet.expired() == FALSE )
		{
			basepet.lock()->Release();
			m_petsList.erase( tempiter );
		}
	}
	m_petsList.clear();

 	m_petsAction.clear();
}

BoostWeak_Ptr(PetAction) PetProcess::Find( int key )
{
	ActionMap::iterator iter = m_petsAction.find( key );

	if( iter != m_petsAction.end() )
	{
		return (*iter).second;
	}

	BoostWeak_Ptr(PetAction) temp;
	
	return temp;
}

void PetProcess::Register( BoostSmart_Ptr( PetObject ) pPet )
{
	m_petsList.push_back( pPet );
}

void PetProcess::UnRegister(CHARACTER *Owner, int itemType, bool isUnregistAll)
{
	if( NULL == Owner ) return;

 	for( PetList::iterator iter = m_petsList.begin(); iter != m_petsList.end(); )
 	{
 		PetList::iterator tempiter = iter;
 		++iter;
 		BoostWeak_Ptr(PetObject) basepet = *tempiter;
 		
 		if( basepet.expired() == FALSE ) 
		{
			if( (-1 == itemType && basepet.lock()->IsSameOwner(&Owner->Object)) 
				|| basepet.lock()->IsSameObject(&Owner->Object, itemType) )
			{
				basepet.lock()->Release();
 				m_petsList.erase( tempiter );

 				if( -1 == itemType || !isUnregistAll ) return;
 			}
 		}
 	}
}

bool PetProcess::LoadData()
{
	char FileName[100];
	sprintf(FileName, "Data\\Local\\pet.bmd");

	int _ver;
	int _array;

	FILE *fp = fopen(FileName, "rb");
	if(fp == NULL)
	{
		char Text[256];
    	sprintf(Text,"%s - File not exist.",FileName);
		g_ErrorReport.Write( Text);
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);

		return FALSE;
	}

	fread( &_ver, sizeof(int), 1, fp );
	fread( &_array, sizeof(int), 1, fp );

	if(_array <= 0 || _array > 1024)
	{
		char Text[256];
		sprintf(Text,"%s - Invalid pet action count (%d).",FileName,_array);
		g_ErrorReport.Write( Text);
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);
		fclose(fp);

		return FALSE;
	}

	int _type;
	int _blendMesh;
	float _scale;
	int _count;
	int *_action = new int[_array];
	float *_speed = new float[_array];

	int _listSize = 0;
	fread( &_listSize, sizeof(DWORD), 1, fp );

	int Size = sizeof(int) + sizeof(int) + sizeof(float) + sizeof(int) + ((sizeof(int) + sizeof(float)) * _array);

	if(_listSize <= 0 || _listSize > 4096)
	{
		char Text[256];
		sprintf(Text,"%s - Invalid pet list count (%d).",FileName,_listSize);
		g_ErrorReport.Write( Text);
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);
		delete [] _action;
		delete [] _speed;
		fclose(fp);

		return FALSE;
	}

	BYTE *Buffer = new BYTE [Size*_listSize];

	if(fread( Buffer,Size*_listSize,1,fp ) != 1)
	{
		char Text[256];
		sprintf(Text,"%s - File read failed.",FileName);
		g_ErrorReport.Write( Text);
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);
		delete [] _action;
		delete [] _speed;
		delete [] Buffer;
		fclose(fp);

		return FALSE;
	}

	DWORD dwCheckSum;
	if(fread(&dwCheckSum,sizeof ( DWORD),1,fp) != 1)
	{
		char Text[256];
		sprintf(Text,"%s - Checksum read failed.",FileName);
		g_ErrorReport.Write( Text);
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);
		delete [] _action;
		delete [] _speed;
		delete [] Buffer;
		fclose(fp);

		return FALSE;
	}

	if ( dwCheckSum != GenerateCheckSum2( Buffer, Size*_listSize, 0x7F1D))
	{
		char Text[256];
    	sprintf(Text,"%s - File corrupted.",FileName);
		g_ErrorReport.Write( Text);
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);
		delete [] _action;
		delete [] _speed;
		delete [] Buffer;
		fclose(fp);
		
		return FALSE;
	}
	else
	{
		BYTE *pSeek = Buffer;
		for(int i=0;i<_listSize;i++)
		{
			_type = 0;
			_scale = 0.0f;
			_blendMesh = -1;
			_count = 0;
			ZeroMemory( _action, sizeof(int)*_array );
			ZeroMemory( _speed, sizeof(float)*_array );

			BuxConvert(pSeek,Size);

			memcpy(&_type, pSeek, sizeof(_type));
			pSeek += sizeof(_type);

			memcpy(&_blendMesh, pSeek, sizeof(_blendMesh));
			pSeek += sizeof(_blendMesh);

			memcpy(&_scale, pSeek, sizeof(_scale));
			pSeek += sizeof(_scale);
			
			memcpy(&_count, pSeek, sizeof(_count));
			pSeek += sizeof(_count);
			
			memcpy(_action, pSeek, sizeof(int)*_array);
			pSeek += sizeof(int)*_array;
			
			memcpy(_speed, pSeek, sizeof(float)*_array);
			pSeek += sizeof(float)*_array;

			if(_count > 0 && _count <= _array)
			{
				PetInfoPtr petInfo = PetInfo::Make();
				petInfo->SetBlendMesh( _blendMesh );
				petInfo->SetScale( _scale );
				petInfo->SetActions( _count, _action, _speed );

				m_petsInfo.insert( make_pair(ITEM_HELPER+_type, petInfo) );
			}
		}
	}
	delete [] _action;
	delete [] _speed;
	delete [] Buffer;
	fclose(fp);

	return TRUE;
}

bool PetProcess::IsPet(int itemType)
{
	InfoMap::iterator iter = m_petsInfo.find(itemType);
	if( iter == m_petsInfo.end() ) return FALSE;
  	
	BoostWeak_Ptr(PetInfo) petInfo = (*iter).second;
	if( petInfo.expired() ) return FALSE;

	return TRUE;
}


bool PetProcess::CreatePet( int itemType, int modelType, vec3_t Position, CHARACTER *Owner, int SubType, int LinkBone )
{
	if ( NULL == Owner ) return FALSE;

	PetObjectPtr _tempPet = PetObject::Make();
	if( _tempPet->Create( itemType, modelType, Position, Owner, SubType, LinkBone ) )
	{
		InfoMap::iterator iter = m_petsInfo.find(itemType);
		if( iter == m_petsInfo.end() ) return FALSE;

  		BoostWeak_Ptr(PetInfo) petInfo = (*iter).second;
		
		int _count = 0;
		int *action = NULL;
		float *speed = NULL;
		if( petInfo.expired() == FALSE ) 
		{
			_count = petInfo.lock()->GetActionsCount();
 			action = petInfo.lock()->GetActions();
			speed =  petInfo.lock()->GetSpeeds();

			_tempPet->SetScale( petInfo.lock()->GetScale() );
			_tempPet->SetBlendMesh( petInfo.lock()->GetBlendMesh() );
 		}

		for(int i=0; i<_count; i++)
		{
			_tempPet->SetActions( (PetObject::ActionType)i, Find(action[i]), speed[i] );
		}
		Register( _tempPet );
     	return TRUE;
	}

	return FALSE;
}

void PetProcess::DeletePet(CHARACTER *Owner, int itemType, bool allDelete)
{
	if( NULL == Owner ) return;

	UnRegister( Owner, itemType, allDelete );
}

void PetProcess::SetCommandPet(CHARACTER *Owner, int targetKey, PetObject::ActionType cmdType )
{
	if( NULL == Owner ) return;

 	for( PetList::iterator iter = m_petsList.begin(); iter != m_petsList.end(); )
 	{
 		PetList::iterator tempiter = iter;
 		++iter;
 		BoostWeak_Ptr(PetObject) basepet = *tempiter;
 		
 		if( basepet.expired() == FALSE ) 
		{
			if( basepet.lock()->IsSameOwner(&Owner->Object) )
			{
				basepet.lock()->SetCommand( targetKey, cmdType );
			}
 		}
 	}
}

void PetProcess::UpdatePets()
{
 	for( PetList::iterator iter = m_petsList.begin(); iter != m_petsList.end(); )
 	{
 		PetList::iterator tempiter = iter;
 		++iter;
 		BoostWeak_Ptr(PetObject) basepet = *tempiter;
 		
 		if( basepet.expired() == false ) 
		{
 			basepet.lock()->Update();
 		}
 	}
}

void PetProcess::RenderPets()
{
	for( PetList::iterator iter = m_petsList.begin(); iter != m_petsList.end(); )
 	{
 		PetList::iterator tempiter = iter;
 		++iter;
 		BoostWeak_Ptr(PetObject) basepet = *tempiter;
 		
 		if( basepet.expired() == FALSE ) 
		{
			basepet.lock()->Render();
 		}
 	}
}
