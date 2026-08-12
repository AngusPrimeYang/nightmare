"""Fix G1 station editor-vs-PIE height mismatch.

Open World landscape is ~Z=100 when fully loaded in the Editor, but PIE streaming
often exposes a lower / missing surface under the lot — so the same GS_* Z looks
buried in viewport and floating in PIE.

Authoritative ground = GS_LotPad (BlockAll cube). Station parts use STATION_Z = pad top.
"""
import json
import socket
import time

HOST, PORT = "127.0.0.1", 55557
CUBE_MESH = "/Engine/BasicShapes/Cube.Cube"
PAD_BP = "BP_GrayPad"
CUBE_BP = "BP_GrayCube"

# Pad top = design ground (matches fully-loaded L_Dev landscape ~Z100).
STATION_Z = 100.0
PAD_SCALE = [36.0, 32.0, 0.5]  # 3600 x 3200 x 50uu half-height → thickness 50
PAD_HALF_Z = 50.0 * PAD_SCALE[2]  # 25
PAD_CENTER = [0.0, -100.0, STATION_Z - PAD_HALF_Z]
PAD_NAME = "GS_LotPad"

PARTS = {
    "GS_StoreFloor": ([0.0, 400.0, 5.0], [12.0, 10.0, 0.1]),
    "GS_StoreWall_N": ([0.0, 900.0, 150.0], [12.0, 0.2, 3.0]),
    "GS_StoreWall_W": ([-600.0, 400.0, 150.0], [0.2, 10.0, 3.0]),
    "GS_StoreWall_E": ([600.0, 400.0, 150.0], [0.2, 10.0, 3.0]),
    "GS_StoreWall_S_L": ([-400.0, -100.0, 150.0], [4.0, 0.2, 3.0]),
    "GS_StoreWall_S_R": ([400.0, -100.0, 150.0], [4.0, 0.2, 3.0]),
    "GS_StoreDoorLintel": ([0.0, -100.0, 280.0], [2.0, 0.25, 0.4]),
    "GS_StoreDoorJamb_L": ([-100.0, -100.0, 140.0], [0.25, 0.25, 2.8]),
    "GS_StoreDoorJamb_R": ([100.0, -100.0, 140.0], [0.25, 0.25, 2.8]),
    "GS_StoreRoof": ([0.0, 400.0, 320.0], [12.5, 10.5, 0.15]),
    "GS_StoreEave_S": ([0.0, -120.0, 305.0], [12.8, 0.4, 0.2]),
    "GS_StoreEave_N": ([0.0, 920.0, 305.0], [12.8, 0.4, 0.2]),
    "GS_StoreWindow_L": ([-250.0, -115.0, 180.0], [1.6, 0.08, 1.2]),
    "GS_StoreWindow_R": ([250.0, -115.0, 180.0], [1.6, 0.08, 1.2]),
    "GS_CanopyDeck": ([0.0, -200.0, 450.0], [20.0, 16.0, 0.2]),
    "GS_CanopyEdge_N": ([0.0, 580.0, 465.0], [20.2, 0.35, 0.45]),
    "GS_CanopyEdge_S": ([0.0, -980.0, 465.0], [20.2, 0.35, 0.45]),
    "GS_CanopyEdge_E": ([1000.0, -200.0, 465.0], [0.35, 16.2, 0.45]),
    "GS_CanopyEdge_W": ([-1000.0, -200.0, 465.0], [0.35, 16.2, 0.45]),
    "GS_CanopyPost_FL": ([-900.0, -900.0, 225.0], [0.4, 0.4, 4.5]),
    "GS_CanopyPost_FR": ([900.0, -900.0, 225.0], [0.4, 0.4, 4.5]),
    "GS_CanopyPost_BL": ([-900.0, 500.0, 225.0], [0.4, 0.4, 4.5]),
    "GS_CanopyPost_BR": ([900.0, 500.0, 225.0], [0.4, 0.4, 4.5]),
    "GS_PumpIsland_1": ([-300.0, -200.0, 15.0], [3.0, 1.5, 0.3]),
    "GS_PumpIsland_2": ([200.0, -200.0, 15.0], [3.0, 1.5, 0.3]),
    "GS_PumpBody_1": ([-300.0, -200.0, 120.0], [0.8, 0.6, 1.8]),
    "GS_PumpBody_2": ([200.0, -200.0, 120.0], [0.8, 0.6, 1.8]),
    "GS_PumpHead_1": ([-300.0, -200.0, 220.0], [1.0, 0.7, 0.35]),
    "GS_PumpHead_2": ([200.0, -200.0, 220.0], [1.0, 0.7, 0.35]),
    "GS_SignPole": ([-900.0, -200.0, 300.0], [0.5, 0.5, 6.0]),
    "GS_SignBoard": ([-900.0, -200.0, 650.0], [4.0, 0.3, 2.0]),
    "GS_PriceBoard": ([-900.0, -200.0, 520.0], [2.0, 0.2, 1.0]),
    "GS_Curb_A": ([-400.0, -450.0, 8.0], [10.0, 0.35, 0.16]),
    "GS_Curb_B": ([400.0, -450.0, 8.0], [10.0, 0.35, 0.16]),
    "GS_Trash_1": ([-160.0, -40.0, 40.0], [0.5, 0.5, 0.8]),
    "GS_IceBox": ([180.0, -30.0, 55.0], [1.2, 0.8, 1.1]),
}

LIGHT_Z = STATION_Z + 290.0
LIGHT_XY = [
    (-450.0, 50.0), (0.0, 50.0), (450.0, 50.0),
    (-450.0, 400.0), (0.0, 400.0), (450.0, 400.0),
    (-450.0, 750.0), (0.0, 750.0), (450.0, 750.0),
]


def send(cmd, params=None, timeout=60):
    last = None
    for attempt in range(5):
        s = socket.socket()
        s.settimeout(timeout)
        try:
            s.connect((HOST, PORT))
            s.sendall(json.dumps({"type": cmd, "params": params or {}}).encode())
            data = b""
            while True:
                chunk = s.recv(65536)
                if not chunk:
                    raise ConnectionError("empty")
                data += chunk
                try:
                    return json.loads(data.decode())
                except json.JSONDecodeError:
                    continue
        except OSError as e:
            last = e
            time.sleep(1.0)
        finally:
            try:
                s.close()
            except OSError:
                pass
    return {"status": "error", "error": str(last)}


def ensure_pad_bp():
    send("create_blueprint", {"name": PAD_BP, "parent_class": "Actor"})
    send(
        "add_component_to_blueprint",
        {
            "blueprint_name": PAD_BP,
            "component_type": "StaticMeshComponent",
            "component_name": "CubeMesh",
            "location": [0, 0, 0],
            "rotation": [0, 0, 0],
            "scale": [1, 1, 1],
        },
    )
    send(
        "set_static_mesh_properties",
        {
            "blueprint_name": PAD_BP,
            "component_name": "CubeMesh",
            "static_mesh": CUBE_MESH,
        },
    )
    send(
        "set_component_property",
        {
            "blueprint_name": PAD_BP,
            "component_name": "CubeMesh",
            "property_name": "CollisionEnabled",
            "property_value": "QueryAndPhysics",
        },
    )
    send(
        "set_component_property",
        {
            "blueprint_name": PAD_BP,
            "component_name": "CubeMesh",
            "property_name": "CollisionProfileName",
            "property_value": "BlockAll",
        },
    )
    send(
        "set_physics_properties",
        {
            "blueprint_name": PAD_BP,
            "component_name": "CubeMesh",
            "simulate_physics": False,
            "gravity_enabled": False,
        },
    )
    r = send("compile_blueprint", {"blueprint_name": PAD_BP})
    print("pad_bp_compile", r.get("status"), r.get("error") or "")
    time.sleep(0.5)


def main():
    ensure_pad_bp()

    send("delete_actor", {"name": PAD_NAME})
    time.sleep(0.2)
    r = send(
        "spawn_blueprint_actor",
        {
            "blueprint_name": PAD_BP,
            "actor_name": PAD_NAME,
            "location": PAD_CENTER,
            "rotation": [0.0, 0.0, 0.0],
        },
    )
    print("spawn_pad", r.get("status"), r.get("error") or "")
    send(
        "set_actor_transform",
        {
            "name": PAD_NAME,
            "location": PAD_CENTER,
            "rotation": [0.0, 0.0, 0.0],
            "scale": PAD_SCALE,
        },
    )
    print(f"pad top Z={STATION_Z} center={PAD_CENTER} scale={PAD_SCALE}")

    ok = 0
    for name, (rel, scale) in PARTS.items():
        loc = [rel[0], rel[1], rel[2] + STATION_Z]
        r = send(
            "set_actor_transform",
            {
                "name": name,
                "location": loc,
                "rotation": [0.0, 0.0, 0.0],
                "scale": scale,
            },
        )
        status = r.get("status")
        print(f"[{status}] {name} -> {loc}")
        if status == "success":
            ok += 1
        time.sleep(0.02)

    for idx, (x, y) in enumerate(LIGHT_XY):
        name = f"StoreCeilLight_{idx}"
        loc = [x, y, LIGHT_Z]
        send(
            "set_actor_transform",
            {
                "name": name,
                "location": loc,
                "rotation": [0.0, 0.0, 0.0],
                "scale": [1.0, 1.0, 1.0],
            },
        )
        print(f"light {name} -> {loc}")
        time.sleep(0.02)

    spawn_z = STATION_Z + 120.0
    level = send("get_actors_in_level")
    for a in (level.get("result") or {}).get("actors") or []:
        if isinstance(a, dict) and "PlayerStart" in a.get("name", ""):
            send(
                "set_actor_transform",
                {
                    "name": a["name"],
                    "location": [0.0, -600.0, spawn_z],
                    "rotation": [0.0, 90.0, 0.0],
                    "scale": [1.0, 1.0, 1.0],
                },
            )
            print(f"PlayerStart -> (0,-600,{spawn_z})")
            break

    print(
        f"STATION_ON_LOT_PAD STATION_Z={STATION_Z} parts_ok={ok}/{len(PARTS)} "
        f"pad={PAD_NAME} — Ctrl+S then PIE: walk on gray pad"
    )
    return 0 if ok == len(PARTS) else 1


if __name__ == "__main__":
    raise SystemExit(main())
