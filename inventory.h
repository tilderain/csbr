
#ifndef _INVENTORY_H
#define _INVENTORY_H

#define MAXLISTLEN		100

struct Item
{
	int itemId;
	int ammo;
	int maxammo;
	
};

struct ShopItem
{
	int itemId;
	int price;
	int ammo;
	int maxammo;
	int flag;
	//custom script?
};


struct stSelector
{
	uchar flashstate, animtimer;
	
	int spacing_x, spacing_y;
	int cursel, lastsel;
	int sprite;
	int nitems;
	int sound;
	int rowlen;
	
	int scriptbase;
	Item items[MAXLISTLEN];
	ShopItem shopitems[8]; //max for now
	
};

struct stInventory
{
	int x, y, w, h;
	int curRow;
	int openDelayTimer;
	bool doneDrawing;
	bool exiting;
	
	int selection;
	
	stSelector itemsel;
	stSelector *curselector;
	
	char lockinput;
};

struct stShop
{
	int x, y, w, h;
	
	bool inShop;
	
	bool inBuySellSelection;
	
	bool inSell;
	
	stSelector itemsel;
	stSelector *curselector;
	
	char lockinput;
	
	int fState;
	int fTimer;
	
	int fCurSel;
	
	float sellPrice;
	
};

bool inventory_init(int param);
void inventory_tick(void);

enum INVENTORY
{
	ITEM_NONE = 0,
	ITEM_FRONTIER,
	ITEM_DOGGY_GUN,
	ITEM_FIREBALL,	//ITEM_STANTAS_KEY, //STANTAAAAA!!!!!!!!!!!!
	ITEM_DOG_VULCAN,
	ITEM_MISSILE,
	ITEM_LIFE_CAPSULE,
	ITEM_BUBBLER,
	ITEM_WAVER,
	ITEM_CYNICAL_BLADE,
	ITEM_MISSILE_2,
	ITEM_GUM_BASE,
	ITEM_CHARCOAL,
	ITEM_EXPLOSIVE,
	ITEM_PUPPY,
	ITEM_LIFE_POT,
	ITEM_CUREALL,
	ITEM_CLINIC_KEY,
	ITEM_BOOSTER08,
	ITEM_ARMS_BARRIER,
	ITEM_TURBOCHARGE,
	ITEM_AIRTANK,
	ITEM_COUNTER,
	ITEM_BOOSTER20,
	ITEM_MIMIGA_MASK,
	ITEM_TELEPORTER_KEY,
	ITEM_SUES_LETTER,
	ITEM_CONTROLLER,
	ITEM_BROKEN_SPRINKLER,
	ITEM_SPRINKLER,
	ITEM_TOW_ROPE,
	ITEM_CLAY_FIGURE_MEDAL,
	ITEM_LITTLE_MAN,
	ITEM_MUSHROOM_BADGE,
	ITEM_MA_PIGNON,
	ITEM_CURLYS_UNDERWEAR,
	ITEM_ALIEN_MEDAL,
	ITEM_CHACOS_LIPSTICK,
	ITEM_WHIMSICAL_STAR,
	ITEM_IRON_BOND
};

#endif
