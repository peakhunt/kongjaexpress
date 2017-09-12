///////////////////////////////////////////////////////////////////////////////
//
// it looks like hkim thinks JSON has already dominated configuration and
// protocol syntax world
//
///////////////////////////////////////////////////////////////////////////////
#ifndef __CONFIG_READER_DEF_H__
#define __CONFIG_READER_DEF_H__

typedef union
{
   char*    s;
   int      i;
   double   d;
} ConfigParamValue;

typedef enum
{
   ConfigParamType_Integer,
   ConfigParamType_Double,
   ConfigParamType_String,
} ConfigParamType;

typedef struct
{
   char*             path;
   ConfigParamType   type;
   int               offset;
} ConfigParamDef;

#define CONFIG_PARAM_DEF(prefix, name, ptype, ds_type)\
   {\
      .path    =  #prefix"."#name,\
      .type    = ptype,\
      .offset  = offsetof(ds_type, name),\
   }

extern int init_config_reader(const char* filename);
extern void deinit_config_reader(void);

extern int config_get_param(const char* path, ConfigParamValue* v);
extern int config_load_params(ConfigParamDef* defs, int num_defs, void* base);
extern int config_dump_params(ConfigParamDef* defs, int num_defs, void* base);

#endif //!__CONFIG_READER_DEF_H__
