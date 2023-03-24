
import socket
import time


msgFromClient = "Hello UDP Server"

bytesToSend = str.encode(msgFromClient)

serverAddressPort = ("10.10.10.221", 20001)

bufferSize = 1024
localIP = "10.10.10.213"

localPort = 20002


# Create a UDP socket at client side

UDPClientSocket = socket.socket(family=socket.AF_INET, type=socket.SOCK_DGRAM)

UDPClientSocket.bind((localIP, localPort))

# Send to server using created UDP socket

total_time = 0
num_iter = 1
for i in range(num_iter):

    UDPClientSocket.sendto(bytesToSend, serverAddressPort)
    send_time = time.time_ns()
    
    
    msgFromServer = UDPClientSocket.recvfrom(bufferSize)
    recv_time = time.time_ns()
    total_time += (recv_time - send_time)


#msg = "Message from Server {}".format(msgFromServer[0])
#
#print(msg)
print("Average latency is: " + str(total_time/num_iter/1000) + " us")
