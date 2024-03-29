@cd..
@cd bin
@echo *** Run ZPG ***

@rem Just print preset options
zpg preset

@rem Call in a separate window:
@rem start zpg.exe
@rem Run in this window
@rem zpg.exe preset 0 test_grid.txt
@rem zpg.exe preset 0 -p -a output.txt

@rem -- Caves --
zpg preset walk_from_centre -e -p -v -w 72 -h 32 -b
@rem zpg preset 2 -e -p -v -w 72 -h 32 -s 0

@rem segmented line + wanders
@rem zpg.exe preset 3 -s 1610068662 -p -a output.txt

@rem prefab to prefab segmented walk with png output
@rem zpg preset 11 -i preset11_raw.png -c preset11_colours.png -w 128 -h 128 -v

@rem -- Perlin noise --
@rem zpg.exe preset 6 -g

@rem -- Room connection test --
@rem zpg.exe preset 14 -p -v -e -w 16 -h 10 -s 0

@rem -- Room seed test --
@rem zpg preset 16 -e -v -w 20 -h 10 -s 0

@rem prefab walk test
@rem zpg preset 11 -p -v -s 1610130853

@rem zpg.exe preset 12 -p -v -a output.txt
@rem zpg.exe preset 13 -s 42
@rem zpg.exe preset 13 -e -v -w 8 -h 8
@rem zpg.exe preset 13 -e -s 42 -v -w 8 -h 8
@rem zpg.exe script test_script.txt test_grid_2.txt
@cd..
@cd buildwin