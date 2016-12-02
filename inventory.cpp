
// the in-game inventory screen

#include "nx.h"
#include "inventory.h"
#include "inventory.fdh"

#define ARMS_X			10
#define ARMS_Y			8

#define ITEMS_X			10
#define ITEMS_Y			60

static stInventory inv;

// can't enter Inven if
//  * script is running
//  * fade is in progress
//  * player is dead

// param is passed as 1 when returning from Map System.
bool inventory_init(int param)
{
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
			if (inv.itemsel.items[i] == 2)
			{
				inv.curselector->cursel = i;
				// textbox NOT up until they move the selector
				inv.curselector->lastsel = i;
				break;
			}
		}
	}
	
	return 0;
}


void inventory_tick(void)
{
	// run the selectors
	RunSelector(inv.curselector);
	
	// draw
	DrawScene();
	DrawInventory();
	if (inv.doneDrawing) textbox.Draw();
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
		inv.itemsel.items[i] = player->inventory[i];
	
	inv.itemsel.spacing_x = 56;
	inv.itemsel.spacing_y = sprites[SPR_ITEMIMAGE].h ;//16
	inv.itemsel.sprite = SPR_SELECTOR_ITEMS;
	inv.itemsel.sound = SND_MENU_MOVE;
	inv.itemsel.rowlen = 4;
	inv.itemsel.scriptbase = 5000;
	
	// after an item has been used don't bring up the script of whatever item
	// the selector is moved to
	inv.curselector->lastsel = inv.curselector->cursel;
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
		if (inv.exiting)
		{
			inv.curRow -= ROW_SPEED;
			if (inv.curRow <= 0)
			{
				inv.curRow = 0;
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
			draw_sprite(x, y, SPR_ITEMIMAGE, inv.itemsel.items[i], 0);
			
			x += inv.itemsel.spacing_x;
			
			if (++c >= inv.itemsel.rowlen)
			{
				x = inv.x;
				y += inv.itemsel.spacing_y;
				c = 0;
			}
		}
		// - draw the player ----
		
		DrawSelector(&inv.itemsel, inv.x, inv.y);
		
		if (player->curWeapon != WPN_NONE)
		{
			int s, frame;
			//todo don't be lazy
			switch(player->curWeapon)
			{
				case WPN_SUPER_MISSILE: s = SPR_SUPER_MLAUNCHER; break;
				case WPN_NEMESIS: s = SPR_NEMESIS; break;
				case WPN_BUBBLER: s = SPR_BUBBLER; break;
				case WPN_SPUR: s = SPR_SPUR; break;
				case WPN_BLADE: s = SPR_BLADEARMS; break;
			
				default:
					s = SPR_WEAPONS_START + (player->curWeapon * 2);
				break;
			}
			
			// draw the gun at the player's Action Point. Since guns have their Draw Point set
			// to point at their handle, this places the handle in the player's hand.
			draw_sprite_at_dp(157, \
							45, \
							s, 0, 1);
		}
		
		draw_sprite(152, 32, SPR_MYCHAR, 0, 1);
	}
}


static void RunSelector(stSelector *selector){
int nrows;
int currow, curcol;

	if (inv.lockinput)
	{
		if (GetCurrentScript()==-1) inv.lockinput = 0;
		else return;
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
	
	if (justpushed(RIGHTKEY))
	{
		sound(selector->sound);
		
		// at end of row?
		if (curcol==selector->rowlen-1 || selector->cursel+1 >= selector->nitems)
		{	// wrap to beginning of row
			selector->cursel = (currow * selector->rowlen);
		}
		else selector->cursel++;
	}
	
	if (justpushed(DOWNKEY))
	{

		selector->cursel += selector->rowlen;
			
		// don't go past last item
		if (selector->cursel >= selector->nitems)
			selector->cursel = (selector->nitems - 1);
				
		sound(selector->sound);

	}
	
	if (justpushed(UPKEY))
	{
		//need to account for weapon and plug
		if (currow == 0){
		selector->cursel += 12; //this will probably never ever change
		sound(selector->sound);
		}else{
		selector->cursel -= selector->rowlen;
		sound(selector->sound);
		}
	}

	// bring up scripts
	if (selector->cursel != selector->lastsel)
	{
		selector->lastsel = selector->cursel;
		
		StartScript(selector->items[selector->cursel] + selector->scriptbase, SP_ARMSITEM);
	}
	
	
	if (selector == &inv.armssel)		// selecting a weapon
	{
		if (buttonjustpushed())
		{	// select the new weapon
			weapon_slide(LEFT, selector->items[selector->cursel]);
			inv.exiting = true;
		}
	}
	else									// selecting an item
	{
		if (justpushed(JUMPKEY))
		{
			if (inv.selection == -1){
				inv.selection = selector->cursel;
				sound(SND_MENU_SELECT);
			}else{
				
			//swap items
			//special behavior for trash and plug and weapon
			player->inventory[selector->cursel] = selector->items[inv.selection];
			player->inventory[inv.selection] = selector->items[selector->cursel];
			sound(SND_MENU_SELECT);
			inv.selection = -1;
			RefreshInventoryScreen();
			StartScript(selector->items[selector->cursel] + selector->scriptbase, SP_ARMSITEM);
			}
		}
		if (justpushed(FIREKEY)) 
		{
			// bring up "more info" or "equip" script for this item
			StartScript(selector->items[selector->cursel] + selector->scriptbase + 1000, SP_ARMSITEM);
			inv.lockinput = 1;
		}
		if (justpushed(INVENTORYKEY)) 
		{
			inv.exiting = true;
		}
		
	}
}
static void ExitInventory(void)
{
	StopScripts();
	game.setmode(GM_NORMAL);
}



static void DrawSelector(stSelector *selector, int x, int y)
{
int selx, sely;
int xsel, ysel;

	if (selector == inv.curselector)
	{
		// flash the box
		if (++selector->animtimer > 1)
		{
			selector->animtimer = 0;
			selector->flashstate ^= 1;
		}
	}
	else
	{	// permanently dim
		selector->flashstate = 1;
		selector->animtimer = 99;		// light up immediately upon becoming active
	}
	
	if (selector->rowlen)
	{
		xsel = (selector->cursel % selector->rowlen);
		ysel = (selector->cursel / selector->rowlen);
	}
	else xsel = ysel = 0;
	
	selx = x + (xsel * selector->spacing_x);
	sely = y + (ysel * selector->spacing_y);
	draw_sprite(selx - 7, sely - 4, selector->sprite, selector->flashstate, 0);
	
	if (inv.selection != -1){
		//todo replace with blinking hand
		xsel = (inv.selection % selector->rowlen);
		ysel = (inv.selection / selector->rowlen);
		selx = x + (xsel * selector->spacing_x);
		sely = y + (ysel * selector->spacing_y);
		draw_sprite(selx - 7, sely - 4, selector->sprite, 1, 0);
	}
}

