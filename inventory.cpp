
// the in-game inventory screen

#include "nx.h"
#include "inventory.h"
#include "inventory.fdh"

#define ARMS_X			10
#define ARMS_Y			8

#define ITEMS_X			10
#define ITEMS_Y			60

//placeholders until i get hearts working
#define HEALTH_X			38 + 170
#define HEALTH_Y			8 + 9

#define FRAME_XP_MAX		3			// "MAX" when XP is at max on L3

#define INVWEAPON_X 54
#define INVWEAPON_Y 16

#define INVPLUG_X 54
#define INVPLUG_Y 48

int itemToBeThrown;

// can't enter Inven if
//  * script is running
//  * fade is in progress
//  * player is dead

//int invWeapons = 

//

// param is passed as 1 when returning from Map System.

stInventory inv;
stShop shop;

bool inventory_init(int param)
{
	//param == 2 then do shop things
	if (param == 2) {
		memset(&inv, 0, sizeof(inv));
		memset(&shop, 0, sizeof(shop));
		
		inv.selection = -1;
		shop.curselector = &shop.itemsel;
		shop.itemsel.cursel = RefreshShopScreen();
		shop.curselector->lastsel = -9999;		// run the script first time
		shop.inShop = true;
		
		shop.itemsel.shopitems[0].itemId = 2;
		shop.itemsel.shopitems[0].price = 70;
		shop.itemsel.shopitems[0].ammo = 180;
		shop.itemsel.shopitems[0].maxammo = 180;
		
		shop.itemsel.shopitems[2].itemId = 15;
		shop.itemsel.shopitems[2].price = 40;
		shop.itemsel.shopitems[2].ammo = 0;
		shop.itemsel.shopitems[2].maxammo = 0;
		
		shop.itemsel.shopitems[4].itemId = 1;
		shop.itemsel.shopitems[4].price = 50;
		shop.itemsel.shopitems[4].ammo = 75;
		shop.itemsel.shopitems[4].maxammo = 75;
		
	} else {
		memset(&inv, 0, sizeof(inv));
		
		inv.curselector = &inv.itemsel;
		inv.itemsel.cursel = RefreshInventoryScreen();
		inv.selection = -1;
		inv.curselector->lastsel = -9999;		// run the script first time
	
		// returning from map system?
		if (param == 1)
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
	if (shop.inShop){
		RunShopSelector(shop.curselector);
		DrawScene();
		DrawShop();
		textbox.Draw();
	} else {
		if (!inv.exiting) RunSelector(inv.curselector);
		// draw
		DrawScene();
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
	inv.x = 38;
	inv.y = 8;
	int ROW_SPEED = 16;
	
	if (inv.exiting == true)
	{
		inv.doneDrawing = false;
	}
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
				inv.curRow = 0; //is this really neccesary?
				inv.doneDrawing = false;
				inv.exiting == false;
				inv.lockinput = false;
				ExitInventory();
			}
			draw_sprite_clip_length(inv.x, inv.y, SPR_INVENTORY_SCREEN, 0, inv.curRow);
		}
		else 
		{
			inv.curRow += ROW_SPEED;
			draw_sprite_clip_length(inv.x, inv.y, SPR_INVENTORY_SCREEN, 0, inv.curRow);
			if (inv.curRow >= sprites[SPR_INVENTORY_SCREEN].h)
			{
				inv.curRow = sprites[SPR_INVENTORY_SCREEN].h;
				inv.doneDrawing = true;
			}
		}
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
		// - draw the weapons ----

		
		// start of items box
		inv.x = 54;
		inv.y = 80;
		
		x = inv.x;
		y = inv.y;

		// draw the arms
		
		// - draw the items ----


		
		c = 0;
		for(i=0;i<inv.itemsel.nitems;i++)
		{
			draw_sprite(x, y, SPR_ITEMIMAGE, inv.itemsel.items[i].itemId, 0);
			
			if (!player->inventory[i].maxammo) { 
			// do nothing
			} else {
				DrawNumberGray(x+17, y+8, player->inventory[i].ammo); //weird position
				//add support for infinity symbol?
			}
			x += inv.itemsel.spacing_x;
			
			if (++c >= inv.itemsel.rowlen)
			{
				x = inv.x;
				y += inv.itemsel.spacing_y;
				c = 0;
			}
		}
		
		if (player->curWeapon){
			draw_sprite(INVWEAPON_X, INVWEAPON_Y, SPR_ITEMIMAGE, player->curWeapon, 0);
			DrawNumber(INVWEAPON_X+17, INVWEAPON_Y+8, player->weapons[player->curWeapon].ammo);
		}
		
		if (player->equipmask){
			draw_sprite(INVPLUG_X, INVPLUG_Y, SPR_ITEMIMAGE, GetPlug(), 0);
		}
		// - draw the player ----
		
		DrawSelector(&inv.itemsel, inv.x, inv.y);
		
		int s, spr, frame;
		if (player->curWeapon != WPN_NONE)
		{
				GetSpriteForGun(player->curWeapon, 0, &spr, &frame);
				draw_sprite_at_dp(157, 45, spr, frame, LEFT);	
		}
		s = (player->equipmask & EQUIP_MIMIGA_MASK) ? SPR_MYCHAR_MIMIGA : SPR_MYCHAR;
		draw_sprite(152, 32, s, 0, LEFT);
		
		// - draw the health ----
		//magic numbers everywhere
		inv.x = 38;
		inv.y = 8;
				
		DrawHealthBar(HEALTH_X, HEALTH_Y, player->hp, player->maxHealth);
		// - draw the money ----
		
		draw_sprite(inv.x + 205, inv.y + 42, SPR_XPBAR, FRAME_XP_MAX, 0);
		// cion Number
		DrawNumber(HEALTH_X - 1, inv.y + 41, player->xp); // 36 difference of x
		
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
		if (selector->cursel == 16){
			selector->cursel = 3;
		}
		else if (curcol==selector->rowlen-1 || selector->cursel+1 >= selector->nitems)
		{	// wrap to beginning of row
			selector->cursel = (currow * selector->rowlen);
		}
		else selector->cursel++;
	} else if (justpushed(DOWNKEY)){
		if (selector->cursel == 16) {
			selector->cursel = 17;
			sound(selector->sound);
		} else if (selector->cursel == 17) {
			selector->cursel = 0;
			sound(selector->sound);
		} else {
			if (currow == 3 && curcol == 0){
				selector->cursel = 16;
				sound(selector->sound);
			}else{
				if (currow == 3){
					selector->cursel -= 12;
					sound(selector->sound);
				} else {
				selector->cursel += selector->rowlen;
				sound(selector->sound);
				}
			}
		}
	} else if (justpushed(UPKEY)){
		//need to account for weapon and plug
		//holy hell this is a mess
		if (selector->cursel == 16) {
			selector->cursel = 12;
			sound(selector->sound);
		} else if (selector->cursel == 17) {
			selector->cursel = 16;
			sound(selector->sound);
		} else if (currow == 0){
			if (curcol == 0){
				selector->cursel = 17;
				sound(selector->sound);
			} else {
				selector->cursel += 12; //this will probably never ever change
				sound(selector->sound);
			}
		} else {
			selector->cursel -= selector->rowlen;
			sound(selector->sound);
		}
	}

	// bring up scripts
	if (selector->cursel != selector->lastsel)
	{
		selector->lastsel = selector->cursel;
		
		switch(selector->cursel){
			case 16: StartScript(player->curWeapon + selector->scriptbase, SP_ARMSITEM); break;
		    case 17: StartScript(GetPlug() + selector->scriptbase, SP_ARMSITEM); break;
			default: StartScript(selector->items[selector->cursel].itemId + selector->scriptbase, SP_ARMSITEM);
		}
	}
	
	else									// selecting an item
	{
		if (justpushed(JUMPKEY)){
			if (inv.selection == -1){
				inv.selection = selector->cursel;
				sound(SND_MENU_SELECT);
			} else {
				//swap items
				//special behavior for trash and plug and weapon
				//please clean this up
				//ever heard of a switch case?
				//have to handle hasweapon or remove it
				if (inv.selection == 15 || selector->cursel == 15){
					if (inv.selection == 15){
						SwapTrash(inv.selection, selector->cursel);
					} else {
						SwapTrash(selector->cursel, inv.selection);
					}
				} else if (inv.selection == 16 || selector->cursel == 16){
					if (inv.selection == 17 || selector->cursel == 17){
						sound(SND_BONK_HEAD);
					} else if (inv.selection == 16){
						SwapWeapon(inv.selection, selector->cursel);
						StartScript(selector->items[selector->cursel].itemId + selector->scriptbase, SP_ARMSITEM);
					} else { 
						SwapWeapon(selector->cursel, inv.selection);
						StartScript(player->curWeapon + selector->scriptbase, SP_ARMSITEM);
					}
				} else if (inv.selection == 17 || selector->cursel == 17) {
					if (inv.selection == 16 || selector->cursel == 16) {
						sound(SND_BONK_HEAD);
					} else if (inv.selection == 17){
						SwapPlug(inv.selection, selector->cursel);
						StartScript(selector->items[selector->cursel].itemId + selector->scriptbase, SP_ARMSITEM);
					} else {
						SwapPlug(selector->cursel, inv.selection);
						StartScript(GetPlug() + selector->scriptbase, SP_ARMSITEM);
					}
				} else {
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
		inv.itemsel.items[i].itemId = player->inventory[i].itemId;
	
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
	
	inv.x = 38; //settings for item box
	inv.y = 72;
	
	inv.w = 244;
	inv.h = 80;
	
	TextBox::DrawFrame(inv.x, inv.y, inv.w, inv.h, shop_); 
	draw_sprite(inv.x, inv.y, SPR_SHOP_ITEM, 0, 0);
	
	inv.x = 38; //settings for shop box
	inv.y = 32;
	
	inv.w = 244;
	inv.h = 40;
	
	TextBox::DrawFrame(inv.x, inv.y, inv.w, inv.h, shop_); 
	draw_sprite(inv.x, inv.y, SPR_SHOP_SHOP, 0, 0);
	
	inv.x = 198;
	inv.y = 8;
	
	inv.w = 84;
	inv.h = 24;
	
	TextBox::DrawFrame(inv.x, inv.y, inv.w, inv.h, shop_); 
	draw_sprite(inv.x, inv.y, SPR_SHOP_MONEY, 0, 0);

	// start of shop box
	inv.x = 54;
	inv.y = 37;
	
	x = inv.x;
	y = inv.y;
	
	c = 0;
	for(i=0;i<8;i++)
	{
		draw_sprite(x, y, SPR_ITEMIMAGE, shop.itemsel.shopitems[i].itemId, 0);
		if (!shop.itemsel.shopitems[i].maxammo) { 
		// do nothing
		} else {
			DrawNumber(x+17, y+8, shop.itemsel.shopitems[i].ammo); //weird position
			//add support for infinity symbol?
		}
		x += shop.itemsel.spacing_x;
		
		if (++c >= shop.itemsel.rowlen)
		{
			x = inv.x;
			y += shop.itemsel.spacing_y;
			c = 0;
		}
	}
		
	DrawSelector(&shop.itemsel, inv.x, inv.y);
	
	// start of items box
	inv.x = 54;
	inv.y = 80;
	
	x = inv.x;
	y = inv.y;

	c = 0;
	for(i=0;i<player->ninventory;i++)
	{
		if ( !(i==15) ){
			draw_sprite(x, y, SPR_ITEMIMAGE, inv.itemsel.items[i].itemId, 0);
		} else{
			draw_sprite(x, y, SPR_ITEMIMAGE, 40, 0); //trash... placeholder for now?
		}
		
		if (!player->inventory[i].maxammo) { 
		// do nothing
		} else {
			DrawNumberGray(x+17, y+8, player->inventory[i].ammo); //weird position
			//add support for infinity symbol?
		}
		x += shop.itemsel.spacing_x;
		
		if (++c >= shop.itemsel.rowlen)
		{
			x = inv.x;
			y += shop.itemsel.spacing_y;
			c = 0;
		}
	}
	
	// - draw the health ----
	//magic numbers everywhere
	inv.x = 38;
	inv.y = 8;
			
	// - draw the money ----
	
	draw_sprite(inv.x + 205, inv.y + 10, SPR_XPBAR, FRAME_XP_MAX, 0);
	// cion Number
	DrawNumber(inv.x + 169, inv.y + 9, player->xp);
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
		StartScript(selector->shopitems[selector->cursel].itemId + selector->scriptbase, SP_ARMSITEM); 
	}
	
	if (justpushed(JUMPKEY)){
		//bring up shop dialogue, then do purchase
		lastammoinc = selector->shopitems[selector->cursel].price;
		StartScript(1400, SP_ARMSITEM);
		shop.lockinput = 1;
	}
	if (justpushed(FIREKEY)) 
	{
		StartScript(selector->items[selector->cursel].itemId + selector->scriptbase + 1000, SP_ARMSITEM);
		shop.lockinput = 1;
	}
	if (justpushed(INVENTORYKEY)) 
	{
		ExitInventory(); //then go to shop exit script.
	}
}

enum SWAP
{
	SWAP_NONE = 0,
	SWAP_WEAPON = 1,
	SWAP_PLUG = 2,
};


void SwapItem(int loc1, int loc2){
	Item temp; //it's only the location of the item that's stored in a selection
	temp.itemId = player->inventory[loc1].itemId; //so we need to dupe the item attributes temporarily
	temp.ammo = player->inventory[loc1].ammo;
	temp.maxammo = player->inventory[loc1].maxammo;
	
	player->inventory[loc1].itemId = player->inventory[loc2].itemId;
	player->inventory[loc1].ammo = player->inventory[loc2].ammo;
	player->inventory[loc1].maxammo = player->inventory[loc2].maxammo;
	
	player->inventory[loc2].itemId = temp.itemId;
	player->inventory[loc2].ammo = temp.ammo;
	player->inventory[loc2].maxammo = temp.maxammo;
	
	sound(SND_MENU_SELECT);
	inv.selection = -1;
	RefreshInventoryScreen();
}

void SwapWeapon(int loc1, int loc2){
	//loc1 should always be 16
	if (player->inventory[loc2].maxammo || player->inventory[loc2].itemId == 0){
		Item temp;
		temp.itemId = player->curWeapon;
		temp.ammo = player->weapons[player->curWeapon].ammo;
		temp.maxammo = player->weapons[player->curWeapon].maxammo;
		
		player->curWeapon = player->inventory[loc2].itemId;
		player->weapons[player->curWeapon].ammo = player->inventory[loc2].ammo;
		player->weapons[player->curWeapon].maxammo = player->inventory[loc2].maxammo;
		
		player->inventory[loc2].itemId = temp.itemId;
		player->inventory[loc2].ammo = temp.ammo;
		player->inventory[loc2].maxammo = temp.maxammo;
		
		sound(SND_MENU_SELECT);
		inv.selection = -1;
		RefreshInventoryScreen();
		return;
	}
	if (loc1 == loc2) {
		sound(SND_MENU_SELECT);
		inv.selection = -1;
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
	game.modeShop = false;
	shop.inShop = false;
	game.setmode(GM_NORMAL);
}

static void DrawSelector(stSelector *selector, int x, int y)
{
int selx, sely;
int xsel, ysel;

	if (inv.selection == -1){
		selector->flashstate = 1;
		selector->animtimer = 0;
	} else if (++selector->animtimer > 30)
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
		case 17: draw_sprite(INVPLUG_X - 7, INVPLUG_Y + 2, selector->sprite, 0, 0); break;
		default: draw_sprite(selx - 7, sely + 2, selector->sprite, 0, 0);
	}
	
	if (inv.selection != -1){
		xsel = (inv.selection % selector->rowlen);
		ysel = (inv.selection / selector->rowlen);
		selx = x + (xsel * selector->spacing_x);
		sely = y + (ysel * selector->spacing_y);
		switch(inv.selection){
			case 16: draw_sprite(INVWEAPON_X - 7, INVWEAPON_Y + 2, selector->sprite, selector->flashstate, 0); break;
			case 17: draw_sprite(INVPLUG_X - 7, INVPLUG_Y + 2, selector->sprite, selector->flashstate, 0); break;
			default: draw_sprite(selx - 7, sely + 2, selector->sprite, selector->flashstate, 0);
		}
	}
}

