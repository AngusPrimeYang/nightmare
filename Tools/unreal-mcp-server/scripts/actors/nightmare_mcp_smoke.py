#!/usr/bin/env python
"""Nightmare UnrealMCP TCP smoke helpers (direct to Editor port 55557).

Preserved commands for next session — does not require Cursor MCP tools to be loaded.

Usage (from repo root, Editor open with UnrealMCP listening):

  Tools\\uv\\bin\\uv.exe --directory Tools\\unreal-mcp-server run --python 3.12 ^
    python scripts\\actors\\nightmare_mcp_smoke.py list

  Tools\\uv\\bin\\uv.exe --directory Tools\\unreal-mcp-server run --python 3.12 ^
    python scripts\\actors\\nightmare_mcp_smoke.py spawn
"""

from __future__ import annotations

import json
import socket
import sys
from typing import Any, Dict, Optional


HOST = "127.0.0.1"
PORT = 55557


def send_command(command: str, params: Optional[Dict[str, Any]] = None) -> Dict[str, Any]:
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(30)
    sock.connect((HOST, PORT))
    try:
        payload = json.dumps({"type": command, "params": params or {}}).encode("utf-8")
        sock.sendall(payload)
        chunks = []
        while True:
            chunk = sock.recv(65536)
            if not chunk:
                break
            chunks.append(chunk)
            try:
                return json.loads(b"".join(chunks).decode("utf-8"))
            except json.JSONDecodeError:
                continue
        return {"status": "error", "error": "empty response"}
    finally:
        sock.close()


def cmd_list() -> int:
    resp = send_command("get_actors_in_level", {})
    print(json.dumps(resp, indent=2)[:8000])
    ok = resp.get("status") == "success"
    print(f"LIST_OK={ok}")
    return 0 if ok else 1


def cmd_spawn() -> int:
    # Preserved spawn command (smoke cube near origin / Player Start area)
    create = send_command(
        "create_actor",
        {
            "name": "NightmareMcpSmokeCube",
            "type": "StaticMeshActor",
            "location": [200.0, 0.0, 100.0],
            "rotation": [0.0, 0.0, 0.0],
            "scale": [1.0, 1.0, 1.0],
        },
    )
    print(json.dumps(create, indent=2)[:4000])
    props = send_command("get_actor_properties", {"name": "NightmareMcpSmokeCube"})
    print("--- props ---")
    print(json.dumps(props, indent=2)[:4000])
    ok = create.get("status") == "success"
    print(f"SPAWN_OK={ok}")
    return 0 if ok else 1


def main() -> int:
    if len(sys.argv) < 2 or sys.argv[1] not in ("list", "spawn"):
        print(__doc__)
        print("Usage: nightmare_mcp_smoke.py [list|spawn]")
        return 2
    if sys.argv[1] == "list":
        return cmd_list()
    return cmd_spawn()


if __name__ == "__main__":
    raise SystemExit(main())
