// TiberianDawn.DLL and RedAlert.dll and corresponding source code is free
// software: you can redistribute it and/or modify it under the terms of
// the GNU General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.

// TiberianDawn.DLL and RedAlert.dll and corresponding source code is distributed
// in the hope that it will be useful, but with permitted additional restrictions
// under Section 7 of the GPL. See the GNU General Public License in LICENSE.TXT
// distributed with this program. You should have received a copy of the
// GNU General Public License along with permitted additional restrictions
// with this program. If not, see https://github.com/electronicarts/CnC_Remastered_Collection
#ifndef SERVERRES_H
#define SERVERRES_H

class GameResultClass
{
public:
	#pragma pack(push, 1)
	struct GameResultEntry {
		char Name[12];
		unsigned short ScoredPoints;
		short TotalDeaths;
		short chosentype;
		short IsWinnerOrLoser;
		int PlayerCount;
		int TimeIngame;
		int dword1C_score;
		float timing3_score;
	};

	struct StateStruct {
		int TimeLimit;
		int ScoreLimit;
		int LifeLimit;
		int GameMode;
		int IsLadderGame;
	};
	#pragma pack(pop)
	
public:
	GameResultClass(int icount = 0);
	~GameResultClass(void);
	
	void Reset(void);
	bool Grow(void);
	
	bool Push_Result(GameResultEntry *res);
	GameResultEntry *Pop_Result(void);
	
	void Set_Game_State(StateStruct *state);
	StateStruct *Get_Game_State(void);
	void Get_Game_State(StateStruct *state);
	
	bool Host_To_Net(void **dataptr, int *totalsize);
	bool Net_To_Host(unsigned char *d, int unk);

private:
	int GrowCount;
	int InitialCount;
	StateStruct GameState;
	int ActiveCount;
	int TotalCount;
	GameResultEntry *Results;
};	

#endif
