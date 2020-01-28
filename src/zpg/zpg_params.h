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

static void ZPG_InitParams()
{
    ZPGParam* param;
    param = &g_paramTypes[g_numParamTypes++];
    param->asciChar = 'v';
    param->type = ZPG_PARAM_TYPE_FLAG;
    param->data.flag = ZPG_API_FLAG_PRINT_WORKING;
    param->helpText = "-v Verbose. Print each step\n";

    param = &g_paramTypes[g_numParamTypes++];
    param->type = ZPG_PARAM_TYPE_FLAG;
    param->asciChar = 'p';
    param->data.flag = ZPG_API_FLAG_PRINT_RESULT;
    param->helpText = "-p Print final result\n";

    param = &g_paramTypes[g_numParamTypes++];
    param->type = ZPG_PARAM_TYPE_FUNCTION;
    param->asciChar = 'a';
    param->data.func = ZPG_ParamAsciOutputFile;
    param->helpText = "-a <filename> Save final result as asci to filename\n";

    param = &g_paramTypes[g_numParamTypes++];
    param->type = ZPG_PARAM_TYPE_FUNCTION;
    param->asciChar = 'i';
    param->data.func = ZPG_ParamImageOutputFile;
    param->helpText = "-i <filename> Save final result as png to filename\n";
}

static void ZPG_Params_PrintHelp()
{
    printf("--- OPTIONS ---\n");
    for (i32 i = 0; i < g_numParamTypes; ++i)
    {
        printf("%s", g_paramTypes[i].helpText);
    }
    printf("\n");
}

static ZPGPresetCfg ZPG_Params_ReadForPreset(i32 argc, char** argv)
{
    ZPGPresetCfg cfg = {};
    // skip first two args.
    // Read third arg as preset number
    
    cfg.preset = atoi(argv[2]);
    
    i32 i = 3;
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
        for (i32 j = 0; j < g_numParamTypes; ++j)
        {
            ZPGParam* param = &g_paramTypes[j];
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
            }
        }
        if (bRecognised == NO)
        {
            printf("Unrecognised setting %s\n", arg);
        }
    }
    printf("Run preset %d\n", cfg.preset);
    printf("Read flags %d\n", cfg.flags);
    if (cfg.asciOutput != NULL)
    {
        printf("Save ASCI to %s\n", cfg.asciOutput);
    }
    if (cfg.imageOutput != NULL)
    {
        printf("Save PNG output to %s\n", cfg.imageOutput);
    }
    return cfg;
}

#endif // ZPG_PARAMS_H