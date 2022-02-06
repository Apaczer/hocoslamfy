/*
 * Hocoslamfy, score screen code file
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

#include <stdbool.h>
#include <stdint.h>
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <unistd.h>
#include <sys/types.h>

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>

#include "main.h"
#include "init.h"
#include "platform.h"
#include "game.h"
#include "score.h"
#include "bg.h"
#include "text.h"
#include "audio.h"
#include "path.h"

static bool  WaitingForRelease = false;
static char* ScoreMessage      = NULL;
static const char* HighScoreFilePath = "highscore";

static const char users[1][10] =
{
	"User 01"
};

static int currentUser = 0;

void ScoreGatherInput(bool* Continue)
{
	SDL_Event ev;

	while (SDL_PollEvent(&ev))
	{
		if (IsEnterGamePressingEvent(&ev))
			WaitingForRelease = true;
		else if (IsEnterGameReleasingEvent(&ev))
		{
			WaitingForRelease = false;
			ToGame();
			if (ScoreMessage != NULL)
			{
				free(ScoreMessage);
				ScoreMessage = NULL;
			}
			return;
		}
		else if (IsExitGameEvent(&ev))
		{
			*Continue = false;
			if (ScoreMessage != NULL)
			{
				free(ScoreMessage);
				ScoreMessage = NULL;
			}
			return;
		}
		else if (IsLeftEvent(&ev))
		{
			if (currentUser == 0)
			{
				currentUser = sizeof(users) / sizeof(char[10]) - 1;
			}
			else
			{
				currentUser--;
			}
		}
		else if (IsRightEvent(&ev))
		{
			if (currentUser == sizeof(users) / sizeof(char[10]) - 1)
			{
				currentUser = 0;
			}
			else {
				currentUser++;
			}
		}
		else if (IsScreenshotEvent(&ev))
		{
			MakeScreenshot();
		}
	}
}

void ScoreDoLogic(bool* Continue, bool* Error, Uint32 Milliseconds)
{
	AdvanceBackground(Milliseconds);
}

void ScoreOutputFrame()
{
	DrawBackground();

	SDL_Rect HeaderDestRect = {
		.x = (SCREEN_WIDTH - GameOverFrame->w) / 2,
		.y = ((SCREEN_HEIGHT / 6) - GameOverFrame->h) / 2,
		.w = GameOverFrame->w,
		.h = GameOverFrame->h
	};
	SDL_Rect HeaderSourceRect = {
		.x = 0,
		.y = 0,
		.w = GameOverFrame->w,
		.h = GameOverFrame->h
	};
	SDL_BlitSurface(GameOverFrame, &HeaderSourceRect, Screen, &HeaderDestRect);

	RenderLeft((SCREEN_WIDTH / 2) - 80 - 16 - 2, (SCREEN_HEIGHT / 6) + ((SCREEN_HEIGHT / 12) - 5));
	RenderRight((SCREEN_WIDTH / 2) + 80 + 2, (SCREEN_HEIGHT / 6) + ((SCREEN_HEIGHT / 12) - 5));

	char fontFilePath[256];
	snprintf(fontFilePath, 256, DATA_PATH "%s", "ProFont.ttf");

	// load font and its outline
	TTF_Font* font = TTF_OpenFont(fontFilePath, 10);
	TTF_Font* font_outline = TTF_OpenFont(fontFilePath, 10); 
	TTF_SetFontOutline(font_outline, 1); 

	// render text and text outline
	SDL_Color white = {0xFF, 0xFF, 0xFF}; 
	SDL_Color black = {0x00, 0x00, 0x00}; 
	SDL_Surface *bg_surface = TTF_RenderText_Blended(font_outline, users[currentUser], black); 
	SDL_Surface *fg_surface = TTF_RenderText_Blended(font, users[currentUser], white); 
	SDL_Rect rect = {1, 1, fg_surface->w, fg_surface->h}; 

	// blit text onto its outline
	SDL_BlitSurface(fg_surface, NULL, bg_surface, &rect); 
	SDL_FreeSurface(fg_surface); 

	// we now have RGBA white text with black outline in bg_surface
	SDL_Rect dstrect = { 10, 10, bg_surface->w, bg_surface->h };
	SDL_BlitSurface(bg_surface, NULL, Screen, &dstrect);
	SDL_FreeSurface(bg_surface);
	TTF_CloseFont(font_outline);
	TTF_CloseFont(font);

	if (SDL_MUSTLOCK(Screen))
		SDL_LockSurface(Screen);
/*
	PrintStringOutline(users[currentUser],
		SDL_MapRGB(Screen->format, 255, 255, 255),
		SDL_MapRGB(Screen->format, 0, 0, 0),
		Screen->pixels,
		Screen->pitch,
		0,
		(SCREEN_HEIGHT / 6) + ((SCREEN_HEIGHT / 12) - 6),
		SCREEN_WIDTH,
		18,
		CENTER,
		MIDDLE);
*/

	PrintStringOutline(ScoreMessage,
		SDL_MapRGB(Screen->format, 255, 255, 255),
		SDL_MapRGB(Screen->format, 0, 0, 0),
		Screen->pixels,
		Screen->pitch,
		0,
		(SCREEN_HEIGHT / 6) * 2,
		SCREEN_WIDTH,
		SCREEN_HEIGHT - ((SCREEN_HEIGHT / 6) * 2),
		CENTER,
		MIDDLE);
	if (SDL_MUSTLOCK(Screen))
		SDL_UnlockSurface(Screen);

	SDL_Flip(Screen);
}

void ToScore(uint32_t Score, enum GameOverReason GameOverReason, uint32_t HighScore)
{
	if (ScoreMessage != NULL)
	{
		free(ScoreMessage);
		ScoreMessage = NULL;
	}
	int Length = 2, NewLength;
	ScoreMessage = malloc(Length);

	const char* GameOverReasonString = "";
	switch (GameOverReason)
	{
		case FIELD_BORDER_COLLISION:
#if SCREEN_WIDTH < 320
			GameOverReasonString = "You flew too far away\nfrom the field";
#else
			GameOverReasonString = "You flew too far away from the field";
#endif
			break;
		case RECTANGLE_COLLISION:
#if SCREEN_WIDTH < 320
			GameOverReasonString = "You crashed into\na bamboo shoot";
#else
			GameOverReasonString = "You crashed into a bamboo shoot";
#endif
			break;
	}

	const char *MaybeNew;
	if (Score > HighScore)
	{
		MaybeNew = "NEW ";
		HighScore = Score;
		PlaySFXHighScore();
	} else {
		MaybeNew = "";
	}

	while ((NewLength = snprintf(ScoreMessage, Length,
		"%s\n\n"
#if SCREEN_HEIGHT < 240
		"Score: %" PRIu32 " / %sHigh: %" PRIu32 "\n\n"
#else
		"Your score was %" PRIu32 "\n\n"
		"%sHigh Score: %" PRIu32 "\n\n"
#endif
		"Press %s to play again\nor %s to exit",
		GameOverReasonString, Score, MaybeNew, HighScore,
		GetEnterGamePrompt(), GetExitGamePrompt())
		) >= Length)
	{
		Length = NewLength + 1;
		ScoreMessage = realloc(ScoreMessage, Length);
	}

	GatherInput = ScoreGatherInput;
	DoLogic     = ScoreDoLogic;
	OutputFrame = ScoreOutputFrame;
}

void SaveHighScore(uint32_t Score)
{
	char path[256];
	GetFullPath(path, HighScoreFilePath);

	FILE *fp = fopen(path, "w");

	if (!fp)
	{
		fprintf(stderr, "%s: Unable to open file.\n", path);
		return;
	}

	fprintf(fp, "%" PRIu32, Score);
	fclose(fp);
}

void GetFileLine(char *str, uint32_t size, FILE *fp)
{
	int i = 0;
	for (i = 0; i < size - 1; i++)
	{
		int c = fgetc(fp);
		if (c == EOF || c == '\n')
		{
			str[i] = '\0';
			break;
		}
		str[i] = c;
	}
	str[size - 1] = '\0';
}

uint32_t GetHighScore()
{
	char path[256];
	GetFullPath(path, HighScoreFilePath);

	FILE *fp = fopen(path, "r");

	if (!fp)
		return 0;

	char line[256];
	GetFileLine(line, 256, fp);
	fclose(fp);
	fp = NULL;
	
	uint32_t hs = 0;
	if (sscanf(line, "%" SCNu32, &hs) != 1)
		return 0;

	return hs;
}