from socket import *

s = socket(AF_INET, SOCK_STREAM)

s.connect(("", 8080))

s.send("Hey server, client here\n")

print s.recv(4040)

s.close()
