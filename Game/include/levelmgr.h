#pragma once
#include "baseTypes.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef enum lvlUpdOutcome_t {
    LUO_STARTWAVES,
    LUO_CONTINUE,
    LUO_NEXTWAVE,
    LUO_HISCORES,
    LUO_TITLE
} LUO;

typedef enum levelType_t {
    LEVELTYPE_TITLE,
    LEVELTYPE_WAVE,
    LEVELTYPE_WAVE_ENDLESS,
    LEVELTYPE_HISCORES
} LevelType;

typedef struct leveldef_t {
    LevelType type;
    
    uint8_t numBounders;
    uint8_t numHunters;
    uint8_t numShadowLords;
} LevelDef;

typedef struct level_t Level;


void levelMgrInit();
void levelMgrShutdown();

Level* levelMgrLoad(const LevelDef* levelDef);
void levelMgrUnload(Level* level);

void levelMgrDraw(Level* level);
LUO levelMgrUpdate(Level* level, uint32_t milliseconds);

LevelType levelGetType(const Level* const level);


#ifdef __cplusplus
}
#endif