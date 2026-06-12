#!/usr/bin/env python3
"""
llm_local.py — call the local LLM running on the dedicated Arc A750 PC.

The Arc PC runs an OpenAI-compatible server (LM Studio or Ollama). This thin
client lets a Claude-run agent offload bulk/cheap passes (summarize a huge
disassembly dump, classify many entries, draft translations) to it via Bash.

Config via env (override per call as needed):
  LLM_LOCAL_URL    base URL incl. /v1   (default http://192.168.1.50:1234/v1)
                   LM Studio default port 1234; Ollama 11434.
  LLM_LOCAL_MODEL  model id             (default qwen2.5-coder-7b-instruct)

Usage:
  echo "PROMPT" | tools/re/llm_local.py
  tools/re/llm_local.py -p "PROMPT" [--system "..."] [--model X] [--max 2048] [--temp 0.2]
  tools/re/llm_local.py --ping            # health check / list models
"""
import os, sys, json, argparse, urllib.request, urllib.error

URL   = os.environ.get("LLM_LOCAL_URL", "http://192.168.1.50:1234/v1")
MODEL = os.environ.get("LLM_LOCAL_MODEL", "qwen2.5-coder-7b-instruct")


def _post(path, payload, timeout):
    req = urllib.request.Request(URL.rstrip("/") + path,
                                 data=json.dumps(payload).encode(),
                                 headers={"Content-Type": "application/json"})
    with urllib.request.urlopen(req, timeout=timeout) as r:
        return json.loads(r.read())


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("-p", "--prompt")
    ap.add_argument("--system", default="You are a precise assistant for "
                    "reverse-engineering and data tasks. Be terse and factual.")
    ap.add_argument("--model", default=MODEL)
    ap.add_argument("--max", type=int, default=2048)
    ap.add_argument("--temp", type=float, default=0.2)
    ap.add_argument("--timeout", type=int, default=600)
    ap.add_argument("--ping", action="store_true")
    args = ap.parse_args()

    if args.ping:
        try:
            req = urllib.request.Request(URL.rstrip("/") + "/models")
            with urllib.request.urlopen(req, timeout=15) as r:
                data = json.loads(r.read())
            ids = [m.get("id") for m in data.get("data", [])]
            print(f"OK {URL} — models: {ids}")
            return 0
        except Exception as e:
            print(f"UNREACHABLE {URL}: {e}", file=sys.stderr)
            return 2

    prompt = args.prompt if args.prompt is not None else sys.stdin.read()
    if not prompt.strip():
        print("empty prompt", file=sys.stderr); return 1

    payload = {
        "model": args.model,
        "messages": [{"role": "system", "content": args.system},
                     {"role": "user", "content": prompt}],
        "temperature": args.temp,
        "max_tokens": args.max,
        "stream": False,
    }
    try:
        resp = _post("/chat/completions", payload, args.timeout)
    except urllib.error.URLError as e:
        print(f"request failed ({URL}): {e}\n"
              f"  set LLM_LOCAL_URL to the Arc PC, e.g. "
              f"export LLM_LOCAL_URL=http://<arc-pc-ip>:1234/v1", file=sys.stderr)
        return 2
    print(resp["choices"][0]["message"]["content"])
    return 0


if __name__ == "__main__":
    sys.exit(main())
