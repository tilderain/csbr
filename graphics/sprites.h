
#ifndef _SPRITES_H
#define _SPRITES_H

#define MAX_SPRITESHEETS	64
#define MAX_SPRITES			1024

#include "../siflib/sif.h"
extern SIFSprite sprites[MAX_SPRITES];


namespace Sprites
{
	bool Init();
	void Close();
	void FlushSheets();
	
	static void LoadSheetIfNeeded(int spr);
	
	static void BlitSprite(int x, int y, int s, int frame, uint8_t dir, \
						int xoff, int yoff, int wd, int ht);

	static void BlitSprite_Nonaligned(int x, int y, int s, int frame, uint8_t dir, \
						int xoff, int yoff, int wd, int ht);

	void draw_sprite(int x, int y, int s, int frame=0, uint8_t dir=0);
	void draw_sprite_at_dp(int x, int y, int s, int frame=0, uint8_t dir=0);


	void draw_sprite_nonaligned(int x, int y, int s, int frame=0, uint8_t dir=0);
	void draw_sprite_at_dp_nonaligned(int x, int y, int s, int frame=0, uint8_t dir=0);

	void draw_sprite_clipped(int x, int y, int s, int frame, uint8_t dir, int clipx1, int clipx2, int clipy1, int clipy2);
	void draw_sprite_clip_width(int x, int y, int s, int frame, int wd);
	void draw_sprite_clip_length(int x, int y, int s, int frame, int ln);
	void draw_sprite_chopped(int x, int y, int s, int frame, int wd, int repeat_at);
	void draw_sprite_repeating_x(int x, int y, int s, int frame, int wd);
	
	NXSurface *get_spritesheet(int sheetno);
	int create_spritesheet(int wd, int ht);
	void draw_sprite_to_surface(NXSurface *dst, int x, int y, int s, int frame, uint8_t dir);

};




#endif
