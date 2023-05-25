import socket
import common
import json
import time
import queue
import ctypes
import threading
import sys
PORT = 12345
SERVER = "localhost"
ADDR = (SERVER, PORT)
MSG_LENGTH = 100
FORMAT = "utf-8"

class ServerHandler:
    KEEP_ALIVE_TIMEOUT = 10 #TODO change back

    #client -> handler
    KEEP_ALIVE_REQUEST = 0
    INFO_REQUEST = 1
    DISCONNECT_REQUEST = 2
    MOVE_REQUEST = 3
    #handler -> client
    KEEP_ALIVE_REPLY = 0
    INFO_REPLY = 1
    GAME_CREATED_REPLY = 2
    ILLEGAL_MOVE_REPLY = 3
    MOVE_ACCEPTED_REPLY = 4
    GAME_ENDED_REPLY = 5

    def __init__(self, ADDR):
        self.incoming_queue = queue.Queue()
        self.outgoing_queue = queue.Queue()
        self.connection = client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client.connect(ADDR)
        self.last_incoming_message_time = time.time()
        self.last_outgoing_message_time = time.time()
        self.thread = threading.Thread(target=self.handle_server)
        self.thread.start()
        self.connection.setblocking(False)
    
    def receive(self):
        try:
            message_length_bytes = self.connection.recv(4)
            if len(message_length_bytes) != 4:
                return None, None
            message_length = int.from_bytes(message_length_bytes, "little")
            if message_length == 0:
                return None, None
            if message_length > 10000:
                raise Exception("big message incoming")
            message_bytes = self.connection.recv(message_length)
            if len(message_bytes) != message_length:
                return None, None
            message, arguments = self.decode_message(message_bytes)
            if message is not None:
                self.last_incoming_message_time = time.time()
            return message, arguments
        except BlockingIOError:
            return None, None
    
    def send(self, message, arguments):
        message = self.encode_message(message, arguments)
        self.connection.send(message)
        self.last_outgoing_message_time = time.time()
    
    def decode_message(self, message):
        message_string = message.decode("utf-8")
        message_dict = json.loads(message_string)
        arguments = None
        try:
            request = message_dict["request"]
            if request == ServerHandler.INFO_REPLY:
                arguments = message_dict["state"]
        except KeyError:
            request = None
        return request, arguments
    
    def encode_message(self, message, arguments):
        message = {"request": message}
        if arguments is not None:
            message = {"request": message} | arguments
        message_string = json.dumps(message)
        message_bytes = message_string.encode("utf-8")
        length_bytes = bytes(ctypes.c_uint32(len(message_bytes)))
        return length_bytes + message_bytes

    def client_timed_out(self):
        return time.time() - self.last_incoming_message_time >= ServerHandler.KEEP_ALIVE_TIMEOUT
    
    def is_time_to_send_keep_alive(self):
        return time.time() - self.last_outgoing_message_time >= ServerHandler.KEEP_ALIVE_TIMEOUT / 2
    
    def handle_server(self):
        """sanitizes commands sent by the client, also handles checking for lost connections via timeout"""
        while True:
            #receive and handle incoming message
            request, arguments = self.receive()
            if request is None or request == ServerHandler.KEEP_ALIVE_REPLY:
                pass
            else:
                self.outgoing_queue.put(request, arguments)
            #check if the client has timed out
            if self.client_timed_out():
                self.disconnect()
                return
            #handle outgoing messages
            try:
                pass
            except queue.Empty:
                pass
            if self.is_time_to_send_keep_alive():
                self.send(ServerHandler.KEEP_ALIVE_REPLY, None)
    
    def disconnect(self):
        self.connection.close()
        print("server timed out")
        sys.exit()


if __name__ == "__main__":
    server = ServerHandler(ADDR)
    while True:
        print(server.outgoing_queue.get())

