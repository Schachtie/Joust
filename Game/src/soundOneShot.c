#include <assert.h>
#include <stdlib.h>

#include "soundOneShot.h"
#include "sound.h"


typedef struct soundOneShot_t {
	int32_t soundId;
	uint32_t lengthMS;
} SoundOneShot;


static uint32_t _currentSoundLength = 0;
static int32_t _currentSoundId = SOUND_NOSOUND;
static uint32_t _internalTimer = 0;
static bool _isSoundPlaying = false;


/// <summary>
/// Creates a soundOneShot object.
/// </summary>
/// <param name="fileName"></param>
/// <param name="lengthMS"></param>
/// <returns></returns>
SoundOneShot* soundOneShotNew(const char* fileName, uint32_t lengthMS)
{
	SoundOneShot* soundOneShot = (SoundOneShot*)malloc(sizeof(SoundOneShot));
	if (soundOneShot != NULL)
	{
		soundOneShot->soundId = soundLoad(fileName);
		soundOneShot->lengthMS = lengthMS;
	}
	return soundOneShot;
}

/// <summary>
/// Deletes the soundOneShot object.
/// </summary>
/// <param name="soundOneShot"></param>
void soundOneShotDelete(SoundOneShot* soundOneShot)
{
	soundUnload(soundOneShot->soundId);
	free(soundOneShot);
}


/// <summary>
/// Updates the internal timer for the sound currently playing.
///		<para>
/// Note: MUST BE CALLED EVERY FRAME TO FUNCTION PROPERLY.
///		</para>
/// </summary>
/// <param name="milliseconds"></param>
void soundOneShotUpdateInternalFields(uint32_t milliseconds)
{
	_internalTimer += milliseconds;
	if (_internalTimer >= _currentSoundLength)
	{
		_isSoundPlaying = false;
	}
}


/// <summary>
/// Plays a sound in isolation.
/// </summary>
/// <param name="soundOneShot"></param>
/// <param name="priority"> - True will kill the currently playing sound to play the passed in soundOneShot.</param>
void soundOneShotPlayIsolated(const SoundOneShot* const soundOneShot, bool priority)
{
	assert(soundOneShot != NULL);
	if (_isSoundPlaying == false || priority == true)
	{
		soundStop(_currentSoundId);
		_currentSoundLength = soundOneShot->lengthMS;
		_currentSoundId = soundOneShot->soundId;
		_internalTimer = 0;
		_isSoundPlaying = true;
		soundPlay(soundOneShot->soundId);
	}
}
