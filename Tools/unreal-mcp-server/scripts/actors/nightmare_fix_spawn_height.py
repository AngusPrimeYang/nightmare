"""Raise PlayerStart + pickups above landscape so PIE is not underground."""
import json
import socket
import time

HOST, PORT = "127.0.0.1", 55557
# Open World terrain near origin varies; previous Z=100 buried spawn at y=-550.
SPAWN = [0.0, -600.0, 350.0]
YAW = 90.0  # face +Y (store)


def send(cmd, params=None, timeout=60):
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


def main():
    level = send("get_actors_in_level")
    actors = (level.get("result") or {}).get("actors") or []
    ps = None
    for a in actors:
        if isinstance(a, dict) and "PlayerStart" in a.get("name", ""):
            ps = a
            break
    if not ps:
        print("NO_PLAYERSTART")
        return 1
    print("before", ps)
    r = send(
        "set_actor_transform",
        {
            "name": ps["name"],
            "location": SPAWN,
            "rotation": [0.0, YAW, 0.0],
            "scale": [1.0, 1.0, 1.0],
        },
    )
    print("playerstart", r.get("status"), (r.get("result") or {}).get("location"))

    for name, loc in [
        ("GrayPickup_A", [-80.0, -520.0, SPAWN[2] + 40.0]),
        ("GrayPickup_B", [80.0, -520.0, SPAWN[2] + 40.0]),
        ("GrayPickup_C", [0.0, -480.0, SPAWN[2] + 40.0]),
    ]:
        r = send(
            "set_actor_transform",
            {
                "name": name,
                "location": loc,
                "rotation": [0.0, 0.0, 0.0],
                "scale": [1.0, 1.0, 1.0],
            },
        )
        print(name, r.get("status"), loc)

    print("SPAWN_RAISED — Ctrl+S then PIE (fall onto ground OK)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
