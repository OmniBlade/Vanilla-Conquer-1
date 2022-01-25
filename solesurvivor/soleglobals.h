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

#ifndef SOLEGLOBALS_H
#define SOLEGLOBALS_H

#include "common/timer.h"
#include "common/vector.h"
#include "function.h"

extern bool LogTeams;
extern bool OfflineMode;
extern bool WDTRadarAdded;
extern bool ServerConnectionLost;
extern bool CratesDisabled;
extern bool IsServerAdmin;
extern bool UseAltArt;
extern CELL FlagHomes[4];
extern time_t IdleTime;

extern HousesType Side;
extern HousesType SolePlayerHouse;
extern RTTIType ChosenRTTI;
extern int ChosenType;
extern int Steel;
extern int Green;
extern int Orange;
extern int TeamScores[4];
extern char TeamMessages[10][80];
extern int GameOption_577AEC;
extern int GameOption_577AF0;
extern int GameOption_577B00;
extern int CurrentVoiceTheme;
extern int GameEvent;
extern int ClientEvent1;
extern int ColorListTimer_5586C0;
extern unsigned GameOptionsBitfield;
extern bool GameOption_577B08;
extern bool GameOption_577B0C;
extern bool SoleEnhancedDefense;
extern bool DebugLogTeams;
extern bool CountingDown;
extern bool ClientEvent2;

extern int ClientFPS;
extern int LastServerAIFrame;
extern int CommStatsSpeedScale;
extern int RecievedBytesSec;
extern int SentBytesSec;
extern int SentTCP;
extern int SentUDP;
extern int RecievedTCP;
extern int RecievedUDP;
extern int StatTiming1;
extern int StatTiming2;
extern CountDownTimerClass FramerateUpdateTimer;
extern CountDownTimerClass TransmisionStatsTimer;
extern CountDownTimerClass RegisterCratesTimer;
extern CountDownTimerClass SquadGameStartTimer;
extern TimerClass GameTimer;
extern char SoleHost[40];

extern int Density;
extern int CrateDensity;

class ProtocolClass;
class ListenerClass;
extern ProtocolClass* ListenerProtocol;
extern ListenerClass* Listener;

class ReliableProtocolClass;
class ReliableCommClass;
extern DynamicVectorClass<ReliableProtocolClass*> ReliableProtocols;
extern DynamicVectorClass<ReliableCommClass*> ReliableComms;
extern DynamicVectorClass<ReliableProtocolClass*> AdminProtocols;
extern DynamicVectorClass<ReliableCommClass*> AdminComms;

class SolePlayerClass;
extern DynamicVectorClass<SolePlayerClass*> SolePlayers;

struct NewDeleteData;
struct HealthData;
struct DamageData;
struct CrushData;
struct CaptureData;
struct CargoData;
struct FlagData;
struct CTFData;
struct MovementData;
struct TargetData;
struct FireAtData;
struct DoTurnData;
struct CrateData;
struct PerCellData;
struct TechnoData;
extern DynamicVectorClass<NewDeleteData*> NewDeletePackets;
extern DynamicVectorClass<HealthData*> HealthPackets;
extern DynamicVectorClass<DamageData*> DamagePackets;
extern DynamicVectorClass<CrushData*> CrushPackets;
extern DynamicVectorClass<CaptureData*> CapturePackets;
extern DynamicVectorClass<CargoData*> CargoPackets;
extern DynamicVectorClass<FlagData*> FlagPackets;
extern DynamicVectorClass<CTFData*> CTFPackets;
extern DynamicVectorClass<MovementData*> MovementPackets;
extern DynamicVectorClass<TargetData*> TargetPackets;
extern DynamicVectorClass<FireAtData*> FireAtPackets;
extern DynamicVectorClass<DoTurnData*> DoTurnPackets;
extern DynamicVectorClass<CrateData*> CratePackets;
extern DynamicVectorClass<PerCellData*> PerCellPackets;
extern DynamicVectorClass<TechnoData*> TechnoPackets;

#ifdef _WIN32
extern HINSTANCE hWSockInstance;
#endif

#endif
