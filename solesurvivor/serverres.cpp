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
#include "serverres.h"

GameResultClass::GameResultClass(int icount)
{
	InitialCount = icount;
	
	if (InitialCount <= 0) {
		InitialCount = 20;
	}
	
	ActiveCount = 0;
	GrowCount = InitialCount;
	TotalCount = GrowCount;
	Results = new GameResultEntry[TotalCount];
	
	if (Results == NULL) {
		TotalCount = 0;
	}
}

GameResultClass::~GameResultClass(void)
{
	if (TotalCount && Results) {
		delete[] Results;
	}
}

void GameResultClass::Reset(void)
{
	if (Results) {
		delete[] Results;
	}
	Results = 0;
		
	if (InitialCount == 0) {
		InitialCount = 20;
	}
	
	ActiveCount = 0;
	GrowCount = InitialCount;
	TotalCount = GrowCount;
	Results = new GameResultEntry[TotalCount];
	
	if (Results == NULL) {
		TotalCount = 0;
	}
}

bool GameResultClass::Grow(void)
{
	GameResultEntry *res;
	int count;
	
	if (GrowCount == 0) {
		return true;
	}
	
	count = TotalCount + GrowCount;
	res = new GameResultEntry[count];
	if (!res) {
		return true;
	}
	
	memcpy(res, Results, sizeof(GameResultEntry) * TotalCount);
	delete[] Results;
	
	Results = res;
	TotalCount = count;
	
	return false;
}

bool GameResultClass::Push_Result(GameResultEntry *res)
{
	int count;

	if (res == NULL) {
		return true;
	}
	
	if (ActiveCount == TotalCount) {
		if (GameResultClass::Grow()) {
			return true;
		}
	}
	
	count = ActiveCount;
	
	memcpy(&Results[count], res, sizeof(GameResultEntry));
	ActiveCount++;
	return false;
}

GameResultClass::GameResultEntry *GameResultClass::Pop_Result(void)
{
	GameResultEntry *res;
	int idx;

	if (ActiveCount == 0) {
		return NULL;
	}
	
	idx = ActiveCount - 1;
	res = new GameResultEntry;
	
	if (res == NULL) {
		return NULL;
	}
	
	memcpy(res, &Results[idx], sizeof(GameResultEntry));
	
	ActiveCount--;
	
	return res;
}

void GameResultClass::Set_Game_State(StateStruct *state)
{
	if (state != NULL) {
		memcpy(&GameState, state, sizeof(StateStruct));
	}
}

GameResultClass::StateStruct *GameResultClass::Get_Game_State(void)
{
	StateStruct *state = new StateStruct;
	
	if (state == NULL) {
		return NULL;
	}
	
	memcpy(state, &GameState, sizeof(StateStruct));
	return state;
}

void GameResultClass::Get_Game_State(GameResultClass::StateStruct *state)
{
	if (state != NULL) {
		memcpy(state, &GameState, sizeof(StateStruct));
	}
}


bool GameResultClass::Host_To_Net(void **dataptr, int *totalsize)
{
	StateStruct s;
	u_long count;
	unsigned char *d;
	GameResultEntry *r;
	int size;
	int i;

	if (dataptr == NULL || totalsize == 0) {
		return true;
	}
	
	size = sizeof(StateStruct);
	size += (ActiveCount * sizeof(GameResultEntry));
	size += sizeof(ActiveCount);
	d = new unsigned char[size];
	
	if (d == NULL) {
		return true;
	}
	
	s.TimeLimit = htonl(GameState.TimeLimit);
	s.ScoreLimit = htonl(GameState.ScoreLimit);
	s.LifeLimit = htonl(GameState.LifeLimit);
	s.GameMode = htonl(GameState.GameMode);
	s.IsLadderGame = htonl(GameState.IsLadderGame);
	
	memcpy(d, &s, sizeof(StateStruct));

	count = htonl(ActiveCount);
	memcpy(d + sizeof(StateStruct), &count, sizeof(count));
	
	printf("Number of players in game results: %d, %d\n", ActiveCount, count);
	
	r = (GameResultEntry*)((unsigned char *)d + sizeof(StateStruct) + sizeof(ActiveCount));
	
	for (i = 0; i < ActiveCount; i++) {
		memcpy(r, &Results[i], sizeof(GameResultEntry));
		r->ScoredPoints = htons(r->ScoredPoints);
		r->TotalDeaths = htons(r->TotalDeaths);
		r->chosentype = htons(r->chosentype);
		r->IsWinnerOrLoser = htons(r->IsWinnerOrLoser);
		r->PlayerCount = htonl(r->PlayerCount);
		r->TimeIngame = htonl(r->TimeIngame);
		r->dword1C_score = htonl(r->dword1C_score);
		//BUG, this won't set the unmodified number..
		r->timing3_score = (float)htonl(*(long *)&r->timing3_score);
		r++;
	}
	
	*dataptr = d;
	*totalsize = size;
	return false;
}

bool GameResultClass::Net_To_Host(unsigned char *d, int unk)
{
	int count;
	GameResultEntry *r;
	int i;

	if (d == NULL || unk == 0) {
		return true;
	}
	r = (GameResultEntry *)d;
	
	memcpy(&GameState, r, sizeof(StateStruct));
	GameState.TimeLimit = ntohl(GameState.TimeLimit);
	GameState.ScoreLimit = ntohl(GameState.ScoreLimit);
	GameState.LifeLimit = ntohl(GameState.LifeLimit);
	GameState.GameMode = ntohl(GameState.GameMode);
	GameState.IsLadderGame = ntohl(GameState.IsLadderGame);
	
	r = (GameResultEntry *)((unsigned char *)r + sizeof(StateStruct));
	count = *(int *)r;
	ActiveCount = ntohl(count);
	
	r = (GameResultEntry*)((unsigned char *)r + sizeof(ActiveCount));
	
	while (ActiveCount > TotalCount) {
		if (GameResultClass::Grow()) {
			return true;
		}
	}
	
	for (i = 0; i < ActiveCount; i++) {
		memcpy(&Results[i], r, sizeof(GameResultEntry));
		
		Results[i].ScoredPoints = ntohs(Results[i].ScoredPoints);
		Results[i].TotalDeaths = ntohs(Results[i].TotalDeaths);
		Results[i].chosentype = ntohs(Results[i].chosentype);
		Results[i].IsWinnerOrLoser = ntohs(Results[i].IsWinnerOrLoser);
		Results[i].PlayerCount = ntohl(Results[i].PlayerCount);
		Results[i].TimeIngame = ntohl(Results[i].TimeIngame);
		Results[i].dword1C_score = ntohl(Results[i].dword1C_score);
		//BUG, this won't set the unmodified number..
		Results[i].timing3_score = (float)ntohl(*(long *)&Results[i].timing3_score);
		++r;
	}
	return false;
}
