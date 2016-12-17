
#include "weapons.h"
#include "snake.fdh"

// settings for wavy and acceleration of levels 2 & 3
#define SNAKE_VERT_SPEED		0x400
#define SNAKE_ACCEL_SPEED		0x80

INITFUNC(AIRoutines)
{
	ONTICK(OBJ_SNAKE1_SHOT, ai_snake);
	
	ONTICK(OBJ_SNAKE23_SHOT, ai_snake_23);
	ONTICK(OBJ_SNAKE_TRAIL, ai_snake_trail);
}

/*
void c------------------------------() {}
*/

void ai_snake(Object *o)
{
	if (o->state == 0)
	{
		o->state = 1;
		
		o->frame = random(0, 2);
	}
	
	if (--o->shot.ttl < 0)
	{
		// emphasis wave effect
		if (o->shot.dir == LEFT || o->shot.dir == RIGHT)
			o->y += o->yinertia;
		else
			o->x += o->xinertia;
		
		shot_dissipate(o, EFFECT_STARPOOF);
		return;
	}
	
	// spin in shot direction
	if (o->shot.dir == LEFT)
	{
		if (--o->frame < 0) o->frame = sprites[o->sprite].nframes - 1;
	}
	else
	{
		if (++o->frame >= sprites[o->sprite].nframes) o->frame = 0;
	}
	
	if (damage_enemies(o))
	{
		shot_dissipate(o, EFFECT_STARPOOF);
	} else if (IsBlockedInShotDir(o)) {
		shot_spawn_effect(o, EFFECT_FISHY);
		o->Delete();
	}
}



void ai_snake_23(Object *o)
{

		// accelerate the shot
		switch(o->shot.dir)
	{
		case LEFT:  o->xinertia = -0x900; break;
		case UP:    o->yinertia = -0x900; break;
		case RIGHT: o->xinertia = 0x900; break;
		case DOWN:  o->yinertia = 0x900; break;
	}
	
	// periodically abruptly change the wave's direction
	
	// spawn trails
	Object *trail = create_fire_trail(o, OBJ_SNAKE_TRAIL, o->shot.level);
	trail->frame = random(0, 2);
	trail->animtimer = 0;
	
	// ... and all the basic logic from level 1
	ai_snake(o);
}

/*
void c------------------------------() {}
*/

void ai_snake_trail(Object *o)
{
	if (++o->animtimer > 1)
	{
		o->animtimer = 0;
		o->frame += 3;
		
		if (o->frame >= sprites[o->sprite].nframes)
			o->Delete();
	}
}

