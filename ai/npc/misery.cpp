
#include "../stdai.h"
#include "misery.fdh"

INITFUNC(AIRoutines)
{
	ONTICK(OBJ_MISERY_FLOAT, ai_misery_float);
	ONTICK(OBJ_MISERY_STAND, ai_misery_stand);
	ONTICK(OBJ_MISERYS_BUBBLE, ai_miserys_bubble);
}

/*
void c------------------------------() {}
*/

// teleport in and float
void ai_misery_float(Object *o)
{
	switch(o->state)
	{
		case 0:
			o->state = 1;
			o->x += (1 << CSF);	// check Undead Core intro to prove this is correct
			o->xmark = o->x;
			o->ymark = o->y;
			o->frame = 0;
			o->timer = 0;
		case 1:
			if (DoTeleportIn(o, 1))
				o->state = 10;
		break;
		
		case 10:	// floating
			o->state = 11;
			o->timer = 0;
			o->animframe = 0;
			o->yinertia = (1<<CSF);
		case 11:
			if (o->y > o->ymark) o->yinertia -= 16;
			if (o->y < o->ymark) o->yinertia += 16;
			if (o->yinertia > 0x100) o->yinertia = 0x100;
			if (o->yinertia < -0x100) o->yinertia = -0x100;
		break;
		
		case 13:	// fall from floaty
			o->frame = 1;
			o->flags &= ~FLAG_IGNORE_SOLID;
			
			o->yinertia += 0x40;
			if (o->yinertia > 0x5ff) o->yinertia = 0x5ff;
			
			if (o->blockd)
			{
				sound(SND_THUD);
				o->yinertia = 0;
				o->state = 14;
				o->flags |= FLAG_IGNORE_SOLID;
				o->animframe = 2;
				o->animtimer = 30;		// blink as soon as hit ground
			}
		break;
		
		case 14: break;			// standing
		
		// spawn the bubble which picks up Toroko in Shack
		case 15:
			o->frame = 4;
			o->timer = 0;
			o->state = 16;
		case 16:
		{
			o->timer++;
			
			if (o->timer == 30)
			{
				sound(SND_BUBBLE);
				CreateObject(o->x, o->y - (16 << CSF), OBJ_MISERYS_BUBBLE);
			}
			
			if (o->timer >= 50)
				o->state = 14;
		}
		break;
		
		case 20: 	// fly away
			o->state = 21;
			o->frame = 0;
			o->yinertia = 0;
			o->flags |= FLAG_IGNORE_SOLID;
		case 21:
			o->yinertia -= 0x20;
			if (o->y < -0x1000) o->Delete();
		break;
		
		case 25:	// big spell
		case 26:	// she flashes, then a clap of thunder
			ai_misery_stand(o);
		break;
		case 27:	// return to standing after lightning strike
			if (++o->timer > 16)
				o->state = 14;
		break;
	}
	
	if (o->state==11 || o->state==14)
	{
		// blink
		if (o->animtimer)
		{
			o->animtimer--;
			o->frame = 1;
		}
		else
		{
			o->frame = 0;
			if (random(0, 100) == 1)
			{
				o->animtimer = 30;
			}
		}
		
		o->frame += o->animframe;
	}
}

void ai_miserys_bubble(Object *o)
{
	switch(o->state)
	{
		case 0:
		{
			o->x -= (3 << CSF);
			o->y -= (4 << CSF);
			o->state = 1;
		}break;
		case 4:
		{
			sound(SND_LITTLE_CRASH);
			SmokeClouds(o, 20, 12, 12);
			o->Delete();
		}break;
	}
	ANIMATE(12, 1, 2);
}

static Object *mbubble_find_target(void)
{
	Object *target = FindObjectByID2(1000);
	if (!target)
	{
		staterr("ai_miserys_bubble: failed to find a target object with ID2=1000");
		return NULL;
	}
	
	return target;
}


/*
void c------------------------------() {}
*/

void ai_misery_stand(Object *o)
{
	switch(o->state)
	{
		case 0:
			o->state = 1;
		case 1:
			o->frame = 2;
			randblink(o, 3, 4);
		break;
		
		case 20:	// she flys away
			o->state = 21;
			o->frame = 0;
			o->yinertia = 0;
			o->flags |= FLAG_IGNORE_SOLID;
		case 21:
			o->yinertia -= 0x20;
			if (o->y < -0x1000) o->Delete();
		break;
		
		// big spell
		// she flashes, then a clap of thunder,
		// and she teleports away.
		case 25:
			o->state = 26;
			o->timer = 0;
			o->frame = 5;
			o->animtimer = 0;
		case 26:
			ANIMATE(0, 5, 7);
			if (++o->timer == 20)
			{
				sound(SND_LIGHTNING_STRIKE);
				flashscreen.Start();
				o->state = 27;
				o->timer = 0;
				o->frame = 4;
			}
		break;
		case 27:
		{
			if (++o->timer > 50)
			{	// return to standing
				o->state = 0;
			}
		}
		break;
		
		case 30:	// she throws up her staff like she's summoning something
			o->timer = 0;
			o->state++;
			o->frame = 2;
		case 31:
			if (o->timer==10) o->frame = 4;
			if (o->timer==130) o->state = 1;
			o->timer++;
		break;
		
		// fire at DOCTOR_GHOST
		case 40:
		{
			o->state = 41;
			o->timer = 0;
			o->frame = 4;
		}
		case 41:
		{
			o->timer++;
			
			if (o->timer == 30 || \
				o->timer == 40 || \
				o->timer == 50)
			{
				Object *shot = CreateObject(o->x+(16<<CSF), o->y, OBJ_IGOR_SHOT);
				shot->xinertia = 0x600;
				shot->yinertia = random(-0x200, 0);
				
				sound(SND_SNAKE_FIRE);
			}
			
			if (o->timer > 50)
				o->state = 0;
		}
		break;
	}
}

















