
#include "../stdai.h"
#include "../weed/weed.fdh"
#include "village.fdh"

INITFUNC(AIRoutines)
{
	ONTICK(OBJ_TOROKO_SHACK, ai_toroko_shack);
	
	ONTICK(OBJ_MUSHROOM_ENEMY, ai_mushroom_enemy);
	ONTICK(OBJ_KAZUMA_AT_COMPUTER, ai_mushroom_enemy);
	ONTICK(OBJ_GIANT_MUSHROOM_ENEMY, ai_jumper_soa);
	ONTICK(OBJ_GRAVEKEEPER, ai_gravekeeper);
	
	ONTICK(OBJ_CAGE, ai_cage);
	ONSPAWN(OBJ_FLOWERS_PENS1, onspawn_snap_to_ground);
}

/*
void c------------------------------() {}
*/

// scared toroko attacking in Shack
void ai_toroko_shack(Object *o)
{
	//debug("Y:%d  Yinertia:%d  blockd:%d  flags:%d", o->y>>CSF, o->yinertia, o->blockd, o->flags);
	
	switch(o->state)
	{
		case 0:
			o->timer = 0;
			o->yinertia = -(2 << CSF);
			o->y -= (2 << CSF);
			o->flags |= FLAG_IGNORE_SOLID;
			o->state = 1;
		case 1:
		{
			ANIMATE(2, 0, 3);
			XMOVE(0x100);
			
			if (o->yinertia >= 0)
			{
				o->flags &= ~FLAG_IGNORE_SOLID;
				
				if (o->blockd)
				{
					ASSERT((o->y % TILE_H) == 0);
					
					o->yinertia = 0;
					o->state = 2;
				}
			}
		}
		break;
		
		case 2:
			o->frame = o->timer = o->animtimer = 0;
			o->state = 3;
		case 3:
		{
			ANIMATE(2, 0, 3);
			XACCEL(0x40);
			
			if (++o->timer >= 50)
			{
				o->timer = 40;
				o->xinertia = -o->xinertia;
				o->dir = (o->dir == RIGHT) ? LEFT:RIGHT;
			}
			
			if (o->timer >= 35)
				o->flags |= FLAG_SHOOTABLE;
			
			if (o->shaketime)
			{
				o->state = 4;
				o->frame = 4;
				o->yinertia = -(2 << CSF);
				o->flags &= ~FLAG_SHOOTABLE;
				o->damage = 0;
				o->timer = 0;
			}
		}
		break;
		
		case 4:
			XMOVE(0x100);
			
			if (++o->timer > 2 && o->blockd)
			{
				o->state = 5;
				o->flags |= FLAG_SCRIPTONACTIVATE;
			}
		break;
		
		case 5:
			o->xinertia = 0;
			o->frame = 5;
		break;
	}
	
	o->yinertia += 0x40;
	LIMITX(0x400);
	LIMITY(0x5ff);
}


/*
void c------------------------------() {}
*/

// mushroom enemy ("Pignon") from Mimiga Graveyard
void ai_mushroom_enemy(Object *o)
{
	enum {
			INIT = 0,
			STANDING,
			BLINKING,
			WALKING, WALKING2,
			SHOT,
			JUMPING
	};
	
	switch(o->state)
	{
		case 0:
			o->frame = 0;
			o->animtimer = 0;
			o->xinertia = 0;
			o->state = STANDING;
		case STANDING:		// stand around
		{
			o->state = WALKING;
			o->frame = 0;
			o->timer = random(30, 300);
		}
		break;
		
		case WALKING:		// walking
			o->state++;
			o->frame = 2;
			o->animtimer = 0;
		case WALKING+1:
		{
			if (!--o->timer){
				o->yinertia += -0x420;
				o->state = JUMPING;
			}
			if (o->blockl && o->dir == LEFT) //if blockl in 8 pixels (1/2 a block) jump.
			{
				o->dir = RIGHT;
				o->xinertia = 0x100;
			}
			else if (o->blockr && o->dir == RIGHT)
			{
				o->dir = LEFT;
				o->xinertia = -0x100;
			}
			
			if (!o->blockd){
				o->state = JUMPING;
			}
			static const uint8_t walkanimframes[] = { 0, 2, 0, 4 };
		
			if (++o->animtimer >= 5)
			{
				o->animtimer = 0;
				if (++o->animframe >= 4) o->animframe = 0;
			}
			o->frame = walkanimframes[o->animframe];
			
			XMOVE(0x100);
		}
		break;
		
		case SHOT:		// hit by shot
			if (o->blockd)
				o->state = INIT;
		break;
		
		case JUMPING:		// hit by shot
			o->frame = 1;
			if (o->blockl && o->dir == LEFT) //if blockl in 8 pixels (1/2 a block) jump.
			{
				o->dir = RIGHT;
				o->xinertia = 0x100;
			}
			else if (o->blockr && o->dir == RIGHT)
			{
				o->dir = LEFT;
				o->xinertia = -0x100;
			}
			if (o->blockd)
			{
				o->timer = random(30, 300);
				o->state = WALKING;
			}
			XMOVE(0x100);
		break;
	}
	
	if (o->shaketime && o->state != SHOT)
	{
		o->state = SHOT;
		o->yinertia = -0x300;
		o->frame = 6;
	}
	
	o->yinertia += 0x25;
	LIMITY(0x5ff);
}

void ai_jumper_soa(Object *o)
{
	enum {
			INIT = 0,
			JUMPING,
			CROUCHING,
			SHOT
	};
	
	switch(o->state)
	{
		case 0:
			o->frame = 0;
			o->animtimer = 0;
			o->xinertia = 0;
			o->timer2 = 1;
			o->state = JUMPING;	
		case JUMPING:		// hit by shot
			o->frame = 0;
			if (o->blockl && o->dir == LEFT) //if blockl in 8 pixels (1/2 a block) jump.
			{
				o->dir = RIGHT;
				o->xinertia = 0x110;
			}
			else if (o->blockr && o->dir == RIGHT)
			{
				o->dir = LEFT;
				o->xinertia = -0x110;
			}
			if (o->blockd)
			{
				o->state = CROUCHING;
			}
			XMOVE(0x100);
		break;
		case CROUCHING:		// hit by shot
			o->frame = 1;
			o->xinertia = 0;
			if (++o->timer >= 7)
			{
				if (++o->timer2 >= 5)
				{
					o->timer2 = 0;
					o->yinertia = -0x750;
				}
				else
				{
					o->yinertia = -0x200;
				}
				o->timer = 0;
				o->state = JUMPING;
			}
		break;
		case SHOT:		// hit by shot
			if (o->blockd)
				o->state = INIT;
		break;
	}
	
	if (o->shaketime && o->state != SHOT)
	{
		o->state = SHOT;
		o->yinertia = -0x300;
		o->frame = 2;
	}
	
	o->yinertia += 0x25;
	LIMITY(0x5ff);
}


// guy with knife, Mimiga Graveyard
void ai_gravekeeper(Object *o)
{
	switch(o->state)
	{
		case 0:
			o->flags |= FLAG_SHOOTABLE;
			o->flags |= FLAG_INVULNERABLE;
			o->nxflags |= NXFLAG_FOLLOW_SLOPE;
			
			o->damage = 0;
			o->state = 1;
		case 1:
		{
			o->frame = 0;
			FACEPLAYER;
			
			// start walking when player comes near
			if (pdistlx(128 << CSF) && pdistly2(48<<CSF, 32<<CSF))
			{
				o->state = 2;
				o->animtimer = 0;
			}
			
			// start walking if shot
			if (o->shaketime)
			{
				o->state = 2;
				o->frame = 1;
				o->animtimer = 0;
				o->flags |= FLAG_INVULNERABLE;
			}
		}
		break;
		
		case 2:		// walking
		{
			ANIMATE(6, 0, 3);
			
			// reached knife range of player?
			// (no, he doesn't check Y)
			if (pdistlx(10 << CSF))
			{
				o->state = 3;
				o->timer = 0;
				
				sound(SND_FIREBALL);
				o->flags &= ~FLAG_INVULNERABLE;
			}
			
			FACEPLAYER;
			XMOVE(0x100);
		}
		break;
		
		case 3:		// knife raised
		{
			o->frame = 4;
			o->xinertia = 0;
			
			if (++o->timer > 40)
			{
				o->state = 4;
				o->timer = 0;
				sound(SND_SLASH);
			}
		}
		break;
		
		case 4:		// knife frame 2
		{
			o->frame = 5;
			o->damage = 1;
			
			if (++o->timer > 2)
			{
				o->state = 5;
				o->timer = 0;
			}
		}
		break;
		
		case 5:		// knife frame 3
		{
			o->frame = 6;
			if (++o->timer > 60)
				o->state = 0;
			
			if (o->xinertia > 0 && o->blockl) o->xinertia = 0;
			if (o->xinertia < 0 && o->blockr) o->xinertia = 0;
		}
		break;
	}
	
	o->yinertia += 0x20;
	LIMITX(0x400);
	LIMITY(0x5ff);
}

/*
void c------------------------------() {}
*/

void ai_cage(Object *o)
{
	if (o->state == 0)
	{
		// the Cage's drawpoint is used to tweak the positioning
		// of the camera, which is <FON'd on it, during the post-Egg cutscene,
		// see the rules for draw points in map_scroll_do().
		o->x += o->DrawPointX();
		o->y += o->DrawPointY();
		o->state = 1;
	}
}













