///////////////////////////////////////////////////////////////////////////////
//
// it looks like hkim thinks JSON has already dominated configuration and
// protocol syntax world
//
///////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "common.h"
#include "util.h"
#include "cJSON.h"
#include "config_reader.h"

///////////////////////////////////////////////////////////////////////////////
//
// module statics & prototypes
//
///////////////////////////////////////////////////////////////////////////////
static char* load_file_to_buffer(const char* filename);
static cJSON* get_json_obj_from_path(const char* path);
static char* extract_subpath_and_advance(const char** current);

static cJSON*  _json_root = NULL;


///////////////////////////////////////////////////////////////////////////////
//
// utilities
//
///////////////////////////////////////////////////////////////////////////////
static char*
load_file_to_buffer(const char* filename)
{
  int         fd,
              ret;
  char*       buffer;
  struct stat sb;

  fd = open(filename,  O_RDONLY);
  if(fd < 0)
  {
    pr_err("failed to open config file: %s, %s\n", filename, strerror(errno));
    goto error1;
  }

  if(fstat(fd, &sb) == -1)
  {
    pr_err("fcntl failed: %s\n", strerror(errno));
    goto error2;
  }

  buffer = malloc(sb.st_size + 1);
  if(buffer == NULL)
  {
    pr_err("failed to allocate buffer %s\n");
    goto error2;
  }

  ret = read(fd, buffer, sb.st_size);
  if(ret != sb.st_size)
  {
    pr_err("failed in reading. %d, %d, %s\n", ret, sb.st_size, strerror(errno));
    goto error3;
  }

  buffer[sb.st_size] = '\0';
  close(fd);
  return buffer;

error3:
  free(buffer);
error2:
  close(fd);
error1:
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////
//
// utilities
//
///////////////////////////////////////////////////////////////////////////////
static char*
extract_subpath_and_advance(const char** current)
{
  char     *end,
           *buffer;

  if(*current == NULL)
  {
    return NULL;
  }

  end = strchr(*current, '.');
  if(end == NULL)
  {
    buffer = strdup(*current);
    *current = NULL;
  }
  else
  {
    buffer = strndup(*current, end - *current);
    *current += (end - *current + 1);
  }

  return buffer;
}

static cJSON*
get_json_obj_from_path(const char* path)
{
  const char* current = path;
  char*       subpath;
  cJSON*      node = _json_root;

  while((subpath = extract_subpath_and_advance(&current)) != NULL)
  {
    node = cJSON_GetObjectItem(node, subpath);
    free(subpath);

    if(node == NULL)
    {
      return NULL;
    }
  }
  return node == _json_root ? NULL : node;
}

///////////////////////////////////////////////////////////////////////////////
//
// accessors
//
///////////////////////////////////////////////////////////////////////////////
int
config_get_param(const char* path, ConfigParamValue* v)
{
  cJSON* node;

  node = get_json_obj_from_path(path);
  if(node == NULL)
  {
    pr_err("can't find parameter: %s\n", path);
    return -1;
  }

  switch(node->type)
  {
    case cJSON_Number:
      v->d = node->valuedouble;
      v->i = node->valueint;
      break;

    case cJSON_String:
      v->s = node->valuestring;
      break;

    default:
      BUG("unsupported object type: %d\n", node->type);
      break;
  }

  return 0;
}

#ifdef __TEST_CODE__
static void test()
{
  const char* params[] =
  {
    "file_info.description",
    "file_info.version",
    "file_info.Last_Revision",
    "ping_task_config.ping_interval_when_not_connectd",
    "ping_task_config.ping_interval_when_connected",
    "ping_task_config.num_responses_to_move_to_connected",
    "ping_task_config.num_misses_to_move_to_not_connected",
    "ping_task_config.ping_target",
    "ping_task_config.target_interface",
    "ping_task_config.ping_data_to_send",
  };
  int i;
  ConfigParamValue  v;

  for(i = 0; i < sizeof(params)/sizeof(char*); i++)
  {
    if(config_get_param(params[i], &v) != 0)
    {
      pr_err("failed to get param: %s\n", params[i]);
    }
    else
    {
      pr_err("got param: %s\n", params[i]);
    }
  }
}
#endif 

///////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
///////////////////////////////////////////////////////////////////////////////
int
init_config_reader(const char* filename)
{
  char* buffer;

  buffer = load_file_to_buffer(filename);
  if(buffer == NULL)
  {
    return -1;
  }

  _json_root = cJSON_Parse(buffer);
  free(buffer);

  if(_json_root == NULL)
  {
    pr_err("failed to parse config file: %s\n", filename);
    return -1;
  }

#ifdef __TEST_CODE__
  pr_dbg("\n%s\n", cJSON_Print(_json_root));
  test();
#endif
  return 0;
}

void
deinit_config_reader(void)
{
  if(_json_root != NULL)
  {
    cJSON_Delete(_json_root);
    _json_root = NULL;
  }
}

int
config_load_params(ConfigParamDef* defs, int num_defs, void* base)
{
  int               i;
  ConfigParamValue  v;

  for(i = 0; i < num_defs; i++)
  {
    char* ptr;

    if(config_get_param(defs[i].path, &v) != 0)
    {
      pr_err("failed to get parameter: %s\n", defs[i].path);
      return -1;
    }

    ptr = (char*)((char*)base + defs[i].offset);

    switch(defs[i].type)
    {
      case ConfigParamType_Integer:
        *((int*)ptr) = v.i;
        break;

      case ConfigParamType_Double:
        *((double*)ptr) = v.d;
        break;

      case ConfigParamType_String:
        *((char**)ptr) = v.s;
        break;
    }
  }
  return 0;
}

int
config_dump_params(ConfigParamDef* defs,  int num_defs, void* base)
{
  int               i;

  for(i = 0; i < num_defs; i++)
  {
    char* ptr;

    ptr = (char*)((char*)base + defs[i].offset);

    switch(defs[i].type)
    {
      case ConfigParamType_Integer:
        pr_info("%-60s : %d\n", defs[i].path, *((int*)ptr));
        break;

      case ConfigParamType_Double:
        pr_info("%-60s : %f\n", defs[i].path, *((double*)ptr));
        break;

      case ConfigParamType_String:
        pr_info("%-60s : %s\n", defs[i].path, *((char**)ptr));
        break;
    }
  }
  return 0;
}
