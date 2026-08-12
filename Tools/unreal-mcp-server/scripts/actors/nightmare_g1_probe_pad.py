import json, socket, time

def send(cmd, params=None):
    s = socket.socket(); s.settimeout(60)
    s.connect(("127.0.0.1", 55557))
    try:
        s.sendall(json.dumps({"type": cmd, "params": params or {}}).encode())
        d = b""
        while True:
            c = s.recv(65536)
            if not c: break
            d += c
            try: return json.loads(d.decode())
            except json.JSONDecodeError: continue
    finally:
        s.close()

for n in ("GS_LotPad", "GS_StoreFloor", "GS_PumpIsland_1"):
    r = send("get_actor_properties", {"name": n})
    root = r.get("result") or r
    loc = root.get("location") if isinstance(root, dict) else None
    sc = root.get("scale") if isinstance(root, dict) else None
    print(n, "loc", loc, "scale", sc)
