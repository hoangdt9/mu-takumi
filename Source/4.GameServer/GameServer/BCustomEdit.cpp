#include "stdafx.h"
#if (FIXTELE ==1)
#include "BCustomEdit.h"
#include "GameMain.h"
#include "Util.h"
#include "Path.h"
#include "Util.h"
#include "EffectManager.h"
BCustomEdit gBCustomEdit;
// -------------------------------------------------------------------------------

BCustomEdit::BCustomEdit()
{

}
// -------------------------------------------------------------------------------

BCustomEdit::~BCustomEdit()
{

}
// -------------------------------------------------------------------------------

void BCustomEdit::Load()
{
	this->m_SetSkillTeleFix.clear();

	this->SkillFixData(gPath.GetFullPath("Custom\\FixTele.xml")); //Fix Skill Tele

}
// -------------------------------------------------------------------------------

void BCustomEdit::SkillFixData(char* FilePath)
{

	int sk_Index = 0;
	pugi::xml_document file;
	pugi::xml_parse_result res = file.load_file(FilePath);
	if (res.status != pugi::status_ok) {
		ErrorMessageBox("File %s load fail. Error: %s", FilePath, res.description());
		return;
	}
	//--
	pugi::xml_node oBCustomEdit = file.child("BCustomEdit");
	//--

	pugi::xml_node eSkillSuDungKhiTele = oBCustomEdit.child("SkillSuDungKhiTele");
	for (pugi::xml_node rSkillSuDungKhiTele = eSkillSuDungKhiTele.child("Skill"); rSkillSuDungKhiTele; rSkillSuDungKhiTele = rSkillSuDungKhiTele.next_sibling()) {
		SKILKTELEFIX_DATA info;
		info.Index = sk_Index;
		info.IDSkill = rSkillSuDungKhiTele.attribute("IDSkill").as_int();
		this->m_SetSkillTeleFix.insert(std::pair<int, SKILKTELEFIX_DATA>(info.IDSkill, info));
		sk_Index++;
	}
	this->m_DataSkillGiatLui.clear();
	pugi::xml_node eSkillBackSpring = oBCustomEdit.child("SkillBackSpring");
	for (pugi::xml_node rSkillBackSpring = eSkillBackSpring.child("Skill"); rSkillBackSpring; rSkillBackSpring = rSkillBackSpring.next_sibling()) {
		DATA_SKILL_GIATLUI infoS;
		infoS.IDSkill = rSkillBackSpring.attribute("IDSkill").as_int();
		infoS.Rate = rSkillBackSpring.attribute("Rate").as_int();
		infoS.Type = rSkillBackSpring.attribute("Type").as_int();
		this->m_DataSkillGiatLui.insert(std::pair<int, DATA_SKILL_GIATLUI>(infoS.IDSkill, infoS));
	}

	this->m_DataSkillChoang.clear();
	pugi::xml_node eSkillChoang = oBCustomEdit.child("SkillChoang");
	for (pugi::xml_node rSkillChoang = eSkillChoang.child("Skill"); rSkillChoang; rSkillChoang = rSkillChoang.next_sibling()) {
		DATA_SKILL_CHOANGCHAM infoChoang;
		infoChoang.IDSkill = rSkillChoang.attribute("IDSkill").as_int();
		infoChoang.Rate = rSkillChoang.attribute("Rate").as_int();
		infoChoang.Delay = rSkillChoang.attribute("Delay").as_int();
		this->m_DataSkillChoang.insert(std::pair<int, DATA_SKILL_CHOANGCHAM>(infoChoang.IDSkill, infoChoang));
	}
	this->m_DataSkillCham.clear();
	pugi::xml_node eSkillCham = oBCustomEdit.child("SkillCham");
	for (pugi::xml_node rSkillCham = eSkillCham.child("Skill"); rSkillCham; rSkillCham = rSkillCham.next_sibling()) {
		DATA_SKILL_CHOANGCHAM infoCham;
		infoCham.IDSkill = rSkillCham.attribute("IDSkill").as_int();
		infoCham.Rate = rSkillCham.attribute("Rate").as_int();
		infoCham.Delay = rSkillCham.attribute("Delay").as_int();
		this->m_DataSkillCham.insert(std::pair<int, DATA_SKILL_CHOANGCHAM>(infoCham.IDSkill, infoCham));
	}
}

BOOL BCustomEdit::CheckSkillOK(int Skill) //Get Skill Co trong Data hay khong
{
	std::map<int, SKILKTELEFIX_DATA>::iterator it = this->m_SetSkillTeleFix.find(Skill);
	if (it == this->m_SetSkillTeleFix.end())
	{
		return TRUE;
	}
	return FALSE;
}
void BCustomEdit::FixSkillGiatLui(int aIndex, int bIndex, int Skill)
{
	std::map<int, DATA_SKILL_GIATLUI>::iterator it = this->m_DataSkillGiatLui.find(Skill);
	if (it != this->m_DataSkillGiatLui.end())
	{
		if ((rand() % 100) < it->second.Rate) // 
		{
			if (it->second.Type == 0)
			{
				gObjBackSpring(&gObj[bIndex], &gObj[aIndex]);
			}
			else
			{
				gObjBackSpring2(&gObj[bIndex], &gObj[aIndex], it->second.Type);
			}
		}
	}
	//Choang
	std::map<int, DATA_SKILL_CHOANGCHAM>::iterator itChoang = this->m_DataSkillChoang.find(Skill);
	if (itChoang != this->m_DataSkillChoang.end())
	{
		if ((rand() % 100) < itChoang->second.Rate) // 
		{
			gEffectManager.AddEffect(&gObj[bIndex], 0, EFFECT_STERN, itChoang->second.Delay, 0, 0, 0, 0);
		}
	}
	//Cham
	std::map<int, DATA_SKILL_CHOANGCHAM>::iterator itCham = this->m_DataSkillCham.find(Skill);
	if (itCham != this->m_DataSkillCham.end())
	{
		if ((rand() % 100) < itCham->second.Rate) // 
		{
			gEffectManager.AddEffect(&gObj[bIndex], 0, EFFECT_ICE_ARROW, itCham->second.Delay, 0, 0, 0, 0);
		}
	}

}
#endif