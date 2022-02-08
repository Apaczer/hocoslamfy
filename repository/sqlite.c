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
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

#include "log.c/src/log.h"
#include "path.h"
#include "score.h"
#include "repository.h"

static sqlite3* db;
static char* version = "0";

static int ReadPlayersList(ListOfPlayer* players)
{
    sqlite3_stmt* res;
    int rc = sqlite3_prepare_v2(db, "SELECT player_id, player_name, player_highscore FROM player;", -1, &res, 0);    
    if (rc != SQLITE_OK) {
        log_error("Failed to fetch data: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    players->list = malloc(sizeof(Player));
    players->count = 0;
    while ((rc = sqlite3_step(res)) == SQLITE_ROW)
    {
        void* reallocated = realloc(players->list, sizeof(Player) * (players->count + 1));
        if (reallocated == NULL)
        {
            log_error("Reallocation failed");
            sqlite3_finalize(res);
            exit(EXIT_FAILURE);
        }
        else
        {
            players->list = reallocated;
        }

        char* raw = (char*)sqlite3_column_text(res, 1);
        char* playerName = malloc(sizeof(char) * (int)sqlite3_column_bytes(res, 1));
        strcpy(playerName, raw);

        Player player = {
            .PlayerId = sqlite3_column_int(res, 0),
            .PlayerName = playerName,
            .Highscore = sqlite3_column_int(res, 2)
        };

        players->list[players->count++] = player;
    }

    sqlite3_finalize(res);
    return 0;
}

static void FreePlayersList(ListOfPlayer* players)
{
    for (int i = 0; i < players->count; i++)
    {
        free(players->list[i].PlayerName);
    }
    free(players->list);
}

static void UpgradeToVersion1_0_0(bool* Continue, bool* Error)
{
    char* targetVersion = "1.0.0";
    char* zErrMsg = 0;
    char statement[1024];
    snprintf(statement, 1024, "INSERT INTO version (version_number) VALUES ('%s');", targetVersion);
    int rc = sqlite3_exec(db, statement, NULL, 0, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        log_error("Create Version table, SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        *Error = true;
        return;
    }

    uint32_t score = GetHighScore();
    snprintf(statement, 1024, "CREATE TABLE player (player_id INTEGER PRIMARY KEY, player_name TEXT, player_highscore INTEGER); INSERT INTO player (player_name, player_highscore) VALUES ('%s', %d);", "Player1", score);
    rc = sqlite3_exec(db, statement, NULL, 0, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        log_error("Create Player table, SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        *Error = true;
        return;
    }
    log_debug("Player table created");

    version = targetVersion;
    log_debug("Database version updated to: %s", version);
}

static void UpgradeIfRequired(bool* Continue, bool* Error)
{
    if (strcmp(version, "0") == 0)
    {
        log_debug("Upgrade from version 0");
        UpgradeToVersion1_0_0(Continue, Error);
    }
}

static int ReadVersion(void* NotUsed, int columns, char** columnTexts, char** columnNames)
{
    if (columns > 0 && columnTexts[0])
    {
        version = columnTexts[0];
        log_debug("Local version updated to: %s", version);
    }

    return 0;
}

void InitializeRepository(bool* Continue, bool* Error)
{
	char databaseFile[256];
    GetFullPath(databaseFile, "hocoslamfy.db");

    int rc = sqlite3_open(databaseFile, &db);
    if (rc)
    {
        log_error("Can't open database: %s\n", sqlite3_errmsg(db));
        *Error = true;
        return;
    }

    char *zErrMsg = 0;
    rc = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS version (version_number TEXT); SELECT version_number FROM version;", ReadVersion, 0, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        log_error("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
        *Error = true;
        return;
    }

    UpgradeIfRequired(Continue, Error);

    ListOfPlayer players;
    if (ReadPlayersList(&players) == 0 && players.list && players.count)
    {
        for (int i = 0; i < players.count; i++)
        {
            log_debug("Player %s found, Id: %d, Highscore: %d", players.list[i].PlayerName, players.list[i].PlayerId, players.list[i].Highscore);
        }
        FreePlayersList(&players);
    }

    log_trace("InitializeRepository finished");
}

void FinalizeRepository(void)
{
    sqlite3_close(db);
}