"""Probe GS_* / PlayerStart / Landscape Z in the loaded level."""
import json
import socket
import time

HOST, PORT = "127.0.0.1", 55557


def send(cmd, params=None, timeout=90):
    last = None
    for attempt in range(5):
        s = socket.socket()
        s.settimeout(timeout)
        try:
            s.connect((HOST, PORT))
            s.sendall(json.dumps({"type": cmd, "params": params or {}}).encode())
            data = b""
            while True:
                chunk = s.recv(65536)
                if not chunk:
                    raise ConnectionError("empty")
                data += chunk
                try:
                    return json.loads(data.decode())
                except json.JSONDecodeError:
                    continue
        except OSError as e:
            last = e
            time.sleep(1.0)
        finally:
            try:
                s.close()
            except OSError:
                pass
    return {"status": "error", "error": str(last)}


def loc_of(props):
    """Best-effort extract location from get_actor_properties payload."""
    if not isinstance(props, dict):
        return None
    root = props.get("result") or props
    if not isinstance(root, dict):
        return None
    for key in ("location", "Location", "actor_location", "ActorLocation"):
        v = root.get(key)
        if isinstance(v, (list, tuple)) and len(v) >= 3:
            return [float(v[0]), float(v[1]), float(v[2])]
        if isinstance(v, dict) and all(k in v for k in ("x", "y", "z")):
            return [float(v["x"]), float(v["y"]), float(v["z"])]
        if isinstance(v, dict) and all(k in v for k in ("X", "Y", "Z")):
            return [float(v["X"]), float(v["Y"]), float(v["Z"])]
    # nested transform
    t = root.get("transform") or root.get("Transform") or root.get("RootComponent")
    if isinstance(t, dict):
        return loc_of({"result": t})
    return None


def main():
    names = [
        "GS_StoreFloor",
        "GS_CanopyPost_FL",
        "GS_CanopyDeck",
        "GS_PumpIsland_1",
        "GS_SignPole",
        "GS_Curb_A",
        "StoreCeilLight_0",
    ]
    for n in names:
        r = send("get_actor_properties", {"name": n})
        loc = loc_of(r)
        print(f"{n}: loc={loc}")
        # print a short dump if loc missing
        if loc is None:
            print("  raw:", json.dumps(r, ensure_ascii=False)[:500])

    level = send("get_actors_in_level")
    actors = (level.get("result") or {}).get("actors") or []
    print(f"TOTAL_ACTORS={len(actors)}")
    for a in actors:
        if not isinstance(a, dict):
            continue
        name = a.get("name", "")
        if (
            name.startswith("GS_")
            or name.startswith("StoreCeil")
            or "PlayerStart" in name
            or "Landscape" in name
            or "Floor" in name
        ):
            print("LIST", json.dumps(a, ensure_ascii=False)[:350])
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
