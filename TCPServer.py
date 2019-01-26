from socket import *

s = socket(AF_INET, SOCK_STREAM)
s.bind(("", 8080))

s.listen(2)
r,addr = s.accept()
r.send("Hey client, you made a connection with me, the server")

s.close()
