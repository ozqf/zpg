@cd..
@cd bin
@echo *** Run ZPG ***
@rem Call in a separate window:
@rem start zpg.exe
@rem Run in this window
@rem zpg.exe preset 0 test_grid.txt
@rem zpg.exe preset 12 -p -v -a output.txt
zpg.exe preset 14 -g
@rem zpg.exe script test_script.txt test_grid_2.txt
@cd..
@cd buildwin