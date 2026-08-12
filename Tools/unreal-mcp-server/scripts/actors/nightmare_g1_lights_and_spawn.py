"""G1 follow-up: indoor n*n ceiling lights + unstick spawn (raise station, move PlayerStart, no mesh collision)."""
import json
import socket
import sys

HOST, PORT = "127.0.0.1", 55557
BP = "BP_GasStationGraybox"
N = 3  # n*n ceiling lights
STATION_Z = 90.0  # lift so store floor sits near landscape (~z100)
LIGHT_Z = 290.0  # under StoreRoof (rel z 320)
GROUND_Z = 350.0

MESH_PARTS = [
    "StoreFloor", "StoreWall_N", "StoreWall_W", "StoreWall_E", "StoreWall_S_L", "StoreWall_S_R",
    "StoreDoorLintel", "StoreDoorJamb_L", "StoreDoorJamb_R", "StoreRoof", "StoreEave_S", "StoreEave_N",
    "StoreWindow_L", "StoreWindow_R", "StoreWindowSill_L", "StoreWindowSill_R", "StoreCornice_L", "StoreCornice_R",
    "CanopyDeck", "CanopyEdge_N", "CanopyEdge_S", "CanopyEdge_E", "CanopyEdge_W",
    "CanopyLight_1", "CanopyLight_2", "CanopyLight_3",
    "CanopyPost_FL", "CanopyPost_FR", "CanopyPost_BL", "CanopyPost_BR",
    "CanopyCap_FL", "CanopyCap_FR", "CanopyCap_BL", "CanopyCap_BR",
    "PumpIsland_1", "PumpIsland_2", "PumpIslandEdge_1", "PumpIslandEdge_2",
    "PumpBody_1", "PumpBody_2", "PumpHead_1", "PumpHead_2", "PumpHose_1", "PumpHose_2",
    "PumpNozzle_1", "PumpNozzle_2",
    "SignPole", "SignBoard", "SignBoardFrame", "PriceBoard", "SignBase",
    "Curb_A", "Curb_B", "Curb_C", "Trash_1", "Trash_2", "IceBox", "IceBoxLid", "Bench",
]


def send(cmd, params=None, timeout=120):
    s = socket.socket()
    s.settimeout(timeout)
    s.connect((HOST, PORT))
    try:
        s.sendall(json.dumps({"type": cmd, "params": params or {}}).encode("utf-8"))
        data = b""
        while True:
            chunk = s.recv(65536)
            if not chunk:
                return {"status": "error", "error": "empty"}
            data += chunk
            try:
                return json.loads(data.decode("utf-8"))
            except json.JSONDecodeError:
                continue
    finally:
        s.close()


def ok(label, r):
    status = r.get("status")
    print(f"[{status}] {label}")
    if status != "success":
        print(" ", json.dumps(r, ensure_ascii=False)[:400])
    return status == "success"


def grid_positions(n):
    # Store interior inset: X -450..450, Y 50..750 (relative to BP at origin)
    x0, x1 = -450.0, 450.0
    y0, y1 = 50.0, 750.0
    if n <= 1:
        return [(0.0, 400.0, LIGHT_Z)]
    pts = []
    for j in range(n):
        for i in range(n):
            x = x0 + (x1 - x0) * i / (n - 1)
            y = y0 + (y1 - y0) * j / (n - 1)
            pts.append((x, y, LIGHT_Z))
    return pts


def main():
    # 1) Disable collision on graybox meshes (visual only — avoid landscape/wall traps)
    for name in MESH_PARTS:
        send(
            "set_component_property",
            {
                "blueprint_name": BP,
                "component_name": name,
                "property_name": "BodyInstance.CollisionEnabled",
                "property_value": "NoCollision",
            },
        )
        # Also try direct enum-style property used by some builds
        send(
            "set_component_property",
            {
                "blueprint_name": BP,
                "component_name": name,
                "property_name": "CollisionEnabled",
                "property_value": "NoCollision",
            },
        )
    print(f"collision_clear attempted on {len(MESH_PARTS)} meshes")

    # 2) Add n*n PointLightComponents under store ceiling
    lights = grid_positions(N)
    for idx, (x, y, z) in enumerate(lights):
        cname = f"StoreCeilLight_{idx}"
        r = send(
            "add_component_to_blueprint",
            {
                "blueprint_name": BP,
                "component_type": "PointLightComponent",
                "component_name": cname,
                "location": [x, y, z],
                "rotation": [0.0, 0.0, 0.0],
                "scale": [1.0, 1.0, 1.0],
            },
        )
        ok(f"add {cname} @ {[x,y,z]}", r)
        send(
            "set_component_property",
            {
                "blueprint_name": BP,
                "component_name": cname,
                "property_name": "Intensity",
                "property_value": "5000",
            },
        )
        send(
            "set_component_property",
            {
                "blueprint_name": BP,
                "component_name": cname,
                "property_name": "AttenuationRadius",
                "property_value": "800",
            },
        )
        send(
            "set_component_property",
            {
                "blueprint_name": BP,
                "component_name": cname,
                "property_name": "bUseInverseSquaredFalloff",
                "property_value": "true",
            },
        )

    ok("compile", send("compile_blueprint", {"blueprint_name": BP}))

    # 3) Raise station so floor near walkable landscape height
    ok(
        "raise_station",
        send(
            "set_actor_transform",
            {
                "name": "GasStationGraybox",
                "location": [0.0, 0.0, STATION_Z],
                "rotation": [0.0, 0.0, 0.0],
                "scale": [1.0, 1.0, 1.0],
            },
        ),
    )

    # 4) Move PlayerStart to open canopy forecourt (south of pumps), facing store (+Y)
    level = send("get_actors_in_level")
    actors = (level.get("result") or {}).get("actors") or []
    ps_name = None
    for a in actors:
        if isinstance(a, dict) and "PlayerStart" in a.get("name", ""):
            ps_name = a["name"]
            break
    if not ps_name:
        print("ERROR no PlayerStart")
        sys.exit(2)

    ok(
        "move_playerstart",
        send(
            "set_actor_transform",
            {
                "name": ps_name,
                "location": [0.0, -600.0, GROUND_Z],
                "rotation": [0.0, 90.0, 0.0],
                "scale": [1.0, 1.0, 1.0],
            },
        ),
    )

    # 5) Relocate pickups near new start (still easy to collect)
    pickups = [
        ("GrayPickup_A", [-80.0, -480.0, GROUND_Z + 30.0]),
        ("GrayPickup_B", [80.0, -480.0, GROUND_Z + 30.0]),
        ("GrayPickup_C", [0.0, -420.0, GROUND_Z + 30.0]),
    ]
    for name, loc in pickups:
        ok(
            f"move_{name}",
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

    # Respawn station instance so new light components appear (delete + spawn)
    for attempt in range(3):
        try:
            send("delete_actor", {"name": "GasStationGraybox"})
            break
        except OSError as e:
            print(f"delete retry {attempt}: {e}")
            import time
            time.sleep(1.0)
    import time
    time.sleep(0.5)
    for attempt in range(3):
        try:
            r = send(
                "spawn_blueprint_actor",
                {
                    "blueprint_name": BP,
                    "actor_name": "GasStationGraybox",
                    "location": [0.0, 0.0, STATION_Z],
                    "rotation": [0.0, 0.0, 0.0],
                },
            )
            ok("respawn_station", r)
            break
        except OSError as e:
            print(f"spawn retry {attempt}: {e}")
            time.sleep(1.0)
    send(
        "set_actor_transform",
        {
            "name": "GasStationGraybox",
            "location": [0.0, 0.0, STATION_Z],
            "rotation": [0.0, 0.0, 0.0],
            "scale": [1.0, 1.0, 1.0],
        },
    )

    print(f"G1_LIGHTS_SPAWN_DONE N={N} lights={len(lights)} station_z={STATION_Z}")
    print("Please Ctrl+S then PIE: spawn at canopy forecourt, walk freely; store ceiling should be lit.")


if __name__ == "__main__":
    main()
