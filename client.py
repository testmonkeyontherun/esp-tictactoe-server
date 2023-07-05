import socket
import config
import json
import time
import queue
import ctypes
import threading
import sys


class ServerHandler:
    #connects to a server, then handles all messages to and from it
    KEEP_ALIVE_TIMEOUT = config.KEEP_ALIVE_TIMEOUT

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
        self.receive_buffer = []
        self.incoming_queue = queue.Queue()
        self.outgoing_queue = queue.Queue()
        self.connection = client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client.connect(ADDR)
        self.last_incoming_message_time = time.time()
        self.last_outgoing_message_time = time.time()
        self.thread = threading.Thread(target=self.handle_server, daemon=True)
        self.thread.start()
        self.connection.setblocking(False)
    
    def receive(self):
        #receive and parse a message sent by the server if no message is available, return (None, None)
        try:
            self.receive_buffer += self.connection.recv(4 - len(self.receive_buffer))
            if len(self.receive_buffer) != 4:
                return None, None
            message_length = int.from_bytes(self.receive_buffer, "little")
            self.receive_buffer = []
            if message_length == 0:
                return None, None
            if message_length > config.MAX_MSG_LENGTH:
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
        #send message to the server
        message = self.encode_message(message, arguments)
        self.connection.send(message)
        self.last_outgoing_message_time = time.time()
    
    def decode_message(self, message):
        #parse and validate message the contents are returned as a dict
        message_string = message.decode(config.FORMAT)
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
        #takes a message and encodes according to docs
        message = {"request": message}
        if arguments is not None:
            message = message | arguments
        message_string = json.dumps(message)
        message_bytes = message_string.encode(config.FORMAT)
        length_bytes = bytes(ctypes.c_uint32(len(message_bytes)))
        return length_bytes + message_bytes

    def client_timed_out(self):
        #check if the client has timed out
        return time.time() - self.last_incoming_message_time >= ServerHandler.KEEP_ALIVE_TIMEOUT
    
    def is_time_to_send_keep_alive(self):
        #check if it is time to send KEEP_ALIVE_REQUEST
        return time.time() - self.last_outgoing_message_time >= ServerHandler.KEEP_ALIVE_TIMEOUT / 2
    
    def handle_server(self):
        """sanitizes commands sent by the client, also handles checking for lost connections via timeout"""
        while True:
            #receive and handle incoming message
            request, arguments = self.receive()
            if request is None or request == ServerHandler.KEEP_ALIVE_REPLY:
                pass
            elif request == ServerHandler.GAME_ENDED_REPLY:
                self.outgoing_queue.put((request, arguments))
                return
            else:
                self.outgoing_queue.put((request, arguments))
            #check if the client has timed out
            if self.client_timed_out():
                self.disconnect()
                return
            #handle outgoing messages
            try:
                message = self.incoming_queue.get_nowait()
                if message == "disconnect":
                    self.send(ServerHandler.DISCONNECT_REQUEST, None)
                    print("Disconnected")
                    self.outgoing_queue.put((ServerHandler.GAME_ENDED_REPLY, None))
                    self.connection.close()
                    return
                else:
                    self.send(ServerHandler.MOVE_REQUEST, {"move": message})
            except queue.Empty:
                pass
            if self.is_time_to_send_keep_alive():
                self.send(ServerHandler.KEEP_ALIVE_REPLY, None)
    
    def disconnect(self):
        #forfeit the game
        self.connection.close()
        print("server timed out")
        sys.exit()


user_input = queue.Queue()
def handle_user():
    #the main interaction point of the user, does not provide feedback
    while True:
        user_input.put(input())

def draw_state(state):
    #responsible for drawing the grid, and important messages for the user
    client_id = state["client_id"]
    if state["players"][0] == client_id:
        opponent_id = state["players"][1]
    else:
        opponent_id = state["players"][0]
    symbols = {client_id: "x", opponent_id: "o", None: " "}
    board = [symbols[cell] for cell in state["board"]]
    print("{}|{}|{}\n-----\n{}|{}|{}\n-----\n{}|{}|{}\n".format(*board))

    if state["status"]["current_state"] == "playing":
        to_move = state["to_move"] == client_id
        if to_move:
            print("your_move: ")
        else:
            print("waiting for opponent!")
    elif state["status"]["current_state"] == "draw":
        print("draw")
    else:
        if state["status"]["winner"] == client_id:
            print("you won for reason: " + state["status"]["reason"])
        else:
            print("you lost for reason: " + state["status"]["reason"])



if __name__ == "__main__":
    server = ServerHandler((config.SERVER, config.PORT))
    user_handler = threading.Thread(target=handle_user, daemon=True)
    user_handler.start()
    print("waiting for game!")
    game = None
    
    while True:
        try:
            #handle the servers messages
            message, arguments = server.outgoing_queue.get_nowait()
            if message == ServerHandler.INFO_REPLY:
                game = arguments
                draw_state(game)
            elif message == ServerHandler.GAME_CREATED_REPLY:
                pass
            elif message == ServerHandler.ILLEGAL_MOVE_REPLY:
                print("illegal move!")
                draw_state(game)
            elif message == ServerHandler.MOVE_ACCEPTED_REPLY:
                pass
            elif message == ServerHandler.GAME_ENDED_REPLY:
                sys.exit()
            else:
                print(message)

        except queue.Empty:
            pass

        try:
            #validate the messages sent by user, then pass them along
            message = user_input.get_nowait()
            try:
                message = int(message)
            except ValueError:
                message = "disconnect"
            server.incoming_queue.put(message)
        except queue.Empty:
            pass
