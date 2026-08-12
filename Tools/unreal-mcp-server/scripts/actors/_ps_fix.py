import json,socket
def send(cmd,params=None):
 s=socket.socket();s.settimeout(30);s.connect(("127.0.0.1",55557))
 try:
  s.sendall(json.dumps({"type":cmd,"params":params or {}}).encode());d=b""
  while True:
   c=s.recv(65536)
   if not c:return {}
   d+=c
   try:return json.loads(d.decode())
   except:continue
 finally:s.close()
level=send("get_actors_in_level");actors=(level.get("result") or {}).get("actors") or []
for a in actors:
 if isinstance(a,dict) and "PlayerStart" in a.get("name",""):
  print("PS",a)
  print(send("set_actor_transform",{"name":a["name"],"location":[0.0,-550.0,100.0],"rotation":[0.0,90.0,0.0],"scale":[1,1,1]}))
