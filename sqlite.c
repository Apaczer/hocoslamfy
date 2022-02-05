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

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

#include "log.c/src/log.h"
#include "path.h"

static sqlite3* db;
static char* version = "0";

static void UpgradeToVersion1(bool* Continue, bool* Error)
{
    log_trace("UpgradeToVersion1 called");

    char *zErrMsg = 0;
    int rc = sqlite3_exec(db, "INSERT INTO version (version_number) VALUES ('1.0.0');", NULL, 0, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        log_error("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        *Error = true;
        return;
    }

    log_trace("UpgradeToVersion1 finished");
}

static void UpgradeIfRequired(bool* Continue, bool* Error)
{
    log_debug("Version is: %s", version);
    if (strcmp(version, "0") == 0)
    {
        UpgradeToVersion1(Continue, Error);
    }
}

static int ReadVersion(void* NotUsed, int columns, char** columnTexts, char** columnNames)
{
    log_trace("ReadVersion called");
    if (columns > 0 && columnTexts[0])
    {
        version = columnTexts[0];
        log_debug("Version updated to: %s", version);
    }

    return 0;
}

void InitializeRepository(bool* Continue, bool* Error)
{
    log_trace("InitializeRepository called");

	char databaseFile[256];
    GetFullPath(databaseFile, "hocoslamfy.db");

    int rc = sqlite3_open(databaseFile, &db);
    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        *Error = true;
        return;
    }

    char *zErrMsg = 0;
    rc = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS version (version_number TEXT); SELECT version_number FROM version;", ReadVersion, 0, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        *Error = true;
        return;
    }

    UpgradeIfRequired(Continue, Error);
    log_trace("InitializeRepository finished");
}

extern void FinalizeRepository(void)
{
    sqlite3_close(db);
}