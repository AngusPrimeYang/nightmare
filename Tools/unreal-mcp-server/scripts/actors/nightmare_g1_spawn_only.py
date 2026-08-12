"""Lightweight G1 restore: spawn existing BP + world PointLights + PlayerStart/pickups.
Avoids re-compiling the huge BP (that has been crashing the Editor).
"""
import json
import socket
import sys
import time

HOST, PORT = "127.0.0.1", 55557
BP = "BP_GasStationGraybox"
N = 3
STATION_Z = 90.0
LIGHT_Z_REL = 290.0
GROUND_Z = 350.0


def send(cmd, params=None, timeout=90):
    last_err = None
    for attempt in range(5):
        s = socket.socket()
        s.settimeout(timeout)
        try:
            s.connect((HOST, PORT))
            s.sendall(json.dumps({"type": cmd, "params": params or {}}).encode("utf-8"))
            data = b""
            while True:
                chunk = s.recv(65536)
                if not chunk:
                    raise ConnectionError("empty")
                data += chunk
                try:
                    return json.loads(data.decode("utf-8"))
                except json.JSONDecodeError:
                    continue
        except OSError as e:
            last_err = e
            time.sleep(1.5 + attempt)
        finally:
            try:
                s.close()
            except OSError:
                pass
    return {"status": "error", "error": str(last_err)}


def ok(label, r):
    print(f"[{r.get('status')}] {label}")
    if r.get("status") != "success":
        print(" ", json.dumps(r, ensure_ascii=False)[:400])
        return False
    return True


def wait_mcp(max_sec=600):
    print(f"Waiting MCP {PORT} up to {max_sec}s...")
    t0 = time.time()
    while time.time() - t0 < max_sec:
        s = socket.socket()
        s.settimeout(2)
        try:
            s.connect((HOST, PORT))
            print("MCP ready")
            return True
        except OSError:
            time.sleep(2)
        finally:
            try:
                s.close()
            except OSError:
                pass
    return False


def light_world_positions(n):
    x0, x1 = -450.0, 450.0
    y0, y1 = 50.0, 750.0
    z = STATION_Z + LIGHT_Z_REL
    out = []
    for j in range(n):
        for i in range(n):
            if n == 1:
                out.append((0.0, 400.0, z))
            else:
                x = x0 + (x1 - x0) * i / (n - 1)
                y = y0 + (y1 - y0) * j / (n - 1)
                out.append((x, y, z))
    return out


def delete_matching(pattern_substr):
    level = send("get_actors_in_level")
    actors = (level.get("result") or {}).get("actors") or []
    for a in actors:
        if not isinstance(a, dict):
            continue
        name = a.get("name", "")
        if pattern_substr in name:
            send("delete_actor", {"name": name})
            print(f"deleted {name}")


def main():
    if not wait_mcp():
        print("ABORT no MCP")
        return 1

    # Clear previous station + old ceiling lights
    delete_matching("GasStation")
    delete_matching("StoreCeilLight")
    time.sleep(0.3)

    # Prefer spawning BP (meshes). If it fails, continue with lights-only so user still sees something.
    r = send(
        "spawn_blueprint_actor",
        {
            "blueprint_name": BP,
            "actor_name": "GasStationGraybox",
            "location": [0.0, 0.0, STATION_Z],
            "rotation": [0.0, 0.0, 0.0],
        },
    )
    bp_ok = ok("spawn_bp", r)
    if bp_ok:
        ok(
            "fix_scale",
            send(
                "set_actor_transform",
                {
                    "name": "GasStationGraybox",
                    "location": [0.0, 0.0, STATION_Z],
                    "rotation": [0.0, 0.0, 0.0],
                    "scale": [1.0, 1.0, 1.0],
                },
            ),
        )

    # Always place world PointLights (reliable even if BP lights missing / crashy)
    for idx, (x, y, z) in enumerate(light_world_positions(N)):
        name = f"StoreCeilLight_{idx}"
        r = send(
            "spawn_actor",
            {
                "name": name,
                "type": "PointLight",
                "location": [x, y, z],
                "rotation": [0.0, 0.0, 0.0],
            },
        )
        ok(f"light {name}", r)
        # Best-effort intensity
        send(
            "set_actor_property",
            {"name": name, "property_name": "Intensity", "property_value": "5000"},
        )
        send(
            "set_actor_property",
            {
                "name": name,
                "property_name": "PointLightComponent.Intensity",
                "property_value": "5000",
            },
        )
        send(
            "set_actor_property",
            {
                "name": name,
                "property_name": "PointLightComponent.AttenuationRadius",
                "property_value": "800",
            },
        )
        time.sleep(0.05)

    level = send("get_actors_in_level")
    actors = (level.get("result") or {}).get("actors") or []
    for a in actors:
        if isinstance(a, dict) and "PlayerStart" in a.get("name", ""):
            ok(
                "playerstart",
                send(
                    "set_actor_transform",
                    {
                        "name": a["name"],
                        "location": [0.0, -600.0, GROUND_Z],
                        "rotation": [0.0, 90.0, 0.0],
                        "scale": [1.0, 1.0, 1.0],
                    },
                ),
            )
            break

    for name, loc in [
        ("GrayPickup_A", [-80.0, -480.0, GROUND_Z + 30.0]),
        ("GrayPickup_B", [80.0, -480.0, GROUND_Z + 30.0]),
        ("GrayPickup_C", [0.0, -420.0, GROUND_Z + 30.0]),
    ]:
        ok(
            name,
            send(
                "set_actor_transform",
                {
                    "name": name,
                    "location": loc,
                    "rotation": [0.0, 0.0, 0.0],
                    "scale": [1.0, 1.0, 1.0],
                },
            ),
        )

    print(json.dumps(send("find_actors_by_name", {"pattern": "GasStation|StoreCeil"}), indent=2)[:1500])
    if not bp_ok:
        print("WARN: BP spawn failed — meshes missing; lights should still be present.")
        print("Re-run mesh rebuild in smaller batches if needed.")
        return 2
    print("G1_LIGHT_SPAWN_DONE — Ctrl+S now")
    return 0


if __name__ == "__main__":
    sys.exit(main())
