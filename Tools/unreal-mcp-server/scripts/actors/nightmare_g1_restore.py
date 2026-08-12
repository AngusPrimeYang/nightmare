"""Full G1 restore: BP cubes + 3x3 ceiling lights + NoCollision + spawn + PlayerStart/pickups."""
import json
import socket
import sys
import time

HOST, PORT = "127.0.0.1", 55557
BP = "BP_GasStationGraybox"
CUBE = "/Engine/BasicShapes/Cube.Cube"
N = 3
STATION_Z = 100.0
LIGHT_Z = 290.0
GROUND_Z = 350.0

PARTS = [
    ("StoreFloor", [0.0, 400.0, 5.0], [12.0, 10.0, 0.1]),
    ("StoreWall_N", [0.0, 900.0, 150.0], [12.0, 0.2, 3.0]),
    ("StoreWall_W", [-600.0, 400.0, 150.0], [0.2, 10.0, 3.0]),
    ("StoreWall_E", [600.0, 400.0, 150.0], [0.2, 10.0, 3.0]),
    ("StoreWall_S_L", [-400.0, -100.0, 150.0], [4.0, 0.2, 3.0]),
    ("StoreWall_S_R", [400.0, -100.0, 150.0], [4.0, 0.2, 3.0]),
    ("StoreDoorLintel", [0.0, -100.0, 280.0], [2.0, 0.25, 0.4]),
    ("StoreDoorJamb_L", [-100.0, -100.0, 140.0], [0.25, 0.25, 2.8]),
    ("StoreDoorJamb_R", [100.0, -100.0, 140.0], [0.25, 0.25, 2.8]),
    ("StoreRoof", [0.0, 400.0, 320.0], [12.5, 10.5, 0.15]),
    ("StoreEave_S", [0.0, -120.0, 305.0], [12.8, 0.4, 0.2]),
    ("StoreEave_N", [0.0, 920.0, 305.0], [12.8, 0.4, 0.2]),
    ("StoreWindow_L", [-250.0, -115.0, 180.0], [1.6, 0.08, 1.2]),
    ("StoreWindow_R", [250.0, -115.0, 180.0], [1.6, 0.08, 1.2]),
    ("StoreWindowSill_L", [-250.0, -115.0, 115.0], [1.8, 0.15, 0.15]),
    ("StoreWindowSill_R", [250.0, -115.0, 115.0], [1.8, 0.15, 0.15]),
    ("StoreCornice_L", [-250.0, -115.0, 250.0], [1.8, 0.15, 0.12]),
    ("StoreCornice_R", [250.0, -115.0, 250.0], [1.8, 0.15, 0.12]),
    ("CanopyDeck", [0.0, -200.0, 450.0], [20.0, 16.0, 0.2]),
    ("CanopyEdge_N", [0.0, 580.0, 465.0], [20.2, 0.35, 0.45]),
    ("CanopyEdge_S", [0.0, -980.0, 465.0], [20.2, 0.35, 0.45]),
    ("CanopyEdge_E", [1000.0, -200.0, 465.0], [0.35, 16.2, 0.45]),
    ("CanopyEdge_W", [-1000.0, -200.0, 465.0], [0.35, 16.2, 0.45]),
    ("CanopyLight_1", [-500.0, -200.0, 430.0], [0.4, 0.4, 0.15]),
    ("CanopyLight_2", [0.0, -200.0, 430.0], [0.4, 0.4, 0.15]),
    ("CanopyLight_3", [500.0, -200.0, 430.0], [0.4, 0.4, 0.15]),
    ("CanopyPost_FL", [-900.0, -900.0, 225.0], [0.4, 0.4, 4.5]),
    ("CanopyPost_FR", [900.0, -900.0, 225.0], [0.4, 0.4, 4.5]),
    ("CanopyPost_BL", [-900.0, 500.0, 225.0], [0.4, 0.4, 4.5]),
    ("CanopyPost_BR", [900.0, 500.0, 225.0], [0.4, 0.4, 4.5]),
    ("CanopyCap_FL", [-900.0, -900.0, 455.0], [0.7, 0.7, 0.25]),
    ("CanopyCap_FR", [900.0, -900.0, 455.0], [0.7, 0.7, 0.25]),
    ("CanopyCap_BL", [-900.0, 500.0, 455.0], [0.7, 0.7, 0.25]),
    ("CanopyCap_BR", [900.0, 500.0, 455.0], [0.7, 0.7, 0.25]),
    ("PumpIsland_1", [-300.0, -200.0, 15.0], [3.0, 1.5, 0.3]),
    ("PumpIsland_2", [200.0, -200.0, 15.0], [3.0, 1.5, 0.3]),
    ("PumpIslandEdge_1", [-300.0, -200.0, 32.0], [3.2, 1.7, 0.08]),
    ("PumpIslandEdge_2", [200.0, -200.0, 32.0], [3.2, 1.7, 0.08]),
    ("PumpBody_1", [-300.0, -200.0, 120.0], [0.8, 0.6, 1.8]),
    ("PumpBody_2", [200.0, -200.0, 120.0], [0.8, 0.6, 1.8]),
    ("PumpHead_1", [-300.0, -200.0, 220.0], [1.0, 0.7, 0.35]),
    ("PumpHead_2", [200.0, -200.0, 220.0], [1.0, 0.7, 0.35]),
    ("PumpHose_1", [-250.0, -200.0, 90.0], [0.12, 0.9, 0.12]),
    ("PumpHose_2", [250.0, -200.0, 90.0], [0.12, 0.9, 0.12]),
    ("PumpNozzle_1", [-220.0, -200.0, 55.0], [0.2, 0.35, 0.15]),
    ("PumpNozzle_2", [280.0, -200.0, 55.0], [0.2, 0.35, 0.15]),
    ("SignPole", [-900.0, -200.0, 300.0], [0.5, 0.5, 6.0]),
    ("SignBoard", [-900.0, -200.0, 650.0], [4.0, 0.3, 2.0]),
    ("SignBoardFrame", [-900.0, -200.0, 650.0], [4.3, 0.15, 2.3]),
    ("PriceBoard", [-900.0, -200.0, 520.0], [2.0, 0.2, 1.0]),
    ("SignBase", [-900.0, -200.0, 20.0], [1.2, 1.2, 0.4]),
    ("Curb_A", [-400.0, -450.0, 8.0], [10.0, 0.35, 0.16]),
    ("Curb_B", [400.0, -450.0, 8.0], [10.0, 0.35, 0.16]),
    ("Curb_C", [0.0, 50.0, 8.0], [16.0, 0.35, 0.16]),
    ("Trash_1", [-160.0, -40.0, 40.0], [0.5, 0.5, 0.8]),
    ("Trash_2", [-200.0, -20.0, 28.0], [0.4, 0.4, 0.55]),
    ("IceBox", [180.0, -30.0, 55.0], [1.2, 0.8, 1.1]),
    ("IceBoxLid", [180.0, -30.0, 115.0], [1.25, 0.85, 0.12]),
    ("Bench", [350.0, 50.0, 35.0], [1.8, 0.5, 0.35]),
]


def send(cmd, params=None, timeout=120):
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
                    raise ConnectionError("empty recv")
                data += chunk
                try:
                    return json.loads(data.decode("utf-8"))
                except json.JSONDecodeError:
                    continue
        except OSError as e:
            last_err = e
            time.sleep(1.0 + attempt)
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
        print(" ", json.dumps(r, ensure_ascii=False)[:300])
    return status == "success"


def soft(label, r):
    """Treat already-exists / soft failures as OK for restore."""
    if ok(label, r):
        return True
    blob = json.dumps(r).lower()
    if "already" in blob or "exists" in blob:
        print(f"  (reuse) {label}")
        return True
    return False


def light_grid(n):
    x0, x1 = -450.0, 450.0
    y0, y1 = 50.0, 750.0
    if n <= 1:
        return [(0.0, 400.0, LIGHT_Z)]
    out = []
    for j in range(n):
        for i in range(n):
            x = x0 + (x1 - x0) * i / (n - 1)
            y = y0 + (y1 - y0) * j / (n - 1)
            out.append((x, y, LIGHT_Z))
    return out


def wait_mcp(max_sec=300):
    print(f"Waiting for MCP on {HOST}:{PORT} (up to {max_sec}s)...")
    t0 = time.time()
    while time.time() - t0 < max_sec:
        s = socket.socket()
        s.settimeout(2)
        try:
            s.connect((HOST, PORT))
            s.close()
            print("MCP ready")
            return True
        except OSError:
            time.sleep(2)
        finally:
            try:
                s.close()
            except OSError:
                pass
    return False


def main():
    if not wait_mcp():
        print("ABORT: Editor/MCP not up")
        sys.exit(1)

    soft("create_blueprint", send("create_blueprint", {"name": BP, "parent_class": "Actor"}))

    for name, loc, scale in PARTS:
        soft(
            f"add {name}",
            send(
                "add_component_to_blueprint",
                {
                    "blueprint_name": BP,
                    "component_type": "StaticMeshComponent",
                    "component_name": name,
                    "location": loc,
                    "rotation": [0.0, 0.0, 0.0],
                    "scale": scale,
                },
            ),
        )
        soft(
            f"mesh {name}",
            send(
                "set_static_mesh_properties",
                {"blueprint_name": BP, "component_name": name, "static_mesh": CUBE},
            ),
        )
        send(
            "set_component_property",
            {
                "blueprint_name": BP,
                "component_name": name,
                "property_name": "BodyInstance.CollisionEnabled",
                "property_value": "NoCollision",
            },
        )
        send(
            "set_component_property",
            {
                "blueprint_name": BP,
                "component_name": name,
                "property_name": "CollisionEnabled",
                "property_value": "NoCollision",
            },
        )

    for idx, (x, y, z) in enumerate(light_grid(N)):
        cname = f"StoreCeilLight_{idx}"
        soft(
            f"add {cname}",
            send(
                "add_component_to_blueprint",
                {
                    "blueprint_name": BP,
                    "component_type": "PointLightComponent",
                    "component_name": cname,
                    "location": [x, y, z],
                    "rotation": [0.0, 0.0, 0.0],
                    "scale": [1.0, 1.0, 1.0],
                },
            ),
        )
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

    if not soft("compile", send("compile_blueprint", {"blueprint_name": BP})):
        sys.exit(2)

    time.sleep(0.5)
    send("delete_actor", {"name": "GasStationGraybox"})
    time.sleep(0.5)
    if not soft(
        "spawn",
        send(
            "spawn_blueprint_actor",
            {
                "blueprint_name": BP,
                "actor_name": "GasStationGraybox",
                "location": [0.0, 0.0, STATION_Z],
                "rotation": [0.0, 0.0, 0.0],
            },
        ),
    ):
        sys.exit(3)

    soft(
        "fix_scale",
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

    level = send("get_actors_in_level")
    actors = (level.get("result") or {}).get("actors") or []
    ps_name = None
    for a in actors:
        if isinstance(a, dict) and "PlayerStart" in a.get("name", ""):
            ps_name = a["name"]
            break
    if ps_name:
        soft(
            "playerstart",
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
    else:
        print("WARN: no PlayerStart found")

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

    found = send("find_actors_by_name", {"pattern": "GasStation"})
    print(json.dumps(found, indent=2)[:1200])
    print("G1_RESTORE_DONE — please Ctrl+S")
    return 0


if __name__ == "__main__":
    sys.exit(main())
