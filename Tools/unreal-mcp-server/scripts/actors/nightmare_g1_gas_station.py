"""G1 — build BP_GasStationGraybox (cube graybox) and spawn at world origin."""
import json
import socket
import sys

HOST, PORT = "127.0.0.1", 55557
BP = "BP_GasStationGraybox"
CUBE = "/Engine/BasicShapes/Cube.Cube"

# name -> (location, scale)  — optional rotation as 3rd tuple entry
PARTS = [
    # Store
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
    # Canopy
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
    # Pumps
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
    # Sign
    ("SignPole", [-900.0, -200.0, 300.0], [0.5, 0.5, 6.0]),
    ("SignBoard", [-900.0, -200.0, 650.0], [4.0, 0.3, 2.0]),
    ("SignBoardFrame", [-900.0, -200.0, 650.0], [4.3, 0.15, 2.3]),
    ("PriceBoard", [-900.0, -200.0, 520.0], [2.0, 0.2, 1.0]),
    ("SignBase", [-900.0, -200.0, 20.0], [1.2, 1.2, 0.4]),
    # Curbs / props
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
    err = r.get("error") or (r.get("result") or {}).get("error")
    already = (r.get("result") or {}).get("already_exists")
    line = f"[{status}] {label}"
    if already:
        line += " (exists)"
    if err and status != "success":
        line += f" ERR={err}"
        print(line)
        return False
    print(line)
    return status == "success" or already


def main():
    fails = 0
    r = send("create_blueprint", {"name": BP, "parent_class": "Actor"})
    if not ok("create_blueprint", r):
        # reuse if already exists
        if "already" not in str(r).lower() and not (r.get("result") or {}).get("already_exists"):
            print("ABORT create")
            sys.exit(1)

    for name, loc, scale in PARTS:
        r = send(
            "add_component_to_blueprint",
            {
                "blueprint_name": BP,
                "component_type": "StaticMeshComponent",
                "component_name": name,
                "location": loc,
                "rotation": [0.0, 0.0, 0.0],
                "scale": scale,
            },
        )
        if not ok(f"add {name}", r):
            # component may already exist from partial run
            if "already" not in str(r).lower():
                fails += 1
                continue
        r = send(
            "set_static_mesh_properties",
            {
                "blueprint_name": BP,
                "component_name": name,
                "static_mesh": CUBE,
            },
        )
        if not ok(f"mesh {name}", r):
            fails += 1

    if not ok("compile", send("compile_blueprint", {"blueprint_name": BP})):
        sys.exit(2)

    # remove previous instance if any
    send("delete_actor", {"name": "GasStationGraybox"})
    r = send(
        "spawn_blueprint_actor",
        {
            "blueprint_name": BP,
            "actor_name": "GasStationGraybox",
            "location": [0.0, 0.0, 0.0],
            "rotation": [0.0, 0.0, 0.0],
        },
    )
    if not ok("spawn", r):
        sys.exit(3)

    # First mesh can leak into actor root scale on some MCP builds — force identity.
    ok(
        "fix_scale",
        send(
            "set_actor_transform",
            {
                "name": "GasStationGraybox",
                "location": [0.0, 0.0, 0.0],
                "rotation": [0.0, 0.0, 0.0],
                "scale": [1.0, 1.0, 1.0],
            },
        ),
    )

    found = send("find_actors_by_name", {"pattern": "GasStation"})
    print(json.dumps(found, indent=2)[:1500])
    print(f"G1_DONE parts={len(PARTS)} fails={fails}")
    sys.exit(0 if fails == 0 else 4)


if __name__ == "__main__":
    main()
