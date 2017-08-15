
#ifndef _SAVESELECT_H
#define _SAVESELECT_H

#include "../pause/message.h"

#define SS_LOADING		0
#define SS_SAVING		1
#define MAX_SAVE_SLOTS	5

class TB_SaveSelect
{
public:
	TB_SaveSelect();
	
	void ResetState();
	void SetVisible(bool enable, bool saving=SS_LOADING);
	
	bool IsVisible();
	void Draw();
	
	void SaveSettingsAndReturn();
	
	Options::Message *msg;
	int overwriteKey;
	
private:
	void Run_Input();
	void DrawProfile(int x, int y, int index);
	void DrawExtendedInfo();
	
	bool fVisible;
	bool fSaving;
	int fCurSel;
	int fNumFiles;
	
	struct { int x, y, w, h; } fCoords;
};


#endif
