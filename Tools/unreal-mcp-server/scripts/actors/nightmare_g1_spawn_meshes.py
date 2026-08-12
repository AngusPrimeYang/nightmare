"""Spawn only GS_* gray cubes from BP_GrayCube. Does not touch lights."""
import json, socket, time, sys

HOST, PORT = "127.0.0.1", 55557
CUBE_BP = "BP_GrayCube"
CUBE_MESH = "/Engine/BasicShapes/Cube.Cube"
STATION_Z = 100.0  # pad top / design ground; see nightmare_g1_ground_station.py (GS_LotPad)

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
    ("GS_SignPole", [-900.0, -200.0, 300.0], [0.5, 0.5, 6.0]),
    ("GS_SignBoard", [-900.0, -200.0, 650.0], [4.0, 0.3, 2.0]),
    ("GS_PriceBoard", [-900.0, -200.0, 520.0], [2.0, 0.2, 1.0]),
    ("GS_Curb_A", [-400.0, -450.0, 8.0], [10.0, 0.35, 0.16]),
    ("GS_Curb_B", [400.0, -450.0, 8.0], [10.0, 0.35, 0.16]),
    ("GS_Trash_1", [-160.0, -40.0, 40.0], [0.5, 0.5, 0.8]),
    ("GS_IceBox", [180.0, -30.0, 55.0], [1.2, 0.8, 1.1]),
]


def send(cmd, params=None, timeout=60):
    last = None
    for attempt in range(6):
        s = socket.socket(); s.settimeout(timeout)
        try:
            s.connect((HOST, PORT))
            s.sendall(json.dumps({"type": cmd, "params": params or {}}).encode())
            d = b""
            while True:
                c = s.recv(65536)
                if not c: raise ConnectionError("empty")
                d += c
                try: return json.loads(d.decode())
                except json.JSONDecodeError: continue
        except OSError as e:
            last = e
            print(f"retry {attempt} {e}")
            time.sleep(2.0)
        finally:
            try: s.close()
            except OSError: pass
    return {"status": "error", "error": str(last)}


def main():
    # ensure tiny BP
    send("create_blueprint", {"name": CUBE_BP, "parent_class": "Actor"})
    send("add_component_to_blueprint", {
        "blueprint_name": CUBE_BP, "component_type": "StaticMeshComponent",
        "component_name": "CubeMesh", "location": [0,0,0], "rotation": [0,0,0], "scale": [1,1,1],
    })
    send("set_static_mesh_properties", {
        "blueprint_name": CUBE_BP, "component_name": "CubeMesh", "static_mesh": CUBE_MESH,
    })
    send("set_component_property", {
        "blueprint_name": CUBE_BP, "component_name": "CubeMesh",
        "property_name": "CollisionEnabled", "property_value": "QueryAndPhysics",
    })
    send("set_component_property", {
        "blueprint_name": CUBE_BP, "component_name": "CubeMesh",
        "property_name": "CollisionProfileName", "property_value": "BlockAll",
    })
    r = send("compile_blueprint", {"blueprint_name": CUBE_BP})
    print("compile", r.get("status"))
    if r.get("status") != "success" and "already" not in json.dumps(r).lower():
        # continue anyway if exists
        pass

    time.sleep(1.0)
    fails = 0
    for i, (name, loc, scale) in enumerate(PARTS):
        world = [loc[0], loc[1], loc[2] + STATION_Z]
        send("delete_actor", {"name": name})
        r = send("spawn_blueprint_actor", {
            "blueprint_name": CUBE_BP, "actor_name": name,
            "location": world, "rotation": [0,0,0],
        })
        if r.get("status") != "success":
            print("FAIL spawn", name, r)
            fails += 1
            time.sleep(2.0)
            # retry once
            r = send("spawn_blueprint_actor", {
                "blueprint_name": CUBE_BP, "actor_name": name,
                "location": world, "rotation": [0,0,0],
            })
            if r.get("status") != "success":
                continues = True
                continue
        send("set_actor_transform", {
            "name": name, "location": world, "rotation": [0,0,0], "scale": scale,
        })
        print(f"ok {i+1}/{len(PARTS)} {name}")
        time.sleep(0.08)

    level = send("get_actors_in_level")
    actors = (level.get("result") or {}).get("actors") or []
    gs = [a["name"] for a in actors if isinstance(a, dict) and a.get("name","").startswith("GS_")]
    print("GS_count", len(gs), "fails", fails)
    print("MESH_DONE")
    return 0 if len(gs) >= 30 else 2


if __name__ == "__main__":
    sys.exit(main())
