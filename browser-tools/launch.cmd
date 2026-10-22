@echo off
rem launch.cmd - double-click launcher for the AC1 browser-tools launchpad (the "AC1 Browser Tools"
rem desktop shortcut points here). Opens http://127.0.0.1:8760/ ; launched again while it runs, it
rem only opens the page. Closing this window stops the launchpad and the tools it started.
title AC1 browser tools
cd /d "%~dp0.."
python browser-tools\launchpad\server.py --open %*
if errorlevel 1 pause
