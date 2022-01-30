/*
 * Hocoslamfy, background rendering code file
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

#include <math.h>

#include <SDL/SDL.h>
#include "main.h"
#include "init.h"
#include "game.h"
#include "bg.h"

// The width after which the background repeats. (In pixels of the screen.)
#define BG_WIDTH 160
// The X coordinates from which the various layers of the background start to
// be rendered. (In meters.)
static       float    BG_X     [BG_LAYER_COUNT] = {
	0.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 0.0f
};
// The speed at which the X coordinate moves, for each piece of the background.
// (In meters per second.)
static const float    BG_Speed [BG_LAYER_COUNT] = {
	BG_SPEED_1, BG_SPEED_2, BG_SPEED_3, BG_SPEED_4,
	BG_SPEED_5, BG_SPEED_3, BG_SPEED_4, BG_SPEED_5
};

// The Y coordinates at which the various layers of the background start, and
// their heights, on the screen. (In pixels of the screen.)
static const uint32_t BG_StartY[BG_LAYER_COUNT] = {
	/* "Sky.png", "Mountains.png", "Clouds3.png", "Clouds2.png",
	 * "Clouds1.png", "Grass3.png", "Grass2.png", "Grass1.png" */
#if SCREEN_HEIGHT == 160
	 30,  84,  12,  -2,
	-16, 130, 138, 150
#else
	 50, 128,  32,  16,
	  0, 180, 190, 204
#endif
};
static const uint32_t BG_Height[BG_LAYER_COUNT] = {
	140,  60,  28,  28,
	 32,  20,  28,  36
};

void AdvanceBackground(uint32_t Milliseconds)
{
	uint32_t i;
	for (i = 0; i < BG_LAYER_COUNT; i++)
	{
		BG_X[i] = fmodf(BG_X[i] + SCREEN_WIDTH * BG_Speed[i] * Milliseconds / 1000, BG_WIDTH);
	}
}

void DrawBackground(void)
{
	uint32_t i;
	for (i = 0; i < BG_LAYER_COUNT; i++)
	{
		SDL_Rect SourceRect = {
			.x = (int) BG_X[i],
			.y = 0,
			.w = SCREEN_WIDTH,
			.h = BG_Height[i] };
		SDL_Rect DestRect = {
			.x = 0,
			.y = BG_StartY[i],
			.w = SCREEN_WIDTH,
			.h = BG_Height[i] };
		SDL_BlitSurface(BackgroundImages[i], &SourceRect, Screen, &DestRect);
	}
}
