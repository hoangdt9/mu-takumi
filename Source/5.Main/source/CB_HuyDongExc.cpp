#include "stdafx.h"
#include "CB_HuyDongExc.h"
#include "NewUISystem.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "Other.h"
#include "Protocol.h"

CBHuyDongExc* gCBHuyDongExc = nullptr;
//==Huy Exc
int mHuyExcWC[6] = { NULL };
int mHuyExcWP[6] = { NULL };
int mHuyExcGP[6] = { NULL };
int mHuyExcRate[6] = { NULL };

CBHuyDongExc::CBHuyDongExc()
{
	this->ItemCacheSelect = nullptr;
	this->ItemCacheShow = 0;
	this->ItemCacheTime = 0;
}

CBHuyDongExc::~CBHuyDongExc()
{

}

void CBHuyDongExc::OpenWindow()
{

	if ((GetTickCount() - gInterface.Data[eWindowHuyDongExc].EventTick) > 300)
	{
		gInterface.Data[eWindowHuyDongExc].EventTick = GetTickCount();

		if (gInterface.Data[eWindowHuyDongExc].OnShow)
		{

			gInterface.Data[eWindowHuyDongExc].OnShow = 0;

			return;
		}

		gInterface.Data[eWindowHuyDongExc].OnShow ^= 1;
	}
}
int m_OpExcSelectedIndex = -1;
bool TextSelectHover = false;
DWORD HuyExcEffect = 0;
bool HuyExcCheckSelectOpt(int Count, int IndexOpt)
{
	if (!gCBHuyDongExc) return 0;
	if (SEASON3B::IsPress(VK_LBUTTON) && TextSelectHover && (GetTickCount() - gCBHuyDongExc->ItemCacheTime) > 300)
	{

		m_OpExcSelectedIndex = IndexOpt;
		gCBHuyDongExc->ItemCacheTime = GetTickCount();
		//gInterface.DrawMessage(1, "Click OP Exc %d", m_OpExcSelectedIndex);
		return 1;
	}
	if (m_OpExcSelectedIndex == IndexOpt && !TextSelectHover)
	{
		return 1;
	}
	return 0;
}
int GetCoinHuyExc(int TypeCoin)
{
	int Coin = 0;

	switch (m_OpExcSelectedIndex)
	{
	case 1:
	{
		if (TypeCoin == 1)
		{
			Coin = mHuyExcWC[0];
		}
		else if (TypeCoin == 2)
		{
			Coin = mHuyExcWP[0];
		}
		else if (TypeCoin == 3)
		{
			Coin = mHuyExcGP[0];
		}
		else if (TypeCoin == 4)
		{
			Coin = mHuyExcRate[0];
		}
	}
	break;
	case 2:
	{
		if (TypeCoin == 1)
		{
			Coin = mHuyExcWC[1];
		}
		else if (TypeCoin == 2)
		{
			Coin = mHuyExcWP[1];
		}
		else if (TypeCoin == 3)
		{
			Coin = mHuyExcGP[1];
		}
		else if (TypeCoin == 4)
		{
			Coin = mHuyExcRate[1];
		}
	}
	break;
	case 4:
	{
		if (TypeCoin == 1)
		{
			Coin = mHuyExcWC[2];
		}
		else if (TypeCoin == 2)
		{
			Coin = mHuyExcWP[2];
		}
		else if (TypeCoin == 3)
		{
			Coin = mHuyExcGP[2];
		}
		else if (TypeCoin == 4)
		{
			Coin = mHuyExcRate[2];
		}
	}
	break;
	case 8:
	{
		if (TypeCoin == 1)
		{
			Coin = mHuyExcWC[3];
		}
		else if (TypeCoin == 2)
		{
			Coin = mHuyExcWP[3];
		}
		else if (TypeCoin == 3)
		{
			Coin = mHuyExcGP[3];
		}
		else if (TypeCoin == 4)
		{
			Coin = mHuyExcRate[3];
		}
	}
	break;
	case 16:
	{
		if (TypeCoin == 1)
		{
			Coin = mHuyExcWC[4];
		}
		else if (TypeCoin == 2)
		{
			Coin = mHuyExcWP[4];
		}
		else if (TypeCoin == 3)
		{
			Coin = mHuyExcGP[4];
		}
		else if (TypeCoin == 4)
		{
			Coin = mHuyExcRate[4];
		}
	}
	break;
	case 32:
	{
		if (TypeCoin == 1)
		{
			Coin = mHuyExcWC[5];
		}
		else if (TypeCoin == 2)
		{
			Coin = mHuyExcWP[5];
		}
		else if (TypeCoin == 3)
		{
			Coin = mHuyExcGP[5];
		}
		else if (TypeCoin == 4)
		{
			Coin = mHuyExcRate[5];
		}
	}
	break;
	default:
		break;
	}

	return Coin;
}
int CheckWindowHuyExc = -1;
void CBHuyDongExc::DrawWindowHuyExc()
{
	if (!gInterface.Data[eWindowHuyDongExc].OnShow)
	{
		if (CheckWindowHuyExc == 1)
		{
			this->ItemCacheSelect = NULL;
			m_OpExcSelectedIndex = -1;
			this->ItemCacheShow = false;
			CheckWindowHuyExc = -1;

			XULY_CGPACKET pMsg;
			pMsg.header.set(0xFA, 0x27, sizeof(pMsg));
			pMsg.ThaoTac = 0; //RollBack Item
			DataSend((LPBYTE)& pMsg, pMsg.header.size);
		}
		return;
	}
	if (!g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_INVENTORY))
	{
		g_pNewUISystem->Show(SEASON3B::INTERFACE_INVENTORY);
	}
	if (gInterface.Data[eWindowHuyDongExc].OnShow)
	{
		if (CheckWindowHuyExc == -1)
		{
			CheckWindowHuyExc = 1;
		}
	}

	float CuaSoW = 290;
	float CuaSoH = 260;

	float StartY = ((MAX_WIN_HEIGHT - 51) / 2) - (CuaSoH / 2);
	float StartX = (MAX_WIN_WIDTH / 2) - (CuaSoW / 2);

	int FontSize = 14;
	float KhoangCachDong = 15;
	int GetInfoLoadItem = 0;
	if (g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, CuaSoW, CuaSoH, eWindowHuyDongExc, gOther.Text_HuyDong[0]))
	{
		//gElemental.gDrawWindowCustom(StartX, StartY, CuaSoW, CuaSoH, eWindowHuyDongExc, "Huỷ Dòng Exc");
		//=== Info Dmg Huy Hieu Cuong Hoa
		DWORD ColorBoxItem = 0x00000096;
		if (SEASON3B::CheckMouseIn(StartX + 7, StartY + 85, 120, 90) == 1)
		{
			if (SEASON3B::IsPress(VK_RBUTTON) && this->ItemCacheSelect != NULL && HuyExcEffect == 0)
			{
				//pDrawMessage("UnStick Item", 1);
				this->ItemCacheSelect = NULL;
				m_OpExcSelectedIndex = -1;
				this->ItemCacheShow = false;

				PlayBuffer(25, 0, 0);
				gInterface.Data[eWindowHuyDongExc].EventTick = GetTickCount();

				XULY_CGPACKET pMsg;
				pMsg.header.set(0xFA, 0x27, sizeof(pMsg));
				pMsg.ThaoTac = 0; //RollBack Item
				DataSend((LPBYTE)& pMsg, pMsg.header.size);
			}
		}
		if (this->ItemCacheSelect != NULL && this->ItemCacheShow)
		{
			ColorBoxItem = 0x4D808036;

		}

		TextDraw((HFONT)g_hFont, StartX, StartY + 35, 0xFFD940FF, 0x0, 290.0, 0, 3, gOther.Text_HuyDong[1]);

		TextDraw((HFONT)g_hFont, StartX + 12, StartY + 50, 0xFF8940FF, 0x0, 250, 0, 1, gOther.Text_HuyDong[2]);
		//CustomFont.Draw(HHCHFontBig, StartX + 21, StartY + 65, 0x40FFFCFF, 0x0, 250, 0, 0, "WC: 50,000");
		//CustomFont.Draw(HHCHFontBig, StartX + 21, StartY + 65, 0x40FFFCFF, 0x0, 250, 0, 3, "WP: 50,000");
		//CustomFont.Draw(HHCHFontBig, StartX + 21, StartY + 65, 0x40FFFCFF, 0x0, 240, 0, 4, "GP: 50,000");
		//== Khung Item
		g_pBCustomMenuInfo->DrawInfoBox(StartX + 10, StartY + 65, 100, 90, ColorBoxItem, 0, 0);
		//==Khung Info
		g_pBCustomMenuInfo->DrawInfoBox(StartX + 125, StartY + 65, 145, 90, ColorBoxItem, 0, 0);
		if (this->ItemCacheSelect != NULL && this->ItemCacheShow)
		{

			GetInfoLoadItem = (int)(intptr_t)this->ItemCacheSelect;
			g_pNewUISystem->RenderItem3DFree(StartX + 35, StartY + 30, 50, 90, *(WORD*)GetInfoLoadItem, *(DWORD*)(GetInfoLoadItem + 2), *(BYTE*)(GetInfoLoadItem + 23), *(BYTE*)(GetInfoLoadItem + 24), 0, 1.3);
			TextDraw((HFONT)g_hFont, StartX + 12, StartY + 67, 0xFFFFFFFF, 0xD1710AA5, 105, 0, 3, BGetItemName(*(WORD*)GetInfoLoadItem, 0));



			//==Show Info Exc
			TextDraw((HFONT)g_hFont, StartX + 127, StartY + 67, 0xFFFFFFFF, 0xD1710AA5, 150, 0, 3, gOther.Text_HuyDong[3]);
			int nCountText = 0;
			for (int i = 0; i < (int)this->ItemCacheSelect->SpecialNum; i++)
			{
				float TextX = StartX + 130;
				float TextY = StartY + 85 + (nCountText * 12);
				DWORD ColorSelect = 0x0;
				float TextW = 140;
				TextSelectHover = false;
				if (SEASON3B::CheckMouseIn(TextX, TextY, 105, 12) == true)
				{
					ColorSelect = 0xFF8C40FF;
					TextSelectHover = true;
				}
				switch (this->ItemCacheSelect->Special[i])
				{
					// opt 32
				case CBHuyDongExc::LifeIncrease:
					if (HuyExcCheckSelectOpt(nCountText, 32))
					{
						ColorSelect = 0xFF4040FF;
					}
					TextDraw((HFONT)g_hFont, TextX, TextY, 0xFFFFFFFF, ColorSelect, TextW, 0, 0, GlobalText[622]);
					nCountText++;
					break;
				case CBHuyDongExc::ExcellentDamage:
					if (HuyExcCheckSelectOpt(nCountText, 32))
					{
						ColorSelect = 0xFF4040FF;
					}
					TextDraw((HFONT)g_hFont, TextX, TextY, 0xFFFFFFFF, ColorSelect, TextW, 0, 0, GlobalText[628]);
					nCountText++;
					//pSetItemTextLine(Line, "Ty le sat thuong hoan hao +%d%%",
					break;
					// opt 16
				case CBHuyDongExc::ManaIncrease:
					if (HuyExcCheckSelectOpt(nCountText, 16))
					{
						ColorSelect = 0xFF4040FF;
					}
					TextDraw((HFONT)g_hFont, TextX, TextY, 0xFFFFFFFF, ColorSelect, TextW, 0, 0, GlobalText[623]);
					nCountText++;
					//pSetItemTextLine(Line, "Gia tang luong Mana +%d%%",
					break;
				case CBHuyDongExc::DamageByLevel:
					if (HuyExcCheckSelectOpt(nCountText, 16))
					{
						ColorSelect = 0xFF4040FF;
					}
					TextDraw((HFONT)g_hFont, TextX, TextY, 0xFFFFFFFF, ColorSelect, TextW, 0, 0, GlobalText[629]);
					nCountText++;
					//pSetItemTextLine(Line, "Gia tang cap do tan cong +Level/%d",
					break;
				case CBHuyDongExc::IncreaseWizardry: //Gay Phep
					if (HuyExcCheckSelectOpt(nCountText, 16))
					{
						ColorSelect = 0xFF4040FF;
					}
					TextDraw((HFONT)g_hFont, TextX, TextY, 0xFFFFFFFF, ColorSelect, TextW, 0, 0, GlobalText[631]);
					nCountText++;
					//pSetItemTextLine(Line, "Gia tang cap do tan cong ( phep thuat ) +Level/%d",
					break;
					// opt 8
				case CBHuyDongExc::DamageDecrease: //Set Item
					if (HuyExcCheckSelectOpt(nCountText, 8))
					{
						ColorSelect = 0xFF4040FF;
					}
					TextDraw((HFONT)g_hFont, TextX, TextY, 0xFFFFFFFF, ColorSelect, TextW, 0, 0, GlobalText[624]);
					nCountText++;
					//pSetItemTextLine(Line, "Giam suc sat thuong +%d%%",
					break;
				case CBHuyDongExc::IncreaseDamage: //==Vu Khi Vat Ly
					if (HuyExcCheckSelectOpt(nCountText, 8))
					{
						ColorSelect = 0xFF4040FF;
					}
					TextDraw((HFONT)g_hFont, TextX, TextY, 0xFFFFFFFF, ColorSelect, TextW, 0, 0, GlobalText[630], this->ItemCacheSelect->SpecialValue[i]);
					nCountText++;
					//pSetItemTextLine(Line, "Gia tang suc sat thuong +%d%%",
					break;
				case CBHuyDongExc::IncreaseWizardryDmg: //==Vu Khi Phep
					if (HuyExcCheckSelectOpt(nCountText, 8))
					{
						ColorSelect = 0xFF4040FF;
					}
					TextDraw((HFONT)g_hFont, TextX, TextY, 0xFFFFFFFF, ColorSelect, TextW, 0, 0, GlobalText[632], this->ItemCacheSelect->SpecialValue[i]);
					nCountText++;
					//pSetItemTextLine(Line, "Gia tang suc sat thuong phep thuat +%d%%",
					break;
					// opt 4
				case CBHuyDongExc::Reflect:
					if (HuyExcCheckSelectOpt(nCountText, 4))
					{
						ColorSelect = 0xFF4040FF;
					}
					TextDraw((HFONT)g_hFont, TextX, TextY, 0xFFFFFFFF, ColorSelect, TextW, 0, 0, GlobalText[625]);
					nCountText++;
					//pSetItemTextLine(Line, "Phan hoi +%d%% luc sat thuong",
					break;
				case CBHuyDongExc::IncreaseSpeed:
					if (HuyExcCheckSelectOpt(nCountText, 4))
					{
						ColorSelect = 0xFF4040FF;
					}
					TextDraw((HFONT)g_hFont, TextX, TextY, 0xFFFFFFFF, ColorSelect, TextW, 0, 0, GlobalText[633], this->ItemCacheSelect->SpecialValue[i]);
					nCountText++;
					//pSetItemTextLine(Line, "Gia tang toc do tan cong phep thuat +%d",
					break;
					// opt 2
				case CBHuyDongExc::DefenseSuccess:
					if (HuyExcCheckSelectOpt(nCountText, 2))
					{
						ColorSelect = 0xFF4040FF;
					}
					TextDraw((HFONT)g_hFont, TextX, TextY, 0xFFFFFFFF, ColorSelect, TextW, 0, 0, GlobalText[626]);
					nCountText++;
					//pSetItemTextLine(Line, "Kha nang tranh don +%d%%",
					break;
				case CBHuyDongExc::LifePerMob:
					if (HuyExcCheckSelectOpt(nCountText, 2))
					{
						ColorSelect = 0xFF4040FF;
					}
					TextDraw((HFONT)g_hFont, TextX, TextY, 0xFFFFFFFF, ColorSelect, TextW, 0, 0, GlobalText[634]);
					nCountText++;
					//pSetItemTextLine(Line, " Phuc hoi HP khi giet quai vat +HP/%d",
					break;
					// opt 1
				case CBHuyDongExc::IncreaseZen:
					if (HuyExcCheckSelectOpt(nCountText, 1))
					{
						ColorSelect = 0xFF4040FF;
					}
					TextDraw((HFONT)g_hFont, TextX, TextY, 0xFFFFFFFF, ColorSelect, TextW, 0, 0, GlobalText[627]);
					nCountText++;
					//pSetItemTextLine(Line, "Tang luong Zen roi ra +%d%%",
					break;
				case CBHuyDongExc::ManaPerMob:
					if (HuyExcCheckSelectOpt(nCountText, 1))
					{
						ColorSelect = 0xFF4040FF;
					}
					TextDraw((HFONT)g_hFont, TextX, TextY, 0xFFFFFFFF, ColorSelect, TextW, 0, 0, GlobalText[635]);
					nCountText++;
					//pSetItemTextLine(Line, "Phuc hoi Mana khi giet quai vat +Mana/%d",
					break;
				}

			}
		}
		else
		{
			TextDraw((HFONT)g_hFont, StartX + 130, StartY + 85, 0xFFFFFFFF, 0xFF000026, 140, 0, 0, gOther.Text_HuyDong[4]);
		}

		//== Khung Coin
		g_pBCustomMenuInfo->DrawInfoBox(StartX + 10, StartY + 170, 100, 50, 0x00000096, 0, 0);
		//======================================================
		TextDraw((HFONT)g_hFont, StartX + 15, StartY + 175, 0xFFFFFFFF, 0x0, (100 / 2), 0, 1, gOther.Text_HuyDong[5]);//"Chi Phi:"); //
		//===============
		TextDraw((HFONT)g_hFont, StartX + 15, StartY + 175 + 13, 0xFFBA19FF, 0x0, (100 / 2), 0, 1, "- WCoin:");//"- WCoin"); //
		TextDraw((HFONT)g_hFont, StartX + 15, StartY + 175 + (13 * 2), 0xFFBA19FF, 0x0, (100 / 2), 0, 1, "- WCoinP"); //
		TextDraw((HFONT)g_hFont, StartX + 15, StartY + 175 + (13 * 3), 0xFFBA19FF, 0x0, (100 / 2), 0, 1, "- GobinP"); //
		if (m_OpExcSelectedIndex == -1)
		{
			TextDraw((HFONT)g_hFont, StartX + 10 + (100 / 2), StartY + 175, 0xDE1616FF, 0x0, 0, 0, 1, gOther.Text_HuyDong[6]); //
			TextDraw((HFONT)g_hFont, StartX + 10 + (100 / 2), StartY + 175 + 13, 0xDE1616FF, 0x0, 0, 0, 1, gOther.Text_HuyDong[6]); //
			TextDraw((HFONT)g_hFont, StartX + 10 + (100 / 2), StartY + 175 + (13 * 2), 0xDE1616FF, 0x0, 0, 0, 1, gOther.Text_HuyDong[6]); //
			TextDraw((HFONT)g_hFont, StartX + 10 + (100 / 2), StartY + 175 + (13 * 3), 0xDE1616FF, 0x0, 0, 0, 1, gOther.Text_HuyDong[6]); //

		}
		else {
			TextDraw((HFONT)g_hFont, StartX + 20 + (100 / 2), StartY + 175, 0x19FF9FFF, 0x0, 0, 0, 1, "%s%%", gInterface.NumberFormat(GetCoinHuyExc(4))); //
			TextDraw((HFONT)g_hFont, StartX + 20 + (100 / 2), StartY + 175 + 13, 0x19FF9FFF, 0x0, 0, 0, 1, "%s", gInterface.NumberFormat(GetCoinHuyExc(1))); //
			TextDraw((HFONT)g_hFont, StartX + 20 + (100 / 2), StartY + 175 + (13 * 2), 0x19FF9FFF, 0x0, 0, 0, 1, "%s", gInterface.NumberFormat(GetCoinHuyExc(2))); //
			TextDraw((HFONT)g_hFont, StartX + 20 + (100 / 2), StartY + 175 + (13 * 3), 0x19FF9FFF, 0x0, 0, 0, 1, "%s", gInterface.NumberFormat(GetCoinHuyExc(3))); //
		}
		//==============

		TextDraw((HFONT)g_hFont, StartX + 127, StartY + 170, 0xFFFFFFFF, 0x0073FFFF, 150, 0, 3, gOther.Text_HuyDong[7]);

		if (HuyExcEffect > GetTickCount() && HuyExcEffect != 0)
		{
			RenderMixEffect(1, StartX + 45, StartY + 80);

		}
		else if (HuyExcEffect != 0)
		{
			HuyExcEffect = 0;
			if (m_OpExcSelectedIndex != -1)
			{
				XULY_CGPACKET pMsg;
				pMsg.header.set(0xFA, 0x26, sizeof(pMsg));
				pMsg.ThaoTac = m_OpExcSelectedIndex; //
				DataSend((LPBYTE)& pMsg, pMsg.header.size);

				this->ItemCacheSelect = NULL;
				m_OpExcSelectedIndex = -1;
				this->ItemCacheShow = false;
			}
			//gInterface.Data[eWindowNPCCHHT].OnShow = false;
		}
		//===Draw Button
		if (g_pBCustomMenuInfo->DrawButton(StartX + 175, StartY + 185, 120, 12, gOther.Text_HuyDong[8], 70) && this->ItemCacheSelect != NULL && m_OpExcSelectedIndex != -1 && this->ItemCacheShow)

			//if (nInterface.BDrawButton(StartX + 147, StartY + 185, 90, 30, FALSE, gOther.Text_HuyDong[8]) && this->ItemCacheSelect != NULL && m_OpExcSelectedIndex != -1 && this->ItemCacheShow)
		{


			if (CharacterAttribute->PrintPlayer.Coin1 < GetCoinHuyExc(1) || CharacterAttribute->PrintPlayer.Coin2 < GetCoinHuyExc(2) || CharacterAttribute->PrintPlayer.Coin3 < GetCoinHuyExc(3))
			{
				gInterface.OpenMessageBox("Error", gOther.Text_HuyDong[9]);
				return;
			}

			HuyExcEffect = GetTickCount() + 2000;
		}
	}
}
void CBHuyDongExc::RecvProtocol(BYTE * Recv)
{
	if (!Recv) return;
	HUYEXC_GSSENDINFO* pMsg = (HUYEXC_GSSENDINFO*)Recv;
	if (pMsg->Request)
	{
		for (size_t i = 0; i < 6; i++)
		{
			mHuyExcWC[i] = pMsg->mWC[i];
			mHuyExcWP[i] = pMsg->mWP[i];
			mHuyExcGP[i] = pMsg->mGP[i];
			mHuyExcRate[i] = pMsg->mRate[i];

		}
		this->ItemCacheShow = true;
	}

}
bool CBHuyDongExc::SendItem(ITEM * ItemSell, int Slot, bool KeyClick)
{
	if (KeyClick && gInterface.Data[eWindowHuyDongExc].OnShow)// Send Item Rao Ban
	{
		if (ItemSell->Type < 0) { return 0; }
		if (this->ItemCacheSelect != NULL && this->ItemCacheShow || (GetTickCount() - this->ItemCacheTime) < 300)
		{
			//gInterface.DrawMessage(1, "Item Khong Hop Le" );
			return 0;
		}
		if (!this->ItemCacheShow)
		{
			this->ItemCacheSelect = NULL;
		}
		//memcpy(&gCBChotroi.ItemCacheSelect,item)
		this->ItemCacheSelect = g_pNewItemMng->CreateItem(ItemSell);;
		this->ItemCacheTime = GetTickCount();

		CBHuyDongExc::PMSG_HDCLIENTSENDGS pMsg = { 0 };

		pMsg.h.set(0xFA, 0x25, sizeof(pMsg));
		pMsg.Source = Slot;
		::PlayBuffer(SOUND_GET_ITEM01);
		DataSend((BYTE*)& pMsg, pMsg.h.size);
		return 1;
	}

	return 0;
}