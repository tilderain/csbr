
/*
	The save select box (for multiple save files).
*/

#include "../nx.h"
#include "../profile.h"
#include "../inventory.h"
#include "../replay.h"
#include "TextBox.h"	// for textbox coordinates; MSG_W etc
#include "SaveSelect.h"
#include "SaveSelect.fdh"

// moved here as static data so that the compiler will shut up about a circular dependency
// that happens if you try to include profile.h from SaveSelect.h.
static Profile fProfiles[MAX_SAVE_SLOTS];
static bool fHaveProfile[MAX_SAVE_SLOTS];
int fPicXOffset;

TB_SaveSelect::TB_SaveSelect()
{
}

/*
void c------------------------------() {}
*/

void TB_SaveSelect::ResetState()
{
	fVisible = false;
}

void TB_SaveSelect::SetVisible(bool enable, bool saving)
{
	fVisible = enable;
	if (!enable) return;
	game.showmapnametime = 0;
	
	fCoords.x = 38;
	fCoords.y = 8;
	fCoords.w = 244;
	fCoords.h = 152;
	fNumFiles = MAX_SAVE_SLOTS;
	fSaving = saving;
	
	fCurSel = settings->last_save_slot;
	fPicXOffset = -24;
	
	// load all profiles
	memset(fHaveProfile, 0, sizeof(fHaveProfile));
	for(int i=0;i<fNumFiles;i++)
	{
		if (!profile_load(GetProfileName(i), &fProfiles[i]))
			fHaveProfile[i] = true;
	}
	
	textbox.ClearText();
}

bool TB_SaveSelect::IsVisible()
{
	return fVisible;
}

/*
void c------------------------------() {}
*/

void TB_SaveSelect::Run_Input()
{
int start;

	if (justpushed(DOWNKEY))
	{
		start = fCurSel;
		for(;;)
		{
			fCurSel++;
			if (fCurSel >= fNumFiles) fCurSel = 0;
			
			if (fSaving) break;
			if (fHaveProfile[fCurSel]) break;
			if (fCurSel == start) break;
		}
		
		sound(SND_MENU_MOVE);
		fPicXOffset = -24;
	}
	
	if (justpushed(UPKEY))
	{
		start = fCurSel;
		for(;;)
		{
			fCurSel--;
			if (fCurSel < 0) fCurSel = fNumFiles - 1;
			
			if (fSaving) break;
			if (fHaveProfile[fCurSel]) break;
			if (fCurSel == start) break;
		}
		
		sound(SND_MENU_MOVE);
		fPicXOffset = -24;
	}
	
	if (justpushed(JUMPKEY))
	{
		// when shown in a replay, the box is shown and everything just like what was done
		// originally, but we won't actually overwrite any save files.
		if (!Replay::IsPlaying())
		{
			if (fHaveProfile[fCurSel] && !(settings->last_save_slot == fCurSel) && fSaving)
			{
				StartScript(14); //overwrite existing save
			} 
			else
			{
			if (fSaving)
				game_save(fCurSel);
			
			sound(SND_MENU_SELECT);
			settings->last_save_slot = fCurSel;
			settings_save();		// record new save/load slot
			}
		}
		
		SetVisible(false);
		
		// when the script hit the <SVP, it froze itself in an artifical <WAI9999
		// waiting for us to complete. Now unfreeze it so can say "Game saved.",
		// or for loading, just end.
		ScriptInstance *s = GetCurrentScriptInstance();
		if (s) s->delaytimer = 0;
	}
	if (justpushed(FIREKEY || INVENTORYKEY))
	{
		if (fSaving)
		{
			SetVisible(false);
			StartScript(15); //Save cancelled.
		} 
		else
		{
			game.setmode(GM_TITLE);
			SetVisible(false);
		}
	}
}


void TB_SaveSelect::DrawProfile(int x, int y, int index)
{
Profile *p = &fProfiles[index];
const int w = fCoords.w - 33;

	int sidewd = sprites[SPR_SAVESELECTOR_SIDES].w;
	int repeatwd = w - (sidewd * 2);
	int frame = (index == fCurSel) ? 0 : 1;
	
	draw_sprite(x, y, SPR_SAVESELECTOR_SIDES, frame, LEFT);
	draw_sprite_repeating_x(x+sidewd, y, SPR_SAVESELECTOR_MIDDLE, frame, repeatwd);
	draw_sprite(x+sidewd+repeatwd, y, SPR_SAVESELECTOR_SIDES, frame, RIGHT);
	
	y += 5;
	const int FONT_SPACING = 5;
	
	if (fHaveProfile[index])
	{
		const char *stage = map_get_stage_name(p->stage);
		font_draw(x+8, y-3, stage, FONT_SPACING);
		
		// draw health.
		DrawHealth(x+w - 64, y, p);
		
		draw_sprite(x+w - 24, y + 1, SPR_XPBAR, 3, 0);
		// cion Number
		DrawNumber(x+w - 24 - 36, y, p->xp); // 36 difference of x
	}
	else if (fCurSel == index)
	{
		const char *str = "available";
		int fx = (w / 2) - (GetFontWidth(str, FONT_SPACING) / 2);
		font_draw(x+fx, y-1, str, FONT_SPACING);
	}
}


void TB_SaveSelect::DrawExtendedInfo()
{
Profile *p = &fProfiles[fCurSel];
int x, y, s;
	
	if (fPicXOffset < 0)
	{
		fPicXOffset += 8;
		set_clip_rect(MSG_X+4, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	}
	
	// player pic
	draw_sprite((MSG_X+8) + fPicXOffset, MSG_NORMAL_Y+8, SPR_SELECTOR_ARMS);
	
	x = (MSG_X + 12) + fPicXOffset;
	y = MSG_NORMAL_Y + 12;
	s = PSelectSprite();
	
	draw_sprite(x, y, s, 0, RIGHT);
	
	// player gun
	if (p->curWeapon != WPN_NONE)
	{
		int spr, frame;
		GetSpriteForGun(p->curWeapon, 0, &spr, &frame);
		
		draw_sprite_at_dp(x + sprites[s].frame[0].dir[RIGHT].actionpoint.x, \
						  y + sprites[s].frame[0].dir[RIGHT].actionpoint.y, \
						  spr, frame, RIGHT);
	}
	
	clear_clip_rect();
	
	// whimsical stars
	if (p->equipmask & EQUIP_WHIMSTAR)
	{
		x = MSG_X + 12;
		
		for(int i=0;i<3;i++)
		{
			static int frames[] = { 1, 0, 2 };
			draw_sprite(x, y+20, SPR_WHIMSICAL_STAR, frames[i]);
			x += 10;
		}
	}
	
	// WEAPONS:
	x = MSG_X + 64;
	y = MSG_NORMAL_Y + 8;
	
	// xp of current weapon
	if (p->curWeapon != WPN_NONE)
	{
		int xb = MSG_X + 7;
		int yb = MSG_NORMAL_Y + 30;
		
		draw_sprite(xb, yb, SPR_ITEMIMAGE, p->curWeapon, 0);
		DrawNumber(xb+9, yb+8, p->weapons[p->curWeapon].ammo);
		
	}
	
	// ITEMS:
	x = (MSG_X + 64) - 14;
	y = MSG_NORMAL_Y + 4;
	
	int c = 0;
	int i;
	for(i=0;i<p->ninventory;i++)
	{
			draw_sprite(x, y, SPR_ITEMIMAGE, p->inventory[i].itemId, 0);
			
			if (!p->inventory[i].maxammo) { 
			// do nothing
			} else {
				DrawNumberGray(x+17, y+8, p->inventory[i].ammo); //weird position
				//add support for infinity symbol?
			}
			x += 48;
			
			if (++c >= 4)
			{
				x = (MSG_X + 64) - 14;
				y += 16;
				c = 0;
			}
	}
	
	// Booster

	draw_sprite(x, y, SPR_ITEMIMAGE, GetPlug()); // doesn't work at the moment, need to fix

	
}


// I don't think it's possible to get 3-digit max life
// without hacking, but I accounted for it anyway.
static void DrawHealth(int xright, int y, Profile *p)
{
int i, hx;
	hx = xright - 28 - 16;
	for(i=0;i<p->maxhp;i++){
		if (i==0){ //skip a heart
			continue;
		}
		if (i < p->hp){
			draw_sprite(hx, y, SPR_HEALTHBAR, 0); //full frame
		} else {
		draw_sprite(hx, y, SPR_HEALTHBAR, 4); //empty frame
		}
		hx += sprites[SPR_HEALTHBAR].w;
		if (i==6){
			
		}
	}
}



void TB_SaveSelect::Draw(void)
{
	if (!fVisible)
		return;
	
	// handle user input
	Run_Input();
	
	// draw frame
	TextBox::DrawFrame(fCoords.x, fCoords.y, fCoords.w, fCoords.h);
	
	// draw selectors/options
	int x = fCoords.x + 16;
	int y = fCoords.y + 15;
	
	for(int i=0;i<fNumFiles;i++)
	{
		DrawProfile(x, y, i);
		y += (sprites[SPR_SAVESELECTOR_MIDDLE].h + 10);
	}
	
	// draw extended info for current selection
	if (fHaveProfile[fCurSel])
		DrawExtendedInfo();
}











