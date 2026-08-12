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

r = send("get_actors_in_level")
actors = (r.get("result") or {}).get("actors") or r.get("actors") or []
# print compact for PlayerStart / Gas / Pickup / Dev
keys = ("PlayerStart", "Gas", "Pickup", "Gray", "Dev", "Floor", "Landscape")
for a in actors if isinstance(actors, list) else []:
    name = a.get("name", "") if isinstance(a, dict) else str(a)
    if any(k.lower() in name.lower() for k in keys) or "Start" in name:
        print(json.dumps(a, ensure_ascii=False)[:400])
print("TOTAL", len(actors) if isinstance(actors, list) else type(actors))
