@echo off
rem viewer.cmd - double-click launcher for tools/loop/viewer.py (a desktop shortcut can point here).
rem Watches the newest overnight.sh work dir and opens http://localhost:8765/ in the default
rem browser; a second launch just opens the page of the one already running.  Arguments pass
rem through, e.g.  viewer.cmd --replay C:\path\to\work_dir
title AC1 match viewer
cd /d "%~dp0..\.."
python tools\loop\viewer.py --open %*
if errorlevel 1 pause
