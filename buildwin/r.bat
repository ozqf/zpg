@echo off
cd..
cd bin
@echo *** Run ZPG ***
@rem Call in a separate window:
@rem start zpg.exe
@rem Run in this window
zpg.exe preset 13 test_grid.txt
@rem zpg.exe script test_script.txt test_grid_2.txt
cd..
cd buildwin
@echo on