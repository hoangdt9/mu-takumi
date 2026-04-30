#include "StdAfx.h"
#include "CB_AutoNapGame.h"
#include "NewUISystem.h"
#include "CBInterface.h"
#include "CUIController.h"
#include "CharacterManager.h"
#include "Util.h"
#include "Protocol.h"
#include "NewUIBase.h"
#include "Other.h"
#include "ExternalObject/curl/curl.h"
#include "MocDonate.h"

CUITextInputBox* InputGiaTriNap = NULL;
CUITextInputBox* CInputCaptCha = NULL;
char GetTextGiaTriNap[9];
char CGetTextCaptCha[5];
char* mTextStatus[] = { "Quét QR", "Hoàn Tất", "Đã Huỷ" };
int NumberQR = -1;

CBAutoNapGame gCBAutoNapGame;

CBAutoNapGame::CBAutoNapGame()
{
	this->ClearData();
	this->ShowQRAfterInput = false;
}

CBAutoNapGame::~CBAutoNapGame()
{

}
int CacheQRCode = 168000;
const int windowWidth = 800;
const int windowHeight = 600;
bool GetQRCode = false;
std::vector<unsigned char> qrImageData;
bool UpdateQRImg = false;
char url[255] = { 0 };

size_t writeCallback(void* contents, size_t size, size_t nmemb, std::vector<unsigned char>* imageData) {
	size_t totalSize = size * nmemb;
	imageData->insert(imageData->end(), static_cast<unsigned char*>(contents), static_cast<unsigned char*>(contents) + totalSize);
	return totalSize;
}
bool Save_Image(const std::string& src, const std::string& dest, int cDumpHeader)
{
	FILE* fp = fopen(src.c_str(), "rb");
	if (fp == NULL)
	{
		return false;
	}

	if (fseek(fp, 0, SEEK_END) != 0)
	{
		fclose(fp);
		return false;
	}

	const long sizeLong = ftell(fp);
	if (sizeLong <= 0 || sizeLong > 0x7fffffffL || sizeLong < cDumpHeader)
	{
		fclose(fp);
		return false;
	}
	if (fseek(fp, 0, SEEK_SET) != 0)
	{
		fclose(fp);
		return false;
	}

	const size_t size = (size_t)sizeLong;
	char* pTempBuf = new char[size];
	if (fread(pTempBuf, 1, size, fp) != size)
	{
		delete[] pTempBuf;
		fclose(fp);
		return false;
	}
	fclose(fp);

	fp = fopen(dest.c_str(), "wb");
	if (fp == NULL)
	{
		delete[] pTempBuf;
		return false;
	}

	fwrite(pTempBuf, 1, cDumpHeader, fp);
	fwrite(pTempBuf, 1, size, fp);
	fclose(fp);

	delete[] pTempBuf;

	return true;
}
bool Convert_Format(const std::string & filename)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	::_splitpath(filename.c_str(), drive, dir, fname, ext);

	std::string strPath = drive; strPath += dir;
	std::string strName = fname;

	if (_stricmp(ext, ".jpg") == 0)
	{
		std::string strSaveName = strPath;
		strSaveName.append(strName);
		strSaveName.append(".OZJ");
		return Save_Image(filename, strSaveName.c_str(), 24);
	}
	else if (_stricmp(ext, ".tga") == 0)
	{
		std::string strSaveName = strPath;
		strSaveName.append(strName);
		strSaveName.append(".OZT");
		return Save_Image(filename, strSaveName.c_str(), 4);
	}
	else if (_stricmp(ext, ".bmp") == 0)
	{
		std::string strSaveName = strPath;
		strSaveName.append(strName);
		strSaveName.append(".OZB");
		return Save_Image(filename, strSaveName.c_str(), 4);
	}

	return false;
}

bool ConvertToJPEG(const std::string & inputFilePath, const std::string & outputFilePath) {

	FREE_IMAGE_FORMAT inputFormat = FreeImage_GetFileType(inputFilePath.c_str(), 0);
	FIBITMAP* inputImage = FreeImage_Load(inputFormat, inputFilePath.c_str());

	if (!inputImage)
	{
		//DrawMessage(1, "ConvertToJPEG 1");
		return false;
	}

	if (FreeImage_Save(FIF_JPEG, inputImage, outputFilePath.c_str(), 80))
	{
		FreeImage_Unload(inputImage);
		return true;
	}
	else {
		FreeImage_Unload(inputImage);
		//DrawMessage(1, "ConvertToJPEG 2");
		return false;
	}

}
void GetQRCodeSePay(LPVOID lpThreadParameter)
{
	if (GetQRCode) return;
	GetQRCode = true;


	//std::string url = "https://qr.sepay.vn/img?bank=VietCapitalBank&acc=9021171869294&amount=10000&des=%d";
	qrImageData.clear();
	CURL* curl = curl_easy_init();
	if (curl)
	{


		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &qrImageData);
		CURLcode res = curl_easy_perform(curl);

		if (res == CURLE_OK)
		{

			FILE* fp = fopen("Data\\Custom\\MaQRNapThe.png", "wb");
			if (fp) {
				fwrite(&qrImageData[0], 1, qrImageData.size(), fp);
				fclose(fp);
				DeleteFileA("Data\\Custom\\MaQRNapThe.ozj");
				if (ConvertToJPEG("Data\\Custom\\MaQRNapThe.png", "Data\\Custom\\MaQRNapThe.jpg"))
				{
					if (Convert_Format(".\\Data\\Custom\\MaQRNapThe.jpg"))
					{
						UpdateQRImg = true;
						DeleteFileA("Data\\Custom\\MaQRNapThe.png");
						DeleteFileA("Data\\Custom\\MaQRNapThe.jpg");
						//gInterface.DrawMessage(1, "Load QR OK");
					}
				}
			}
		}
		curl_easy_cleanup(curl);
	}
	GetQRCode = false;
}
void KhoiTaoQRCodeThanhToan(char* NameBank, char* STK, int MenhGiaNap, char* NoiDungThanhToan)
{
	if (GetQRCode/* || MenhGiaNap <= 0 || MenhGiaNap > 2000000000*/) return;
	//NameBank = "VietCapitalBank";
	//STK = "9021171869294";
	//MenhGiaNap = 10000;
	//NoiDungThanhToan = "Test nap the";
	//wsprintf(url, "https://qr.sepay.vn/img?bank=%s&acc=%s&amount=%d&des=%s", NameBank, STK, MenhGiaNap, NoiDungThanhToan);
	//wsprintf(url, "https://vietqr.co/api/generate/VCCB/%s/VIETQR.CO/%d/%s?style=2&logo=1&isMask=1&bg=61", STK, MenhGiaNap, NoiDungThanhToan);
	if (MenhGiaNap > 0)
	{
		wsprintf(url, "https://img.vietqr.io/image/%s-%s-qr_only.png?amount=%d&addInfo=%s", NameBank, STK, MenhGiaNap, NoiDungThanhToan);
	}
	else
	{
		wsprintf(url, "https://img.vietqr.io/image/%s-%s-qr_only.png?addInfo=%s", NameBank, STK, NoiDungThanhToan);
	}
	//gInterface.DrawMessage(1, url);
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)GetQRCodeSePay, 0, 0, 0);

}
void MainQRCodeProc()
{
	//gInterface.DrawMessage(1, "MainQRCodeProc");
	if (UpdateQRImg)
	{
		DeleteBitmap(CacheQRCode, 0);
		LoadBitmap((char*)"Custom\\MaQRNapThe.jpg", CacheQRCode, 0x2601, 0x2900, 1, 0);
		UpdateQRImg = false;
	}
}
char* DrawTimeUnix(int timestamp)
{
	time_t rawtime = static_cast<time_t>(timestamp);
	struct tm* timeinfo;
	timeinfo = localtime(&rawtime);
	int year = timeinfo->tm_year + 1900;
	int month = timeinfo->tm_mon + 1;
	int day = timeinfo->tm_mday;
	int hour = timeinfo->tm_hour;
	int minute = timeinfo->tm_min;
	char ztemp[255] = { 0 };
	sprintf_s(ztemp, "%02d:%02d %02d/%02d/%02d", hour, minute, day, month, year % 100);
	return strdup(ztemp);
}

void CBAutoNapGame::OpenWindowBanking()
{

	if (GetTickCount() < gInterface.Data[eWindowAutoBaking].EventTick + 300) return;
	gInterface.Data[eWindowAutoBaking].EventTick = GetTickCount();
	gInterface.Data[eWindowAutoBaking].OnShow ^= 1;
	if (gInterface.Data[eWindowAutoBaking].OnShow)
	{
		XULY_CGPACKET pMsg;
		pMsg.header.set(0xD3, 0x7B, sizeof(pMsg));
		pMsg.ThaoTac = 1;
		DataSend((LPBYTE)& pMsg, pMsg.header.size);
	}
}
void CBAutoNapGame::ClearData()
{
	this->mCBDataLichSuNapThe.clear();
	this->mTyleNapTien = 0;
	this->mGiaTriNapThapNhat = 0;
}

void CBAutoNapGame::RecvDataListThanhToan(BYTE* recv)
{
	if (!recv) return;
	this->ClearData();
	LISTCOUNDAUTONAP_CLIENT* lpMsg = (LISTCOUNDAUTONAP_CLIENT*)recv;

	this->mTyleNapTien = lpMsg->TyleNapTien;
	this->mGiaTriNapThapNhat = lpMsg->GiaTriNapThapNhat;

	for (int n = 0; n < lpMsg->count; n++)
	{
		DataLichSuNapThe* lpInfo = (DataLichSuNapThe*)(((BYTE*)lpMsg) + sizeof(LISTCOUNDAUTONAP_CLIENT) + (sizeof(DataLichSuNapThe) * n));
		this->mCBDataLichSuNapThe.push_back(*lpInfo);

	}
	if (ShowQRAfterInput == true && !this->mCBDataLichSuNapThe.empty())
	{
		if (!gInterface.Data[eWindowQRCode].OnShow && !GetQRCode)
		{
			NumberQR = 0;
			KhoiTaoQRCodeThanhToan(gProtect.m_MainInfo.Bank_NameBank, gProtect.m_MainInfo.Bank_STK, this->mCBDataLichSuNapThe[0].TienNap, this->mCBDataLichSuNapThe[0].Checking);
			gInterface.Data[eWindowQRCode].OnShow = 1;
			gInterface.Data[eWindowQRCode].EventTick = GetTickCount();
			ShowQRAfterInput = false;
		}
	}
}




void CBAutoNapGame::DrawQRCodeThanhToan()
{
	if (!gInterface.Data[eWindowQRCode].OnShow)
	{
		if (NumberQR != -1) NumberQR = -1;
		return;

	}

	float WindowW = 160;
	float WindowH = 220;

	float StartX = (MAX_WIN_WIDTH / 2) - (WindowW / 2);
	float StartY = 45.0;
	gInterface.Data[eWindowQRCode].X = gInterface.Data[eWindowAutoBaking].X + gInterface.Data[eWindowAutoBaking].Width;
	gInterface.Data[eWindowQRCode].Y = gInterface.Data[eWindowAutoBaking].Y;
	WindowH = gInterface.Data[eWindowAutoBaking].Height;
	if ((gInterface.Data[eWindowAutoBaking].X + gInterface.Data[eWindowAutoBaking].Width + gInterface.Data[eWindowQRCode].Width) >= MAX_WIN_WIDTH)
	{
		gInterface.Data[eWindowAutoBaking].X = 640 - (gInterface.Data[eWindowAutoBaking].Width + gInterface.Data[eWindowQRCode].Width);
	}
	if (g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, WindowW, WindowH, eWindowQRCode, gOther.TextVN_NAPGAME[0]))
	{
		StartY += 10;
		float StartXInfo = StartX + 10;
		float StartYInfo = StartY + 25;
		float WInfo = (WindowW - 20);
		float HInfo = WindowH - 100;


		//===Info Bank
		TextDraw((HFONT)g_hFontBold, StartXInfo, StartYInfo, 0xFFDE26FF, 0x3a4b3978, WInfo, 0, 3, gOther.TextVN_NAPGAME[1]); //"Thông Tin");
		gInterface.DrawBarForm(StartXInfo, StartYInfo + 15, WInfo, HInfo, 0.0, 0.0, 0.0, 0.5);
		if (GetQRCode)
		{
			char buf[25];
			int step = GetTickCount() / 1000 % 4;
			sprintf(buf, "Loading data%%.%ds", step);
			TextDraw((HFONT)g_hFont, StartXInfo + 1, StartYInfo + 55, 0xffffffff, 0x0, WInfo, 0, 3, buf, "...");
		}
		else
		{

			RenderBitmap(CacheQRCode, StartXInfo + 1, StartYInfo + 15, WInfo, WInfo, 0.0, 0.0, 1.0, 1.0, 1, 1, 0.0); //QR Nap The
		}
		//==Info Bank
		TextDraw((HFONT)g_hFontBold, StartXInfo, StartYInfo + 15 + WInfo, 0xF0F0F0FF, 0x00AAFF78, WInfo, 0, 3, gOther.TextVN_NAPGAME[2], gProtect.m_MainInfo.Bank_ChuTK);
		TextDraw((HFONT)g_hFontBold, StartXInfo, StartYInfo + 15 + WInfo + 10, 0xF0F0F0FF, 0x00AAFF78, WInfo, 0, 3, gProtect.m_MainInfo.Bank_ChiNhanh);
		TextDraw((HFONT)g_hFontBold, StartXInfo, StartYInfo + 15 + WInfo + 20, 0xF0F0F0FF, 0x00AAFF78, WInfo, 0, 3, gOther.TextVN_NAPGAME[3], this->mCBDataLichSuNapThe[NumberQR].Checking);
		TextDraw((HFONT)g_hFontBold, StartXInfo, StartYInfo + 15 + WInfo + 30, 0xFFD000FF, 0x3a4b3978, WInfo, 0, 3, gOther.TextVN_NAPGAME[4]);
	}
}
void CBAutoNapGame::DrawWindowAutoBanking()
{
	if (!gInterface.Data[eWindowAutoBaking].OnShow)
	{

		if (!InputGiaTriNap)
		{
			ShowQRAfterInput = false;
			InputGiaTriNap = nullptr;
		}
		return;
	}
	float WindowW = 365;
	float WindowH = 250;
	gInterface.Data[eWindowAutoBaking].Width = WindowW;
	float StartX = (MAX_WIN_WIDTH / 2) - (WindowW / 2);
	float StartY = 25.0;
	if (g_pBCustomMenuInfo->gDrawWindowCustom(&StartX, &StartY, WindowW, WindowH, eWindowAutoBaking, gOther.TextVN_NAPGAME[5]))
	{
		StartY += 15;
		TextDraw((HFONT)g_hFontBold, StartX + 15, StartY + 25, 0x00FFDDFF, 0x0, WindowW, 0, 1, gOther.TextVN_NAPGAME[6]);
		TextDraw((HFONT)g_hFontBold, StartX + 15, StartY + 35, 0xFF9100FF, 0x0, WindowW, 0, 1, gOther.TextVN_NAPGAME[7]);
		TextDraw((HFONT)g_hFontBold, StartX + 15, StartY + 50, 0x3CFF00FF, 0x0, WindowW, 0, 1, gOther.TextVN_NAPGAME[8]);
		TextDraw((HFONT)g_hFontBold, StartX + 15, StartY + 65, 0x3CFF00FF, 0x0, WindowW, 0, 1, gOther.TextVN_NAPGAME[9]);
		float StartXInfo = StartX + 10;
		float StartYInfo = StartY + 80;
		float WInfo = (WindowW - 20) / 10;
		float HInfo = WindowH - 120;
		float TyleInfoBank = 4.0f;

		//===Info Bank
		TextDraw((HFONT)g_hFontBold, StartXInfo, StartYInfo, 0xFFDE26FF, 0x3a4b3978, WInfo * TyleInfoBank, 0, 3, gOther.TextVN_NAPGAME[10]);
		gInterface.DrawBarForm(StartXInfo, StartYInfo + 15, WInfo * TyleInfoBank, HInfo, 0.0, 0.0, 0.0, 0.5);
		//==Input Nap Tien
		float NTCoinX = StartXInfo + 55;
		float NTCoinY = StartYInfo + 45;
		float NTW = 70;
		int mGiaTriNap = atoi(GetTextGiaTriNap);
		//this->mTyleNapTien = 10000;
		if (mGiaTriNap > 0)
		{
			int GiaTriWC = static_cast<int>(mGiaTriNap * static_cast<double>(this->mTyleNapTien) / 100.0);
			TextDraw((HFONT)g_hFont, NTCoinX - 45, NTCoinY - 12 - 2, 0xFFE500FF, 0x0, 0, 0, 3, gOther.TextVN_NAPGAME[11], gInterface.NumberFormat(atoi(GetTextGiaTriNap)), gInterface.NumberFormat(GiaTriWC));
		}
		TextDraw((HFONT)g_hFont, NTCoinX - 45, NTCoinY - 2, 0xFFFFFFFF, 0x0, 0, 0, 1, gOther.TextVN_NAPGAME[12]);
		gInterface.DrawBarForm(NTCoinX - 3, NTCoinY - 2.5, NTW, 12, 1.0, 1.0, 1.0, 0.8);
		if (!InputGiaTriNap)
		{
			InputGiaTriNap = new CUITextInputBox;
			InputGiaTriNap->Init(pGameWindow, NTCoinX, NTCoinY, 8);
			InputGiaTriNap->SetBackColor(0, 0, 0, 0);
			InputGiaTriNap->SetTextColor(255, 0, 0, 0);
			InputGiaTriNap->SetFont((HFONT)g_hFont);
			InputGiaTriNap->SetState(UISTATE_NORMAL);
			InputGiaTriNap->SetOption(UIOPTION_NUMBERONLY);
			InputGiaTriNap->SetPosition(NTCoinX, NTCoinY);
			InputGiaTriNap->SetText("0");
		}
		else
		{
			InputGiaTriNap->SetPosition(NTCoinX, NTCoinY);
			InputGiaTriNap->Render();
			InputGiaTriNap->GetText(GetTextGiaTriNap, sizeof(GetTextGiaTriNap));
			CInputCaptCha->SetTabTarget(CInputCaptCha);
			if (SEASON3B::CheckMouseIn(NTCoinX - 3, NTCoinY - 2.5, NTW, 20) == 1)
			{

				if (GetKeyState(VK_LBUTTON) & 0x8000)
				{
					InputGiaTriNap->GiveFocus(1);
					PlayBuffer(25, 0, 0);
				}
			}
		}
		//==
	   //===Captcha
		TextDraw((HFONT)g_hFontBold, NTCoinX - 50, NTCoinY + 25, 0x62FF00FF, 0x0, WInfo * TyleInfoBank, 0, 3, gOther.TextVN_NAPGAME[13]);
		float CaptChaX = NTCoinX - 45;
		float CaptChaY = NTCoinY + 35;
		if (!CInputCaptCha)
		{
			CInputCaptCha = new CUITextInputBox;
			CInputCaptCha->Init(pGameWindow, CaptChaX, CaptChaY, 4);
			CInputCaptCha->SetBackColor(0, 0, 0, 0);
			CInputCaptCha->SetTextColor(255, 255, 157, 0);
			CInputCaptCha->SetFont((HFONT)g_hFont);
			CInputCaptCha->SetState(UISTATE_NORMAL);
			CInputCaptCha->SetOption(UIOPTION_NUMBERONLY);
			CInputCaptCha->SetPosition(CaptChaX, CaptChaY);
		}
		else
		{
			CInputCaptCha->SetTabTarget(InputGiaTriNap);
			CInputCaptCha->SetPosition(CaptChaX, CaptChaY);
			gInterface.RenderCaptchaNumber(CaptChaX, CaptChaY, CInputCaptCha, gInterface.vCaptcha.c_str());
			CInputCaptCha->GetText(CGetTextCaptCha, 4 + 1);
		}
		std::string CaptchaInput(CGetTextCaptCha);
		if (g_pBCustomMenuInfo->DrawButton(StartX + 57, StartY + 92, 100, 12, gOther.TextVN_NAPGAME[18], 80))
		{
			gMocDonate.OpenWindowMocNap();
		}
				
		if (g_pBCustomMenuInfo->DrawButton(CaptChaX + 22, CaptChaY + 30, 100, 12, gOther.TextVN_NAPGAME[14], 80))
		{


			if (!gInterface.check_Captcha(gInterface.vCaptcha, CaptchaInput))
			{
				gInterface.OpenMessageBox("Error", gOther.TextVN_NAPGAME[15]);
			}
			else
			{

				XULY_CGPACKET pMsg;
				pMsg.header.set(0xD3, 0x7C, sizeof(pMsg));
				pMsg.ThaoTac = atoi(GetTextGiaTriNap);
				DataSend((LPBYTE)& pMsg, pMsg.header.size);
				this->ShowQRAfterInput = true;


			}
			gInterface.vCaptcha = gInterface.generateCaptcha(4); //tao lai Captcha
		}
		//===Lich Su THanh Toan
		float LichSuX = (StartXInfo + WInfo * TyleInfoBank) + 3;
		float LichSuY = (StartYInfo);
		float TyleInfoLichSu = 5.6f;
		TextDraw((HFONT)g_hFontBold, LichSuX, LichSuY, 0xFFDE26FF, 0x3a4b3978, WInfo * TyleInfoLichSu, 0, 3, gOther.TextVN_NAPGAME[16]);
		gInterface.DrawBarForm(LichSuX, LichSuY + 15, WInfo * TyleInfoLichSu, HInfo, 0.0, 0.0, 0.0, 0.5);
		LichSuY += 17;
		float KCTextLichSu = 15;
		float Col = (WInfo * TyleInfoLichSu) / 3;
		for (int i = 0; i < this->mCBDataLichSuNapThe.size(); i++)
		{
			if (i > 8) break;
			int GiaTriNap = this->mCBDataLichSuNapThe[i].TienNap;
			char* LenhNap = this->mCBDataLichSuNapThe[i].Checking;
			TextDraw((HFONT)g_hFontBold, LichSuX, LichSuY + (i * KCTextLichSu), 0x26FFD7A4, 0x0, Col, 0, 3, gOther.TextVN_NAPGAME[17], gInterface.NumberFormat(GiaTriNap));
			TextDraw((HFONT)g_hFontBold, LichSuX + Col, LichSuY + (i * KCTextLichSu), 0xF1FF26A4, 0x0, Col + 15, 0, 3, DrawTimeUnix(atoi(LenhNap)));
			if (this->mCBDataLichSuNapThe[i].Status == 0)
			{
				TextDraw((HFONT)g_hFontBold, LichSuX + (Col * 2) + 15, LichSuY + (i * KCTextLichSu), 0xFFFFFFA9, 0xFF6314FF, Col - 15, 0, 3, mTextStatus[this->mCBDataLichSuNapThe[i].Status]);
				if (SEASON3B::CheckMouseIn(LichSuX + (Col * 2) + 15, LichSuY + (i * KCTextLichSu), Col - 15, 15) == 1)
				{
					if (GetTickCount() - gInterface.Data[eWindowQRCode].EventTick > 300) //Click
					{
						if (GetKeyState(1) & 0x8000)
						{
							PlayBuffer(25, 0, 0);

							if ((!gInterface.Data[eWindowQRCode].OnShow || NumberQR != i) && !GetQRCode)
							{
								KhoiTaoQRCodeThanhToan(gProtect.m_MainInfo.Bank_NameBank, gProtect.m_MainInfo.Bank_STK, GiaTriNap, LenhNap);
							}
							gInterface.Data[eWindowQRCode].OnShow = 1;
							gInterface.Data[eWindowQRCode].EventTick = GetTickCount();
							NumberQR = i;
						}
					}

				}
			}
			else
			{
				TextDraw((HFONT)g_hFontBold, LichSuX + (Col * 2) + 15, LichSuY + (i * KCTextLichSu), 0xFFFFFFA9, (this->mCBDataLichSuNapThe[i].Status == 1) ? 0x0084FFA9 : 0xBFBFBFA9, Col - 15, 0, 3, mTextStatus[this->mCBDataLichSuNapThe[i].Status]);
			}
		}

	}
	DrawQRCodeThanhToan();
	MainQRCodeProc();
}
