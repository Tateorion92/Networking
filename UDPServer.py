from socket import *

port=8080
ip="127.0.0.1"

s=socket(AF_INET,SOCK_DGRAM)
s.bind((ip,port))

while True:
	data, addr= s.recvfrom(1024)
	print("message: ",data)
