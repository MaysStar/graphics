#if !defined(GAME_H)
#include "base_def.h"

struct boss
{
    char Name[32];
    char Title[64];
    i32 DifficultyTier;

    f32 MaxHealth;
    f32 CurrentHealth;
    f32 DamagePerSecond;
};

struct level
{
    char Name[32];
    char Environment[48];
    f32 TravelDistance;
    f32 CompletionTime;

    boss Boss;
};

struct game_map
{
    level Levels[8];
    u32 LevelCount;
    u32 CurrentLevelIndex;
};

struct player
{
    char Name[32];
    f32 MaxHealth;
    f32 CurrentHealth;
    f32 DamagePerSecond;
    f32 TravelSpeed;
};

enum encounter_state
{
    EncounterState_Exploration,
    EncounterState_BossIntro,
    EncounterState_BossFight,
    EncounterState_LevelComplete,
    EncounterState_GameComplete,
};

struct encounter_metrics
{
    f32 TimeInState;
    f32 LevelProgress;
};

struct game_state
{
    player Player;
    game_map Map;
    encounter_state EncounterState;
    encounter_metrics Metrics;
};

void InitializeGameState(game_state* Game);
void UpdateGameState(game_state* Game, f32 DeltaTime);
const level* GetCurrentLevel(const game_state* Game);

#define GAME_H
#endif
