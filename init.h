/*
 * Hocoslamfy, initialisation header
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

#ifndef _INIT_H_
#define _INIT_H_

#include <stdbool.h>

#ifndef SCREEN_WIDTH
#define SCREEN_WIDTH  320
#endif
#ifndef SCREEN_HEIGHT
#define SCREEN_HEIGHT 240
#endif
#ifndef SCREEN_BPP
#define SCREEN_BPP    32
#endif

#ifdef OPK
#define DATA_PATH "./"
#else
#define DATA_PATH "./data/"
#endif

void Initialize(bool* Continue, bool* Error);
void Finalize(void);

#endif /* !defined(_INIT_H_) */
