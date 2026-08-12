import json, socket

def send(cmd, params=None, timeout=90):
    s = socket.socket(); s.settimeout(timeout)
    s.connect(("127.0.0.1", 55557))
    try:
        s.sendall(json.dumps({"type": cmd, "params": params or {}}).encode())
        d = b""
        while True:
            c = s.recv(65536)
            if not c: return {"status":"error","error":"empty"}
            d += c
            try: return json.loads(d.decode())
            except json.JSONDecodeError: continue
    finally:
        s.close()

def show(label, r):
    print(f"=== {label} ===")
    print(json.dumps(r, indent=2)[:2000])
    return r

# recreate pickup BP if needed
r = send("create_blueprint", {"name": "BP_Pickup", "parent_class": "NightmarePickupActor"})
show("create", r)
if r.get("status") != "success" and "already exists" not in str(r.get("error","")):
    # try rename conflict from old Actor parent version
    pass

show("mesh", send("add_component_to_blueprint", {
    "blueprint_name": "BP_Pickup",
    "component_type": "StaticMeshComponent",
    "component_name": "GrayboxMesh",
    "scale": [0.6, 0.6, 0.6],
}))
show("setmesh", send("set_static_mesh_properties", {
    "blueprint_name": "BP_Pickup",
    "component_name": "GrayboxMesh",
    "static_mesh": "/Engine/BasicShapes/Cube.Cube",
}))
show("compile", send("compile_blueprint", {"blueprint_name": "BP_Pickup"}))

# near PlayerStart ~(-200,0,92)
spawns = [
    ("BP_Pickup_A", [-50.0, 0.0, 120.0]),
    ("BP_Pickup_B", [50.0, 80.0, 120.0]),
    ("BP_Pickup_C", [0.0, -100.0, 120.0]),
]
for name, loc in spawns:
    show(f"spawn_{name}", send("spawn_blueprint_actor", {
        "blueprint_name": "BP_Pickup",
        "actor_name": name,
        "location": loc,
        "rotation": [0,0,0],
    }))

show("find", send("find_actors_by_name", {"pattern": "BP_Pickup"}))
print("GRAYBOX_PICKUP_DONE")
