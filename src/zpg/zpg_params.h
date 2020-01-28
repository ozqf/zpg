#ifndef ZPG_PARAMS_H
#define ZPG_PARAMS_H

#include "zpg_internal.h"

#define ZPG_PARAM_TYPE_FLAG 0
#define ZPG_PARAM_TYPE_STRING 1
#define ZPG_PARAM_TYPE_FUNCTION 2

static i32 ZPG_ParamPrintResult(i32 argc, char** argv, ZPGPresetCfg* cfg)
{
    printf("Set print result\n");
    cfg->flags |= ZPG_API_FLAG_PRINT_RESULT;
    return 0;
}

static i32 ZPG_ParamAsciOutputFile(i32 argc, char** argv, ZPGPresetCfg* cfg)
{
    if (argc < 1)
    {
        printf("Error: -a must specify file name for asci output\n");
        return 0;
    }
    cfg->asciOutput = argv[1];
    return 1;
}

static i32 ZPG_ParamImageOutputFile(i32 argc, char** argv, ZPGPresetCfg* cfg)
{
    if (argc < 1)
    {
        printf("Error: -a must specify file name for asci output\n");
        return 0;
    }
    cfg->imageOutput = argv[1];
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
    paramTypes[numParamTypes].type = ZPG_PARAM_TYPE_FUNCTION;
    paramTypes[numParamTypes].asciChar = 'a';
    paramTypes[numParamTypes].data.func = ZPG_ParamAsciOutputFile;
    numParamTypes++;
    paramTypes[numParamTypes].type = ZPG_PARAM_TYPE_FUNCTION;
    paramTypes[numParamTypes].asciChar = 'i';
    paramTypes[numParamTypes].data.func = ZPG_ParamImageOutputFile;
    numParamTypes++;

    ZPGPresetCfg cfg = {};
    // skip first two args.
    i32 i = 2;
    while (i < argc)
    {
        i32 index = i;
        i++;
        // extract option chararacter
        char* arg = argv[index];
        if (arg[0] != '-') 
        {
            printf("Unrecognised setting %s\n", arg);
            continue;
        }
        char c = arg[1];
        if (c == ' ') { continue; }
        // check param types
        i32 bRecognised = NO;
        for (i32 j = 0; j < numParamTypes; ++j)
        {
            ZPGParam* param = &paramTypes[j];
            if (param->asciChar == c)
            {
                bRecognised = YES;
                switch (param->type)
                {
                    case ZPG_PARAM_TYPE_FLAG:
                    cfg.flags |= param->data.flag;
                    break;
                    case ZPG_PARAM_TYPE_FUNCTION:
                    i += param->data.func((argc - index), &argv[index], &cfg);
                    break;
                }
                //paramTypes[j].func(argc, argv, &cfg);
            }
        }
        if (bRecognised == NO)
        {
            printf("Unrecognised setting %s\n", arg);
        }
    }
    printf("Read flags %d\n", cfg.flags);
    if (cfg.asciOutput != NULL)
    {
        printf("Save ASCI to %s\n", cfg.asciOutput);
    }
    if (cfg.imageOutput != NULL)
    {
        printf("Save PNG output to %s\n", cfg.imageOutput);
    }
}

#endif // ZPG_PARAMS_H