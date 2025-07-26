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
#include "common/endianness.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

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
	unsigned count;
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
	
	s.TimeLimit = htobe32(GameState.TimeLimit);
	s.ScoreLimit = htobe32(GameState.ScoreLimit);
	s.LifeLimit = htobe32(GameState.LifeLimit);
	s.GameMode = htobe32(GameState.GameMode);
	s.IsLadderGame = htobe32(GameState.IsLadderGame);
	
	memcpy(d, &s, sizeof(StateStruct));

	count = htobe32(ActiveCount);
	memcpy(d + sizeof(StateStruct), &count, sizeof(count));
	
	printf("Number of players in game results: %d, %u\n", ActiveCount, count);
	
	r = (GameResultEntry*)((unsigned char *)d + sizeof(StateStruct) + sizeof(ActiveCount));
	
	for (i = 0; i < ActiveCount; i++) {
		memcpy(r, &Results[i], sizeof(GameResultEntry));
		r->ScoredPoints = htobe16(r->ScoredPoints);
		r->TotalDeaths = htobe16(r->TotalDeaths);
		r->chosentype = htobe16(r->chosentype);
		r->IsWinnerOrLoser = htobe16(r->IsWinnerOrLoser);
		r->PlayerCount = htobe32(r->PlayerCount);
		r->TimeIngame = htobe32(r->TimeIngame);
		r->dword1C_score = htobe32(r->dword1C_score);
		//BUG, this won't set the unmodified number..
		r->timing3_score = (float)htobe32(*(long *)&r->timing3_score);
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
	GameState.TimeLimit = be32toh(GameState.TimeLimit);
	GameState.ScoreLimit = be32toh(GameState.ScoreLimit);
	GameState.LifeLimit = be32toh(GameState.LifeLimit);
	GameState.GameMode = be32toh(GameState.GameMode);
	GameState.IsLadderGame = be32toh(GameState.IsLadderGame);
	
	r = (GameResultEntry *)((unsigned char *)r + sizeof(StateStruct));
	count = *(int *)r;
	ActiveCount = be32toh(count);
	
	r = (GameResultEntry*)((unsigned char *)r + sizeof(ActiveCount));
	
	while (ActiveCount > TotalCount) {
		if (GameResultClass::Grow()) {
			return true;
		}
	}
	
	for (i = 0; i < ActiveCount; i++) {
		memcpy(&Results[i], r, sizeof(GameResultEntry));
		
		Results[i].ScoredPoints = be16toh(Results[i].ScoredPoints);
		Results[i].TotalDeaths = be16toh(Results[i].TotalDeaths);
		Results[i].chosentype = be16toh(Results[i].chosentype);
		Results[i].IsWinnerOrLoser = be16toh(Results[i].IsWinnerOrLoser);
		Results[i].PlayerCount = be32toh(Results[i].PlayerCount);
		Results[i].TimeIngame = be32toh(Results[i].TimeIngame);
		Results[i].dword1C_score = be32toh(Results[i].dword1C_score);
		//BUG, this won't set the unmodified number..
		Results[i].timing3_score = (float)be32toh(*(long *)&Results[i].timing3_score);
		++r;
	}
	return false;
}
