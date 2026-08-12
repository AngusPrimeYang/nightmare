import json, socket, time

def send(cmd, params=None, timeout=90):
    s = socket.socket()
    s.settimeout(timeout)
    s.connect(("127.0.0.1", 55557))
    try:
        s.sendall(json.dumps({"type": cmd, "params": params or {}}).encode())
        d = b""
        while True:
            c = s.recv(65536)
            if not c:
                return {"error": "empty"}
            d += c
            try:
                return json.loads(d.decode())
            except json.JSONDecodeError:
                continue
    finally:
        s.close()

print("delete", send("delete_actor", {"name": "GasStationGraybox"}))
time.sleep(0.5)
print("spawn", send("spawn_blueprint_actor", {
    "blueprint_name": "BP_GasStationGraybox",
    "actor_name": "GasStationGraybox",
    "location": [0.0, 0.0, 90.0],
    "rotation": [0.0, 0.0, 0.0],
}))
time.sleep(0.3)
print("scale", send("set_actor_transform", {
    "name": "GasStationGraybox",
    "location": [0.0, 0.0, 90.0],
    "rotation": [0.0, 0.0, 0.0],
    "scale": [1.0, 1.0, 1.0],
}))
print("find", json.dumps(send("find_actors_by_name", {"pattern": "GasStation|PlayerStart|GrayPickup"}), indent=2)[:2000])
