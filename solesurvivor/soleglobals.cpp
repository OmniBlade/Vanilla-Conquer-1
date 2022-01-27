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
#include "soleglobals.h"
#include "common/sockets.h"

bool LogTeams;
bool OfflineMode;
bool WDTRadarAdded;
bool ServerConnectionLost;
bool CratesDisabled = true;
bool IsServerAdmin;
bool UseAltArt;
CELL FlagHomes[4];
time_t IdleTime;
int IonFactor = 200;

HousesType Side;
HousesType SolePlayerHouse;
RTTIType ChosenRTTI;
int ChosenType;
int Steel;
int Green;
int Orange;
int TeamScores[4];
char TeamMessages[10][80];
int GameOption_577AEC;
int GameOption_577AF0;
int GameOption_577B00;
int CurrentVoiceTheme = 1;
int GameEvent;
int ClientEvent1;
int ColorListTimer_5586C0;
unsigned GameOptionsBitfield;
bool GameOption_577B08;
bool GameOption_577B0C;
bool SoleEnhancedDefense;
bool DebugLogTeams;
bool CountingDown;
bool ClientEvent2;

int ClientFPS;
int LastServerAIFrame;
int CommStatsSpeedScale;
int RecievedBytesSec;
int SentBytesSec;
int SentTCP;
int SentUDP;
int RecievedTCP;
int RecievedUDP;
int StatTiming1;
int StatTiming2;
CountDownTimerClass FramerateUpdateTimer;
CountDownTimerClass TransmisionStatsTimer;
CountDownTimerClass RegisterCratesTimer;
CountDownTimerClass SquadGameStartTimer;
char SoleHost[40];

int Density = 200;
int CrateDensity;

// These arrays don't look correct, unclear what they should look like originally.
int StrengthsArray[3] = {800, 30, 120};
int DamagesArray[3] = {240, 20, 80};
int SpeedsArray[3] = {160, 25, 100};
int RateOfFiresArray[3] = {200, 384, 1536};
int RangessArray[6] = {3072, 850, 155, 53, 25, 1920};

ProtocolClass* ListenerProtocol;
ListenerClass* Listener;

DynamicVectorClass<ReliableProtocolClass*> ReliableProtocols;
DynamicVectorClass<ReliableCommClass*> ReliableComms;
DynamicVectorClass<ReliableProtocolClass*> AdminProtocols;
DynamicVectorClass<ReliableCommClass*> AdminComms;

DynamicVectorClass<SolePlayerClass*> SolePlayers;

DynamicVectorClass<NewDeleteData*> NewDeletePackets;
DynamicVectorClass<HealthData*> HealthPackets;
DynamicVectorClass<DamageData*> DamagePackets;
DynamicVectorClass<CrushData*> CrushPackets;
DynamicVectorClass<CaptureData*> CapturePackets;
DynamicVectorClass<CargoData*> CargoPackets;
DynamicVectorClass<FlagData*> FlagPackets;
DynamicVectorClass<CTFData*> CTFPackets;
DynamicVectorClass<MovementData*> MovementPackets;
DynamicVectorClass<TargetData*> TargetPackets;
DynamicVectorClass<FireAtData*> FireAtPackets;
DynamicVectorClass<DoTurnData*> DoTurnPackets;
DynamicVectorClass<CrateData*> CratePackets;
DynamicVectorClass<PerCellData*> PerCellPackets;
DynamicVectorClass<TechnoData*> TechnoPackets;

#ifdef _WIN32
HINSTANCE hWSockInstance = GetModuleHandleA(nullptr);
#endif

// Global class to ensure winsock is started and cleaned up correctly.
class SocketStartupGlobal
{
public:
    SocketStartupGlobal()
    {
        socket_startup();
    }

    ~SocketStartupGlobal()
    {
        socket_cleanup();
    }
} SocketStartup;
