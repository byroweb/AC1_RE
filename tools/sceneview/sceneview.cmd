@echo off
rem sceneview.cmd - double-click launcher for tools/sceneview/server.py (a desktop shortcut can point here).
rem Starts the scene viewer server and opens http://127.0.0.1:8771/ in the default browser; a second
rem launch just opens the page of the one already running. Needs AC1_BIN (your disc image) set, or pass
rem --bin path\to\disc.bin. Arguments pass through, e.g.  sceneview.cmd --port 8772
title AC1 scene viewer
cd /d "%~dp0..\.."
python tools\sceneview\server.py --open %*
if errorlevel 1 pause
