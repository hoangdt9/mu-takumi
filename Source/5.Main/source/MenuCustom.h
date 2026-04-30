#pragma once

enum ButtonMenuCustom
{
	eButtonEventTime,
	eButtonVipShop,
	eButtonRanking,
	eButtonChangeClass,
	eButtonMarKet,
	eButtonDanhHieu,
	eButtonNapGame,
	eButtonJewelBank,
	eButtonVipChar,
	eButtonHuyDongExc,
	eButtonDoiMatKhau,
	eButtonMocNap,
	eButtonLockItem,
	eButtonVQ,
	eButtonMaxValue,
};
class cCustomMenu
{
public:
	cCustomMenu();
	~cCustomMenu();
	void GetCountButton();
	void ActionButton(int TypeButton);
	void Draw();
};
extern cCustomMenu gCustomMenu;