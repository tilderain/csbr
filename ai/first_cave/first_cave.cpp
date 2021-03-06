
#include "../stdai.h"
#include "../weed/weed.fdh"	// for ai_critter
#include "first_cave.fdh"

INITFUNC(AIRoutines)
{
	ONTICK(OBJ_BAT_BLUE, ai_bat_up_down);
	ONTICK(OBJ_CRITTER_HOPPING_BLUE, ai_critter);
	
	ONTICK(OBJ_HERMIT_GUNSMITH, ai_hermit_gunsmith);
	ONTICK(OBJ_DOOR_ENEMY, ai_door_enemy);
}

/*
void c------------------------------() {}
*/

void ai_bat_up_down(Object *o) //propeller soa
{
	switch(o->state)
	{
		case 0:
			o->ymark = o->y;
			o->timer = random(0, 50);
			o->state = 1;
		case 1:
			if (!o->timer--)
			{
				o->state = 2;
				o->yinertia = 0x200;
			}
		break;
		
		case 2:
		{
			if (o->y >= o->ymark)
				o->yinertia -= 0x10;
			else
				o->yinertia += 0x10;
			
			LIMITY(0x250);
			if (o->blocku)
			{
				o->ymark = o->y + (36 << CSF);
				o->yinertia = 0x100;
			}
			
			if (o->blockd)
			{
				o->ymark = o->y - (36 << CSF);
				o->yinertia = -0x100;
			}
			if (o->blockr)
			{
				o->dir = LEFT;
				o->xinertia = -0x100;
			} 
			else if (o->blockl)
			{
				o->dir = RIGHT;
				o->xinertia = 0x100;
			}
		}
		break;
	}
	if (o->dir == LEFT)
	{
		o->xinertia -= 0x10;
	}
	else
	{
		o->xinertia += 0x10;
	}
	if (o->yinertia > 0)
	{
		ANIMATE(2, 4, 7);
	}
	else
	{
		ANIMATE(2, 0, 3);
	}
	LIMITX(0x156);
}

/*
void c------------------------------() {}
*/

void ai_hermit_gunsmith(Object *o)
{
	if (o->state == 0)
	{
		o->SnapToGround();
		o->state = 1;
	}
	
	if (o->dir == RIGHT)
	{
		ai_zzzz_spawner(o);
	}
	else
	{
		o->frame = 0;
		randblink(o, 1, 8);
	}
}



void ai_door_enemy(Object *o)
{
	enum { INIT = 0, WAIT, OPENEYE, CLOSEEYE };
	
	switch(o->state)
	{
		case 0:
			o->state = WAIT;
		
		case WAIT:
		{
			o->frame = 0;
			if (pdistlx(0x8000) && pdistly(0x8000))
			{
				o->animtimer = 0;
				o->state = OPENEYE;
			}
		}
		break;
		
		case OPENEYE:
		{
			if (++o->animtimer > 2)
			{
				o->animtimer = 0;
				o->frame++;
			}
			
			if (o->frame > 2)
			{
				o->frame = 2;
				
				if (!pdistlx(0x8000) || !pdistly(0x8000))
				{
					o->state = CLOSEEYE;
					o->animtimer = 0;
				}
			}
		}
		break;
		
		case CLOSEEYE:
		{
			if (++o->animtimer > 2)
			{
				o->animtimer = 0;
				if (--o->frame <= 0)
				{
					o->frame = 0;
					o->state = WAIT;
				}
			}
		}
		break;
	}
}



