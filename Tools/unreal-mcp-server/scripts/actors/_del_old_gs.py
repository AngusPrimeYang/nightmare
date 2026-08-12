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
print(send("delete_actor",{"name":"GasStationGraybox"}))
