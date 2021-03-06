
// the in-game inventory screen

#include "nx.h"
#include "inventory.h"
#include "inventory.fdh"
#include "math.h"

#define ARMS_X			10
#define ARMS_Y			8

#define ITEMS_X			10
#define ITEMS_Y			60

//placeholders until i get hearts working


#define FRAME_XP_MAX		3			// "MAX" when XP is at max on L3

ShopItem shopTable1[] = 
{
//	item			  		price 	ammo	maxammo flag
	 ITEM_DOGGY_GUN, 		70, 	180, 	180, 	3000,	
	 ITEM_DOGGY_GUN, 	    50, 	100, 	100, 	3001,
	 0, 					0, 		0, 		0, 		0,
	 ITEM_LIFE_POT, 		40, 	0, 		0, 		3002,	
	 
	 ITEM_FRONTIER, 		52, 	48, 	48, 	3003,
	 0, 		0, 		0, 		0, 		0,
	 0, 		0, 		0, 		0, 		0,
	 0, 		0, 		0, 		0, 		0
};

ShopItem shopTable2[] = 
{
//	 item			  		price 	ammo	maxammo flag
	 ITEM_DOGGY_GUN, 		115, 	225, 	225, 	-1,	
	 ITEM_DOG_VULCAN, 		190, 	500, 	500, 	3004,
	 ITEM_FIREBALL, 		85, 	50, 	50, 	3005,	
	 ITEM_FRONTIER, 		85, 	50, 	50, 	3006,
	 
	 0, 	0, 	0, 	0, 	0,
	 0, 	0, 	0, 	0, 	0,
	 0, 	0, 	0, 	0, 	0,
	 ITEM_LIFE_POT, 		100, 	0, 		0, 		3007
};

ShopItem* shopTableTable[] = {
	0, shopTable1, shopTable2
};


/*	[0, 		0, 	0, 	0, 	0,	
	 0,			0, 	0, 	0, 	0,
	 0,			0, 	0, 	0, 	0,	
	 0,			0, 	0, 	0, 	0,
	 0,			0, 	0, 	0, 	0,	
	 0,			0, 	0, 	0, 	0,
	 0,			0, 	0, 	0, 	0,	
	 0,			0, 	0, 	0, 	0],*/

// can't enter Inven if
//  * script is running
//  * fade is in progress
//  * player is dead

//int invWeapons = 

//

// param is passed as 1 when returning from Map System.

int itemToBeThrown;

int shopBoxSlideY;

bool shopBuySellExiting;

stInventory inv;
stShop shop;
bool inventory_init(int param)
{
	//param == 2 then do shop things
	if (param >= 1) {
		memset(&inv, 0, sizeof(inv));
		memset(&shop, 0, sizeof(shop));
		
		inv.selection = -1;
		shop.curselector = &shop.itemsel;
		shop.itemsel.cursel = RefreshShopScreen();
		shop.curselector->lastsel = -9999;		// run the script first time
		shop.inShop = true;
		stat("Entering shop # %d.", param);

		memcpy(shop.itemsel.shopitems, shopTableTable[param], sizeof(shop.itemsel.shopitems));
		shop.inBuySellSelection = true;
		
	} else {
		memset(&inv, 0, sizeof(inv));
		memset(&shop, 0, sizeof(shop));
		
		inv.curselector = &inv.itemsel;
		inv.itemsel.cursel = RefreshInventoryScreen();
		inv.selection = -1;
		inv.curselector->lastsel = -9999;		// run the script first time
		
		inv.openDelayTimer = 7; //a pointless delay to make things feel more... accurate.
		
		inv.lockinput = 1;
	
		// returning from map system?
		if (param == -1)
		{
			inv.curselector = &inv.itemsel;
			
			// highlight Map System
			for(int i=0;i<inv.itemsel.nitems;i++)
			{
				if (inv.itemsel.items[i].itemId == 2)
				{
					inv.curselector->cursel = i;
					// textbox NOT up until they move the selector
					inv.curselector->lastsel = i;
					break;
				}
			}
		}
	}
	
	return 0;
}


void inventory_tick(void)
{
	// run the selectors
	//if we set param 2 do shop things instead.
	if (shop.inShop)
	{
		DrawScene();
		if (!shop.inBuySellSelection)
			{
			if (!shop.inSell)
			{
				DrawShop();
				RunShopSelector(shop.curselector);
			}
			else 
			{
				DrawSell();
				RunSellSelector(shop.curselector);
			}
			textbox.Draw();
		} 
		else 
		{
			DrawBuySellSelection();
		}
	}
	else 
	{
		if (!inv.exiting && inv.doneDrawing) RunSelector(inv.curselector);
		// draw
		DrawScene();
		//DrawBossBar(); looks ugly, but should it be enabled, because it's accurate to the beta?
		DrawInventory();
		if (inv.doneDrawing) textbox.Draw();
	}
}

/*
void c------------------------------() {}
*/


// reload which items & guns are available.
// reset the cursor for the current selector.
// return the slot corresponding to the current weapon.
int RefreshInventoryScreen(void)
{
int i;
int curwpn = 0;
	
	if (game.mode != GM_INVENTORY)
		return 0;

	// setup itemsel...
	inv.itemsel.nitems = player->ninventory;
	//inv.itemsel.items[0] = 0;		// show "no item" in case of no items
	for(i=0;i<player->ninventory;i++)
		inv.itemsel.items[i].itemId = player->inventory[i].itemId;
	
	inv.itemsel.spacing_x = 56;
	inv.itemsel.spacing_y = sprites[SPR_ITEMIMAGE].h ;//16
	inv.itemsel.sprite = SPR_YESNOHAND;
	inv.itemsel.sound = SND_MENU_MOVE;
	inv.itemsel.rowlen = 4;
	inv.itemsel.scriptbase = 5000;
	
	// after an item has been used don't bring up the script of whatever item
	// the selector is moved to
	return curwpn;
}

void UnlockInventoryInput(void)
{
	if (inv.lockinput)
		inv.lockinput = false;
}

/*
void c------------------------------() {}
*/

static void DrawInventory(void)
{
int x, y, w, i, c;

	// draw the box
		
	//inv.w = 84;
	//inv.h = 32;
	inv.x = MSG_X;
	inv.y = (SCREEN_HEIGHT / 2) - (sprites[SPR_INVENTORY_SCREEN].h / 2) + 1;
	static const int ROW_SPEED = 18;

	if (!inv.doneDrawing)
	{
		inv.lockinput = 1;
		if (justpushed(INVENTORYKEY)) // amazing parity with map system
		{
			inv.exiting = true;
		}
		if (inv.exiting)
		{
			inv.curRow -= ROW_SPEED;
			if (inv.curRow <= 0)
			{
				inv.curRow = 0;
				ExitInventory();
			}
		}
		else 
		{
			if(!inv.openDelayTimer)
			{
				inv.curRow += ROW_SPEED;
				if (inv.curRow >= sprites[SPR_INVENTORY_SCREEN].h)
				{
					inv.curRow = sprites[SPR_INVENTORY_SCREEN].h;
					inv.doneDrawing = true;
				}
			} 
			else 
			{
				DrawStatusBar();
				--inv.openDelayTimer;
			}
		}
		set_clip_rect(inv.x, inv.y, \
		sprites[SPR_INVENTORY_SCREEN].w, inv.curRow);
		
		draw_sprite(inv.x, inv.y, SPR_INVENTORY_SCREEN, 0, 0);
		
		DrawInventoryBits();
		
		clear_clip_rect();
	}
/* 	TextBox::DrawFrame(inv.x, inv.y, inv.w, inv.h);
	inv.y += 32;
	TextBox::DrawFrame(inv.x, inv.y, inv.w, inv.h);
	inv.x += 160;
	inv.h = 24;
	TextBox::DrawFrame(inv.x, inv.y, inv.w, inv.h);
	inv.y -= 32;
	TextBox::DrawFrame(inv.x, inv.y, inv.w, inv.h);
	inv.x -= 72;
	inv.w = 68;
	inv.h = 64;
	TextBox::DrawFrame(inv.x, inv.y, inv.w, inv.h);
	inv.x -= 88;
	inv.y += 64;
	inv.w = 244;
	inv.h = 80;
	TextBox::DrawFrame(inv.x, inv.y, inv.w, inv.h); */
	if (inv.doneDrawing)
	{
		draw_sprite(inv.x, inv.y, SPR_INVENTORY_SCREEN, 0, 0);
		
		DrawInventoryBits();
		
		// - draw the health ----

		int HEALTH_X			= inv.x + 170; //38
		int HEALTH_Y			= inv.y + 9; // 8
				
		DrawHealthBar(HEALTH_X, HEALTH_Y, player->hp, player->maxHealth);
		// - draw the money ----
		
		draw_sprite(inv.x + 205, inv.y + 42, SPR_XPBAR, FRAME_XP_MAX, 0);
		// cion Number
		DrawNumber(HEALTH_X - 1, inv.y + 41, player->xp); // 36 difference of x
		
	}
}

static void DrawInventoryBits(void)
{
		int INVWEAPON_X = inv.x + 16;
		int INVWEAPON_Y = inv.y + 8;

		DrawItems(INVWEAPON_X, inv.y + 72, &inv.itemsel, 16, false);

		//draw WEAPON
		if (player->curWeapon)
		{
			draw_sprite(INVWEAPON_X, INVWEAPON_Y, SPR_ITEMIMAGE, player->curWeapon, 0);
			DrawNumber(INVWEAPON_X +17, INVWEAPON_Y + 8, player->weapons[player->curWeapon].ammo);
		}
		
		// - draw the player ----
		
		DrawSelector(&inv.itemsel, inv.x + 16, inv.y + 72);
		
		int s, spr, frame;
		s = PSelectSprite();
		if (player->curWeapon != WPN_NONE)
		{
				GetSpriteForGun(player->curWeapon, 0, &spr, &frame);
				if(s == SPR_MYCHAR_FROG)
				{
					draw_sprite_at_dp(inv.x + 119, inv.y + 37, spr, frame, LEFT);	
				} 
				else 
				{
					draw_sprite_at_dp(inv.x + 119, inv.y + 37, spr, frame, LEFT);	
				}
				
		}
		draw_sprite(inv.x + 114, inv.y + 24, s, 0, LEFT);
		
		if (player->equipmask)
		{
			draw_sprite(INVWEAPON_X, inv.y + 40, SPR_ITEMIMAGE, GetPlug(), 0);
		}
		
}


static void RunSelector(stSelector *selector){
int nrows;
int currow, curcol;

	if (inv.lockinput)
	{
		switch(GetCurrentScript()){ //hacks
			case -1: inv.lockinput = 0; break;
			case 1302: 					   //no for throwing away   
				itemToBeThrown = -1;
				inv.selection = -1; break;
			case 1303: ThrowItem(); break; //yes
			case 8015: itemToBeThrown = inv.itemsel.cursel; ThrowItem(); break; //hacky
			default: return;
		}
	}
	
	if (selector->nitems)
	{
		nrows = (selector->nitems - 1) / selector->rowlen;
		currow = (selector->cursel / selector->rowlen);
		curcol = (selector->cursel % selector->rowlen);
	}
	else
	{
		nrows = currow = curcol = 0;
	}
	
	if (justpushed(LEFTKEY))
	{
		sound(selector->sound);
		
		// at beginning of row?
		if (curcol == 0)
		{	// wrap to end of row
			if (currow < nrows)
				selector->cursel += (selector->rowlen - 1);
			else if (selector->nitems > 0)
				selector->cursel = selector->nitems - 1;
		}
		else selector->cursel--;
	} 
	else if (justpushed(RIGHTKEY))
	{
		sound(selector->sound);
		if (selector->cursel == 16){
			selector->cursel = 3;
		}
		else if (curcol==selector->rowlen-1 || selector->cursel+1 >= selector->nitems)
		{	// wrap to beginning of row
			selector->cursel = (currow * selector->rowlen);
		}
		else selector->cursel++;
	} 
	else if (justpushed(DOWNKEY))
	{
		if (selector->cursel == 16)
		{
			selector->cursel = 17;
			sound(selector->sound);
		} 
		else if (selector->cursel == 17) 
		{
			selector->cursel = 0;
			sound(selector->sound);
		} 
		else 
		{
			if (currow == 3 && curcol == 0)
			{
				selector->cursel = 16;
				sound(selector->sound);
			}
			else
			{
				if (currow == 3)
				{
					selector->cursel -= 12;
					sound(selector->sound);
				}
				else 
				{
				selector->cursel += selector->rowlen;
				sound(selector->sound);
				}
			}
		}
	}
	else if (justpushed(UPKEY))
	{
		//need to account for weapon and plug
		//holy hell this is a mess
		if (selector->cursel == 16) 
		{
			selector->cursel = 12;
			sound(selector->sound);
		} 
		else if (selector->cursel == 17) 
		{
			selector->cursel = 16;
			sound(selector->sound);
		} 
		else if (currow == 0)
		{
			if (curcol == 0)
			{
				selector->cursel = 17;
				sound(selector->sound);
			} 
			else 
			{
				selector->cursel += 12; //this will probably never ever change
				sound(selector->sound);
			}
		}
		else
		{
			selector->cursel -= selector->rowlen;
			sound(selector->sound);
		}
	}

	// bring up scripts
	if (selector->cursel != selector->lastsel)
	{
		selector->lastsel = selector->cursel;
		
		switch(selector->cursel)
		{
			case 16: StartScript(player->curWeapon + selector->scriptbase, SP_ARMSITEM); break;
		    case 17: StartScript(GetPlug() + selector->scriptbase, SP_ARMSITEM); break;
			default: StartScript(selector->items[selector->cursel].itemId + selector->scriptbase, SP_ARMSITEM);
		}
	}
	
	else									// selecting an item
	{
		if (justpushed(JUMPKEY))
		{
			stat("selector %d", selector->cursel);
			if (inv.selection == -1)
			{
				inv.selection = selector->cursel;
				sound(SND_MENU_SELECT);
			} 
			else
			{
				//swap items
				//special behavior for trash and plug and weapon
				//please clean this up
				//ever heard of a switch case?
				//have to handle hasweapon or remove it
				if (inv.selection == 15 || selector->cursel == 15){ //trash
					if (inv.selection == 15){
						SwapTrash(inv.selection, selector->cursel);
					} else {
						SwapTrash(selector->cursel, inv.selection);
					}
				} else if (inv.selection == 16 || selector->cursel == 16){ //WEAPON
					if (inv.selection == 17 || selector->cursel == 17){
						sound(SND_BONK_HEAD);
					} else if (inv.selection == 16){
						SwapWeapon(inv.selection, selector->cursel);
						StartScript(selector->items[selector->cursel].itemId + selector->scriptbase, SP_ARMSITEM);
					} else { 
						SwapWeapon(selector->cursel, inv.selection);
						StartScript(player->curWeapon + selector->scriptbase, SP_ARMSITEM);
					}
				} else if (inv.selection == 17 || selector->cursel == 17) { //PLUG
					if (inv.selection == 16 || selector->cursel == 16) {
						sound(SND_BONK_HEAD);
					} else if (inv.selection == 17){
						SwapPlug(inv.selection, selector->cursel);
						StartScript(selector->items[selector->cursel].itemId + selector->scriptbase, SP_ARMSITEM);
					} else {
						SwapPlug(selector->cursel, inv.selection);
						StartScript(GetPlug() + selector->scriptbase, SP_ARMSITEM);
					}
				} else { //regular
					SwapItem(inv.selection, selector->cursel);
					StartScript(selector->items[selector->cursel].itemId + selector->scriptbase, SP_ARMSITEM);
				}
			}
		}

		if (justpushed(FIREKEY)) 
		{
			// bring up "more info" or "equip" script for this item
			switch(selector->cursel){
				case 16: StartScript(player->curWeapon + selector->scriptbase + 1000, SP_ARMSITEM); 
						 inv.lockinput = 1; break;
				case 17: StartScript(GetPlug() + selector->scriptbase + 1000, SP_ARMSITEM); 
						 inv.lockinput = 1; break;
				default: StartScript(selector->items[selector->cursel].itemId + selector->scriptbase + 1000, SP_ARMSITEM);
						 inv.lockinput = 1;
			}
		}
		if (justpushed(INVENTORYKEY)) 
		{
			inv.exiting = true;
			inv.doneDrawing = false;
		}
		
	}
}

/* what  i want the shop to do
allow you to see your current inventory
allow you to see the shop at the top
allow you to see money
allow you to purchase items

it must include: the custom shop textbox
*/

int RefreshShopScreen(void)
{
int i;
int curwpn = 0;
	
	if (game.mode != GM_INVENTORY)
		return 0;

	// setup itemsel...
	shop.itemsel.nitems = 8;
	//inv.itemsel.items[0] = 0;		// show "no item" in case of no items
	for(i=0;i<player->ninventory;i++)
		shop.itemsel.items[i].itemId = player->inventory[i].itemId;
	
	shop.itemsel.spacing_x = 56;
	shop.itemsel.spacing_y = sprites[SPR_ITEMIMAGE].h ;//16
	shop.itemsel.sprite = SPR_YESNOHAND;
	shop.itemsel.sound = SND_MENU_MOVE;
	shop.itemsel.rowlen = 4;
	shop.itemsel.scriptbase = 5000;
	
	// after an item has been used don't bring up the script of whatever item
	// the selector is moved to
	return curwpn;
}

static void DrawShop(void){
	int x, y, w, i, c;
	bool shop_;

	int inv_basex = MSG_X;
	int inv_basey = (SCREEN_HEIGHT / 2) - (sprites[SPR_INVENTORY_SCREEN].h / 2) + 1;
	
	TextBox::DrawFrame(inv_basex, inv_basey + 64 + shopBoxSlideY, 244, 80, shop_); 
	draw_sprite(inv_basex, inv_basey + 64 + shopBoxSlideY, SPR_SHOP_ITEM, 0, 0);
	
	TextBox::DrawFrame(inv_basex, inv_basey + 24 + shopBoxSlideY, 244, 40, shop_); 
	draw_sprite(inv_basex, inv_basey + 24 + shopBoxSlideY, SPR_SHOP_SHOP, 0, 0);
	
	TextBox::DrawFrame(inv_basex + 160, inv_basey + shopBoxSlideY, 84, 24, shop_); 
	draw_sprite(inv_basex + 160, inv_basey + shopBoxSlideY, SPR_SHOP_MONEY, 0, 0);

	//set_clip_rect(38, 32 + shopBoxSlideY, \
		244, 38);
	
	DrawItemsShop(inv_basex + 16, inv_basey + 29 + shopBoxSlideY, &shop.itemsel, 8); //y is added twice, was unintentional but is a cool effect
	
	//clear_clip_rect();
		
	DrawSelector(&shop.itemsel, inv_basex + 16, inv_basey + 29 + shopBoxSlideY);
	
	DrawItems(inv_basex + 16, inv_basey + 72, &shop.itemsel, 16 + shopBoxSlideY, true);
	
	//magic numbers everywhere
	// - draw the money ----
	
	draw_sprite(inv_basex + 205, inv_basey + 10 + shopBoxSlideY, SPR_XPBAR, FRAME_XP_MAX, 0);
	// cion Number
	DrawNumber(inv_basex + 169, inv_basey + 9 + shopBoxSlideY, player->xp);
	
	shopBoxSlideY /= 1.3;
}

void DrawItems(int xstart, int ystart, stSelector *selector, int nitems, bool inShop){ 
//pass in the selctor cuz apparently you can't pass in an *item
int i;
	int c = 0;
	int x = xstart;
	int y = ystart + shopBoxSlideY;

	for(i=0;i<nitems;i++){
		if ( (i!=15) ){
			draw_sprite(x, y, SPR_ITEMIMAGE, selector->items[i].itemId, 0);
		} else {
			if(inShop)
			{
				if(shop.inSell)
				{
					draw_sprite(x, y, SPR_ITEMIMAGE, 42, 0); //exit
				}
				else
				{
					draw_sprite(x, y, SPR_ITEMIMAGE, 40, 0); //trash... placeholder for now?
				}
			}
		}
		
		if (selector->items[i].itemId && player->inventory[i].maxammo == 0) { 
			if(settings->theme[THEME_INVNUM]){
				DrawNumberGray(x+17, y+8, 1);
			}
		} else if (selector->items[i].itemId){
			DrawNumberGray(x+17, y+8, player->inventory[i].ammo); //weird position
			//add support for infinity symbol?
		}
	
		x += selector->spacing_x;
		
		if (++c >= selector->rowlen){
			x = xstart;
			y += selector->spacing_y;
			c = 0;
		}
	}
}

void DrawItemsShop(int xstart, int ystart, stSelector *selector, int nitems){ 
//pass in the selctor cuz apparently you can't pass in an *item
int i;
	int c = 0;
	int x = xstart;
	int y = ystart + shopBoxSlideY;
	
	for(i=0;i<8;i++){
		if (selector->shopitems[i].itemId && game.flags[selector->shopitems[i].flag]){
			draw_sprite(x, y, SPR_ITEMIMAGE, 41, 0); //out of stock sign
		} else {
			
			draw_sprite(x, y, SPR_ITEMIMAGE, selector->shopitems[i].itemId, 0);
		
			if (selector->shopitems[i].itemId && selector->shopitems[i].maxammo == 0) { 
				if(settings->theme[THEME_INVNUM]){
					DrawNumberGray(x+17, y+8, 1);
				}
			} else if (selector->shopitems[i].itemId){
				DrawNumber(x+17, y+8, selector->shopitems[i].ammo); //weird position
				//add support for infinity symbol?
			}
		}
		x += selector->spacing_x;
		
		if (++c >= selector->rowlen){
			x = xstart;
			y += selector->spacing_y;
			c = 0;
		}
	}
}


static void RunShopSelector(stSelector *selector){
int nrows;
int currow, curcol;
int shopBuy;
	if (shop.lockinput)
	{
		switch(GetCurrentScript()){ //hacks
			case -1: shop.lockinput = 0; break;
			case 1401:	 break;//didn't buy it.
			case 1402:  //bought it
				//change itemid to out of stock? set flags?  break;//buy the item, then do shopkeep dialogue 
				if (selector->shopitems[selector->cursel].price > player->xp){ //bypass ci- cmd, too finnicky
					shop.lockinput = 1;
					StartScript(1510 + random(0,2), SP_ARMSITEM);//not enough money
					RefreshShopScreen();
				} else if (FindInventory(0) == -1) {
					shop.lockinput = 1;
					StartScript(1520 + random(0,2), SP_ARMSITEM);//full
					RefreshShopScreen();
				} else {
					shop.lockinput = 1;
					player->xp -= selector->shopitems[selector->cursel].price;
					shopBuy = FindInventory(0);
					player->inventory[shopBuy].itemId = selector->shopitems[selector->cursel].itemId;
					player->inventory[shopBuy].maxammo = selector->shopitems[selector->cursel].maxammo;
					player->inventory[shopBuy].ammo = selector->shopitems[selector->cursel].ammo;
					if (selector->shopitems[selector->cursel].flag != -1){
						game.flags[selector->shopitems[selector->cursel].flag] = 1;
					}
					StartScript(1500 + random(0,2), SP_ARMSITEM);//buy
					RefreshShopScreen();
				} break;

			default: return;
		}
	}
	
	if (selector->nitems)
	{
		nrows = (selector->nitems - 1) / selector->rowlen;
		currow = (selector->cursel / selector->rowlen);
		curcol = (selector->cursel % selector->rowlen);
	}
	else
	{
		nrows = currow = curcol = 0;
	}
	
	if (justpushed(LEFTKEY)){
		sound(selector->sound);
		// at beginning of row?
		if (curcol == 0)
		{	// wrap to end of row
			if (currow < nrows)
				selector->cursel += (selector->rowlen - 1);
			else if (selector->nitems > 0)
				selector->cursel = selector->nitems - 1;
		}
		else selector->cursel--;
	} else if (justpushed(RIGHTKEY)){
		sound(selector->sound);
		// at end of row?
		if (curcol==selector->rowlen-1 || selector->cursel+1 >= selector->nitems)
		{	// wrap to beginning of row
			selector->cursel = (currow * selector->rowlen);
		}
		else selector->cursel++;
	} else if (justpushed(DOWNKEY)){
		if (currow == 1){
			selector->cursel -= 4;
			sound(selector->sound);
		} else {
		selector->cursel += selector->rowlen;
		sound(selector->sound);
		}
	} else if (justpushed(UPKEY)) {
		if (currow == 0){
			selector->cursel += 4; //this will probably never ever change
			sound(selector->sound);
		} else {
			selector->cursel -= selector->rowlen;
			sound(selector->sound);
		}
	}

	// bring up scripts
	if (selector->cursel != selector->lastsel)
	{
		selector->lastsel = selector->cursel;
		//probably just keep as is
		if (selector->shopitems[selector->cursel].itemId && game.flags[selector->shopitems[selector->cursel].flag]){
			StartScript(5041, SP_ARMSITEM); //out of stock
		} else {
			StartScript(selector->shopitems[selector->cursel].itemId + selector->scriptbase, SP_ARMSITEM); 
		}
	}
	
	if (justpushed(JUMPKEY)){
		//bring up shop dialogue, then do purchase
		lastammoinc = selector->shopitems[selector->cursel].price;
		if (selector->shopitems[selector->cursel].itemId && game.flags[selector->shopitems[selector->cursel].flag]){
			StartScript(1700 + random(0,3), SP_ARMSITEM); // out of stock
		} else {
			StartScript(1400, SP_ARMSITEM);
		}
		shop.lockinput = 1;
	}
	if (justpushed(FIREKEY)) 
	{
		if (selector->shopitems[selector->cursel].itemId && game.flags[selector->shopitems[selector->cursel].flag]){
			StartScript(6041, SP_ARMSITEM); //out of stock
		} else {
			StartScript(selector->shopitems[selector->cursel].itemId + selector->scriptbase + 1000, SP_ARMSITEM);
		}
	}
	if (justpushed(INVENTORYKEY)) 
	{
		shop.inBuySellSelection = true; 
		//then go to shop exit script??
		shop.fState = 2; //STATE_SELECTION
		shopBoxSlideY = -36;
	}
}

enum
{
	STATE_APPEAR,
	STATE_WAIT,
	STATE_SELECTION
};

static void DrawBuySellSelection(void){
bool shop_;

	if (shop.fState == STATE_APPEAR)
	{
		shop.fState = STATE_WAIT;
		shopBoxSlideY = 360;
		shop.fTimer = 10;
	}
	
	inv.w = 68;
	inv.h = 75;
	
	inv.x = (SCREEN_WIDTH / 2) - (inv.w / 2);
	inv.y = (SCREEN_HEIGHT / 2) - (inv.h / 2);
	

	TextBox::DrawFrame(inv.x, inv.y + shopBoxSlideY, inv.w, inv.h, shop_); 

	int INVWEAPON_X = inv.x + 16;
	int INVWEAPON_Y = inv.y + 8;
	
	font_draw(inv.x + 12, INVWEAPON_Y + shopBoxSlideY, "Buy", 0, &shadowfont2);
	
	font_draw(inv.x + 12, inv.y + 24 + shopBoxSlideY, "Sell", 0, &shadowfont2);
	
	font_draw(inv.x + 12, inv.y + 40 + shopBoxSlideY, "Talk", 0, &shadowfont2);
	
	font_draw(inv.x + 12, inv.y + 56 + shopBoxSlideY, "Exit", 0, &shadowfont2);

	// draw hand selector
	if (shop.fState == STATE_SELECTION)
	{
		
		int yoff = 8 + (16 * shop.fCurSel);
		draw_sprite(inv.x - 4, inv.y+yoff + shopBoxSlideY, SPR_YESNOHAND, 0, 0);
	}
	
	switch(shop.fState)
	{
		case STATE_WAIT:
		{
			if (shop.fTimer)
			{
				shop.fTimer--;
				break;
			}
			
			shop.fState = STATE_SELECTION;
		}
		break;
		
		case STATE_SELECTION:
		{
			shopBoxSlideY /= 1.5;
			
			if (justpushed(UPKEY))
			{
				sound(SND_MENU_MOVE);
				shop.fCurSel -= 1;
				
				if (shop.fCurSel == 2) shop.fCurSel = 1; //talk not implemented
				if (shop.fCurSel < 0) shop.fCurSel = 3; //wraparound
			}
			if(justpushed(DOWNKEY))
			{
				sound(SND_MENU_MOVE);
				shop.fCurSel += 1;
				
				if (shop.fCurSel == 2) shop.fCurSel = 3; //talk not implemented
				if (shop.fCurSel > 3) shop.fCurSel = 0; //wraparound
			}
			
			shop.fState = shop.fState % 4;
			
			if (justpushed(JUMPKEY))
			{
				sound(SND_MENU_SELECT);
				lastinputs[JUMPKEY] = true;
				lastpinputs[JUMPKEY] = true;
				
				if (shop.fCurSel == 3) 
				{ 
					shopBuySellExiting = true;
					shop.fState = STATE_WAIT;
					shop.fTimer = 999; break; 
				}
				
				shop.inBuySellSelection = false;
				shop.inSell = (shop.fCurSel == 0) ? false : true;
				
				shopBoxSlideY = 36; //set the slide for both buy and sell
			}

			if (justpushed(FIREKEY) || justpushed(INVENTORYKEY))
			{			
				shopBuySellExiting = true;
				shop.fState = STATE_WAIT;
				shop.fTimer = 999;
			}
		}
		break;
	}
	
	if(shopBuySellExiting)
	{
		if (shopBoxSlideY < 0) shopBoxSlideY = 0;
		shopBoxSlideY += 1;
		shopBoxSlideY *= 2;
		if (shopBoxSlideY >= 200) 
		{
			shopBuySellExiting = false;
			ExitInventory();
		}
	}
}


static void DrawSell(void){
	
	int x, y, w, i, c;
	bool shop_;
	//money box

	int inv_basex = MSG_X;
	int inv_basey = (SCREEN_HEIGHT / 2) - (sprites[SPR_INVENTORY_SCREEN].h / 2) + 1;

	inv.x = inv_basex + 160;
	inv.y = inv_basey + 32 + shopBoxSlideY;
	
	// - draw the money ----
	draw_sprite(inv.x, inv.y, SPR_SHOP_MONEY, 0, 0);
	
	draw_sprite(inv.x + 45, inv.y + 10, SPR_XPBAR, FRAME_XP_MAX, 0);
	// cion Number
	DrawNumber(inv.x + 11 , inv.y + 9, player->xp);
	
	//inventory box
	
	inv.x = inv_basex;
	inv.y = 72 + shopBoxSlideY;
	
	inv.w = 244;
	inv.h = 80;
	
	TextBox::DrawFrame(inv.x, inv.y, inv.w, inv.h, shop_); 
	draw_sprite(inv.x, inv.y, SPR_SHOP_ITEM, 0, 0);
	
	//position of items
	inv.x = inv_basex + 16;
	inv.y = inv_basey + 73;
	
	DrawItems(inv.x, inv.y + shopBoxSlideY, &shop.itemsel, 16, true);

	DrawSelector(&shop.itemsel, inv.x, inv.y + shopBoxSlideY);
	
	shopBoxSlideY /= 1.3;

}

float pricePerAmmo[] =
{
	0, //nothing
	0.7, //frontier
	0.3, //doggy
	0.8, //fireball
	0.3, //vulcan
	1.5, //missile
	475, //life capsule
	0.65, //bubbler
	0.66, //waver
	4, //cynical blade
	5, //superb missle
	0, //superb missile ammo
	666, //charcoal
	9999, //explosive
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
};


static void RunSellSelector(stSelector *selector){
int nrows;
int currow, curcol;
int shopBuy;

	shop.itemsel.nitems = 16; //everything i do is a hack
	
	if (shop.lockinput)
	{
		switch(GetCurrentScript()){ //hacks
			case -1: shop.lockinput = 0; break;
			case 1602:	 break;//didn't buy it.
			case 1603:  //bought it
				//change itemid to out of stock? set flags?  break;//buy the item, then do shopkeep dialogue 
				shop.lockinput = 1;
				player->xp += shop.sellPrice;
				
				player->inventory[selector->cursel].itemId = 0;
				player->inventory[selector->cursel].maxammo = 0;
				player->inventory[selector->cursel].ammo = 0;
				StartScript(1610 + random(0,3), SP_ARMSITEM);//sell
				RefreshShopScreen();
				break;

			default: return;
		}
	}
	
	if (selector->nitems)
	{
		nrows = (selector->nitems - 1) / selector->rowlen;
		currow = (selector->cursel / selector->rowlen);
		curcol = (selector->cursel % selector->rowlen);
	}
	else
	{
		nrows = currow = curcol = 0;
	}
	
	if (justpushed(LEFTKEY)){
		sound(selector->sound);
		// at beginning of row?
		if (curcol == 0)
		{	// wrap to end of row
			if (currow < nrows)
				selector->cursel += (selector->rowlen - 1);
			else if (selector->nitems > 0)
				selector->cursel = selector->nitems - 1;
		}
		else selector->cursel--;
	} else if (justpushed(RIGHTKEY)){
		sound(selector->sound);
		// at end of row?
		if (curcol==selector->rowlen-1 || selector->cursel+1 >= selector->nitems)
		{	// wrap to beginning of row
			selector->cursel = (currow * selector->rowlen);
		}
		else selector->cursel++;
	} else if (justpushed(DOWNKEY)){
		if (currow == 3){
			selector->cursel -= 12;
			sound(selector->sound);
		} else {
		selector->cursel += selector->rowlen;
		sound(selector->sound);
		}
	} else if (justpushed(UPKEY)) {
		if (currow == 0){
			selector->cursel += 12; //this will probably never ever change
			sound(selector->sound);
		} else {
			selector->cursel -= selector->rowlen;
			sound(selector->sound);
		}
	}

	// bring up scripts
	
	
	if (!player->inventory[selector->cursel].maxammo && pricePerAmmo[selector->items[selector->cursel].itemId]){
		shop.sellPrice = pricePerAmmo[selector->items[selector->cursel].itemId];
	} else {
		shop.sellPrice = pricePerAmmo[selector->items[selector->cursel].itemId] * (float)player->inventory[selector->cursel].ammo;
	}
	
	lastammoinc = shop.sellPrice;
	
	if (selector->cursel != selector->lastsel){
		selector->lastsel = selector->cursel;
		//probably just keep as is
		if (shop.sellPrice != 0){
			StartScript(1600, SP_ARMSITEM);
		} else {
			StartScript(0, SP_ARMSITEM);
		}
	}
	
	if (justpushed(JUMPKEY)){
		//bring up shop dialogue, then do purchase
		if (shop.sellPrice != 0) {
			StartScript(1601, SP_ARMSITEM);
			shop.lockinput = 1;
		} //else junk dialogue
		if (selector->cursel == 15) goto exitsell; //????????
		
	}
	if (justpushed(FIREKEY) || justpushed(INVENTORYKEY)) 						//StartScript(selector->items[selector->cursel].itemId + selector->scriptbase + 1000, SP_ARMSITEM);
	{
exitsell:
		shop.inBuySellSelection = true; 
		//then go to shop exit script??
		shop.fState = 2; //STATE_SELECTION
		shopBoxSlideY = -36;
		selector->cursel = 0; //prevents crazy stuff happening in the buy menu
	}
}

enum SWAP
{
	SWAP_NONE = 0,
	SWAP_WEAPON = 1,
	SWAP_PLUG = 2,
};


void SwapItem(int loc1, int loc2){
Item temp;
	temp = player->inventory[loc1];
	player->inventory[loc1] = player->inventory[loc2];
	player->inventory[loc2] = temp;
	
	sound(SND_MENU_SELECT);
	inv.selection = -1;
	RefreshInventoryScreen();
}

void SwapWeapon(int loc1, int loc2){
	//loc1 should always be 16
	if (loc1 == loc2) {
		sound(SND_MENU_SELECT);
		inv.selection = -1;
		RefreshInventoryScreen();
		return;
	}
	if (player->inventory[loc2].maxammo || player->inventory[loc2].itemId == 0){
		Item temp;
		temp.itemId = player->curWeapon;
		temp.ammo = player->weapons[player->curWeapon].ammo;
		temp.maxammo = player->weapons[player->curWeapon].maxammo;
		
		player->curWeapon = player->inventory[loc2].itemId;
		player->weapons[player->curWeapon].ammo = player->inventory[loc2].ammo;
		player->weapons[player->curWeapon].maxammo = player->inventory[loc2].maxammo;
		
		player->inventory[loc2] = temp;
		
		sound(SND_MENU_SELECT);
		inv.selection = -1;
		RefreshInventoryScreen();
		return;
	}
	sound(SND_BONK_HEAD);
	return;
}

int equipidlist[] =
{
	18, //booster08		0x01
	40, //map			0x02 //placeholder
	19, //armsbarrier	0x04
	20, //turbocharge	0x08
	21, //airtank		0x10
	23, //booster20		0x20
	24, //mimigamask	0x40
	38, //whimstar		0x80
	22, //nikumaru		0x100
};

void SwapPlug(int loc1, int loc2){
int i, mask;
	if (loc1 == loc2){
		sound(SND_MENU_SELECT);
		inv.selection = -1;
		return;
	}
	mask = 0x01;
	for(i=0;equipidlist[i];i++){
		if (player->inventory[loc2].itemId == 0){
			player->inventory[loc2].itemId = GetPlug();
			player->equipmask = 0x00;
			sound(SND_MENU_SELECT);
			inv.selection = -1;
			RefreshInventoryScreen();
			return;
		}
		if (equipidlist[i] == player->inventory[loc2].itemId){
			if (player->equipmask) {
				player->inventory[loc2].itemId = GetPlug();
			} else {
				player->inventory[loc2].itemId = 0;
			}
			player->equipmask = mask;
			sound(SND_MENU_SELECT);
			inv.selection = -1;
			RefreshInventoryScreen();
			return;
		}
		
		mask <<= 1;
	}
	sound(SND_BONK_HEAD);
}

int GetPlug(){
int i;
int plugId, mask;
	if (!player->equipmask) return 0;
	mask = 0x01;
	for(i=0;equipidlist[i];i++)
	{
		if (mask == player->equipmask)
		{
			plugId = equipidlist[i];
		}
		
		mask <<= 1;
	}
	return plugId;
}

int forbiddenItems[] = {
	
};


void SwapTrash(int loc1, int loc2){
int i;
	for(i=0;forbiddenItems[i];i++);{
		if (player->inventory[loc2].itemId == forbiddenItems[i]){
			//Can't throw this away!
			StartScript(1300, SP_ARMSITEM);
			inv.lockinput = 1;
			inv.selection = -1;
			RefreshInventoryScreen();
			return;
		}
	}
	itemToBeThrown = loc2;
	StartScript(1301, SP_ARMSITEM);
	inv.lockinput = 1;
}

void ThrowItem(){
	if (itemToBeThrown != -1){
		if (itemToBeThrown == 16){
			player->weapons[player->curWeapon].ammo = 0;
			player->weapons[player->curWeapon].maxammo = 0;
			player->curWeapon = 0;
		} else if (itemToBeThrown == 17){
			player->equipmask = 0;
		} else {
			player->inventory[itemToBeThrown].itemId = 0;
			player->inventory[itemToBeThrown].ammo = 0;
			player->inventory[itemToBeThrown].maxammo = 0;
		}
		itemToBeThrown = -1;
		inv.selection = -1;
		RefreshInventoryScreen();
	}
}

static void ExitInventory(void)
{
	StopScripts();
	game.flags[2999] = false; //textbox gold trim
	shop.inShop = false;
	shop.inSell = false;
	shop.inBuySellSelection = false;
	shopBoxSlideY = 0;
	game.setmode(GM_NORMAL);
}

static void DrawSelector(stSelector *selector, int x, int y)
{
int selx, sely;
int xsel, ysel;

	int INVWEAPON_X = inv.x + 16;
	int INVWEAPON_Y = inv.y + 8;

	if (inv.selection == -1){
		selector->flashstate = 1;
		selector->animtimer = 0;
	} else if (++selector->animtimer > 15)
	{
		selector->animtimer = 0;
		selector->flashstate ^= 1;
	}

	if (selector->rowlen)
	{
		xsel = (selector->cursel % selector->rowlen);
		ysel = (selector->cursel / selector->rowlen);
	}
	else xsel = ysel = 0;
	
	
	selx = x + (xsel * selector->spacing_x);
	sely = y + (ysel * selector->spacing_y);
	
	switch(selector->cursel){
		case 16: draw_sprite(INVWEAPON_X - 7, INVWEAPON_Y + 2, selector->sprite, 0, 0); break;
		case 17: draw_sprite(INVWEAPON_X - 7, inv.y + 40 + 2, selector->sprite, 0, 0); break;
		default: draw_sprite(selx - 7, sely + 2, selector->sprite, 0, 0);
	}
	
	if (inv.selection != -1){ //draw other hand
		xsel = (inv.selection % selector->rowlen);
		ysel = (inv.selection / selector->rowlen);
		selx = x + (xsel * selector->spacing_x);
		sely = y + (ysel * selector->spacing_y);
		switch(inv.selection){
			case 16: draw_sprite(INVWEAPON_X - 7, INVWEAPON_Y + 2, selector->sprite, selector->flashstate, 0); break;
			case 17: draw_sprite(INVWEAPON_X - 7, inv.y + 40 + 2, selector->sprite, selector->flashstate, 0); break;
			default: draw_sprite(selx - 7, sely + 2, selector->sprite, selector->flashstate, 0);
		}
	}
}

