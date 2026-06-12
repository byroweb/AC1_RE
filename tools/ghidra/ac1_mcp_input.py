#!/usr/bin/env python3
"""
ac1_mcp_input.py — fire timed controller inputs at DuckStation via its MCP server.

Why this exists: the AC1 title screen ("Push Start Button") flips to the attract /
demo reel after a few idle seconds. Driving it with one-press-then-screenshot
round-trips is too slow and gets snagged by the demo. This script does the whole
MCP handshake and sends a single frame-timed input_sequence, so the presses land
deterministically with no per-press latency.

Default action = 2 Start presses spaced ~1 s apart (6f press, 60f gap, 6f press),
which steps title -> main menu -> EDAMEH/BAZI JADID (ادامه/بازی جدید) submenu.

Usage:
  ./ac1_mcp_input.py                 # 2 Start presses, ~1 s apart (default)
  ./ac1_mcp_input.py Start Start Cross   # those buttons, each 6f, ~1 s apart
  ./ac1_mcp_input.py --gap 90 Start Start
"""
import sys, json, requests

URL = "http://localhost:2346/mcp"
HEADERS = {"Content-Type": "application/json",
           "Accept": "application/json, text/event-stream"}


def _parse(resp):
    """Return the JSON-RPC result dict from a JSON or SSE response body."""
    ctype = resp.headers.get("Content-Type", "")
    if "text/event-stream" in ctype:
        for line in resp.text.splitlines():
            if line.startswith("data:"):
                return json.loads(line[5:].strip())
        return None
    return resp.json()


def main():
    args = sys.argv[1:]
    gap = 60
    if args and args[0] == "--gap":
        gap = int(args[1]); args = args[2:]
    buttons = args if args else ["Start", "Start"]

    s = requests.Session()
    # 1) initialize -> capture session id
    init = {"jsonrpc": "2.0", "id": 1, "method": "initialize",
            "params": {"protocolVersion": "2024-11-05",
                       "capabilities": {},
                       "clientInfo": {"name": "ac1-input", "version": "1.0"}}}
    r = s.post(URL, headers=HEADERS, json=init)
    r.raise_for_status()
    sid = r.headers.get("Mcp-Session-Id")
    if not sid:
        print("ERROR: no Mcp-Session-Id returned", file=sys.stderr); sys.exit(1)
    h = dict(HEADERS); h["Mcp-Session-Id"] = sid

    # 2) initialized notification
    s.post(URL, headers=h, json={"jsonrpc": "2.0", "method": "notifications/initialized"})

    # 3) build a frame-timed sequence: <button>(6f), gap, <button>(6f), gap...
    seq = []
    for i, b in enumerate(buttons):
        if i:
            seq.append({"buttons": [], "duration_frames": gap})
        seq.append({"buttons": [b], "duration_frames": 6})
    call = {"jsonrpc": "2.0", "id": 2, "method": "tools/call",
            "params": {"name": "input_sequence", "arguments": {"sequence": seq}}}
    r = s.post(URL, headers=h, json=call)
    r.raise_for_status()
    print(json.dumps(_parse(r), indent=2))


if __name__ == "__main__":
    main()
