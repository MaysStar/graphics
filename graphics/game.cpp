#include "game.h"

#include <stdarg.h>

static void CopyString(char* Dest, mm DestCount, const char* Source)
{
    if (!Dest || !DestCount)
    {
        return;
    }

    if (!Source)
    {
        Dest[0] = '\0';
        return;
    }

    mm Index = 0;
    for (; Index + 1 < DestCount && Source[Index]; ++Index)
    {
        Dest[Index] = Source[Index];
    }
    Dest[Index] = '\0';
}

static void DebugLog(const char* Format, ...)
{
    char Buffer[256];
    va_list Args;
    va_start(Args, Format);
    vsnprintf(Buffer, sizeof(Buffer), Format, Args);
    va_end(Args);

    OutputDebugStringA(Buffer);
}

static void InitializeBoss(boss* Boss,
    const char* Name,
    const char* Title,
    i32 DifficultyTier,
    f32 MaxHealth,
    f32 DamagePerSecond)
{
    CopyString(Boss->Name, ArrayCount(Boss->Name), Name);
    CopyString(Boss->Title, ArrayCount(Boss->Title), Title);
    Boss->DifficultyTier = DifficultyTier;
    Boss->MaxHealth = MaxHealth;
    Boss->CurrentHealth = MaxHealth;
    Boss->DamagePerSecond = DamagePerSecond;
}

static void InitializeLevel(level* Level,
    const char* Name,
    const char* Environment,
    f32 TravelDistance,
    f32 CompletionTime,
    boss Boss)
{
    CopyString(Level->Name, ArrayCount(Level->Name), Name);
    CopyString(Level->Environment, ArrayCount(Level->Environment), Environment);
    Level->TravelDistance = TravelDistance;
    Level->CompletionTime = CompletionTime;
    Level->Boss = Boss;
}

const level* GetCurrentLevel(const game_state* Game)
{
    const level* Result = 0;
    if (Game && Game->Map.CurrentLevelIndex < Game->Map.LevelCount)
    {
        Result = Game->Map.Levels + Game->Map.CurrentLevelIndex;
    }

    return Result;
}

static void ChangeEncounterState(game_state* Game, encounter_state NewState)
{
    if (!Game)
    {
        return;
    }

    Game->EncounterState = NewState;
    Game->Metrics.TimeInState = 0.0f;

    if (NewState == EncounterState_GameComplete)
    {
        DebugLog("All levels completed!\n");
        return;
    }

    const level* Level = GetCurrentLevel(Game);
    if (!Level)
    {
        return;
    }

    switch (NewState)
    {
    case EncounterState_Exploration:
    {
        DebugLog("Entering %s - exploring %s\n", Level->Name, Level->Environment);
    } break;

    case EncounterState_BossIntro:
    {
        DebugLog("Encountered %s, %s (Tier %d)\n", Level->Boss.Name, Level->Boss.Title, Level->Boss.DifficultyTier);
    } break;

    case EncounterState_BossFight:
    {
        DebugLog("Battle with %s has begun!\n", Level->Boss.Name);
    } break;

    case EncounterState_LevelComplete:
    {
        DebugLog("%s cleared!\n", Level->Name);
    } break;

    default:
    {
    } break;
    }
}

void InitializeGameState(game_state* Game)
{
    *Game = {};

    player* Player = &Game->Player;
    CopyString(Player->Name, ArrayCount(Player->Name), "Warden");
    Player->MaxHealth = 250.0f;
    Player->CurrentHealth = Player->MaxHealth;
    Player->DamagePerSecond = 28.0f;
    Player->TravelSpeed = 1.75f;

    boss EmberLion = {};
    InitializeBoss(&EmberLion, "Ember Lion", "Guardian of the Furnace", 1, 220.0f, 12.0f);

    boss CrystalMatriarch = {};
    InitializeBoss(&CrystalMatriarch, "Crystal Matriarch", "Voice of the Echoing Hall", 2, 320.0f, 18.0f);

    boss StormColossus = {};
    InitializeBoss(&StormColossus, "Storm Colossus", "Tempest Incarnate", 3, 480.0f, 25.0f);

    InitializeLevel(Game->Map.Levels + 0, "Forge Approach", "Collapsed foundry bridges", 120.0f, 90.0f, EmberLion);
    InitializeLevel(Game->Map.Levels + 1, "Crystal Gallery", "Shattered atrium of Sponza", 150.0f, 120.0f, CrystalMatriarch);
    InitializeLevel(Game->Map.Levels + 2, "Tempest Throne", "Open roof drenched in rain", 180.0f, 150.0f, StormColossus);

    Game->Map.LevelCount = 3;
    Game->Map.CurrentLevelIndex = 0;

    Game->Metrics.LevelProgress = 0.0f;

    ChangeEncounterState(Game, EncounterState_Exploration);
}

static void ResetForNextLevel(game_state* Game)
{
    if (!Game)
    {
        return;
    }

    if (Game->Map.CurrentLevelIndex < Game->Map.LevelCount)
    {
        level* Level = Game->Map.Levels + Game->Map.CurrentLevelIndex;
        Level->Boss.CurrentHealth = Level->Boss.MaxHealth;
    }

    Game->Metrics.LevelProgress = 0.0f;
    Game->Player.CurrentHealth = Game->Player.MaxHealth;
}

void UpdateGameState(game_state* Game, f32 DeltaTime)
{
    if (!Game)
    {
        return;
    }

    Game->Metrics.TimeInState += DeltaTime;

    if (Game->Map.CurrentLevelIndex >= Game->Map.LevelCount)
    {
        Game->EncounterState = EncounterState_GameComplete;
        return;
    }

    level* Level = Game->Map.Levels + Game->Map.CurrentLevelIndex;
    boss* Boss = &Level->Boss;

    switch (Game->EncounterState)
    {
    case EncounterState_Exploration:
    {
        Game->Metrics.LevelProgress += Game->Player.TravelSpeed * DeltaTime;
        if (Game->Metrics.LevelProgress > Level->TravelDistance)
        {
            Game->Metrics.LevelProgress = Level->TravelDistance;
        }

        if (Game->Metrics.LevelProgress >= Level->TravelDistance)
        {
            ChangeEncounterState(Game, EncounterState_BossIntro);
        }
    } break;

    case EncounterState_BossIntro:
    {
        if (Game->Metrics.TimeInState >= 2.0f)
        {
            ChangeEncounterState(Game, EncounterState_BossFight);
        }
    } break;

    case EncounterState_BossFight:
    {
        Boss->CurrentHealth -= Game->Player.DamagePerSecond * DeltaTime;
        if (Boss->CurrentHealth < 0.0f)
        {
            Boss->CurrentHealth = 0.0f;
        }

        Game->Player.CurrentHealth -= Boss->DamagePerSecond * 0.5f * DeltaTime;
        if (Game->Player.CurrentHealth < 0.0f)
        {
            Game->Player.CurrentHealth = 0.0f;
        }

        if (Boss->CurrentHealth <= 0.0f)
        {
            ChangeEncounterState(Game, EncounterState_LevelComplete);
        }
        else if (Game->Player.CurrentHealth <= 0.0f)
        {
            DebugLog("%s fell to %s. Restarting level...\n", Game->Player.Name, Boss->Name);
            ResetForNextLevel(Game);
            ChangeEncounterState(Game, EncounterState_Exploration);
        }
    } break;

    case EncounterState_LevelComplete:
    {
        if (Game->Metrics.TimeInState >= 3.0f)
        {
            ++Game->Map.CurrentLevelIndex;
            if (Game->Map.CurrentLevelIndex >= Game->Map.LevelCount)
            {
                ChangeEncounterState(Game, EncounterState_GameComplete);
            }
            else
            {
                ResetForNextLevel(Game);
                ChangeEncounterState(Game, EncounterState_Exploration);
            }
        }
    } break;

    case EncounterState_GameComplete:
    {
        // NOTE: Nothing to update once the game is complete.
    } break;
    }
}
