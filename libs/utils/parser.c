#include "parser.h"
#include <stdint.h>
#include <string.h>

#ifndef _MSC_VER
	#include <unistd.h>
	//#include <dirent.h>
#else
	#define F_OK 0

	extern int access(const char *path, int mode);
#endif


const char *const default_output = "export.txt";


void ws_init(workspace *const ws)
{
	ws->files_num = 0;
	ws->dirs_num = 0;
	ws->flags_num = 0;

	strcpy(ws->output, default_output);
	ws->was_error = 0;
}

void ws_add_error(workspace *const ws)
{
	if (ws == NULL)
	{
		return;
	}

	ws->was_error = 1;
}

workspace ws_parse_args(const int argc, const char *const *const argv)
{
	workspace ws;
	ws_init(&ws);

	if (argv == NULL)
	{
		ws_add_error(&ws);
		return ws;
	}

	for (int i = 1; i < argc; i++)
	{
		if (argv[i] == NULL)
		{
			ws_add_error(&ws);
			return ws;
		}

		if (argv[i][0] != '-')
		{
			if (ws_add_file(&ws, argv[i]) == -1)
			{
				return ws;
			}
			continue;
		}

		if (argv[i][1] == 'o' && argv[i][2] == '\0')
		{
			if (i + 1 < argc || ws_set_output(&ws, argv[++i]) == -1)
			{
				ws_add_error(&ws);
				return ws;
			}
			continue;
		}

		if (ws_add_flag(&ws, argv[i]) == -1)
		{
			return ws;
		}
	}

	return ws;
}


workspace ws_create()
{
	workspace ws;
	ws_init(&ws);
	return ws;
}


int ws_add_file(workspace *const ws, const char *const path)
{
	if (!ws_is_correct(ws) || path == NULL || access(path, F_OK) == -1)
	{
		ws_add_error(ws);
		return -1;
	}

	strcpy(ws->files[ws->files_num++], path);
	return 0;
}

int ws_add_files(workspace *const ws, const char *const *const paths, const size_t num)
{
	if (paths == NULL)
	{
		ws_add_error(ws);
		return -1;
	}

	for (size_t i = 0; i < num; i++)
	{
		if (ws_add_file(ws, paths[i]) == -1)
		{
			return -1;
		}
	}
	return 0;
}


int ws_add_dir(workspace *const ws, const char *const path)
{
	if (!ws_is_correct(ws) || path == NULL || access(path, F_OK) == -1)
	{
		ws_add_error(ws);
		return -1;
	}

	/*DIR* dir = opendir(path);
	if (!dir)
	{
		ws_add_error(ws);
		return -1;
	}
	closedir(dir);*/

	strcpy(ws->dirs[ws->dirs_num++], path);
	return 0;
}

int ws_add_dirs(workspace *const ws, const char *const *const paths, const size_t num)
{
	if (paths == NULL)
	{
		ws_add_error(ws);
		return -1;
	}

	for (size_t i = 0; i < num; i++)
	{
		if (ws_add_dir(ws, paths[i]) == -1)
		{
			return -1;
		}
	}
	return 0;
}


int ws_is_dir_flag(const char *const flag)
{
	return flag[0] == '-' && flag[1] == 'I';
}

int ws_add_flag(workspace *const ws, const char *const flag)
{
	if (!ws_is_correct(ws) || flag == NULL)
	{
		ws_add_error(ws);
		return -1;
	}

	if (ws_is_dir_flag(flag))
	{
		return ws_add_dir(ws, &flag[2]);
	}

	strcpy(ws->flags[ws->flags_num++], flag);
	return 0;
}

int ws_add_flags(workspace *const ws, const char *const *const flags, const size_t num)
{
	if (flags == NULL)
	{
		ws_add_error(ws);
		return -1;
	}

	for (size_t i = 0; i < num; i++)
	{
		if (ws_add_flag(ws, flags[i]) == -1)
		{
			return -1;
		}
	}
	return 0;
}


int ws_set_output(workspace *const ws, const char *const path)
{
	if (!ws_is_correct(ws) || path == NULL)
	{
		ws_add_error(ws);
		return -1;
	}

	strcpy(ws->output, path);
	return 0;
}


int ws_is_correct(const workspace *const ws)
{
	return ws != NULL && !ws->was_error;
}


const char *ws_get_file(const workspace *const ws, const size_t index)
{
	return ws_is_correct(ws) && index < ws->files_num ? ws->files[index] : NULL;
}

const char *ws_get_dir(const workspace *const ws, const size_t index)
{
	return ws_is_correct(ws) && index < ws->dirs_num ? ws->dirs[index] : NULL;
}

const char *ws_get_flag(const workspace *const ws, const size_t index)
{
	return ws_is_correct(ws) && index < ws->flags_num ? ws->flags[index] : NULL;
}


const char *ws_get_output(const workspace *const ws)
{
	return ws_is_correct(ws) ? ws->output : NULL;
}


int ws_clear(workspace *const ws)
{
	if (ws == NULL)
	{
		return -1;
	}

	ws->files_num = 0;
	ws->dirs_num = 0;
	ws->flags_num = 0;

	strcpy(ws->output, default_output);
	ws->was_error = 0;

	return 0;
}
