@cd..
@cd bin
@echo *** Run ZPG ***
@rem Call in a separate window:
@rem start zpg.exe
@rem Run in this window
@rem zpg.exe preset 0 test_grid.txt
@rem zpg.exe preset 0 -p -a output.txt
@rem zpg.exe preset 12 -p -v -a output.txt
@rem zpg.exe preset 13 -s 42
@rem zpg.exe preset 13 -e -v -w 8 -h 8
zpg.exe preset 13 -e -s 42 -v -w 8 -h 8
@rem zpg.exe script test_script.txt test_grid_2.txt
@cd..
@cd buildwin