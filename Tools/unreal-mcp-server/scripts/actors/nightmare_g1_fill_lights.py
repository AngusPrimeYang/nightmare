import json, socket, time

def send(cmd, params=None, timeout=60):
    last = None
    for attempt in range(5):
        s = socket.socket(); s.settimeout(timeout)
        try:
            s.connect(("127.0.0.1", 55557))
            s.sendall(json.dumps({"type": cmd, "params": params or {}}).encode())
            d = b""
            while True:
                c = s.recv(65536)
                if not c: raise ConnectionError("empty")
                d += c
                try: return json.loads(d.decode())
                except json.JSONDecodeError: continue
        except OSError as e:
            last = e; time.sleep(1.5)
        finally:
            try: s.close()
            except OSError: pass
    return {"status":"error","error":str(last)}

# count GS_ and lights
level = send("get_actors_in_level")
actors = (level.get("result") or {}).get("actors") or []
gs = [a for a in actors if isinstance(a, dict) and a.get("name","").startswith("GS_")]
lights = [a for a in actors if isinstance(a, dict) and "StoreCeilLight" in a.get("name","")]
print("GS_count", len(gs))
print("lights", [a["name"] for a in lights])
print("sample", gs[:3])

STATION_Z = 90.0
LIGHT_Z = STATION_Z + 290.0
N = 3
x0,x1=-450.0,450.0
y0,y1=50.0,750.0
needed = []
for j in range(N):
    for i in range(N):
        needed.append((
            f"StoreCeilLight_{j*N+i}",
            x0+(x1-x0)*i/(N-1),
            y0+(y1-y0)*j/(N-1),
            LIGHT_Z,
        ))

have = {a["name"] for a in lights}
for name,x,y,z in needed:
    if name in have:
        print("have", name)
        continue
    send("delete_actor", {"name": name})
    r = send("spawn_actor", {"name": name, "type": "PointLight", "location": [x,y,z], "rotation":[0,0,0]})
    print(name, r.get("status"), r.get("error"))
    send("set_actor_property", {"name": name, "property_name": "PointLightComponent.Intensity", "property_value": "5000"})
    send("set_actor_property", {"name": name, "property_name": "PointLightComponent.AttenuationRadius", "property_value": "800"})
    time.sleep(0.1)

print("final_lights", send("find_actors_by_name", {"pattern": "StoreCeilLight"}))
print("final_gs", send("find_actors_by_name", {"pattern": "GS_Store"}))
print("FILL_DONE")
