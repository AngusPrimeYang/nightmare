import json, socket

def send(cmd, params=None):
    s = socket.socket()
    s.settimeout(60)
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

print(json.dumps(send("set_actor_transform", {
    "name": "GasStationGraybox",
    "location": [0.0, 0.0, 0.0],
    "rotation": [0.0, 0.0, 0.0],
    "scale": [1.0, 1.0, 1.0],
}), indent=2)[:1000])
print(json.dumps(send("find_actors_by_name", {"pattern": "GasStation"}), indent=2)[:1000])
