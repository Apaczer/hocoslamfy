/*
 * Hocoslamfy, main program file
 * Copyright (C) 2014 Nebuleon Fumika <nebuleon@gcw-zero.com>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "main.h"

#include <stddef.h>
#include <SDL/SDL_events.h>

#include "init.h"
#include "platform.h"

#include "log.c/src/log.h"
#include "path.h"
FILE *logFile;

static bool  Continue                          		= true;
static bool  Error                             		= false;

bool Rumble = false;
bool FollowBee = false;

SDL_Surface* Screen                               	= NULL;
SDL_Surface* TitleScreenFrames[TITLE_FRAME_COUNT] 	= { NULL };
SDL_Surface* BackgroundImages[BG_LAYER_COUNT]     	= { NULL };
SDL_Surface* CharacterFrames                      	= NULL;
SDL_Surface* ColumnImage                          	= NULL;
SDL_Surface* CollisionImage                       	= NULL;
SDL_Surface* GameOverFrame                        	= NULL;
SDL_Surface* ArrowFrames							= NULL;

TGatherInput GatherInput;
TDoLogic     DoLogic;
TOutputFrame OutputFrame;

void CleanUp()
{
	log_trace("CleanUp called");
	Finalize();

	GatherInput = NULL;
	DoLogic = NULL;
	OutputFrame = NULL;

#if LOGGING
	if (logFile)
	{
		fclose(logFile);
	}
#endif
}

int main(int argc, char* argv[])
{
#if LOGGING
	char logFileName[256];
	GetFullPath(logFileName, "hocoslamfy.log");
	logFile = fopen(logFileName, "a");
	if (logFile)
	{
		log_add_fp(logFile, LOG_TRACE);
	}
#endif

	Initialize(&Continue, &Error);
	Uint32 Duration = 16;
	while (Continue)
	{
		GatherInput(&Continue);
		if (!Continue)
			break;
		DoLogic(&Continue, &Error, Duration);
		if (!Continue)
			break;
		OutputFrame();
		Duration = ToNextFrame();
	}

	atexit(CleanUp);

	return Error ? 1 : 0;
}