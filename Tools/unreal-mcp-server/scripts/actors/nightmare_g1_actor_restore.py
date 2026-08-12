"""G1 restore via many BP_GrayCube instances + world PointLights (no mega-BP compile)."""
import json
import socket
import sys
import time

HOST, PORT = "127.0.0.1", 55557
CUBE_BP = "BP_GrayCube"
CUBE_MESH = "/Engine/BasicShapes/Cube.Cube"
N = 3
STATION_Z = 90.0
LIGHT_Z_REL = 290.0
GROUND_Z = 350.0

# name, relative loc, scale  — world = loc + (0,0,STATION_Z)
PARTS = [
    ("GS_StoreFloor", [0.0, 400.0, 5.0], [12.0, 10.0, 0.1]),
    ("GS_StoreWall_N", [0.0, 900.0, 150.0], [12.0, 0.2, 3.0]),
    ("GS_StoreWall_W", [-600.0, 400.0, 150.0], [0.2, 10.0, 3.0]),
    ("GS_StoreWall_E", [600.0, 400.0, 150.0], [0.2, 10.0, 3.0]),
    ("GS_StoreWall_S_L", [-400.0, -100.0, 150.0], [4.0, 0.2, 3.0]),
    ("GS_StoreWall_S_R", [400.0, -100.0, 150.0], [4.0, 0.2, 3.0]),
    ("GS_StoreDoorLintel", [0.0, -100.0, 280.0], [2.0, 0.25, 0.4]),
    ("GS_StoreDoorJamb_L", [-100.0, -100.0, 140.0], [0.25, 0.25, 2.8]),
    ("GS_StoreDoorJamb_R", [100.0, -100.0, 140.0], [0.25, 0.25, 2.8]),
    ("GS_StoreRoof", [0.0, 400.0, 320.0], [12.5, 10.5, 0.15]),
    ("GS_StoreEave_S", [0.0, -120.0, 305.0], [12.8, 0.4, 0.2]),
    ("GS_StoreEave_N", [0.0, 920.0, 305.0], [12.8, 0.4, 0.2]),
    ("GS_StoreWindow_L", [-250.0, -115.0, 180.0], [1.6, 0.08, 1.2]),
    ("GS_StoreWindow_R", [250.0, -115.0, 180.0], [1.6, 0.08, 1.2]),
    ("GS_StoreWindowSill_L", [-250.0, -115.0, 115.0], [1.8, 0.15, 0.15]),
    ("GS_StoreWindowSill_R", [250.0, -115.0, 115.0], [1.8, 0.15, 0.15]),
    ("GS_CanopyDeck", [0.0, -200.0, 450.0], [20.0, 16.0, 0.2]),
    ("GS_CanopyEdge_N", [0.0, 580.0, 465.0], [20.2, 0.35, 0.45]),
    ("GS_CanopyEdge_S", [0.0, -980.0, 465.0], [20.2, 0.35, 0.45]),
    ("GS_CanopyEdge_E", [1000.0, -200.0, 465.0], [0.35, 16.2, 0.45]),
    ("GS_CanopyEdge_W", [-1000.0, -200.0, 465.0], [0.35, 16.2, 0.45]),
    ("GS_CanopyPost_FL", [-900.0, -900.0, 225.0], [0.4, 0.4, 4.5]),
    ("GS_CanopyPost_FR", [900.0, -900.0, 225.0], [0.4, 0.4, 4.5]),
    ("GS_CanopyPost_BL", [-900.0, 500.0, 225.0], [0.4, 0.4, 4.5]),
    ("GS_CanopyPost_BR", [900.0, 500.0, 225.0], [0.4, 0.4, 4.5]),
    ("GS_PumpIsland_1", [-300.0, -200.0, 15.0], [3.0, 1.5, 0.3]),
    ("GS_PumpIsland_2", [200.0, -200.0, 15.0], [3.0, 1.5, 0.3]),
    ("GS_PumpBody_1", [-300.0, -200.0, 120.0], [0.8, 0.6, 1.8]),
    ("GS_PumpBody_2", [200.0, -200.0, 120.0], [0.8, 0.6, 1.8]),
    ("GS_PumpHead_1", [-300.0, -200.0, 220.0], [1.0, 0.7, 0.35]),
    ("GS_PumpHead_2", [200.0, -200.0, 220.0], [1.0, 0.7, 0.35]),
    ("GS_PumpHose_1", [-250.0, -200.0, 90.0], [0.12, 0.9, 0.12]),
    ("GS_PumpHose_2", [250.0, -200.0, 90.0], [0.12, 0.9, 0.12]),
    ("GS_SignPole", [-900.0, -200.0, 300.0], [0.5, 0.5, 6.0]),
    ("GS_SignBoard", [-900.0, -200.0, 650.0], [4.0, 0.3, 2.0]),
    ("GS_PriceBoard", [-900.0, -200.0, 520.0], [2.0, 0.2, 1.0]),
    ("GS_Curb_A", [-400.0, -450.0, 8.0], [10.0, 0.35, 0.16]),
    ("GS_Curb_B", [400.0, -450.0, 8.0], [10.0, 0.35, 0.16]),
    ("GS_Trash_1", [-160.0, -40.0, 40.0], [0.5, 0.5, 0.8]),
    ("GS_IceBox", [180.0, -30.0, 55.0], [1.2, 0.8, 1.1]),
]


def send(cmd, params=None, timeout=60):
    last_err = None
    for attempt in range(4):
        s = socket.socket()
        s.settimeout(timeout)
        try:
            s.connect((HOST, PORT))
            s.sendall(json.dumps({"type": cmd, "params": params or {}}).encode("utf-8"))
            data = b""
            while True:
                chunk = s.recv(65536)
                if not chunk:
                    raise ConnectionError("empty")
                data += chunk
                try:
                    return json.loads(data.decode("utf-8"))
                except json.JSONDecodeError:
                    continue
        except OSError as e:
            last_err = e
            time.sleep(1.0 + attempt * 0.5)
        finally:
            try:
                s.close()
            except OSError:
                pass
    return {"status": "error", "error": str(last_err)}


def ok(label, r):
    status = r.get("status")
    print(f"[{status}] {label}")
    if status != "success":
        print(" ", json.dumps(r, ensure_ascii=False)[:250])
        return False
    return True


def soft(label, r):
    if ok(label, r):
        return True
    if "already" in json.dumps(r).lower() or "exists" in json.dumps(r).lower():
        print(f"  (reuse) {label}")
        return True
    return False


def delete_prefix(prefixes):
    level = send("get_actors_in_level")
    actors = (level.get("result") or {}).get("actors") or []
    for a in actors:
        if not isinstance(a, dict):
            continue
        name = a.get("name", "")
        if any(name.startswith(p) or p in name for p in prefixes):
            send("delete_actor", {"name": name})
            print(f"deleted {name}")


def ensure_cube_bp():
    soft("create_cube_bp", send("create_blueprint", {"name": CUBE_BP, "parent_class": "Actor"}))
    soft(
        "add_mesh",
        send(
            "add_component_to_blueprint",
            {
                "blueprint_name": CUBE_BP,
                "component_type": "StaticMeshComponent",
                "component_name": "CubeMesh",
                "location": [0.0, 0.0, 0.0],
                "rotation": [0.0, 0.0, 0.0],
                "scale": [1.0, 1.0, 1.0],
            },
        ),
    )
    soft(
        "set_mesh",
        send(
            "set_static_mesh_properties",
            {
                "blueprint_name": CUBE_BP,
                "component_name": "CubeMesh",
                "static_mesh": CUBE_MESH,
            },
        ),
    )
    send(
        "set_component_property",
        {
            "blueprint_name": CUBE_BP,
            "component_name": "CubeMesh",
            "property_name": "BodyInstance.CollisionEnabled",
            "property_value": "NoCollision",
        },
    )
    send(
        "set_component_property",
        {
            "blueprint_name": CUBE_BP,
            "component_name": "CubeMesh",
            "property_name": "CollisionEnabled",
            "property_value": "NoCollision",
        },
    )
    return soft("compile_cube_bp", send("compile_blueprint", {"blueprint_name": CUBE_BP}))


def light_positions(n):
    x0, x1 = -450.0, 450.0
    y0, y1 = 50.0, 750.0
    z = STATION_Z + LIGHT_Z_REL
    out = []
    for j in range(n):
        for i in range(n):
            if n == 1:
                out.append((0.0, 400.0, z))
            else:
                out.append(
                    (
                        x0 + (x1 - x0) * i / (n - 1),
                        y0 + (y1 - y0) * j / (n - 1),
                        z,
                    )
                )
    return out


def main():
    # Probe MCP
    r = send("get_actors_in_level")
    if r.get("status") != "success":
        print("ABORT: MCP not ready", r)
        return 1

    delete_prefix(["GS_", "GasStation", "StoreCeilLight"])
    time.sleep(0.3)

    if not ensure_cube_bp():
        print("ABORT: cube BP failed")
        return 2

    fails = 0
    for name, loc, scale in PARTS:
        world = [loc[0], loc[1], loc[2] + STATION_Z]
        # delete if leftover
        send("delete_actor", {"name": name})
        r = send(
            "spawn_blueprint_actor",
            {
                "blueprint_name": CUBE_BP,
                "actor_name": name,
                "location": world,
                "rotation": [0.0, 0.0, 0.0],
            },
        )
        if not soft(f"spawn {name}", r):
            fails += 1
            continue
        soft(
            f"xform {name}",
            send(
                "set_actor_transform",
                {
                    "name": name,
                    "location": world,
                    "rotation": [0.0, 0.0, 0.0],
                    "scale": scale,
                },
            ),
        )
        time.sleep(0.03)

    for idx, (x, y, z) in enumerate(light_positions(N)):
        name = f"StoreCeilLight_{idx}"
        send("delete_actor", {"name": name})
        soft(
            f"light {name}",
            send(
                "spawn_actor",
                {
                    "name": name,
                    "type": "PointLight",
                    "location": [x, y, z],
                    "rotation": [0.0, 0.0, 0.0],
                },
            ),
        )
        send(
            "set_actor_property",
            {
                "name": name,
                "property_name": "PointLightComponent.Intensity",
                "property_value": "5000",
            },
        )
        send(
            "set_actor_property",
            {
                "name": name,
                "property_name": "PointLightComponent.AttenuationRadius",
                "property_value": "800",
            },
        )
        time.sleep(0.03)

    level = send("get_actors_in_level")
    for a in (level.get("result") or {}).get("actors") or []:
        if isinstance(a, dict) and "PlayerStart" in a.get("name", ""):
            soft(
                "playerstart",
                send(
                    "set_actor_transform",
                    {
                        "name": a["name"],
                        "location": [0.0, -600.0, GROUND_Z],
                        "rotation": [0.0, 90.0, 0.0],
                        "scale": [1.0, 1.0, 1.0],
                    },
                ),
            )
            break

    for name, loc in [
        ("GrayPickup_A", [-80.0, -480.0, GROUND_Z + 30.0]),
        ("GrayPickup_B", [80.0, -480.0, GROUND_Z + 30.0]),
        ("GrayPickup_C", [0.0, -420.0, GROUND_Z + 30.0]),
    ]:
        soft(
            name,
            send(
                "set_actor_transform",
                {
                    "name": name,
                    "location": loc,
                    "rotation": [0.0, 0.0, 0.0],
                    "scale": [1.0, 1.0, 1.0],
                },
            ),
        )

    print(json.dumps(send("find_actors_by_name", {"pattern": "GS_|StoreCeil"}), indent=2)[:2000])
    print(f"G1_ACTOR_RESTORE_DONE fails={fails} — Ctrl+S NOW")
    return 0 if fails == 0 else 3


if __name__ == "__main__":
    sys.exit(main())
