#!/usr/bin/env python
"""Spawn G3 graybox NightmarePickupActor instances near PlayerStart after C++ rebuild."""

from __future__ import annotations

import json
import socket
import sys
from typing import Any, Dict, Optional


def send(cmd: str, params: Optional[Dict[str, Any]] = None, timeout: float = 60.0) -> Dict[str, Any]:
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(timeout)
    sock.connect(("127.0.0.1", 55557))
    try:
        sock.sendall(json.dumps({"type": cmd, "params": params or {}}).encode("utf-8"))
        chunks: list[bytes] = []
        while True:
            chunk = sock.recv(65536)
            if not chunk:
                break
            chunks.append(chunk)
            try:
                return json.loads(b"".join(chunks).decode("utf-8"))
            except json.JSONDecodeError:
                continue
        return {"status": "error", "error": "empty"}
    finally:
        sock.close()


def main() -> int:
    spawns = [
        ("GrayPickup_A", [-50.0, 0.0, 120.0]),
        ("GrayPickup_B", [50.0, 80.0, 120.0]),
        ("GrayPickup_C", [0.0, -100.0, 120.0]),
    ]
    for name, loc in spawns:
        resp = send(
            "spawn_actor",
            {
                "name": name,
                "type": "NightmarePickupActor",
                "location": loc,
                "rotation": [0.0, 0.0, 0.0],
                "scale": [1.0, 1.0, 1.0],
            },
        )
        print(name, json.dumps(resp)[:500])
        if resp.get("status") != "success":
            return 1
    print("SPAWN_PICKUPS_OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
