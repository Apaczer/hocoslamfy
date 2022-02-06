/*
 * Hocoslamfy, path helper functions
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

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#ifndef DONT_USE_PWD
#include <pwd.h>
#include <unistd.h>
#endif

static const char* SavePath = ".hocoslamfy";

int MkDir(char *path)
{
#ifndef DONT_USE_PWD
	return mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);
#else
	return mkdir(path);
#endif
}

void GetFullPath(char* path, const char* filename)
{
#ifndef DONT_USE_PWD
#if USE_HOME	
	char *home = getenv("HOME");
	
	snprintf(path, 256, "%s/%s", home, SavePath);
	MkDir(path);	
	
	snprintf(path, 256, "%s/%s/%s", home, SavePath, filename);	
#else
	struct passwd *pw = getpwuid(getuid());
	
	snprintf(path, 256, "%s/%s", pw->pw_dir, SavePath);
	MkDir(path);
	
	snprintf(path, 256, "%s/%s/%s", pw->pw_dir, SavePath, filename);
#endif	
#else
	snprintf(path, 256, "%s", filename);
#endif
}