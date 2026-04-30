#include "stdafx.h"
#include "NewUISystem.h"
#include "CustomCloak.h"

cCustomCloak gCloak;

cCustomCloak::cCustomCloak() // OK
{
	this->Init();
}
// -------------------------------------------------------------------------------
cCustomCloak::~cCustomCloak() // OK
{

}
// -------------------------------------------------------------------------------
void cCustomCloak::Init() // OK
{
	for(int n=0;n < MAX_CUSTOMCLOAK;n++)
	{
		this->m_Cloak[n].Index = -1;

		this->m_Cloak[n].IndexCloak = -1;

		this->m_Cloak[n].IndexStrip = -1;
	}

	for (int n = 0; n < MAX_CUSTOMCEFFECT; n++)
	{
		this->m_CEffect[n].Index = -1;

		this->m_CEffect[n].ItemIndex = -1;

		this->m_CEffect[n].EffectIndex = -1;
	}
}
// -------------------------------------------------------------------------------
void cCustomCloak::Load(CUSTOMCLOAK_DATA* info) // OK
{
	for(int n=0;n < MAX_CUSTOMCLOAK;n++)
	{
		this->SetInfo(info[n]);
	}
}
// -------------------------------------------------------------------------------
void cCustomCloak::SetInfo(CUSTOMCLOAK_DATA info) // OK
{
	if(info.Index < 0 || info.Index >= MAX_CUSTOMCLOAK)
	{
		return;
	}

	this->m_Cloak[info.Index] = info;
}
// -------------------------------------------------------------------------------
// -------------------------------------------------------------------------------
void cCustomCloak::LoadCEffect(CEFFECT_DATA* info) // OK
{
	for (int n = 0; n < MAX_CUSTOMCEFFECT; n++)
	{
		this->SetInfoCEffect(info[n]);
	}
}
// -------------------------------------------------------------------------------
void cCustomCloak::SetInfoCEffect(CEFFECT_DATA info) // OK
{
	if (info.Index < 0 || info.Index >= MAX_CUSTOMCEFFECT)
	{
		return;
	}
	//g_ConsoleDebug->Write(MCD_RECEIVE, "SetInfoCEffect %d %d", info.ItemIndex, info.EffectIndex);

	this->m_CEffect[info.Index] = info;
}

bool cCustomCloak::isCEffect(int ItemIndex) // OK
{
	for (int i = 0; i < MAX_CUSTOMCEFFECT; i++)
	{
		if (ItemIndex == this->m_CEffect[i].ItemIndex)
		{
			return true;
		}
	}
	return false;
}
// -------------------------------------------------------------------------------
CEFFECT_DATA* cCustomCloak::GetInfoCEffect(int index) // OK
{
	if (index < 0 || index >= MAX_CUSTOMCEFFECT)
	{
		return 0;
	}

	if (this->m_CEffect[index].Index != index)
	{
		return 0;
	}

	return &this->m_CEffect[index];
}
// -------------------------------------------------------------------------------
CEFFECT_DATA* cCustomCloak::GetInfoCEffectByItem(int ItemIndex) // OK
{
	for (int n = 0; n < MAX_CUSTOMCEFFECT; n++)
	{
		CEFFECT_DATA* lpInfo = this->GetInfoCEffect(n);

		if (lpInfo == 0)
		{
			continue;
		}

		if (lpInfo->ItemIndex == ItemIndex)
		{
			return lpInfo;
		}
	}

	return 0;
}
// -------------------------------------------------------------------------------
CUSTOMCLOAK_DATA* cCustomCloak::GetInfo(int index) // OK
{
	if(index < 0 || index >= MAX_CUSTOMCLOAK)
	{
		return 0;
	}

	if(this->m_Cloak[index].Index != index)
	{
		return 0;
	}

	return &this->m_Cloak[index];
}
// -------------------------------------------------------------------------------
CUSTOMCLOAK_DATA* cCustomCloak::GetInfoByItem(int ItemIndex) // OK
{
	for(int n=0;n < MAX_CUSTOMCLOAK;n++)
	{
		CUSTOMCLOAK_DATA* lpInfo = this->GetInfo(n);

		if(lpInfo == 0)
		{
			continue;
		}

		if(lpInfo->ItemIndex == ItemIndex)
		{
			return lpInfo;
		}
	}

	return 0;
}
// -------------------------------------------------------------------------------
bool cCustomCloak::isCloak(int ItemIndex) // OK
{
	for (int i = 0 ; i < MAX_CUSTOMCLOAK ; i++)
	{
		if (ItemIndex == this->m_Cloak[i].ItemIndex)
		{
			return true;
		}
	}
	return false;
}

// -------------------------------------------------------------------------------
DWORD cCustomCloak::getCodeCape(int ItemIndex) // OK
{
	for (int i = 0 ; i < MAX_CUSTOMCLOAK ; i++)
	{
		if (ItemIndex == this->m_Cloak[i].ItemIndex)
		{
			return this->m_Cloak[i].IndexCloak;
		}
	}
	return -1;
}

// -------------------------------------------------------------------------------
bool cCustomCloak::isCloakStrip(int ItemIndex) // OK
{
	for (int i = 0 ; i < MAX_CUSTOMCLOAK ; i++)
	{
		if (ItemIndex == this->m_Cloak[i].ItemIndex && this->m_Cloak[i].Mode == 1)
		{
			return true;
		}
	}
	return false;
}
// -------------------------------------------------------------------------------
void cCustomCloak::LoadTexture() // OK
{
	this->code = 0x16305;

	for(int n=0;n < MAX_CUSTOMCLOAK;n++)
	{
		if(this->m_Cloak[n].Index != -1){

			LoadBitmap(this->m_Cloak[n].CloakName, this->code, GL_LINEAR, GL_REPEAT, 1, 0);
			
			this->m_Cloak[n].IndexCloak = this->code;

			this->code++;

			if(this->m_Cloak[n].Mode == 1){

				LoadBitmap(this->m_Cloak[n].StripName, this->code, GL_LINEAR, GL_REPEAT, 1, 0);

				this->m_Cloak[n].IndexStrip = this->code;

				this->code++;
			}
		}
	}
	//=== Load IMG CEffect Render
	LoadBitmap("Item\\black_gold_crom05.jpg", 532771, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\absolute02_swordrender.jpg", 532768, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\black_gold_crom03.jpg", 532770, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Effect\\flame_chrome.jpg", 532847, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\ground_wind2.jpg", 532496, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\alicewing4db_render.jpg", 532836, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\elfwing4da_render01.jpg", 532834, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\elfwing4da_render02.jpg", 532835, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\magicianwing4da_render.jpg", 532831, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\dknightwing4db_render.jpg", 532833, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Effect\\line_of_big_R.jpg", 532817, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\soulknightswordB_render.jpg", 532840, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\soulknightswordA_render.jpg", 532841, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\soulmagicswordA_render.jpg", 532842, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\hs_soularcherbowA01_render.jpg", 532837, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\sd_SoulWizardStaff01_render.jpg", 532838, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\sd_soulsummonerstickA02_render.jpg", 532843, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\soulfighterclawA_render.jpg", 532846, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\hs_blueeyeknightA03r.jpg", 532571, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Effect\\black_gold_crom04_mono.jpg", 533001, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\hs_blueeyeknightA02_render.jpg", 532570, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\fire_15frame_mono.jpg", 532566, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\alpha_line2.jpg", 532575, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\blueeyemagicsword02r.jpg", 532573, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\blueeyemagicsword03r.jpg", 532574, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Effect\\black_gold_crom04.jpg", 533837, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\blueeyesceptre_render.jpg", 532564, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\Blue_eyebow02r.jpg", 532562, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\blue_eye_ws02_render.jpg", 532565, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\bsummoners02_render.jpg", 532563, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\Blueeyeclaw_render.jpg", 532567, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\mace_mastr02s_render.jpg", 532960, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\Darkangel_StaffA_render.jpg", 532798, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\sd_soullance01_render.jpg", 532542, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\runewing3db_render.jpg", 532959, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\runewing4db_render.jpg", 532965, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\runewing4dc_star01.jpg", 532963, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\runewing4dc_star02.jpg", 532964, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\hs_blueeyelanceA02r.jpg", 532568, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Effect\\MMsSs2_r.jpg", 532549, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\mace5master02_render.jpg", 532569, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Effect\\wings_van_render.jpg", 533024, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Effect\\benzia_wing4_render.jpg", 533026, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Effect\\benzia_wing4a_render.jpg", 533027, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Effect\\benzia_wing4b_render.jpg", 533028, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\season_wing_period_render.jpg", 533043, GL_LINEAR, GL_REPEAT, 1, 0);
	LoadBitmap("Item\\firek_sq_mono.jpg", 533029, GL_LINEAR, GL_REPEAT, 1, 0);
}
float Render(float a1, float a2) //Main S13; Not here
{
	float v5;
	float v2 = a2 * 0.01745f;
	float v6 = (float)((int)(v2 * 1000.0f / a1 + timeGetTime()) % (int)(6283.185546875f / a1)) * 0.001f * a1;

	if (v6 >= 3.14f)
		v5 = cosf(v6);
	else
		v5 = -cosf(v6);
	return (float)((v5 + 1.0f) * 0.5f);
}
double sub_4E7B95(float a1)
{
	return (float)sin(a1);
}
double sub_639050(float a1)
{
	double result;

	if (a1 >= 0.0)
		result = a1;
	else
		result = -a1;
	return result;
}
double sub_6581EA(float a1, float a2)
{
	double result; // st7@2

	if (a2 >= (double)a1)
		result = a1;
	else
		result = a2;
	return result;
}
void cCustomCloak::RenderCEffect(int ItemIndex, OBJECT* o, BMD* b) // OK
{

	CEFFECT_DATA* CEffectData = gCloak.GetInfoCEffectByItem(ItemIndex);
	vec3_t WorldPos;
	vec3_t Color;
	if (CEffectData)
	{

		switch (CEffectData->EffectIndex)
		{
		case 0:
			{
				float v1158;
				float v1159;
				float v1700 = Render(-4.0, 0.0) + 0.5;
				Vector(v1700 * 1.0, v1700 * 0.8999999761581421, v1700 * 0.3499999940395355, Color);
				int JointID1[] = { 75,79,80,82,87,88,89 };
				for (int l = 0; l < 7; ++l)
				{
					RenderBrightEffect(b, 32002, JointID1[l], 0.60000002, Color, o); //RenderBrightEffect
				}
				for (int m = 3; m < 18; ++m)
				{
					RenderBrightEffect(b, 32002, m, 0.44999999, Color, o);
				}
				for (int n = 18; n < 46; ++n)
				{
					if (n != 23 && n != 24 && n != 37 && n != 38)
					{
						RenderBrightEffect(b, 32002, n, 0.44999999, Color, o);
					}
				}
				Vector(v1700 * 0.949999988079071, v1700 * 0.1000000014901161, v1700 * 1.0, Color);
				int JointID2[] = { 77,78,90,91 };
				for (int ii = 0; ii < 4; ++ii)
				{
					RenderBrightEffect(b, 32002, JointID2[ii], 0.40000001, Color, o);
				}
				int JointID3[] = { 73,74,81,84 };
				for (int jj = 0; jj < 4; ++jj)
				{
					RenderBrightEffect(b, 32002, JointID3[jj], 0.40000001, Color, o);
				}
				RenderBrightEffect(b, 32002, 1, 1.2, Color, o);
				int JointID4[] = { 2,48,49,50 };
				for (int kk = 0; kk < 4; ++kk)
				{
					RenderBrightEffect(b, 32002, JointID4[kk], 0.40000001, Color, o);
				}
				int JointID5[] = { 76,83,85,86 };
				for (int ll = 0; ll < 4; ++ll)
				{
					Vector(v1700 * 0.2000000029802322, v1700 * 0.2000000029802322, v1700 * 1.0, Color);
					RenderBrightEffect(b, 32002, JointID5[ll], 0.2, Color, o);
					Vector(v1700 * 1.0, v1700 * 1.0, v1700 * 1.0, Color);
					RenderBrightEffect(b, 32229, JointID5[ll], 0.2, Color, o);
				}
				Vector(v1700 * 0.2000000029802322, v1700 * 0.2000000029802322, v1700 * 1.0, Color);
				int JointID6[] = { 90,78 };
				for (int mm = 0; mm < 2; ++mm)
				{
					RenderBrightEffect(b, 32002, JointID6[mm], 0.80000001, Color, o);
				}
				Vector(v1700 * 0.2000000029802322, v1700 * 0.699999988079071, v1700 * 1.0, Color);
				for (int nn = 54; nn < 70; ++nn)
				{
					RenderBrightEffect(b, 32002, nn, 0.40000001, Color, o);
				}
				float v311 = ((int)WorldTime % 15) / 37.5;
				v1700 = v311 + 0.5;
				Vector(v1700 * 0.0, v1700 * 0.1000000014901161, v1700 * 1.0, Color);
				RenderBrightEffect(b, 32002, 51, 4.0, Color, o);
				RenderBrightEffect(b, 32002, 51, 3.5999999, Color, o);
				Vector(1.0, 1.0, 1.0, Color);
				float v1711 = ((int)(WorldTime * 0.09000000357627869) % 90) * 0.01745329238474369;
				float v312 = v1711 + 1.570796370506287;
				float v313 = sub_4E7B95(v312);
				float v1698 = sub_639050(v313);
				float v314 = sub_4E7B95(v1711);
				float v1712 = sub_639050(v314) * 0.800000011920929;
				float v1701 = v1698 * 0.75;
				float v1702 = v1698 * 0.4000000059604645;
				float v1703 = v1698 * 1.0;
				Vector((1.399999976158142 - v1712) * *(float*)& v1701, (1.399999976158142 - v1712) * v1702, (1.399999976158142 - v1712) * v1703, Color);
				if (v1712 - 0.2000000029802322 <= 0.0)
					v1159 = 0.0;
				else
					v1159 = v1712 - 0.2000000029802322;
				float v315 = v1159;
				RenderBrightEffect(b, 32614, 51, v315, Color, o);
				v1711 = ((int)(WorldTime * 0.09000000357627869 + 30.0) % 90)
					* 0.01745329238474369;
				float v316 = sub_4E7B95(v1711);
				v1712 = sub_639050(v316) * 0.800000011920929;
				v1701 = v1698 * 0.75;
				v1702 = v1698 * 0.4000000059604645;
				v1703 = v1698 * 1.0;
				Vector((1.399999976158142 - v1712) * *(float*)& v1701, (1.399999976158142 - v1712) * v1702, (1.399999976158142 - v1712) * v1703, Color);
				if (v1712 - 0.2000000029802322 <= 0.0)
					v1158 = 0.0;
				else
					v1158 = v1712 - 0.2000000029802322;
				float v317 = v1158;
				RenderBrightEffect(b, 32614, 51, v317, Color, o);
				// ----
				// ----
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					o->HiddenMesh);
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					o->HiddenMesh);
				b->RenderMesh(
					2,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					o->HiddenMesh);
				b->RenderMesh(
					3,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					o->HiddenMesh);
				b->RenderMesh(
					3,
					576,
					o->Alpha,
					3,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					532771);
				int BlessedSword01 = 1000;
				float BlessedSword02 = (((int)((BlessedSword01 - WorldTime)) % 1001)) / (double)BlessedSword01;
				(b->BodyLight[0]) = 1.0;
				(b->BodyLight[1]) = 0.89999998;
				(b->BodyLight[2]) = 0.69999999;
				b->RenderMesh(4, 66, o->Alpha, 4, o->BlendMeshLight, BlessedSword02, o->BlendMeshTexCoordV, -1);
				(b->BodyLight[0]) = 0.0;
				(b->BodyLight[1]) = 0.1;
				(b->BodyLight[2]) = 1.0;
				b->RenderMesh(
					5,
					66,
					1.0,
					5,
					1.0,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
			}
			break;
		case 1:
		{


			float v234 = Render(-4.0, 0.0); //Render
			float v1875 = v234 * 0.5 + 0.5;
			Vector(v1875 * 0.4000000059604645, v1875 * 0.550000011920929, v1875 * 0.1500000059604645, Color);
			for (int i43 = 53; i43 < 129; ++i43)
			{
				RenderBrightEffect(b, 32002, i43, 0.64999998, Color, o); //RenderBrightEffect
			}
			RenderBrightEffect(b, 32229, 47, 0.30000001, Color, o);
			RenderBrightEffect(b, 32229, 23, 0.30000001, Color, o);
			Vector(v1875 * 0.8500000238418579, v1875 * 0.1000000014901161, v1875 * 0.8999999761581421, Color);
			RenderBrightEffect(b, 32229, 45, 0.80000001, Color, o);
			RenderBrightEffect(b, 32229, 46, 0.80000001, Color, o);
			RenderBrightEffect(b, 32229, 28, 0.80000001, Color, o);
			RenderBrightEffect(b, 32229, 30, 0.80000001, Color, o);
			Vector(v1875 * 0.1500000059604645, v1875 * 0.1500000059604645, v1875 * 0.8999999761581421, Color);
			int JointID1[] = { 27,32,29,43,34,40,37,35,25,41,42,44 };
			int JointID2[] = { 34,40,36,38 };
			for (int i45 = 0; i45 < 12; ++i45)
			{
				RenderBrightEffect(b, 32002, JointID1[i45], 0.60000002, Color, o);
			}
			for (int i46 = 0; i46 < 4; ++i46)
			{
				RenderBrightEffect(b, 32002, JointID2[i46], 0.30000001, Color, o);
			}
			if (Render(-4.0, 0.0) + 0.5 <= 1.0)
				float v1163 = 1.0;
			else
				float v1163 = Render(-4.0, 0.0) + 0.5;
			Vector(v1875 * 0.2000000029802322, v1875 * 0.300000011920929, v1875 * 1.0, Color);
			RenderBrightEffect(b, 32114, 1, 1.4, Color, o);

			//========EnD TEST
			// ----
			glColor3fv(b->BodyLight);
			// ----	
			b->RenderMesh(
				0,
				2,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				-1);
			float v550 = Render(-4.0, 0.0) * 0.4000000059604645 + 0.6000000238418579;
			b->RenderMesh(
				0,
				66,
				o->Alpha,
				0,
				v550,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				532768);
			b->RenderMesh(
				1,
				2,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				-1);
			b->RenderMesh(
				3,
				70,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				-1);
			(b->BodyLight[0]) = 0.4;
			(b->BodyLight[1]) = 0.4;
			(b->BodyLight[2]) = 1.0;
			b->RenderMesh(
				3,
				2,
				o->Alpha,
				3,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				-1);
			int BlessedSword012 = 1000;
			float BlessedSword022 = (((int)((BlessedSword012 - WorldTime)) % 1001)) / (double)BlessedSword012;
			(b->BodyLight[0]) = 0.69999999;
			(b->BodyLight[1]) = 0.89999998;
			(b->BodyLight[2]) = 1.0;
			b->RenderMesh(2, 66, o->Alpha, 2, o->BlendMeshLight, BlessedSword022, o->BlendMeshTexCoordV, -1);
		}
		break;
		case  2:
		{
			float v1161;
			float v269 = ((int)WorldTime % 15) / 37.5;
			float v1792 = v269 + 0.5;
			int JointID1[] = { 7,8,9,10,11,12,13 };
			int JointID2[] = { 18,19,20,36,43,42,35,29,59,48,49,50 };
			int JointID3[] = { 41,34,54,56,57,58,24,25,27,28,53,23 };
			Vector(v1792* 0.2000000029802322, v1792* 0.300000011920929, v1792* 1.0, Color);
			for (int i14 = 0; i14 < 7; ++i14)
			{
				RenderBrightEffect(b, 32002, JointID1[i14], 0.80000001, Color, o);
			}
			for (int i15 = 0; i15 < 12; ++i15)
			{
				RenderBrightEffect(b, 32002, JointID2[i15], 1.0, Color, o);
			}
			for (int i16 = 0; i16 < 12; ++i16)
			{
				RenderBrightEffect(b, 32002, JointID3[i16], 1.4000001, Color, o);
			}
			RenderBrightEffect(b, 32002, 62, 3.0, Color, o);
			int JointID4[] = { 51,52,21,22 };
			int JointID5[] = { 26,55 };
			Vector(v1792* 1.0, v1792* 0.6499999761581421, v1792* 0.2000000029802322, Color);
			for (int i17 = 0; i17 < 4; ++i17)
			{
				RenderBrightEffect(b, 32002, JointID4[i17], 1.2, Color, o);
			}
			for (int i18 = 0; i18 < 2; ++i18)
			{
				RenderBrightEffect(b, 32002, JointID5[i18], 1.8000001, Color, o);
			}
			int JointID6[] = { 63,64 };
			if (Render(-4.0, 0.0) * 0.5 + 0.5 >= 1.0)
				v1161 = 1.0;
			else
				v1161 = Render(-4.0, 0.0) * 0.5 + 0.5;
			float v1782 = v1161;
			Vector(v1782 * 1.0, v1782 * 1.0, v1782 * 1.0, Color);
			for (int i19 = 0; i19 < 2; ++i19)
			{
				RenderBrightEffect(b, 32002, JointID6[i19], 0.5, Color, o);
			}
			// ----
			glColor3fv(b->BodyLight);
			// ----	
			b->RenderMesh(
				0,
				2,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				o->HiddenMesh);
			b->RenderMesh(
				1,
				2,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				o->HiddenMesh);
			b->RenderMesh(
				2,
				2,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				o->HiddenMesh);
			b->RenderMesh(
				4,
				2,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				o->HiddenMesh);
			double blessedcb03 = (((int)((WorldTime) * 0.08f)) % 16 / 4) * 0.25;
			b->RenderMesh(5, 66, o->Alpha, 5, o->BlendMeshLight, blessedcb03, o->BlendMeshTexCoordV, -1);
			int blessedcb02 = 1000;
			double blessedcb01 = (((int)((blessedcb02 - WorldTime)) % 1001)) / (double)blessedcb02;
			(b->BodyLight[0]) = 1.0;
			(b->BodyLight[1]) = 0.89999998;
			(b->BodyLight[2]) = 0.5;
			b->RenderMesh(3, 66, o->Alpha, 3, o->BlendMeshLight, blessedcb01, o->BlendMeshTexCoordV, -1);
		}
		break;
		case  3:
		{
			float v1160;
			float v1742 = Render(-4.0, 0.0) + 0.5;
			Vector(v1742* 1.0, v1742* 0.8999999761581421, v1742* 0.3499999940395355, Color);
			for (int i35 = 26; i35 < 72; ++i35)
			{
				RenderBrightEffect(b, 32002, i35, 0.2, Color, o);
			}
			for (int i36 = 4; i36 < 15; ++i36)
			{
				RenderBrightEffect(b, 32002, i36, 0.5, Color, o);
			}
			Vector(v1742* 0.300000011920929, v1742* 0.1000000014901161, v1742* 1.0, Color);
			RenderBrightEffect(b, 32002, 16, 0.80000001, Color, o);
			RenderBrightEffect(b, 32002, 1.55, 0.2, Color, o);
			Vector(v1742* 0.2000000029802322, v1742* 0.2000000029802322, v1742* 1.0, Color);
			for (int i37 = 20; i37 < 24; ++i37)
			{
				float v1735 = 0.5;
				if (i37 == 22 || i37 == 23)
					v1735 = 1.0;
				float v284 = v1735 + v1735;
				RenderBrightEffect(b, 32002, i37, v284, Color, o);
			}
			float v285 = ((int)WorldTime % 15) / 37.5;
			v1742 = v285 + 0.5;
			Vector(v1742 * 0.0, v1742 * 0.1000000014901161, v1742 * 1.0, Color);
			RenderBrightEffect(b, 32118, 24, 1.0, Color, o);
			Vector(v1742 * 1.0, v1742 * 0.2000000029802322, v1742 * 0.699999988079071, Color);
			RenderBrightEffect(b, 32002, 24, 4.3000002, Color, o);
			Vector(v1742 * 0.4000000059604645, v1742 * 0.4000000059604645, v1742 * 1.0, Color);
			RenderBrightEffect(b, 32002, 24, 3.3, Color, o);
			// ----
			glColor3fv(b->BodyLight);
			// ----
			b->RenderMesh(
				0,
				2,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				-1);
			b->RenderMesh(
				1,
				66,
				o->Alpha,
				1,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				-1);
			(b->BodyLight[0]) = 0.5;
			(b->BodyLight[1]) = 0.5;
			(b->BodyLight[2]) = 1.0;
			b->RenderMesh(
				2,
				70,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				532770);
			int v552 = 1000;
			float v551 = (((int)((v552 - WorldTime)) % 1001)) / (double)v552;
			b->RenderMesh(3, 66, o->Alpha, 3, o->BlendMeshLight, v551, o->BlendMeshTexCoordV, -1);
		}
		break;
		case  4:
		{
			float v1162;
			float v244 = ((int)WorldTime % 15) / 37.5;
			float v1847 = v244 + 0.5;
			Vector(v1847* 0.2000000029802322, v1847* 0.6000000238418579, v1847* 1.0, Color);
			for (int i1 = 12; i1 < 19; ++i1)
			{
				RenderBrightEffect(b, 32002, i1, 1.6, Color, o);
			}
			RenderBrightEffect(b, 32002, 8, 0.60000002, Color, o);
			float v1846 = Render(-4.0, 0.0) + 0.2000000029802322;
			int JointID1[] = { 3,6,7,21,22,23 };
			Vector(v1846* 1.0, v1846* 1.0, v1846* 1.0, Color);
			for (int i2 = 0; i2 < 6; ++i2)
			{
				RenderBrightEffect(b, 32431, JointID1[i2], 0.80000001, Color, o);
			}
			int JointID2[] = { 1,2,4,5 };
			for (int i3 = 0; i3 < 4; ++i3)
			{
				RenderBrightEffect(b, 32231, JointID2[i3], 0.5, Color, o);
			}
			// ----
			glColor3fv(b->BodyLight);
			// ----
			b->RenderMesh(
				0,
				2,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				o->HiddenMesh);
			b->RenderMesh(
				1,
				2,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				o->HiddenMesh);
			b->RenderMesh(
				4,
				576,
				o->Alpha,
				4,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				532771);
			b->RenderMesh(
				5,
				2,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				o->HiddenMesh);
			(b->BodyLight[0]) = 1.0;
			(b->BodyLight[1]) = 0.89999998;
			(b->BodyLight[2]) = 0.5;
			int v552 = 1000;
			float v551 = (((int)((v552 - WorldTime)) % 1001)) / (double)v552;
			b->RenderMesh(2, 66, o->Alpha, 2, o->BlendMeshLight, v551, o->BlendMeshTexCoordV, -1);
			(b->BodyLight[0]) = 0.0;
			(b->BodyLight[1]) = 0.1;
			(b->BodyLight[2]) = 1.0;
			b->RenderMesh(
				3,
				66,
				1.0,
				3,
				1.0,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				-1);
		}
		break;
		case  5:
		{
			// ----
			glColor3fv(b->BodyLight);
			// ----
			b->RenderMesh(
				0,
				2,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				o->HiddenMesh);
			b->RenderMesh(
				1,
				66,
				o->Alpha,
				1,
				1.0,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				o->HiddenMesh);
			(b->BodyLight[0]) = 1.0;
			(b->BodyLight[1]) = 0.89999998;
			(b->BodyLight[2]) = 0.5;
			int v552 = 1000;
			float v551 = (((int)((v552 - WorldTime)) % 1001)) / (double)v552;
			b->RenderMesh(2, 66, o->Alpha, 2, 1.0, v551, o->BlendMeshTexCoordV, -1);
		}
		break;
		case  6:
		{
			// ----
			glColor3fv(b->BodyLight);
			// ----
			b->RenderMesh(
				0,
				2,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				-1);
			float v1107 = Render(-1.2, 0.0) + 0.300000011920929;
			//if ( v1107 > 1.0 )
			//float v1107 = 1.0;
			b->RenderMesh(
				1,
				66,
				o->Alpha,
				1,
				v1107,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				-1);
			float v43 = (double)(rand() % 15) / 37.5;
			float v1108 = v43 + 0.5;
			//int v1148 = 7;
			float v44 = v1108 * 0.4000000059604645;
			float v45 = v1108 * 0.6000000238418579;
			float v46 = v1108 * 0.800000011920929;
			(b->BodyLight[0]) = v46;
			(b->BodyLight[1]) = v45;
			(b->BodyLight[2]) = v44;
			b->RenderMesh(
				2,
				66,
				o->Alpha,
				2,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				-1);
			float v1109 = 1.0 - Render(4.0, 0.0);
			(b->BodyLight[0]) = 1.00;
			(b->BodyLight[1]) = 1.00;
			(b->BodyLight[2]) = 1.00;
			b->RenderMesh(
				3,
				2,
				o->Alpha,
				3,
				1.0,
				v1109,
				o->BlendMeshTexCoordV,
				-1);
		}
		break;
		case  7:
		{
			// ----
			glColor3fv(b->BodyLight);
			// ----
			b->RenderMesh(
				0,
				2,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				-1);
			b->RenderMesh(
				1,
				66,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				-1);
			b->RenderMesh(
				2,
				66,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				-1);
			float RenderBMace = 0.6499999761581421 - Render(4.0, 0.0);
			(b->BodyLight[0]) = 0.69999999;
			(b->BodyLight[1]) = 0.89999998;
			(b->BodyLight[2]) = 1.0;
			b->RenderMesh(
				3,
				66,
				o->Alpha,
				3,
				o->BlendMeshLight,
				RenderBMace,
				o->BlendMeshTexCoordV,
				-1);
			float BMace1 = (double)(rand() % 20) / 32.5;
			float BMace2 = BMace1 + 0.4000000059604645;
			(b->BodyLight[0]) = 0.2;
			(b->BodyLight[1]) = 0.5;
			(b->BodyLight[2]) = 1.0;
			b->RenderMesh(
				4,
				66,
				o->Alpha,
				4,
				BMace2,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				-1);
		}
		break;
		case  8:
		{
			// ----
			glColor3fv(b->BodyLight);
			// ----
			b->RenderMesh(
				0,
				2,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				-1);
			b->RenderMesh(
				2,
				2,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				-1);
			float v717 = (Render(-4.0, 0.0) * 0.800000011920929 + 0.2000000029802322);
			b->RenderMesh(1, 66, o->Alpha, 1, v717, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1);
			float v716 = ((int)WorldTime % 2000) * 0.0005000000237487257;
			b->RenderMesh(1, 66, o->Alpha, 1, v717, v716, v716, 32771);
			float v717V2 = Render(-8.0, 0.0);
			b->RenderMesh(3, 66, o->Alpha, 3, v717V2, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1);
			float v719 = (int)WorldTime % 15 / 5;
			b->RenderMesh(
				4,
				66,
				o->Alpha,
				4,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				v719 * 32389);
			float v478 = ((int)WorldTime % 20) / 32.5;
			float v720 = v478 + 0.4000000059604645;
			(b->BodyLight[0]) = v720 * 1.0;
			(b->BodyLight[1]) = v720 * 0.80000001;
			(b->BodyLight[2]) = v720 * 0.40000001;
			b->RenderMesh(
				6,
				66,
				o->Alpha,
				6,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				-1);
			(b->BodyLight[0]) = 1.0;
			(b->BodyLight[1]) = 0.60000002;
			(b->BodyLight[2]) = 0.050000001;
			float v718 = ((int)WorldTime % 800) * 0.001249999972060323;
			float v482 = -v718;
			b->RenderMesh(
				5,
				66,
				o->Alpha,
				5,
				o->BlendMeshLight,
				v482,
				o->BlendMeshTexCoordV,
				532496);
		}
		break;
		case  9:
		{
			// ----
			glColor3fv(b->BodyLight);
			// ----
			float magicianwing4da_render = Render(-2.0, 0.0) * 0.699999988079071 + 0.2000000029802322;
			// ----
			b->RenderMesh(
				0,
				2,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				-1);
			b->RenderMesh(
				1,
				2,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				-1);
			b->RenderMesh(
				1,
				2,
				o->Alpha,
				1,
				magicianwing4da_render,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				532831);
			(b->BodyLight[0]) = 0.70980392156;
			(b->BodyLight[1]) = 0.51372549019;
			(b->BodyLight[2]) = 0.27058823529;
			b->RenderMesh(
				0,
				70,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				-1);
			(b->BodyLight[0]) = 0.70980392156;
			(b->BodyLight[1]) = 0.51372549019;
			(b->BodyLight[2]) = 0.27058823529;
			b->RenderMesh(
				0,
				70,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				-1);
		}
		break;
		case 10:
		{
			// ----
			glColor3fv(b->BodyLight);
			// ----
			float dknightwing4db_render = Render(-1.75, 0.0);
			// ----
			b->RenderMesh(
				0,
				2,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				-1);
			b->RenderMesh(
				1,
				2,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				-1);
			b->RenderMesh(
				2,
				2,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				-1);
			b->RenderMesh(
				3,
				2,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				-1);
			int Avatar1 = (((int)((WorldTime) * 0.75f)) % 600 / 40);
			double Avatar2 = (double)(Avatar1 % 4) * 0.25;
			double Avatar3 = (double)(Avatar1 / 4) * 0.25;
			b->RenderMesh(4, 66, o->Alpha, 4, o->BlendMeshLight, Avatar2, Avatar3, o->HiddenMesh);
			b->RenderMesh(
				2,
				2,
				o->Alpha,
				2,
				dknightwing4db_render,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				532833);
			(b->BodyLight[0]) = 0.83137254902;
			(b->BodyLight[1]) = 0.78431372549;
			(b->BodyLight[2]) = 0.66666666666;
			b->RenderMesh(
				0,
				70,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				-1);
			(b->BodyLight[0]) = 0.83137254902;
			(b->BodyLight[1]) = 0.78431372549;
			(b->BodyLight[2]) = 0.66666666666;
			b->RenderMesh(
				0,
				70,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				-1);
		}
		break;
			case 11:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				float elfwing4da = Render(-4.0, 0.0) * 0.5 + 0.5;
				float elfwing4da_render01 = Render(-4.0, 120.0);
				float elfwing4da_render02 = Render(-4.0, 240.0);
				// ----
				b->RenderMesh(
					2,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					1,
					66,
					o->Alpha,
					1,
					elfwing4da,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					1,
					66,
					o->Alpha,
					1,
					elfwing4da_render01,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					532834);
				b->RenderMesh(
					1,
					66,
					o->Alpha,
					1,
					elfwing4da_render02,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					532835);
				b->RenderMesh(
					0,
					66,
					o->Alpha,
					0,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					3,
					66,
					o->Alpha,
					3,
					elfwing4da,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				(b->BodyLight[0]) = 1.00;
				(b->BodyLight[1]) = 0.65098039215;
				(b->BodyLight[2]) = 0.4862745098;
				b->RenderMesh(
					2,
					70,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				(b->BodyLight[0]) = 1.00;
				(b->BodyLight[1]) = 0.65098039215;
				(b->BodyLight[2]) = 0.4862745098;
				b->RenderMesh(
					2,
					70,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
			}
			break;
			case  12:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					2,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					3,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					4,
					66,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				int fall1 = (int)WorldTime % 600 / 40;
				double fall2 = (double)(fall1 % 4) * 0.25;
				double fall3 = (double)(fall1 / 4) * 0.25;
				(b->BodyLight[0]) = 0.50;
				(b->BodyLight[1]) = 0.50;
				(b->BodyLight[2]) = 0.50;
				b->RenderMesh(0, 66, 1.0, 0, o->BlendMeshLight, fall2, fall3, o->HiddenMesh);
				(b->BodyLight[0]) = 0.8;
				(b->BodyLight[1]) = 0.74509803921;
				(b->BodyLight[2]) = 0.59215686274;
				b->RenderMesh(
					1,
					70,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					2,
					70,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					3,
					70,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
			}
			break;
			case  13:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					2,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					3,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					4,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					5,
					66,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				(b->BodyLight[0]) = 1.00;
				(b->BodyLight[1]) = 1.00;
				(b->BodyLight[2]) = 1.00;
				b->RenderMesh(
					3,
					70,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				(b->BodyLight[0]) = 0.23921568627;
				(b->BodyLight[1]) = 0.43921568627;
				(b->BodyLight[2]) = 1.00;
				b->RenderMesh(
					4,
					70,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				(b->BodyLight[0]) = 0.23921568627;
				(b->BodyLight[1]) = 0.43921568627;
				(b->BodyLight[2]) = 1.00;
				b->RenderMesh(
					4,
					70,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
			}
			break;
			case  14:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				float alicewing4db_render = Render(-4.0, 0.0) * 0.1000000014901161 + 0.3;
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					1,
					66,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					2,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					2,
					2,
					o->Alpha,
					2,
					alicewing4db_render,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					532836);
				(b->BodyLight[0]) = 1.00;
				(b->BodyLight[1]) = 0.07843137254;
				(b->BodyLight[2]) = 0.07843137254;
				b->RenderMesh(
					0,
					70,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				(b->BodyLight[0]) = 1.00;
				(b->BodyLight[1]) = 0.07843137254;
				(b->BodyLight[2]) = 0.07843137254;
				b->RenderMesh(
					0,
					70,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
			}
			break;
			case  15:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					2,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					3,
					66,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				(b->BodyLight[0]) = 0.70980392156;
				(b->BodyLight[1]) = 0.53725490196;
				(b->BodyLight[2]) = 0.54509803921;
				b->RenderMesh(
					1,
					70,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				(b->BodyLight[0]) = 0.70980392156;
				(b->BodyLight[1]) = 0.53725490196;
				(b->BodyLight[2]) = 0.54509803921;
				b->RenderMesh(
					1,
					70,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
			}
			break;
			case  16:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					o->HiddenMesh);
				b->RenderMesh(
					1,
					66,
					o->Alpha,
					1,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					o->HiddenMesh);
				b->RenderMesh(
					2,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					o->HiddenMesh);
				float v401 = ((int)WorldTime % 2000) * 0.0005000000237487257;
				float v403 = -v401;
				b->RenderMesh(3, 66, o->Alpha, 3, o->BlendMeshLight, o->BlendMeshTexCoordU, v403, 532817);
			}
			break;
			case  17:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				float v641 = Render(-4.0, 0.0) + 0.300000011920929;
				b->RenderMesh(1, 66, o->Alpha, 1, v641, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1);
				float v401 = ((int)WorldTime % 2000) * 0.0005000000237487257;
				b->RenderMesh(2, 66, o->Alpha, 2, o->BlendMeshLight, o->BlendMeshTexCoordU, v401, 532817);
			}
			break;
			case  18:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					o->HiddenMesh);
				b->RenderMesh(
					2,
					66,
					o->Alpha,
					2,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					o->HiddenMesh);
				(b->BodyLight[0]) = 0.1;
				(b->BodyLight[1]) = 0.2;
				(b->BodyLight[2]) = 1.0;
				float v401 = ((int)WorldTime % 2000) * 0.0005000000237487257;
				b->RenderMesh(1, 66, 1.0, 1, o->BlendMeshLight, o->BlendMeshTexCoordU, v401, 532817);
				float HolyangelSc01 = (double)(rand() % 15) / 37.5;
				float HolyangelSc02 = HolyangelSc01 + 0.5;
				(b->BodyLight[0]) = 0.25;
				(b->BodyLight[1]) = 0.69999999;
				(b->BodyLight[2]) = 1.0;
				b->RenderMesh(
					3,
					66,
					o->Alpha,
					3,
					HolyangelSc02,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
			}
			break;
			case  19:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					o->HiddenMesh);
				b->RenderMesh(
					2,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					o->HiddenMesh);
				b->RenderMesh(
					3,
					66,
					o->Alpha,
					3,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					o->HiddenMesh);
				int v655 = (int)WorldTime % 600 / 40;
				float v657 = (double)(v655 % 4) * 0.25;
				float v659 = (double)(v655 / 4) * 0.25;
				(b->BodyLight[0]) = 0.2;
				(b->BodyLight[1]) = 0.44999999;
				(b->BodyLight[2]) = 1.0;
				b->RenderMesh(1, 66, o->Alpha, 1, o->BlendMeshLight, v657, v659, o->HiddenMesh);
				float v645 = Render(-6.0, 0.0) + 0.300000011920929;
				b->RenderMesh(4, 66, o->Alpha, 4, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1);
				b->RenderMesh(4, 66, o->Alpha, 4, v645, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1);
			}
			break;
			case  20:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					o->HiddenMesh);
				b->RenderMesh(
					2,
					66,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					o->HiddenMesh);
				int v655 = (int)WorldTime % 600 / 40;
				float v657 = (double)(v655 % 4) * 0.25;
				float v659 = (double)(v655 / 4) * 0.25;
				(b->BodyLight[0]) = 0.30000001;
				(b->BodyLight[1]) = 0.60000002;
				(b->BodyLight[2]) = 1.0;
				b->RenderMesh(1, 66, o->Alpha, 1, o->BlendMeshLight, v657, v659, o->HiddenMesh);
			}
			break;
			case  21:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					o->HiddenMesh);
				b->RenderMesh(
					0,
					66,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					o->HiddenMesh);
				float v638 = Render(-4.0, 0.0) + 0.300000011920929;
				(b->BodyLight[0]) = 0.15000001;
				(b->BodyLight[1]) = 0.15000001;
				(b->BodyLight[2]) = 1.00;
				b->RenderMesh(
					0,
					66,
					o->Alpha,
					0,
					v638,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					o->HiddenMesh);
				float v552 = 1000;
				float v551 = (((int)((v552 - WorldTime)) % 1001)) / (double)v552;
				(b->BodyLight[0]) = 0.60000002;
				(b->BodyLight[1]) = 0.60000002;
				(b->BodyLight[2]) = 1.00;
				b->RenderMesh(0, 66, o->Alpha, 0, 1.0, v551, o->BlendMeshTexCoordV, 532771);
				(b->BodyLight[0]) = 0.5;
				(b->BodyLight[1]) = 0.5;
				(b->BodyLight[2]) = 1.00;
				b->RenderMesh(2, 66, o->Alpha, 2, 1.0, o->BlendMeshTexCoordU, v551, 532817);
			}
			break;
			case  22:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					2,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				float v525 = Render(-4.0, 0.0);
				b->RenderMesh(
					0,
					2,
					1.0,
					0,
					v525,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					532840);
				b->RenderMesh(
					1,
					2,
					1.0,
					1,
					v525,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					32841);
				(b->BodyLight[0]) = 1.00;
				(b->BodyLight[1]) = 0.89999998;
				(b->BodyLight[2]) = 0.40000001;
				b->RenderMesh(
					3,
					66,
					1.0,
					3,
					1.0,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
			}
			break;
			case  23:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					1,
					66,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				float v525 = Render(-4.0, 0.0);
				float MagicSword = ((int)WorldTime % 2000) * 0.0005000000237487257;
				b->RenderMesh(
					0,
					2,
					1.0,
					0,
					v525,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					532842);
				(b->BodyLight[0]) = 0.50000001;
				(b->BodyLight[1]) = 0.50000001;
				(b->BodyLight[2]) = 0.50000001;
				b->RenderMesh(1, 66, o->Alpha, 1, o->BlendMeshLight, o->BlendMeshTexCoordU, MagicSword, 532847);
			}
			break;
			case  24:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					1,
					66,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					3,
					66,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					4,
					66,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					5,
					66,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				float v525 = Render(-4.0, 0.0);
				b->RenderMesh(
					0,
					2,
					1.0,
					0,
					v525,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					532837);
				(b->BodyLight[0]) = 0.50000001;
				(b->BodyLight[1]) = 0.50000001;
				(b->BodyLight[2]) = 0.50000001;
				float v401 = ((int)WorldTime % 2000) * 0.0005000000237487257;
				int v655 = (int)WorldTime % 600 / 40;
				float v657 = (double)(v655 % 4) * 0.25;
				float v659 = (double)(v655 / 4) * 0.25;
				b->RenderMesh(1, 66, o->Alpha, 1, o->BlendMeshLight, o->BlendMeshTexCoordU, v401, 532847);
				b->RenderMesh(2, 66, o->Alpha, 2, o->BlendMeshLight, v657, v659, o->HiddenMesh);
			}
			break;
			case  25:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					2,
					66,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				float v525 = Render(-4.0, 0.0);
				b->RenderMesh(
					1,
					2,
					1.0,
					1,
					v525,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					532838);
				int v655 = (int)WorldTime % 600 / 40;
				float v657 = (double)(v655 % 4) * 0.25;
				float v659 = (double)(v655 / 4) * 0.25;
				(b->BodyLight[0]) = 1.00;
				(b->BodyLight[1]) = 1.00;
				(b->BodyLight[2]) = 1.00;
				b->RenderMesh(3, 66, o->Alpha, 3, o->BlendMeshLight, v657, v659, o->HiddenMesh);
			}
			break;
			case  26:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					1,
					66,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				float v525 = Render(-4.0, 0.0);
				b->RenderMesh(
					0,
					2,
					1.0,
					0,
					v525,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					532843);
				(b->BodyLight[0]) = 0.50000001;
				(b->BodyLight[1]) = 0.50000001;
				(b->BodyLight[2]) = 0.50000001;
				float v401 = ((int)WorldTime % 2000) * 0.0005000000237487257;
				b->RenderMesh(1, 66, o->Alpha, 1, o->BlendMeshLight, o->BlendMeshTexCoordU, v401, 532847);
			}
			break;
			case  27:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				float v525 = Render(-4.0, 0.0);
				b->RenderMesh(
					0,
					2,
					1.0,
					0,
					v525,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					532846);
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				(b->BodyLight[0]) = 1.00;
				(b->BodyLight[1]) = 1.00;
				(b->BodyLight[2]) = 1.00;
				int v655 = (int)WorldTime % 600 / 40;
				float v657 = (double)(v655 % 4) * 0.25;
				float v659 = (double)(v655 / 4) * 0.25;
				b->RenderMesh(2, 2, o->Alpha, 2, 1.0, v657, v659, o->HiddenMesh);
			}
			break;
			case  28:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					2,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				int v806 = (int)WorldTime % 1125 / 75;
				float v809 = (double)(v806 % 4) * 0.25;
				float v811 = (double)(v806 / 4) * 0.25;
				(b->BodyLight[0]) = 0.55000001;
				(b->BodyLight[1]) = 0.69999999;
				(b->BodyLight[2]) = 1.0;
				b->RenderMesh(
					3,
					66,
					o->Alpha,
					3,
					v809,
					v811,
					o->BlendMeshTexCoordV,
					-1);
				float v808 = Render(-6.0, 0.0) * 0.800000011920929
					+ 0.2000000029802322;
				if (v808 > 0.800000011920929)
					v808 = 0.80000001;
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					1,
					v808,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					532570);
				float v639 = ((int)WorldTime % 2000)
					* 0.0005000000237487257;
				float v640 = ((int)WorldTime % 2000)
					* 0.0005000000237487257;
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					1,
					o->BlendMeshLight,
					v640,
					v639,
					533001);
				b->RenderMesh(
					2,
					66,
					o->Alpha,
					2,
					v808,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					532571);
			}
			break;
			case  29:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					2,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				float v832 = Render(-6.0, 0.0) * 0.8 + 0.2000000029802322;
				if (v832 > 1.0)
					v832 = 1.0;
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					1,
					v832,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					532574);
				v832 = Render(-6.0, 0.0) * 0.9 + 0.2000000029802322;
				if (v832 > 0.800000011920929)
					v832 = 0.80000001;
				b->RenderMesh(
					2,
					66,
					o->Alpha,
					2,
					v832,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					532573);
				float v670 = o->BlendMeshTexCoordV;
				float v671 = ((int)WorldTime % 10000)
					* 0.0003000000142492354;
				b->RenderMesh(
					2,
					2,
					o->Alpha,
					2,
					0.69999999,
					v671,
					v670,
					533001);
				int v834 = 1200;
				float v833 = (((int)((v834 - WorldTime)) % 1201))
					/ (double)v834;
				b->RenderMesh(
					3,
					66,
					o->Alpha,
					3,
					0.30000001,
					v833,
					o->BlendMeshTexCoordV,
					532575);
			}
			break;
			case  30:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				float v631 = o->BlendMeshTexCoordV;
				float v632 = o->BlendMeshTexCoordU;
				float v633 = Render(-4.0, 0.0);
				b->RenderMesh(
					1,
					66,
					o->Alpha,
					1,
					v633,
					v632,
					v631,
					532564);
				float v634 = ((int)WorldTime % 2000)
					* 0.0005000000237487257;
				float v635 = o->BlendMeshTexCoordU;
				float v636 = Render(-4.0, 0.0) * 0.699999988079071
					+ 0.1500000059604645;
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					1,
					v636,
					v635,
					v634,
					533837);
			}
			break;
			case  31:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				int v854 = (int)WorldTime % 900 / 60;
				float v855 = (double)(v854 % 4) * 0.25;
				float v857 = (double)(v854 / 4) * 0.25;
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				float v643 = o->BlendMeshTexCoordV;
				float v644 = o->BlendMeshTexCoordU;
				float v645 = Render(-4.0, 0.0);
				b->RenderMesh(
					1,
					66,
					o->Alpha,
					1,
					v645,
					v644,
					v643,
					532562);
				float v646 = ((int)WorldTime % 2000)
					* 0.0005000000237487257;
				float v647 = o->BlendMeshTexCoordU;
				float v648 = Render(-4.0, 0.0) * 0.8500000238418579 + 0.15;
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					1,
					v648,
					v647,
					v646,
					533837);
				(b->BodyLight[0]) = 0.1;
				(b->BodyLight[1]) = 0.64999998;
				(b->BodyLight[2]) = 0.5;
				int v649 = o->HiddenMesh;
				float v650 = v857;
				float v651 = v855;
				float v652 = Render(-4.0, 0.0) * 0.699999988079071
					+ 0.300000011920929;
				b->RenderMesh(
					2,
					2,
					o->Alpha,
					2,
					v652,
					v651,
					v650,
					v649);
				b->RenderMesh(
					3,
					2,
					o->Alpha,
					3,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					4,
					2,
					o->Alpha,
					4,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					5,
					2,
					o->Alpha,
					5,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
			}
			break;
			case  32:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				int v849 = (int)WorldTime % 750 / 50;
				float v850 = (double)(v849 % 4) * 0.25;
				float v852 = (double)(v849 / 4) * 0.25;
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				float v653 = o->BlendMeshTexCoordV;
				float v654 = o->BlendMeshTexCoordU;
				float v655 = Render(-4.0, 0.0);
				b->RenderMesh(
					1,
					66,
					o->Alpha,
					1,
					v655,
					v654,
					v653,
					532565);
				float v656 = ((int)WorldTime % 2000)
					* 0.0005000000237487257;
				float v657 = o->BlendMeshTexCoordU;
				float v658 = Render(-4.0, 0.0) * 0.8500000238418579
					+ 0.1500000059604645;
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					1,
					v658,
					v657,
					v656,
					533837);
				(b->BodyLight[0]) = 0.1;
				(b->BodyLight[1]) = 0.30000001;
				(b->BodyLight[2]) = 1.0;
				b->RenderMesh(
					2,
					66,
					o->Alpha,
					2,
					o->BlendMeshLight,
					v850,
					v852,
					-1);
				b->RenderMesh(
					2,
					66,
					o->Alpha,
					2,
					o->BlendMeshLight,
					v850,
					v852,
					-1);
				int v851 = 15;
				signed int v659 = WorldTime;
				v849 = v659 % (36 * v851) / v851;
				float v660 = (double)v849 * 0.02999999932944775;
				v852 = sub_6581EA(1.0, v660);
				(b->BodyLight[0]) = 1.0;
				(b->BodyLight[1]) = 0.60000002;
				(b->BodyLight[2]) = 0.2;
				float v661 = 1.0 - v852;
				b->RenderMesh(
					3,
					66,
					o->Alpha,
					3,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					v661,
					-1);
			}
			break;
			case  33:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				int v859 = (int)WorldTime % 900 / 60;
				float v860 = (double)(v859 % 4) * 0.25;
				float v862 = (double)(v859 / 4) * 0.25;
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					2,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				float v637 = o->BlendMeshTexCoordV;
				float v638 = o->BlendMeshTexCoordU;
				float v639 = Render(-4.0, 0.0);
				b->RenderMesh(
					2,
					66,
					o->Alpha,
					2,
					v639,
					v638,
					v637,
					532563);
				float v640 = ((int)WorldTime % 2000)
					* 0.0005000000237487257;
				float v641 = o->BlendMeshTexCoordU;
				float v642 = Render(-4.0, 0.0) * 0.8500000238418579
					+ 0.1500000059604645;
				b->RenderMesh(
					2,
					2,
					o->Alpha,
					2,
					v642,
					v641,
					v640,
					533837);
				(b->BodyLight[0]) = 0.30000001;
				(b->BodyLight[1]) = 1.0;
				(b->BodyLight[2]) = 0.60000002;
				b->RenderMesh(
					1,
					66,
					o->Alpha,
					1,
					o->BlendMeshLight,
					v860,
					v862,
					-1);
			}
			break;
			case  34:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				float v827 = Render(-8.0, 0.0);
				if (v827 > 1.0)
					v827 = 1.0;
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					0,
					v827,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					532567);
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					1,
					v827,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					532567);
				v827 = Render(-4.0, 0.0) * 0.7 + 0.300000011920929;
				if (v827 > 1.0)
					v827 = 1.0;
				int v826 = (int)WorldTime % 750 / 50;
				float v829 = (double)(v826 % 4) * 0.25;
				float v831 = (double)(v826 / 4) * 0.25;
				(b->BodyLight[0]) = 0.15000001;
				(b->BodyLight[1]) = 0.30000001;
				(b->BodyLight[2]) = 1.0;
				b->RenderMesh(
					2,
					66,
					o->Alpha,
					2,
					v827,
					v829,
					v831,
					532566);
			}
			break;
			case  35:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				double DarkAngelBow = ((int)WorldTime % 2000) * 0.0005000000237487257;
				b->RenderMesh(
					0,
					66,
					o->Alpha,
					0,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					DarkAngelBow,
					532771);
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
			}
			break;
			case  36:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					2,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					3,
					66,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					3,
					576,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					532771);
				float v803 = ((int)(WorldTime * (float)0.1) % 200) * 0.009999999776482582 - 1.0;
				b->RenderMesh(
					4,
					66,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					v803,
					-1);
			}
			break;
			case  37:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				float v808 = Render(-4.0, 0.0);
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					0,
					66,
					o->Alpha,
					0,
					v808,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					532960);
				float v398 = ((int)WorldTime % 2000) * 0.0005000000237487257;
				b->RenderMesh(
					2,
					66,
					o->Alpha,
					2,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					v398,
					532771);
				int v807 = (int)WorldTime % 1200 / 80;
				float v809 = (double)(v807 % 4) * 0.25;
				(b->BodyLight[0]) = 1.0;
				(b->BodyLight[1]) = 0.60000002;
				(b->BodyLight[2]) = 0.34;
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					1,
					o->BlendMeshLight,
					v809,
					o->BlendMeshTexCoordV,
					-1);
			}
			break;
			case  38:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					1.0,
					-1,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				float v525 = Render(-4.0, 0.0);
				b->RenderMesh(
					0,
					2,
					1.0,
					0,
					v525,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					532798);
				float v486 = (b->BodyLight[0]) * 0.300000011920929;
				(b->BodyLight[0]) = v486;
				float v487 = (b->BodyLight[1]) * 0.300000011920929;
				(b->BodyLight[1]) = v487;
				float v488 = (b->BodyLight[2]) * 0.300000011920929;
				(b->BodyLight[2]) = v488;
				b->RenderMesh(
					1,
					66,
					1.0,
					1,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					1,
					576,
					1.0,
					1,
					1.0,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					532771);
			}
			break;
			case  39:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					o->HiddenMesh);
				(b->BodyLight[0]) = 0.5;
				(b->BodyLight[1]) = 0.5;
				(b->BodyLight[2]) = 0.5;
				float v401 = ((int)WorldTime % 2000) * 0.0005000000237487257;
				b->RenderMesh(0, 66, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, v401, 532771);
				(b->BodyLight[0]) = 1.0;
				(b->BodyLight[1]) = 0.69999999;
				(b->BodyLight[2]) = 1.0;
				int v806 = 1000;
				float v551 = (double)(v806 - (int)WorldTime % 1001) / (double)v806;
				b->RenderMesh(2, 66, o->Alpha, 2, o->BlendMeshLight, v551, o->BlendMeshTexCoordV, -1);
				b->RenderMesh(2, 66, o->Alpha, 2, o->BlendMeshLight, v551, o->BlendMeshTexCoordV, -1);
			}
			break;
			case  40:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					1,
					66,
					o->Alpha,
					1,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				float v1011 = ((int)WorldTime % 2000) * 0.0005000000237487257;
				(b->BodyLight[0]) = 1.0;
				(b->BodyLight[1]) = 0.30000001;
				(b->BodyLight[2]) = 1.0;
				b->RenderMesh(
					2,
					66,
					o->Alpha,
					2,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					v1011,
					-1);
			}
			break;
			case  41:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					1.0,
					-1,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					1,
					66,
					1.0,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					1,
					576,
					1.0,
					1,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					532771);
				(b->BodyLight[0]) = 1.0;
				(b->BodyLight[1]) = 0.30000001;
				(b->BodyLight[2]) = 1.0;
				b->RenderMesh(
					2,
					66,
					1.0,
					2,
					1.0,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					2,
					66,
					1.0,
					2,
					1.0,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
			}
			break;
			case  42:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				(b->BodyLight[0]) = 0.5;
				(b->BodyLight[1]) = 0.5;
				(b->BodyLight[2]) = 0.5;
				b->RenderMesh(
					2,
					66,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					2,
					68,
					o->Alpha,
					2,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				float v507 = ((int)WorldTime % 2000) * 0.0005000000237487257;
				b->RenderMesh(
					1,
					66,
					o->Alpha,
					1,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					v507,
					-1);
				(b->BodyLight[0]) = 0.0;
				(b->BodyLight[1]) = 0.5;
				(b->BodyLight[2]) = 1.0;
				b->RenderMesh(
					3,
					66,
					o->Alpha,
					3,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				float v598 = Render(-4.0, 0.0) * 0.699999988079071 + 0.300000011920929;
				b->RenderMesh(
					4,
					66,
					o->Alpha,
					4,
					v598,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
			}
			break;
			case  43:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					2,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					2,
					66,
					o->Alpha,
					2,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				(b->BodyLight[0]) = 0.5;
				(b->BodyLight[1]) = 0.80000001;
				(b->BodyLight[2]) = 1.0;
				float v777 = Render(-4.0, 0.0) + 0.300000011920929;
				b->RenderMesh(
					3,
					66,
					o->Alpha,
					3,
					v777,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
			}
			break;
			case  44:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					1,
					66,
					o->Alpha,
					1,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				float v401 = ((int)WorldTime % 2000) * 0.0005000000237487257;
				float v403 = -v401;
				b->RenderMesh(2, 66, o->Alpha, 2, o->BlendMeshLight, o->BlendMeshTexCoordU, v403, 532817);
				(b->BodyLight[0]) = 0.2;
				(b->BodyLight[1]) = 0.60000002;
				(b->BodyLight[2]) = 1.0;
				b->RenderMesh(
					3,
					66,
					o->Alpha,
					3,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
			}
			break;
			case  45:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				float v503 = (double)(rand() % 20) / 52.5;
				float v601 = v503 + 0.4000000059604645;
				int v1148 = 47;
				float v44 = v601 * 0.4000000059604645;
				float v45 = v601 * 0.6000000238418579;
				float v46 = v601 * 0.800000011920929;
				(b->BodyLight[0]) = v46;
				(b->BodyLight[1]) = v45;
				(b->BodyLight[2]) = v44;
				b->RenderMesh(
					2,
					66,
					o->Alpha,
					2,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				(b->BodyLight[0]) = 1.0;
				(b->BodyLight[1]) = 0.80000001;
				(b->BodyLight[2]) = 0.80000001;
				(b->BodyLight[0]) = v46;
				(b->BodyLight[1]) = v45;
				(b->BodyLight[2]) = v44;
				b->RenderMesh(
					1,
					66,
					o->Alpha,
					1,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				float v600 = Render(-5.0, 0.0) * 0.8 + 0.2000000029802322;
				if (v600 > 1.0)
					float v600 = 1.0;
				(b->BodyLight[0]) = 1.00;
				(b->BodyLight[1]) = 1.00;
				(b->BodyLight[2]) = 1.00;
				b->RenderMesh(
					0,
					66,
					o->Alpha,
					0,
					v600,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					532542);
				b->RenderMesh(
					0,
					66,
					o->Alpha,
					0,
					v600,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					532542);
			}
			break;
			case  46:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				float v595 = Render(-4.0, 0.0);
				b->RenderMesh(
					1,
					66,
					o->Alpha,
					1,
					v595,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				float v508 = (double)(rand() % 20) / 52.5;
				float v597 = v508 + 0.4000000059604645;
				float v596 = ((int)WorldTime % 4000) * 0.0002500000118743628;
				b->RenderMesh(
					2,
					66,
					o->Alpha,
					2,
					v597,
					v596,
					o->BlendMeshTexCoordV,
					-1);
			}
			break;
			case  47:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					1,
					66,
					o->Alpha,
					1,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				int v655 = (int)WorldTime % 600 / 40;
				float v657 = (double)(v655 % 4) * 0.25;
				float v659 = (double)(v655 / 4) * 0.25;
				b->RenderMesh(2, 66, o->Alpha, 2, o->BlendMeshLight, v657, v659, o->HiddenMesh);
				(b->BodyLight[0]) = 0.3;
				(b->BodyLight[1]) = 0.3;
				(b->BodyLight[2]) = 0.3;
				float v1010 = ((int)WorldTime % 2000)
					* 0.0005000000237487257;
				b->RenderMesh(
					1,
					66,
					o->Alpha,
					1,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					v1010,
					532847);
			}
			break;
			case  48:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					2,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				float v1008 = (Render(-4.0, 0.0) * 0.699999988079071
					+ 0.300000011920929)
					* o->Alpha;
				b->RenderMesh(
					3,
					66,
					o->Alpha,
					3,
					v1008,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				(b->BodyLight[0]) = 0.3;
				(b->BodyLight[1]) = 0.3;
				(b->BodyLight[2]) = 0.3;
				b->RenderMesh(
					4,
					66,
					o->Alpha,
					4,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				float v1009 = ((int)WorldTime % 2000)
					* 0.0005000000237487257;
				float v728 = -v1009;
				b->RenderMesh(
					4,
					66,
					o->Alpha,
					4,
					o->BlendMeshLight,
					v728,
					o->BlendMeshTexCoordV,
					532771);
				(b->BodyLight[0]) = 0.55000001;
				(b->BodyLight[1]) = 0.69999999;
				(b->BodyLight[2]) = 1.0;
				int v655 = (int)WorldTime % 600 / 40;
				float v657 = (double)(v655 % 4) * 0.25;
				float v659 = (double)(v655 / 4) * 0.25;
				b->RenderMesh(1, 66, o->Alpha, 1, o->BlendMeshLight, v657, v659, o->HiddenMesh);
			}
			break;
			case  49:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				float v976 = Render(-4.0, 0.0) * 0.7 + 0.300000011920929;
				if (v976 > 1.0)
					v976 = 1.0;
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					1,
					v976,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					532568);
				float v770 = o->BlendMeshTexCoordV,
					v771 = ((int)WorldTime % 2000)
					* 0.0005000000237487257;
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					1,
					v976,
					v771,
					v770,
					532568);
				int v979 = 50;
				int v772 = (int)WorldTime;
				int v975 = v772 % (15 * v979) / v979;
				float v978 = (double)(v975 % 4) * 0.25;
				float v980 = (double)(v975 / 4) * 0.25;
				(b->BodyLight[0]) = 0.15000001;
				(b->BodyLight[1]) = 0.40000001;
				(b->BodyLight[2]) = 1.0;
				b->RenderMesh(
					2,
					66,
					o->Alpha,
					2,
					v976,
					v978,
					v980,
					-1);
				v976 = Render(-4.0, 0.0);
				if (v976 > 1.0)
					v976 = 1.0;
				b->RenderMesh(
					3,
					66,
					o->Alpha,
					3,
					v976,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
			}
			break;
			case  50:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				float v974 = Render(-5.0, 0.0);
				if (v974 > 1.0)
					v974 = 1.0;
				float v775 = o->BlendMeshTexCoordV,
					v776 = ((int)WorldTime % 2000)
					* 0.0005000000237487257;
				b->RenderMesh(
					1,
					66,
					o->Alpha,
					1,
					v974,
					v776,
					v775,
					532549);
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					1,
					v974,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					532569);
			}
			break;
			case  51:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				float v1137 = Render(-2.0, 0.0) * 0.4000000059604645 + 0.6000000238418579;
				b->RenderMesh(
					1,
					66,
					o->Alpha,
					1,
					v1137,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				v1137 = Render(-4.0, 0.0);
				b->RenderMesh(
					2,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					2,
					66,
					o->Alpha,
					2,
					v1137,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					532959);
				b->RenderMesh(
					3,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					3,
					66,
					o->Alpha,
					3,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
			}
			break;
			case  52:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					2,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				float v1121 = Render(-4.0, 0.0) * 0.5 + 0.5;
				float v1122 = Render(-4.0, 120.0);
				float v1119 = Render(-4.0, 240.0);
				b->RenderMesh(
					0,
					66,
					o->Alpha,
					0,
					v1121,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					0,
					66,
					o->Alpha,
					0,
					v1122,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					532963);
				b->RenderMesh(
					0,
					66,
					o->Alpha,
					0,
					v1119,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					532964);
				float v1120 = Render(-4.0, 0.0) * 0.6000000238418579;
				b->RenderMesh(
					2,
					66,
					o->Alpha,
					2,
					v1120,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					532965);
				(b->BodyLight[0]) = 0.73725490196;
				(b->BodyLight[1]) = 0.30980392156;
				(b->BodyLight[2]) = 0.34509803921;
				b->RenderMesh(
					1,
					70,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					1,
					70,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
			}
			break;
			case  53:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					2,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					4,
					66,
					o->Alpha,
					4,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				(b->BodyLight[0]) = 0.3;
				(b->BodyLight[1]) = 0.3;
				(b->BodyLight[2]) = 0.3;
				float v731 = ((int)WorldTime % 2000)
					* 0.0005000000237487257;
				b->RenderMesh(
					2,
					66,
					o->Alpha,
					2,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					v731,
					532771);
				(b->BodyLight[0]) = 1.0;
				(b->BodyLight[1]) = 1.0;
				(b->BodyLight[2]) = 1.0;
				float v1002 = (Render(-4.0, 0.0) * 0.800000011920929
					+ 0.2000000029802322)
					* o->Alpha;
				b->RenderMesh(
					1,
					66,
					o->Alpha,
					1,
					v1002,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					533024);
				v1002 = (Render(-4.0, 0.0) * 0.5 + 0.5)
					* o->Alpha;
				b->RenderMesh(
					2,
					66,
					o->Alpha,
					2,
					v1002,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					3,
					66,
					o->Alpha,
					3,
					v1002,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				int v1004 = 60;
				int v732 = (int)WorldTime;
				int v1001 = v732 % (13 * v1004) / v1004;
				float v1003 = (double)(v1001 % 4) * 0.25;
				float v1005 = (double)(v1001 / 4) * 0.25;
				(b->BodyLight[0]) = 0.25;
				(b->BodyLight[1]) = 0.15000001;
				(b->BodyLight[2]) = 0.23;
				b->RenderMesh(
					5,
					66,
					o->Alpha,
					5,
					o->BlendMeshLight,
					v1003,
					v1005,
					-1);
			}
			break;
			case  54:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				float v997 = Render(-4.0, 0.0) * o->Alpha;
				b->RenderMesh(
					1,
					66,
					o->Alpha,
					1,
					v997,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					533026);
				v997 = (Render(-2.0, 0.0) * 0.699999988079071
					+ 0.300000011920929)
					* o->Alpha;
				b->RenderMesh(
					0,
					66,
					o->Alpha,
					0,
					v997,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					533027);
				v997 = Render(-2.0, 180.0) * o->Alpha;
				(b->BodyLight[0]) = 0.8;
				(b->BodyLight[1]) = 0.8;
				(b->BodyLight[2]) = 0.8;
				b->RenderMesh(
					0,
					66,
					o->Alpha,
					0,
					v997,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					533028);
				b->RenderMesh(
					0,
					66,
					o->Alpha,
					0,
					v997,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					533028);
				int v999 = 80;
				int v735 = (int)WorldTime;
				int v996 = v735 % (15 * v999) / v999;
				float v998 = (double)(v996 % 4) * 0.25;
				float v1000 = (double)(v996 / 4) * 0.25;
				(b->BodyLight[0]) = 0.4;
				(b->BodyLight[1]) = 0.4;
				(b->BodyLight[2]) = 0.4;
				b->RenderMesh(
					2,
					66,
					o->Alpha,
					2,
					o->BlendMeshLight,
					v998,
					v1000,
					-1);
				(b->BodyLight[0]) = 0.5294117647;
				(b->BodyLight[1]) = 0.57647058823;
				(b->BodyLight[2]) = 0.56470588235;
				b->RenderMesh(
					1,
					70,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					1,
					70,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
			}
			break;
			case  55:
			{
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					5,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				float v1503 = Render(-6.0, 0.0) + 0.300000011920929;
				if (v1503 > 1.0)
					v1503 = 1.0;
				b->RenderMesh(
					0,
					66,
					o->Alpha,
					66,
					v1503,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					533043);
				(b->BodyLight[0]) = 1.0;
				(b->BodyLight[1]) = 0.80000001;
				(b->BodyLight[2]) = 0.64999998;
				b->RenderMesh(
					0,
					68,
					o->Alpha,
					0,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					32042);
				(b->BodyLight[0]) = 0.40000001;
				(b->BodyLight[1]) = 0.44999999;
				(b->BodyLight[2]) = 1.0;
				b->RenderMesh(
					2,
					66,
					o->Alpha,
					2,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				(b->BodyLight[0]) = 0.40000001;
				(b->BodyLight[1]) = 0.75999999;
				(b->BodyLight[2]) = 0.94999999;
				int v1505 = 40;
				signed int v49 = WorldTime;
				int v1502 = v49 % (15 * v1505) / v1505;
				float v1504 = (double)(v1502 % 4) * 0.25;
				float v1506 = (double)(v1502 / 4) * 0.25;
				b->RenderMesh(
					3,
					66,
					o->Alpha,
					3,
					o->BlendMeshLight,
					v1504,
					v1506,
					-1);
				(b->BodyLight[0]) = 0.25;
				(b->BodyLight[1]) = 0.34999999;
				(b->BodyLight[2]) = 1.0;
				int wofpeffect1 = (int)(WorldTime * 1.25f) % 600 / 40;
				float wofpeffect2 = (double)(wofpeffect1 % 4) * 0.25;
				float wofpeffect3 = (double)(wofpeffect1 / 4) * 0.25;
				b->RenderMesh(
					4,
					66,
					o->Alpha,
					4,
					o->BlendMeshLight,
					wofpeffect2,
					wofpeffect3,
					533029);
			}
			break;
			case 56:
			{
				if (b->BodyLight[0] != 1 && b->BodyLight[1] != 1 && b->BodyLight[2] != 1)
				{
					Vector(1.f, 1.f, 1.f, b->BodyLight);
					glColor3fv(b->BodyLight);
					b->RenderMesh(0, 2, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
					b->RenderMesh(3, 2, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
					b->RenderMesh(4, 2, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
					b->RenderMesh(5, 66, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
					Vector(1.f, 1.f, 1.f, b->BodyLight);
					b->RenderMesh(3, 70, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
					Vector(0.23921568627, 0.43921568627, 1.f, b->BodyLight);
					b->RenderMesh(4, 70, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
					Vector(0.23921568627, 0.43921568627, 1.f, b->BodyLight);
					b->RenderMesh(4, 70, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
				}
				else
				{
					Vector(1.f, 1.f, 1.f, b->BodyLight);
					glColor3fv(b->BodyLight);
					b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
				}
				glColor3fv(b->BodyLight);
				Vector(1.f, 1.f, 1.f, b->BodyLight);
				b->RenderMesh(3, 70, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
				Vector(0.23921568627, 0.43921568627, 1.f, b->BodyLight);
				b->RenderMesh(4, 70, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
				Vector(0.23921568627, 0.43921568627, 1.f, b->BodyLight);
				b->RenderMesh(4, 70, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
			}
			break;
			case 57:
			{
				if ((b->BodyLight[0]) != 1.0 || (b->BodyLight[1]) != 1.0 || (b->BodyLight[2]) != 1.0)
				{
					(b->BodyLight[0]) = 1.0;
					(b->BodyLight[1]) = 1.0;
					(b->BodyLight[2]) = 1.0;
					glColor3fv(b->BodyLight);
					b->RenderMesh(
						6,
						2,
						o->Alpha,
						o->BlendMesh,
						o->BlendMeshLight,
						o->BlendMeshTexCoordU,
						o->BlendMeshTexCoordV,
						-1);
				}
				else
				{
					(b->BodyLight[0]) = 1.0;
					(b->BodyLight[1]) = 1.0;
					(b->BodyLight[2]) = 1.0;
					glColor3fv(b->BodyLight);
					b->RenderMesh(
						0,
						2,
						o->BlendMesh,
						o->BlendMeshLight,
						o->BlendMeshTexCoordU,
						o->BlendMeshTexCoordV,
						o->HiddenMesh,
						-1);
				}
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					2,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					3,
					66,
					o->Alpha,
					3,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				(b->BodyLight[0]) = 0.70980392156;
				(b->BodyLight[1]) = 0.53725490196;
				(b->BodyLight[2]) = 0.54509803921;
				b->RenderMesh(
					1,
					70,
					o->Alpha,
					1,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				(b->BodyLight[0]) = 0.70980392156;
				(b->BodyLight[1]) = 0.53725490196;
				(b->BodyLight[2]) = 0.54509803921;
				b->RenderMesh(
					1,
					70,
					o->Alpha,
					1,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
			}
			break;
			case  58:
			{
				if ((b->BodyLight[0]) != 1.0 || (b->BodyLight[1]) != 1.0 || (b->BodyLight[2]) != 1.0)
				{
					(b->BodyLight[0]) = 1.0;
					(b->BodyLight[1]) = 1.0;
					(b->BodyLight[2]) = 1.0;
					glColor3fv(b->BodyLight);
					b->RenderMesh(
						6,
						2,
						o->Alpha,
						o->BlendMesh,
						o->BlendMeshLight,
						o->BlendMeshTexCoordU,
						o->BlendMeshTexCoordV,
						-1);
				}
				else
				{
					(b->BodyLight[0]) = 1.0;
					(b->BodyLight[1]) = 1.0;
					(b->BodyLight[2]) = 1.0;
					glColor3fv(b->BodyLight);
					b->RenderMesh(
						5,
						2,
						o->BlendMesh,
						o->BlendMeshLight,
						o->BlendMeshTexCoordU,
						o->BlendMeshTexCoordV,
						o->HiddenMesh,
						-1);
				}
				// ----
				glColor3fv(b->BodyLight);
				// ----
				b->RenderMesh(
					0,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				b->RenderMesh(
					1,
					2,
					o->Alpha,
					o->BlendMesh,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				float v1503 = Render(-6.0, 0.0) + 0.300000011920929;
				if (v1503 > 1.0)
					v1503 = 1.0;
				b->RenderMesh(
					0,
					66,
					o->Alpha,
					66,
					v1503,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					533043);
				(b->BodyLight[0]) = 1.0;
				(b->BodyLight[1]) = 0.80000001;
				(b->BodyLight[2]) = 0.64999998;
				b->RenderMesh(
					0,
					68,
					o->Alpha,
					0,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					32042);
				(b->BodyLight[0]) = 0.40000001;
				(b->BodyLight[1]) = 0.44999999;
				(b->BodyLight[2]) = 1.0;
				b->RenderMesh(
					2,
					66,
					o->Alpha,
					2,
					o->BlendMeshLight,
					o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV,
					-1);
				(b->BodyLight[0]) = 0.40000001;
				(b->BodyLight[1]) = 0.75999999;
				(b->BodyLight[2]) = 0.94999999;
				int v1505 = 40;
				signed int v49 = WorldTime;
				int v1502 = v49 % (15 * v1505) / v1505;
				float v1504 = (double)(v1502 % 4) * 0.25;
				float v1506 = (double)(v1502 / 4) * 0.25;
				b->RenderMesh(
					3,
					66,
					o->Alpha,
					3,
					o->BlendMeshLight,
					v1504,
					v1506,
					-1);
				(b->BodyLight[0]) = 0.25;
				(b->BodyLight[1]) = 0.34999999;
				(b->BodyLight[2]) = 1.0;
				int wofpeffect1 = (int)(WorldTime * 1.25f) % 600 / 40;
				float wofpeffect2 = (double)(wofpeffect1 % 4) * 0.25;
				float wofpeffect3 = (double)(wofpeffect1 / 4) * 0.25;
				b->RenderMesh(
					4,
					66,
					o->Alpha,
					4,
					o->BlendMeshLight,
					wofpeffect2,
					wofpeffect3,
					-1);
			}
			break;
	  case  70:
	  {
		  // ----
		  glColor3fv(b->BodyLight);
		  // ----	
		  b->RenderMesh(
			  0,
			  2,
			  o->Alpha,
			  o->BlendMesh,
			  o->BlendMeshLight,
			  o->BlendMeshTexCoordU,
			  o->BlendMeshTexCoordV,
			  -1);
		  float v550 = Render(-4.0, 0.0) * 0.4000000059604645 + 0.6000000238418579;
		  b->RenderMesh(
			  0,
			  66,
			  o->Alpha,
			  0,
			  v550,
			  o->BlendMeshTexCoordU,
			  o->BlendMeshTexCoordV,
			  532768);
		  b->RenderMesh(
			  1,
			  2,
			  o->Alpha,
			  o->BlendMesh,
			  o->BlendMeshLight,
			  o->BlendMeshTexCoordU,
			  o->BlendMeshTexCoordV,
			  -1);
		  b->RenderMesh(
			  3,
			  70,
			  o->Alpha,
			  o->BlendMesh,
			  o->BlendMeshLight,
			  o->BlendMeshTexCoordU,
			  o->BlendMeshTexCoordV,
			  -1);
		  (b->BodyLight[0]) = 0.4;
		  (b->BodyLight[1]) = 0.4;
		  (b->BodyLight[2]) = 1.0;
		  b->RenderMesh(
			  3,
			  2,
			  o->Alpha,
			  3,
			  o->BlendMeshLight,
			  o->BlendMeshTexCoordU,
			  o->BlendMeshTexCoordV,
			  -1);
		  int BlessedSword012 = 1000;
		  float BlessedSword022 = (((int)((BlessedSword012 - WorldTime)) % 1001)) / (double)BlessedSword012;
		  (b->BodyLight[0]) = 0.69999999;
		  (b->BodyLight[1]) = 0.89999998;
		  (b->BodyLight[2]) = 1.0;
		  b->RenderMesh(2, 66, o->Alpha, 2, o->BlendMeshLight, BlessedSword022, o->BlendMeshTexCoordV, -1);
	  }
	  break;
		default:
		{
			float v1158;
			float v1159;
			float v1700 = Render(-4.0, 0.0) + 0.5;
			Vector(v1700* 1.0, v1700* 0.8999999761581421, v1700* 0.3499999940395355, Color);
			int JointID1[] = { 75,79,80,82,87,88,89 };
			for (int l = 0; l < 7; ++l)
			{
				RenderBrightEffect(b, 32002, JointID1[l], 0.60000002, Color, o); //RenderBrightEffect
			}
			for (int m = 3; m < 18; ++m)
			{
				RenderBrightEffect(b, 32002, m, 0.44999999, Color, o);
			}
			for (int n = 18; n < 46; ++n)
			{
				if (n != 23 && n != 24 && n != 37 && n != 38)
				{
					RenderBrightEffect(b, 32002, n, 0.44999999, Color, o);
				}
			}
			Vector(v1700* 0.949999988079071, v1700* 0.1000000014901161, v1700* 1.0, Color);
			int JointID2[] = { 77,78,90,91 };
			for (int ii = 0; ii < 4; ++ii)
			{
				RenderBrightEffect(b, 32002, JointID2[ii], 0.40000001, Color, o);
			}
			int JointID3[] = { 73,74,81,84 };
			for (int jj = 0; jj < 4; ++jj)
			{
				RenderBrightEffect(b, 32002, JointID3[jj], 0.40000001, Color, o);
			}
			RenderBrightEffect(b, 32002, 1, 1.2, Color, o);
			int JointID4[] = { 2,48,49,50 };
			for (int kk = 0; kk < 4; ++kk)
			{
				RenderBrightEffect(b, 32002, JointID4[kk], 0.40000001, Color, o);
			}
			int JointID5[] = { 76,83,85,86 };
			for (int ll = 0; ll < 4; ++ll)
			{
				Vector(v1700 * 0.2000000029802322, v1700 * 0.2000000029802322, v1700 * 1.0, Color);
				RenderBrightEffect(b, 32002, JointID5[ll], 0.2, Color, o);
				Vector(v1700 * 1.0, v1700 * 1.0, v1700 * 1.0, Color);
				RenderBrightEffect(b, 32229, JointID5[ll], 0.2, Color, o);
			}
			Vector(v1700* 0.2000000029802322, v1700* 0.2000000029802322, v1700* 1.0, Color);
			int JointID6[] = { 90,78 };
			for (int mm = 0; mm < 2; ++mm)
			{
				RenderBrightEffect(b, 32002, JointID6[mm], 0.80000001, Color, o);
			}
			Vector(v1700* 0.2000000029802322, v1700* 0.699999988079071, v1700* 1.0, Color);
			for (int nn = 54; nn < 70; ++nn)
			{
				RenderBrightEffect(b, 32002, nn, 0.40000001, Color, o);
			}
			float v311 = ((int)WorldTime % 15) / 37.5;
			v1700 = v311 + 0.5;
			Vector(v1700* 0.0, v1700* 0.1000000014901161, v1700* 1.0, Color);
			RenderBrightEffect(b, 32002, 51, 4.0, Color, o);
			RenderBrightEffect(b, 32002, 51, 3.5999999, Color, o);
			Vector(1.0, 1.0, 1.0, Color);
			float v1711 = ((int)(WorldTime * 0.09000000357627869) % 90) * 0.01745329238474369;
			float v312 = v1711 + 1.570796370506287;
			float v313 = sub_4E7B95(v312);
			float v1698 = sub_639050(v313);
			float v314 = sub_4E7B95(v1711);
			float v1712 = sub_639050(v314) * 0.800000011920929;
			float v1701 = v1698 * 0.75;
			float v1702 = v1698 * 0.4000000059604645;
			float v1703 = v1698 * 1.0;
			Vector((1.399999976158142 - v1712)** (float*)& v1701, (1.399999976158142 - v1712)* v1702, (1.399999976158142 - v1712)* v1703, Color);
			if (v1712 - 0.2000000029802322 <= 0.0)
				v1159 = 0.0;
			else
				v1159 = v1712 - 0.2000000029802322;
			float v315 = v1159;
			RenderBrightEffect(b, 32614, 51, v315, Color, o);
			v1711 = ((int)(WorldTime * 0.09000000357627869 + 30.0) % 90)
				* 0.01745329238474369;
			float v316 = sub_4E7B95(v1711);
			v1712 = sub_639050(v316) * 0.800000011920929;
			v1701 = v1698 * 0.75;
			v1702 = v1698 * 0.4000000059604645;
			v1703 = v1698 * 1.0;
			Vector((1.399999976158142 - v1712) * *(float*)& v1701, (1.399999976158142 - v1712) * v1702, (1.399999976158142 - v1712) * v1703, Color);
			if (v1712 - 0.2000000029802322 <= 0.0)
				v1158 = 0.0;
			else
				v1158 = v1712 - 0.2000000029802322;
			float v317 = v1158;
			RenderBrightEffect(b, 32614, 51, v317, Color, o);
			// ----
			// ----
			// ----
			glColor3fv(b->BodyLight);
			// ----
			b->RenderMesh(
				0,
				2,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				o->HiddenMesh);
			b->RenderMesh(
				1,
				2,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				o->HiddenMesh);
			b->RenderMesh(
				2,
				2,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				o->HiddenMesh);
			b->RenderMesh(
				3,
				2,
				o->Alpha,
				o->BlendMesh,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				o->HiddenMesh);
			b->RenderMesh(
				3,
				576,
				o->Alpha,
				3,
				o->BlendMeshLight,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				532771);
			int BlessedSword01 = 1000;
			float BlessedSword02 = (((int)((BlessedSword01 - WorldTime)) % 1001)) / (double)BlessedSword01;
			(b->BodyLight[0]) = 1.0;
			(b->BodyLight[1]) = 0.89999998;
			(b->BodyLight[2]) = 0.69999999;
			b->RenderMesh(4, 66, o->Alpha, 4, o->BlendMeshLight, BlessedSword02, o->BlendMeshTexCoordV, -1);
			(b->BodyLight[0]) = 0.0;
			(b->BodyLight[1]) = 0.1;
			(b->BodyLight[2]) = 1.0;
			b->RenderMesh(
				5,
				66,
				1.0,
				5,
				1.0,
				o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV,
				-1);
			}
			break;
		}
	}

}