"""Enable BlockAll on BP_GrayCube so GS_* station parts stop enemies/players."""
import json
import socket
import time

HOST, PORT = "127.0.0.1", 55557
CUBE_BP = "BP_GrayCube"


def send(cmd, params=None, timeout=60):
    s = socket.socket()
    s.settimeout(timeout)
    try:
        s.connect((HOST, PORT))
        s.sendall(json.dumps({"type": cmd, "params": params or {}}).encode())
        d = b""
        while True:
            c = s.recv(65536)
            if not c:
                raise ConnectionError("empty")
            d += c
            try:
                return json.loads(d.decode())
            except json.JSONDecodeError:
                continue
    finally:
        try:
            s.close()
        except OSError:
            pass


def main():
    for prop, val in (
        ("CollisionEnabled", "QueryAndPhysics"),
        ("BodyInstance.CollisionEnabled", "QueryAndPhysics"),
        ("CollisionProfileName", "BlockAll"),
        ("BodyInstance.CollisionProfileName", "BlockAll"),
    ):
        r = send(
            "set_component_property",
            {
                "blueprint_name": CUBE_BP,
                "component_name": "CubeMesh",
                "property_name": prop,
                "property_value": val,
            },
        )
        print(prop, r.get("status"), r.get("error") or "")
        time.sleep(0.1)

    r = send("compile_blueprint", {"blueprint_name": CUBE_BP})
    print("compile", r.get("status"), r.get("error") or "")


if __name__ == "__main__":
    main()
