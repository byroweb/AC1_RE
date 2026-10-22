@echo off
rem tool.cmd - double-click launcher (rename it after the tool and set "launcher" in tool.json).
rem Starts the server and opens its page; a second launch only opens the page.
title AC1 tool
cd /d "%~dp0..\.."
python "%~dp0server.py" --open %*
if errorlevel 1 pause
