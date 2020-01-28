#ifndef ZPG_PARAMS_H
#define ZPG_PARAMS_H

#include "zpg_internal.h"

#define ZPG_PARAM_TYPE_FLAG 0
#define ZPG_PARAM_TYPE_STRING 1
#define ZPG_PARAM_TYPE_FUNCTION 2

static i32 ZPG_ParamPrintResult(i32 argv, char** argc, ZPGPresetCfg* cfg)
{
    printf("Set print result\n");
    cfg->flags |= ZPG_API_FLAG_PRINT_RESULT;
    return 1;
}

static i32 ZPG_ParamPrintWorking(i32 argv, char** argc, ZPGPresetCfg* cfg)
{
    printf("Set print working\n");
    cfg->flags |= ZPG_API_FLAG_PRINT_WORKING;
    return 1;
}

struct ZPGParam
{
    i32 type;
    char asciChar;
    union
    {
        i32 flag;
        zpg_param_fn func;    
    } data;
};

static void ZPG_Params_ReadForPreset(i32 argc, char** argv)
{
    i32 numParamTypes = 0;
    ZPGParam paramTypes[64];
    paramTypes[numParamTypes].asciChar = 'v';
    paramTypes[numParamTypes].type = ZPG_PARAM_TYPE_FLAG;
    paramTypes[numParamTypes].data.flag = ZPG_API_FLAG_PRINT_WORKING;
    numParamTypes++;
    paramTypes[numParamTypes].type = ZPG_PARAM_TYPE_FLAG;
    paramTypes[numParamTypes].asciChar = 'p';
    paramTypes[numParamTypes].data.flag = ZPG_API_FLAG_PRINT_RESULT;
    numParamTypes++;

    ZPGPresetCfg cfg = {};
    // skip first two args.
    for (i32 i = 0; i < argc; ++i)
    {
        // extract option chararacter
        char* arg = argv[i];
        if (arg[0] != '-') { continue; }
        char c = arg[1];
        if (c == ' ') { continue; }
        // check param types
        for (i32 j = 0; j < numParamTypes; ++j)
        {
            ZPGParam* param = &paramTypes[j];
            if (param->asciChar == c)
            {
                switch (param->type)
                {
                    case ZPG_PARAM_TYPE_FLAG:
                    cfg.flags |= param->data.flag;
                    break;
                }
                //paramTypes[j].func(argc, argv, &cfg);
            }
        }
    }
    printf("Read flags %d\n", cfg.flags);
}

#endif // ZPG_PARAMS_H