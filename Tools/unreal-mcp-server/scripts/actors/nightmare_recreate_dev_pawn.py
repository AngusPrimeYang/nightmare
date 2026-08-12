#!/usr/bin/env python
"""Rename broken BP_DevPawn -> BP_DevPawn_Old, recreate as Character with Nightmare components."""

from __future__ import annotations

import json
import socket
import sys
from typing import Any, Dict, Optional


HOST = "127.0.0.1"
PORT = 55557


def send(cmd: str, params: Optional[Dict[str, Any]] = None, timeout: float = 90.0) -> Dict[str, Any]:
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(timeout)
    sock.connect((HOST, PORT))
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
        return {"status": "error", "error": "empty response"}
    finally:
        sock.close()


def show(label: str, resp: Dict[str, Any]) -> Dict[str, Any]:
    print(f"=== {label} ===")
    print(json.dumps(resp, indent=2)[:2500])
    return resp


def main() -> int:
    show("rename", send("rename_blueprint", {"old_name": "BP_DevPawn", "new_name": "BP_DevPawn_Old"}))

    created = show(
        "create",
        send("create_blueprint", {"name": "BP_DevPawn", "parent_class": "Character"}),
    )
    parent = ""
    if isinstance(created.get("result"), dict):
        parent = str(created["result"].get("parent_class", ""))
    if created.get("status") != "success" or parent != "Character":
        print(f"FAIL: expected parent_class Character, got {parent!r}")
        return 1

    for ctype, name in (
        ("NightmareStaminaComponent", "NightmareStamina"),
        ("NightmareInventoryComponent", "NightmareInventory"),
        ("NightmareMatchComponent", "NightmareMatch"),
    ):
        show(
            f"add_{name}",
            send(
                "add_component_to_blueprint",
                {
                    "blueprint_name": "BP_DevPawn",
                    "component_type": ctype,
                    "component_name": name,
                },
            ),
        )

    show("compile", send("compile_blueprint", {"blueprint_name": "BP_DevPawn"}))
    show(
        "auto_possess",
        send(
            "set_pawn_properties",
            {"blueprint_name": "BP_DevPawn", "auto_possess_player": "Player0"},
        ),
    )
    show(
        "spawn",
        send(
            "spawn_blueprint_actor",
            {
                "blueprint_name": "BP_DevPawn",
                "actor_name": "BP_DevPawn_Dev",
                "location": [200.0, 200.0, 200.0],
                "rotation": [0.0, 0.0, 0.0],
            },
        ),
    )
    print("RECREATE_OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
