#ifndef ZPG_PARAMS_H
#define ZPG_PARAMS_H

#include "zpg_internal.h"

#define ZPG_PARAM_TYPE_FLAG 0
#define ZPG_PARAM_TYPE_STRING 1
#define ZPG_PARAM_TYPE_FUNCTION 2
#define ZPG_PARAM_TYPE_INTEGER 3

// static i32 ZPG_ParamPrintResult(i32 argc, char** argv, ZPGPresetCfg* cfg)
// {
//     printf("Set print result\n");
//     cfg->flags |= ZPG_API_FLAG_PRINT_RESULT;
//     return 0;
// }

static i32 ZPG_ParamReadInt(i32 argc, char** argv, ZPGPresetCfg* cfg, i32 offsetBytes)
{
    if (argc <= 0) { printf("Cannot read int - no more params!\n"); return 0; }
    i32 val = atoi(argv[1]);
    i32* target = (i32*)((u8*)cfg + offsetBytes);
    *target = val;
    //printf("Set int at %d to %d\n\n", offsetBytes, *target);
    return 1;
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
        printf("Error: -i must specify file name for raw image output\n");
        return 0;
    }
    cfg->imageOutput = argv[1];
    return 1;
}

static i32 ZPG_ParamPictureOutputFile(i32 argc, char** argv, ZPGPresetCfg* cfg)
{
    if (argc < 1)
    {
        printf("Error: -c must specify file name for picture output\n");
        return 0;
    }
    cfg->pictureOutput = argv[1];
    return 1;
}

static void ZPG_InitParams()
{
    ZPGParam* param;

    ////////////////////////////////////////////////////
    // bit flag params
    
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
    param->type = ZPG_PARAM_TYPE_FLAG;
    param->asciChar = 'e';
    param->data.flag = ZPG_API_FLAG_NO_ENTITIES;
    param->helpText = "-e Skip adding entities\n";

    param = &g_paramTypes[g_numParamTypes++];
    param->type = ZPG_PARAM_TYPE_FLAG;
    param->asciChar = 'g';
    param->data.flag = ZPG_API_FLAG_PRINT_GREYSCALE;
    param->helpText = "-g print greyscale if applicable\n";

    param = &g_paramTypes[g_numParamTypes++];
    param->type = ZPG_PARAM_TYPE_FLAG;
    param->asciChar = 'm';
    param->data.flag = ZPG_API_FLAG_PRINT_FINAL_ALLOCS;
    param->helpText = "-m (debugging) print remaining allocs after run\n";

    param = &g_paramTypes[g_numParamTypes++];
    param->type = ZPG_PARAM_TYPE_FLAG;
    param->asciChar = 'b';
    param->data.flag = ZPG_API_FLAG_SOLID_BORDER;
    param->helpText = "-b Create a solid border around the generated map\n";
	
    ////////////////////////////////////////////////////
	// Integer params

    // param = &g_paramTypes[g_numParamTypes++];
    // param->type = ZPG_PARAM_TYPE_FLAG;
    // param->asciChar = 's';
    // param->data.flag = ZPG_API_FLAG_NO_ENTITIES;
    // param->helpText = "-s Integer Seed. If not specified a random one is used\n";
    u8* start = NULL;
    u8* end = NULL;
    ZPGPresetCfg cfg = {};
    param = &g_paramTypes[g_numParamTypes++];
    param->type = ZPG_PARAM_TYPE_INTEGER;
    param->asciChar = 's';
    start = (u8*)&cfg;
    end = (u8*)&cfg.seed;
    param->data.integerOffsetBytes = (i32)(end - start);
    param->helpText = "-s Integer Seed. If not specified a random one is used\n";

    param = &g_paramTypes[g_numParamTypes++];
    param->type = ZPG_PARAM_TYPE_INTEGER;
    param->asciChar = 'w';
    start = (u8*)&cfg;
    end = (u8*)&cfg.width;
    param->data.integerOffsetBytes = (i32)(end - start);
    param->helpText = "-w Result width in pixels/characters. If not specified a default is used\n";

    param = &g_paramTypes[g_numParamTypes++];
    param->type = ZPG_PARAM_TYPE_INTEGER;
    param->asciChar = 'h';
    start = (u8*)&cfg;
    end = (u8*)&cfg.height;
    param->data.integerOffsetBytes = (i32)(end - start);
    param->helpText = "-h Result height in pixels/characters. If not specified a default is used\n";
    
	param = &g_paramTypes[g_numParamTypes++];
    param->type = ZPG_PARAM_TYPE_INTEGER;
    param->asciChar = 'r';
    start = (u8*)&cfg;
    end = (u8*)&cfg.roomCount;
    param->data.integerOffsetBytes = (i32)(end - start);
    param->helpText = "-r Intended room count\n";

    ////////////////////////////////////////////////////
	// Function params
    param = &g_paramTypes[g_numParamTypes++];
    param->type = ZPG_PARAM_TYPE_FUNCTION;
    param->asciChar = 'a';
    param->data.func = ZPG_ParamAsciOutputFile;
    param->helpText = "-a <filename> Save final result as asci to filename\n";

    param = &g_paramTypes[g_numParamTypes++];
    param->type = ZPG_PARAM_TYPE_FUNCTION;
    param->asciChar = 'i';
    param->data.func = ZPG_ParamImageOutputFile;
    param->helpText = "-i <filename> Save final result as raw png to filename\n";
    
    param = &g_paramTypes[g_numParamTypes++];
    param->type = ZPG_PARAM_TYPE_FUNCTION;
    param->asciChar = 'c';
    param->data.func = ZPG_ParamPictureOutputFile;
    param->helpText = "-c <filename> Save final result using tile colours to filename\n";
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

/*
Returns 0 if okay
error code if not
*/
static zpgError ZPG_Params_ReadForPreset(ZPGPresetCfg* cfg, i32 argc, char** argv)
{
    // skip first two args.
    // Read third arg as preset number
    
    // cfg->preset = atoi(argv[2]);
    cfg->preset = argv[2];
    
    i32 i = 3;
    while (i < argc)
    {
        i32 index = i;
        i++;
        // extract option chararacter
        char* arg = argv[index];
        if (arg[0] != '-') 
        {
            printf("ABORT Setting labels must start with '-': %s\n", arg);
            return ZPG_ERROR_UNRECOGNISED_OPTION;
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
                    cfg->flags |= param->data.flag;
                    break;
                    case ZPG_PARAM_TYPE_INTEGER:
                    i += ZPG_ParamReadInt(
                        (argc - index),
                        &argv[index],
                        cfg,
                        param->data.integerOffsetBytes);
                    break;
                    case ZPG_PARAM_TYPE_FUNCTION:
                    i += param->data.func(
                        (argc - index),
                        &argv[index],
                        cfg);
                    break;
                    default:
                    printf("Unknown option %d\n", c);
                    break;
                }
            }
        }
        if (bRecognised == NO)
        {
            printf("Unrecognised setting %s\n", arg);
			return ZPG_ERROR_UNRECOGNISED_OPTION;
        }
    }
    printf("Run preset \"%s\"\n", cfg->preset);
    printf("Read flags %d\n", cfg->flags);
    if (cfg->asciOutput != NULL)
    {
        printf("Save ASCI to %s\n", cfg->asciOutput);
    }
    if (cfg->imageOutput != NULL)
    {
        printf("Save PNG output to %s\n", cfg->imageOutput);
    }
	return 0;
}

#endif // ZPG_PARAMS_H