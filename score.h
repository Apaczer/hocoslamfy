/*
 * Hocoslamfy, score screen header
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

#ifndef _SCORE_H_
#define _SCORE_H_

enum GameOverReason
{
	FIELD_BORDER_COLLISION,
	RECTANGLE_COLLISION
};

extern void ToScore(uint32_t Score, enum GameOverReason GameOverReason);
extern uint32_t GetHighScore(void);

#endif /* !defined(_SCORE_H_) */
