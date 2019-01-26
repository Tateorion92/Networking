from socket import *

port=8080
ip="127.0.0.1"

s=socket(AF_INET,SOCK_DGRAM)
s.sendto("hello",(ip,port))
recv=s.recvfrom(2048)
s.close()
