#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#ifndef DONT_USE_PWD
#include <pwd.h>
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