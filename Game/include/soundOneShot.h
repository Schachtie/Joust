#pragma once

#include "stdbool.h"
#include "stdint.h"


typedef struct soundOneShot_t SoundOneShot;


SoundOneShot* soundOneShotNew(const char* fileName, uint32_t lengthMS);
void soundOneShotDelete(SoundOneShot* oneShot);

void soundOneShotUpdateInternalFields(uint32_t milliseconds);

void soundOneShotPlayIsolated(const SoundOneShot* const soundOneShot, bool priority);
