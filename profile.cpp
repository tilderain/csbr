
#include "nx.h"
#include "profile.h"
#include "profile.fdh"

#define PF_WEAPONS_OFFS		0x38
#define PF_CURWEAPON_OFFS	0x24
#define PF_INVENTORY_OFFS	0x4C
#define PF_TELEPORTER_OFFS	0x158
#define PF_FLAGS_OFFS		0x218

#define MAX_WPN_SLOTS		1
#define MAX_TELE_SLOTS		8

// load savefile #num into the given Profile structure.
bool profile_load(const char *pfname, Profile *file)
{
int i, curweaponslot;
FILE *fp;

	stat("Loading profile from %s...", pfname);
	memset(file, 0, sizeof(Profile));
	
	fp = fileopen(pfname, "rb");
	if (!fp)
	{
		staterr("profile_load: unable to open '%s'", pfname);
		return 1;
	}
	
	if (!fverifystring(fp, "Do041220"))
	{
		staterr("profile_load: invalid savegame format: '%s'", pfname);
		fclose(fp);
		return 1;
	}
	
	file->stage = fgetl(fp);
	file->songno = fgetl(fp);
	
	file->px = fgetl(fp);
	file->py = fgetl(fp);
	file->pdir = CVTDir(fgetl(fp));
	
	file->maxhp = fgeti(fp);
	file->num_whimstars = fgeti(fp);
	file->hp = fgeti(fp);
	
	file->xp = fgeti(fp);						// unknown value
	
	curweaponslot = fgetl(fp);		// current weapon (slot, not number, converted below)
	fgetl(fp);						// unknown value
	file->equipmask = fgetl(fp);	// equipped items
	
	// load weapons
	fseek(fp, PF_WEAPONS_OFFS, SEEK_SET);
	for(i=0;i<MAX_WPN_SLOTS;i++)
	{
		int type = fgetl(fp);
		if (!type) break;
		
		int level = fgetl(fp);
		int xp = fgetl(fp);
		int maxammo = fgetl(fp);
		int ammo = fgetl(fp);
		
		file->weapons[type].level = (level - 1);
		file->weapons[type].xp = xp;
		file->weapons[type].ammo = ammo;
		file->weapons[type].maxammo = maxammo;
		

		file->curWeapon = type;

	}
	
	// load inventory
	file->ninventory = 0;
	fseek(fp, PF_INVENTORY_OFFS, SEEK_SET);
	for(i=0;i<MAX_INVENTORY;i++)
	{
		int itemId = fgetl(fp);
		int ammo = fgetl(fp);
		int maxammo = fgetl(fp);
		
		file->inventory[file->ninventory].itemId = itemId;
		file->inventory[file->ninventory].ammo = ammo;
		file->inventory[file->ninventory].maxammo = maxammo;
		file->ninventory++;
	}
	
	// load teleporter slots
	file->num_teleslots = 0;
	fseek(fp, PF_TELEPORTER_OFFS, SEEK_SET);
	for(i=0;i<NUM_TELEPORTER_SLOTS;i++)
	{
		int slotno = fgetl(fp);
		int scriptno = fgetl(fp);
		if (slotno == 0) break;
		
		file->teleslots[file->num_teleslots].slotno = slotno;
		file->teleslots[file->num_teleslots].scriptno = scriptno;
		file->num_teleslots++;
	}
	
	// load flags
	fseek(fp, PF_FLAGS_OFFS, SEEK_SET);
	if (!fverifystring(fp, "FLAG"))
	{
		staterr("profile_load: missing 'FLAG' marker");
		fclose(fp);
		return 1;
	}
	
	fresetboolean();
	for(i=0;i<NUM_GAMEFLAGS;i++)
	{
		file->flags[i] = fbooleanread(fp);
	}
	
	fclose(fp);
	return 0;
}


bool profile_save(const char *pfname, Profile *file)
{
FILE *fp;
int i;

	//stat("Writing saved game to %s...", pfname);
	fp = fileopen(pfname, "wb");
	if (!fp)
	{
		staterr("profile_save: unable to open %s", pfname);
		return 1;
	}
	
	fputstringnonull("Do041220", fp);
	
	fputl(file->stage, fp);
	fputl(file->songno, fp);
	
	fputl(file->px, fp);
	fputl(file->py, fp);
	fputl((file->pdir == RIGHT) ? 2:0, fp);
	
	fputi(file->maxhp, fp);
	fputi(file->num_whimstars, fp);
	fputi(file->hp, fp);
	fputi(file->xp, fp);
	
	fseek(fp, 0x2C, SEEK_SET);
	fputi(file->equipmask, fp);
	
	// save weapons
	fseek(fp, PF_WEAPONS_OFFS, SEEK_SET);
	int slotno = 0, curweaponslot = 0;
	
	for(i=0;i<MAX_WPN_SLOTS;i++)
	{
		fputl(player->curWeapon, fp);
		fputl(file->weapons[player->curWeapon].level + 1, fp);
		fputl(file->weapons[player->curWeapon].xp, fp);
		fputl(file->weapons[player->curWeapon].maxammo, fp);
		fputl(file->weapons[player->curWeapon].ammo, fp);
	}
	
	if (slotno < MAX_WPN_SLOTS)
		fputl(0, fp);	// 0-type weapon: terminator
	
	// go back and save slot no of current weapon
	fseek(fp, PF_CURWEAPON_OFFS, SEEK_SET);
	fputl(curweaponslot, fp);
	
	// save inventory
	fseek(fp, PF_INVENTORY_OFFS, SEEK_SET);
	for(i=0;i<MAX_INVENTORY;i++)
	{
		fputl(file->inventory[i].itemId, fp);
		fputl(file->inventory[i].ammo, fp);
		fputl(file->inventory[i].maxammo, fp);
	}
	
	fputl(0, fp);
	
	// write teleporter slots
	fseek(fp, PF_TELEPORTER_OFFS, SEEK_SET);
	for(i=0;i<MAX_TELE_SLOTS;i++)
	{
		if (i < file->num_teleslots)
		{
			fputl(file->teleslots[i].slotno, fp);
			fputl(file->teleslots[i].scriptno, fp);
		}
		else
		{
			fputl(0, fp);
			fputl(0, fp);
		}
	}
	
	// write flags
	fseek(fp, PF_FLAGS_OFFS, SEEK_SET);
	fputstringnonull("FLAG", fp);
	
	fresetboolean();
	for(i=0;i<NUM_GAMEFLAGS;i++)
	{
		fbooleanwrite(file->flags[i], fp);
	}
	
	fbooleanflush(fp);
	
	fclose(fp);
	return 0;
}


/*
void c------------------------------() {}
*/

// returns the filename for a save file given it's number
const char *GetProfileName(int num)
{
	if (num == 0)
		return "profile.dat";
	else
		return stprintf("profile%d.dat", num+1);
}

// returns whether the given save file slot exists
bool ProfileExists(int num)
{
	return file_exists(GetProfileName(num));
}

bool AnyProfileExists()
{
	for(int i=0;i<MAX_SAVE_SLOTS;i++)
		if (ProfileExists(i)) return true;
	
	return false;
}



