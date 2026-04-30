#include "stdafx.h"
#include "CustomMap.h"
#include "MapManager.h"

CCustomMap gCustomMap;

ImgPathMap m_mapImgPath;

void CCustomMap::OpenScritp(PathMap* thisa)
{
	this->MapPath.clear();

	for (int m = 0; m < MAX_MAP; m++)
	{
		if (thisa[m].world <= WD_81KARUTAN2 || thisa[m].world > WD_ENDMAP)
		{
			continue;
		}

		this->MapPath.insert(std::pair<int, PathMap>(thisa[m].world, thisa[m]));
	}
}

PathMap* CCustomMap::GetMapInfo(int world)
{
	listMap::iterator li = this->MapPath.find(world);

	if (li != this->MapPath.end())
	{
		return (PathMap*)& li->second;
	}
	return NULL;
}

char* CCustomMap::GetNameMap(int world)
{
	listMap::iterator li = this->MapPath.find(world);

	if (li != this->MapPath.end())
	{
		return li->second.MapName;
	}
	return "No Found";
}

char* CCustomMap::GetMapName(int iMap)
{
	if (iMap >= 82)
	{
		return gCustomMap.GetNameMap(iMap);
	}
	else
	{
		//return pGetMapName(iMap);
	}
}