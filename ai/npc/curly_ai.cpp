
#include "../stdai.h"
#include "curly_ai.fdh"

#define CAI_INIT			20			// ANP'd to this by the entry script in Lab M
#define CAI_START			21			// ANP'd to this by Almond script
#define CAI_RUNNING			22
#define CAI_KNOCKEDOUT		40			// knocked out at beginning of Almond battle
#define CAI_ACTIVE			99

INITFUNC(AIRoutines)
{
	ONTICK(OBJ_CURLY_AI, ai_curly_ai);
	
	ONTICK(OBJ_CAI_GUN, ai_cai_gun);
	AFTERMOVE(OBJ_CAI_GUN, aftermove_cai_gun);
	AFTERMOVE(OBJ_CAI_WATERSHIELD, aftermove_cai_watershield);
}

// curly that fights beside you
void ai_curly_ai(Object *o)
{
int xdist, ydist;
int xlimit;
char reached_p;
int otiley;
char seeking_player = 0;
char wantdir;


	debug("Su Console");
	debug("TGT: [%d,%d] %d", o->xmark>>CSF, o->ymark>>CSF, game.curlytarget.timeleft);
	debug("State: %d", o->state);
	debug("");
	debug("RPT %d", o->curly.reachptimer);
	debug("TJT %d", o->curly.tryjumptime);
	debug("IJT %d:%d", o->curly.impjumptime, o->curly.impjump);
	game.debug.god = 1;

	static const int swalkanimframes[] = { 0, 2, 0, 3 };

	switch(o->state)
	{
		case 0:
			o->xinertia = 0;
			o->yinertia += 0x20;
		break;
		
		case CAI_INIT:			// set to this by an ANP in Maze M
			o->x = player->x;
			o->y = player->y;
		case CAI_START:			// set here after she stops being knocked out in Almond
		{
			o->invisible = 0;
			o->state = CAI_ACTIVE;
			o->timer = 0;

		}
		break;
		
		case CAI_KNOCKEDOUT:
		{
			o->timer = 0;
			o->state = CAI_KNOCKEDOUT+1;
			o->frame = 15;
		}
		case CAI_KNOCKEDOUT+1:
		{
			if (++o->timer > 1000)
			{	// start fighting
				o->state = CAI_START;
			}
			else if (o->timer > 750)
			{	// stand up
				o->flags &= ~FLAG_SCRIPTONACTIVATE;
				o->frame = 0;
			}
		}
		break;
	}
	
	if (o->state != CAI_ACTIVE) return;
	
	// first figure out where our target is
	
	// hack in case player REALLY leaves her behind. this works because of the way
	// the level is in a Z shape. first we check to see if the player is on the level below ours.
	if ((player->y > o->y && ((player->y - o->y) > 160<<CSF)) || o->state==999)
	{

			seeking_player = 1;		// stop when reach exit door
		
		o->ymark = o->y;
	}
	else
	{
		// if we get real far away from the player leave the enemies alone and come find him
		if (!pdistlx(160<<CSF)) game.curlytarget.timeleft = 0;
		
		// if we're attacking an enemy head towards the enemy else return to the player
		if (game.curlytarget.timeleft)
		{
			o->xmark = game.curlytarget.x;
			o->ymark = game.curlytarget.y;
			
			game.curlytarget.timeleft--;
			if (game.curlytarget.timeleft==60 && random(0, 2)==0) CaiJUMP(o);
		}
		else
		{
			o->xmark = player->x;
			o->ymark = player->y;
			seeking_player = 1;
		}
	}
	
	
	// calculate distance to target
	xdist = abs(o->x - o->xmark);
	ydist = abs(o->y - o->ymark);
	
	// face target. I used two seperate IF statements so she doesn't freak out at start point
	// when her x == xmark.
	if (o->x < o->xmark) wantdir = RIGHT;
	if (o->x > o->xmark) wantdir = LEFT;
	if (wantdir != o->dir)
	{
		if (++o->curly.changedirtimer > 4)
		{
			o->curly.changedirtimer = 0;
			o->dir = wantdir;
		}
	}
	else o->curly.changedirtimer = 0;
	
	// if trying to return to the player then go into a rest state when we've reached him
	reached_p = 0;
	if (seeking_player && xdist < (32<<CSF) && ydist < (64<<CSF))
	{
		if (++o->curly.reachptimer > 80)
		{
			o->xinertia *= 2;
			o->xinertia /= 5;
			o->frame = 0;
			reached_p = 1;
		}
	}
	else o->curly.reachptimer = 0;
	
	if (!reached_p)		// if not at rest walk towards target
	{
		// walking animation
		if (++o->animtimer > 4)
		{
			o->animtimer = 0;
			if (++o->animframe > 3) o->animframe = 0;
		}
		
		// walk towards target
		if (o->x > o->xmark) o->xinertia -= 0x20;
		if (o->x < o->xmark) o->xinertia += 0x20;
		o->frame = swalkanimframes[o->animframe];
		
		// jump if we hit a wall
		if ((o->blockr && o->xinertia > 0) || (o->blockl && o->xinertia < 0))
		{
			if (++o->curly.blockedtime > 8)
			{
				CaiJUMP(o);
			}
		}
		else o->curly.blockedtime = 0;
		
		// if our target gets really far away (like p is leaving us behind) and
		// the above jumping isn't getting us anywhere, activate the Improbable Jump
		if ((o->blockl || o->blockr) && xdist > (80<<CSF))
		{
			if (++o->curly.impjumptime > 60)
			{
				if (o->blockd)
				{
					CaiJUMP(o);
					o->curly.impjumptime = -100;
					o->curly.impjump = 1;
				}
			}
		}
		else o->curly.impjumptime = 0;
		
		// if we're below the target try jumping around randomly
		if (o->y > o->ymark && (o->y - o->ymark) > (16<<CSF))
		{
			if (++o->curly.tryjumptime > 20)
			{
				o->curly.tryjumptime = 0;
				if (random(0, 1)) CaiJUMP(o);
			}
		}
		else o->curly.tryjumptime = 0;
	}
	else o->animtimer = o->animframe = 0;		// reset walk anim
	
	// force jump/fall frames
	if (o->yinertia < 0) o->frame = 2;
	else if (!o->blockd) o->frame = 3;
	else if (o->x==o->xmark) o->frame = 0;
	
	// the improbable jump - when AI gets confused, just cheat!
	// jump REALLY high by reducing gravity until we clear the wall
	if (o->curly.impjump > 0)
	{
		o->yinertia += 0x10;
		// deactivate Improbable Jump once we clear the wall or hit the ground
		if (o->dir==LEFT && !o->blockl) o->curly.impjump--;
		if (o->dir==RIGHT && !o->blockr) o->curly.impjump--;
		if (o->yinertia > 0 && o->blockd) o->curly.impjump--;
	}
	else o->yinertia += 0x33;
	
	//fall slower in water
	
	// slow down when we hit bricks
	if (o->blockl || o->blockr)
	{
		// full stop if on ground, partial stop if in air
		xlimit = o->blockd ? 0x000:0x180;
		
		if (o->blockl)
		{
			if (o->xinertia < -xlimit) o->xinertia = -xlimit;
		}
		else if (o->xinertia > xlimit)		// we don't have to test blockr because we already know one or the other is set and that it's not blockl
		{
			o->xinertia = xlimit;
		}
	}
	
	// look up/down at target
	o->curly.look = 0;
	if (!reached_p || abs(o->y - player->y) > (48<<CSF))
	{
		if (o->y > o->ymark && ydist >= (12<<CSF) && (!seeking_player || ydist >= (80<<CSF))) o->curly.look = UP;
		else if (o->y < o->ymark && !o->blockd && ydist >= (80<<CSF)) o->curly.look = DOWN;
	}
	
	if (o->curly.look == UP) o->frame += 4;
	else if (o->curly.look == DOWN) o->frame += 8;
	
	LIMITX(0x300);
	LIMITY(0x5ff);
}

static void CaiJUMP(Object *o)
{
	if (o->blockd && !o->blocku)
	{
		o->yinertia = random(-0x600, -0x300);
		o->frame = 3;
		sound(SND_PLAYER_JUMP);
	}
}

/*
void c------------------------------() {}
*/

void ai_cai_gun(Object *o)
{
Object *curly = o->linkedobject;
Object *shot;
uchar fire;
int x, y, dir;
#define SMALLDIST		(32 << CSF)
#define BIGDIST			(160 << CSF)

	if (!curly) { o->Delete(); return; }
	
	o->frame = 0;
	if (curly->curly.look)
	{
		o->sprite = (curly->curly.gunsprite + 1);
		if (curly->curly.look==DOWN) o->frame = 1;
	}
	else
	{
		o->sprite = curly->curly.gunsprite;
	}
	
	if (game.curlytarget.timeleft)
	{
		// fire when we get close to the target
		if (!curly->curly.look)
		{	// firing LR-- fire when lined up vertically and close by horizontally
			fire = ((abs(o->x - game.curlytarget.x) <= BIGDIST) && (abs(o->y - game.curlytarget.y) <= SMALLDIST));
		}
		else
		{	// firing vertically-- fire when lined up horizontally and close by vertically
			fire = ((abs(o->x - game.curlytarget.x) <= SMALLDIST) && (abs(o->y - game.curlytarget.y) <= BIGDIST));
		}
		
		if (fire)
		{
			// get coordinate of our action point
			x = (o->ActionPointX() - o->DrawPointX());
			y = (o->ActionPointY() - o->DrawPointY());
			dir = curly->curly.look ? curly->curly.look : o->dir;
			
			if (curly->curly.gunsprite==SPR_MGUN)
			{	// she has the Machine Gun
				if (!o->timer)
				{
					o->timer2 = random(2, 6);		// no. shots to fire
					o->timer = random(40, 50);
					o->animtimer = 0;
				}
				
				if (o->timer2)
				{	// create the MGun blast
					if (!o->animtimer)
					{
						FireLevel23MGun(x, y, 2, dir);
						
						o->animtimer = 5;
						o->timer2--;
					}
					else o->animtimer--;
				}
			}
			else
			{	// she has the Polar Star
				if (!o->timer)
				{
					o->timer = random(4, 16);
					if (random(0, 10)==0) o->timer += random(20, 30);
					
					// create the shot
					shot = CreateObject(x, y, OBJ_POLAR_SHOT);
					SetupBullet(shot, x, y, B_PSTAR_L3, dir);
				}
			}
		}
	}
	
	if (o->timer) o->timer--;
}

void aftermove_cai_gun(Object *o)
{
	Object *curly = o->linkedobject;
	if (curly)
	{
		o->x = curly->ActionPointX();
		o->y = curly->ActionPointY();
		o->dir = curly->dir;
	}
}


// curly's air bubble when she goes underwater
void aftermove_cai_watershield(Object *o)
{
	Object *curly = o->linkedobject;
	if (!curly)
	{
		o->Delete();
		return;
	}
	
	static const Point cwp = { 8, 5 };
	if (curly->GetAttributes(&cwp, 1, NULL) & TA_WATER)
	{
		o->invisible = false;
		o->x = curly->x;
		o->y = curly->y;
		
		o->frame = (++o->timer & 2) ? 1 : 0;
	}
	else
	{
		o->invisible = true;
		o->timer = o->frame = 0;
	}
}


