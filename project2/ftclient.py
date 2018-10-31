#!/usr/bin/python
# File: ftclient.py
# Author: Jon Raleigh (OSUID: raleigjo)
# Date 8/12/18
# Description: File transfer client
import socket
import sys
import os.path
import difflib

def initiate_contact():

    c_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    c_socket.connect((sys.argv[1], int(sys.argv[2])))
    return c_socket

def makeRequest(f_socketfd, command, fn, dp):

    request_type = ''

    if command == "-l":
        f_socketfd.send(bytes((command + ' ' + dp + '\n').encode("utf8")))
        print("just sent l")
        request_type = 'l'

    if command == "-g":
        f_socketfd.send(bytes(command + ' ' + fn + ',' + dp + '\n', "utf8"))
        request_type = 'g'

    if command != '-l' and command != '-g':
        print("failure\n")

    return request_type

def processResponse(r, ds, cs):

    if r == 'l':
        filelist = ds.recv(4096)
        print("Receiving directory contents:\n")
        print(filelist)

    if r == 'g':
        response_mssg = cs.recv(4096)
        if response_mssg.decode("latin-1") == "ERROR: FILE NOT FOUND!\n":
            print(response_mssg.decode("latin-1"))
            return

        receiveFile(ds)

    ds.close()


def receiveFile(f_datasocket):

    filename = sys.argv[4]

    if os.path.isfile(filename):
        filename = filename + '_copy'

    with open(filename, 'wb') as f:
        while True:
            data_rcvd = f_datasocket.recv(1000)
            if not data_rcvd:
                break
            f.write(data_rcvd)

    f.close()

def setup_datasock():

    s_port = int(sys.argv[len(sys.argv)-1])
    s_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s_socket.bind(('', s_port))
    s_socket.listen(1)
    sock, addr = s_socket.accept()
    return sock

def send_datasock_info(socket_to_send):
    socket_to_send.send(bytes((sys.argv[len(sys.argv)-1]).encode("utf8")))

# program begins here
print("starting up\n")
socketfd = initiate_contact()
send_datasock_info(socketfd)
print("starting datasocket\n")
datasocket = setup_datasock()
print("sending request\n")
request = makeRequest(socketfd, sys.argv[3], sys.argv[4], sys.argv[len(sys.argv)-1])
print("processing response\n")
processResponse(request, datasocket, socketfd)
print("closing down\n")
socketfd.close()
