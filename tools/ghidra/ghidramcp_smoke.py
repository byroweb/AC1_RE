#!/usr/bin/env python3
"""
ghidramcp_smoke.py — smoke-test the pristine AC_1_USA_2048_RE project through
GhidraMCP in headless mode (no Ghidra GUI). See docs/PRISTINE_PROJECT.md.

Launches `ghidra-mcp --headless` over stdio, then checks that
  * the program is the PSX-loader import (PSX:LE:32:default),
  * search_functions_by_name finds draw_string (lives in the overlay block),
  * decompile_function returns C for it.

Run it with the ghidra-mcp venv's python (it has the `mcp` package):
  <venv>/Scripts/python.exe tools/ghidra/ghidramcp_smoke.py            # Windows
  <venv>/bin/python tools/ghidra/ghidramcp_smoke.py                    # Linux

Env (all optional):
  GHIDRAMCP_CMD           ghidra-mcp executable (default: the one next to the
                          running python, i.e. the venv, else "ghidra-mcp" on PATH)
  AC1_GHIDRA_PROJECT_DIR  project dir (default: ~/ac1_ghidra)
  AC1_GHIDRA_PROJECT      project name (default: AC_1_USA_2048_RE)
  AC1_EXE                 the carved exe (default: <project dir>/SLUS_013_2048.23;
                          must match the program name saved in the project)
  GHIDRA_INSTALL_DIR      needed by PyGhidra; forwarded explicitly because
                          mcp's default child environment drops it.
Exit status 0 = all checks passed.
"""
import asyncio
import json
import os
import sys

from mcp import ClientSession, StdioServerParameters
from mcp.client.stdio import stdio_client

PROJ = os.environ.get("AC1_GHIDRA_PROJECT_DIR", os.path.expanduser("~/ac1_ghidra"))
NAME = os.environ.get("AC1_GHIDRA_PROJECT", "AC_1_USA_2048_RE")
EXE = os.environ.get("AC1_EXE", os.path.join(PROJ, "SLUS_013_2048.23"))
FUNC = sys.argv[1] if len(sys.argv) > 1 else "draw_string"


def ghidra_mcp_cmd():
    """GHIDRAMCP_CMD, else the ghidra-mcp next to this interpreter (venv), else PATH."""
    if os.environ.get("GHIDRAMCP_CMD"):
        return os.environ["GHIDRAMCP_CMD"]
    exe = "ghidra-mcp.exe" if os.name == "nt" else "ghidra-mcp"
    local = os.path.join(os.path.dirname(sys.executable), exe)
    return local if os.path.isfile(local) else "ghidra-mcp"


CMD = ghidra_mcp_cmd()


def text(res):
    return "\n".join(c.text for c in res.content if getattr(c, "text", None))


def check(cond, msg):
    print(("PASS " if cond else "FAIL ") + msg)
    return bool(cond)


async def main():
    params = StdioServerParameters(
        command=CMD,
        args=["--headless", "--project", PROJ, "--project-name", NAME,
              "--binary", EXE, "--discard", "--ready-timeout", "600"],
        env=dict(os.environ),          # keep GHIDRA_INSTALL_DIR / JAVA_HOME for PyGhidra
    )
    ok = True
    async with stdio_client(params, errlog=sys.stderr) as (r, w):
        async with ClientSession(r, w) as s:
            await s.initialize()
            tools = {t.name for t in (await s.list_tools()).tools}
            ok &= check({"get_program_info", "search_functions_by_name",
                         "decompile_function"} <= tools, f"{len(tools)} tools listed")

            info = json.loads(text(await s.call_tool("get_program_info", {})))
            ok &= check(info.get("language_id") == "PSX:LE:32:default",
                        f"program {info.get('name')} language {info.get('language_id')} "
                        f"format {info.get('executable_format')}")

            found = json.loads(text(await s.call_tool(
                "search_functions_by_name", {"query": FUNC})))
            hits = [f for f in found.get("items", []) if f["name"] == FUNC]
            ok &= check(hits, f"search_functions_by_name {FUNC}: "
                              + (", ".join(f"{h['name']} @ {h['address']}" for h in hits)
                                 or "not found"))

            dec = text(await s.call_tool("decompile_function", {"name": FUNC}))
            try:
                code = json.loads(dec).get("code", "")
            except ValueError:
                code = dec
            ok &= check(FUNC in code and "{" in code,
                        f"decompile_function {FUNC}: {len(code)} chars of C")
            print(code.strip().splitlines()[0:6] if code else dec[:300])
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(asyncio.run(main()))
