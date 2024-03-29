"""Game agnostic server for turnbased multiplayer games.
Swapping games can be achieved by changing the Game class and maybe changing matchmaking.
Currently configured to play TicTacToe."""


import threading
import queue
import socket
import config
import time
import random
import json
import ctypes

class Player:
    #abstraction, so the game does not have to handle potentially diferent clients
    def __init__(self, id, queue):
        self.id = id
        self.queue = queue
    def send(self, message):
        self.queue.put(message)
    def get_id(self):
        return self.id
    
class Game:
    #responsible for all game logic.
    def __init__(self, players):
        self.players = players
        self.board = [None] * 9
        self.turn_number = 0
        self.status = {"winner": None, "current_state": "playing", "reason": None}
        self.change_turn()
        
        #self.status = ("playing", None, None) #status (playing, won, draw), winner, (checkmate, draw, forfeit) 
    def copy(self): #important to avoid races
        game_to_return = Game(self.players.copy())
        game_to_return.turn_number = self.turn_number
        game_to_return.to_move = self.to_move
        game_to_return.board = self.board.copy()
        game_to_return.status = self.status
        return game_to_return

    def move(self, player, move):
        #process move request
        if not self.is_running():
            return False
        #check for move out of turn
        if player != self.to_move:
            return False
        #check move validity
        if not self.move_is_valid(move):
            return False
        #update board
        self.board[move] = player
        #check for win
        self.check_win()
        #update player_to_move
        self.change_turn()
        return True
    def check_win(self):
        #check if some player won the game
        won, winner = False, None
        if self.board[0] == self.board[1] and self.board[0] == self.board[2] and self.board[0] is not None:
            won, winner = True, self.board[0]
        elif self.board[3] == self.board[4] and self.board[3] == self.board[5] and self.board[3] is not None:
            won, winner = True, self.board[3]
        elif self.board[6] == self.board[7] and self.board[6] == self.board[8] and self.board[6] is not None:
            won, winner = True, self.board[6]
        elif self.board[0] == self.board[3] and self.board[0] == self.board[6] and self.board[0] is not None:
            won, winner = True, self.board[0]
        elif self.board[1] == self.board[4] and self.board[1] == self.board[7] and self.board[1] is not None:
            won, winner = True, self.board[1]
        elif self.board[2] == self.board[5] and self.board[2] == self.board[8] and self.board[2] is not None:
            won, winner = True, self.board[2]
        elif self.board[0] == self.board[4] and self.board[0] == self.board[8] and self.board[0] is not None:
            won, winner = True, self.board[0]
        elif self.board[2] == self.board[4] and self.board[2] == self.board[6] and self.board[2] is not None:
            won, winner = True, self.board[2]
        if won:
            self.status = {"winner": winner, "current_state": "won", "reason": "Three-in-a-row"}
            return
        for square in self.board:
            if square == None:
                break
        else:
            self.status = {"winner": None, "current_state": "draw", "reason": "All-squares-filled"}
        
    def change_turn(self):
        #helper function for changing the state after each turn
        if self.is_running():
            self.to_move = self.get_player_by_turn_number(self.turn_number)
            self.turn_number += 1
    def move_is_valid(self, move):
        #bounds checking
        return move >= 0 and move <= 8 and self.board[move] is None
    
    def is_running(self):
        #as it says
        return self.status["current_state"] == "playing"
    
    def forfeit(self, player):
        #inform all players about a forfeit, then end the game
        for index, current_player in enumerate(self.players):
            if current_player == player:
                winning_index = index + 1
                break
        else:
            raise Exception("Unknown Player")
        winner = self.get_player_by_turn_number(winning_index)
        self.status = {"winner": winner, "current_state": "won", "reason": "forfeit"}

    def get_player_by_turn_number(self, turn_number):
        #lookup
        return self.players[turn_number % len(self.players)]
    def encode(self):
        #for sending to the players
        return {"board": self.board, "players": self.players, "to_move": self.to_move, "turn_number": self.turn_number, "status": self.status}

class GameManager:
    #Players make request, the game processes them in order and replys if necessary
    #handler -> game
    DISCONNECT_REQUEST = 0
    MOVE_REQUEST = 1
    INFO_REQUEST = 2
    #game -> handler
    ILLEGAL_MOVE_REPLY = 1
    MOVE_ACCEPTED_REPLY = 2
    INFO_REPLY = 3
    GAME_ENDED_REPLY = 4

    def __init__(self, players):
        self.thread = threading.Thread(target=self.init, args=(players,), daemon=True)
        self.thread.start()     
    
    def init(self, players):
        self.input_queue = queue.Queue()
        self.players = players
        self.sendall(self.input_queue)
        self.game = Game([player.get_id() for player in self.players])
        self.run()

    def sendall(self, message):
        #broadcast to all players
        for player in self.players:
            player.send(message)

    def get_player_by_id(self, id):
        #lookup
        for pos, player in enumerate(self.players):
            if id == player.get_id():
                return self.players[pos]
        else:
            raise Exception("Unknown Player")
    
    def make_move(self, player, move):
        # process move request informs all players about changes after each move
        if not self.game.move(player, move):
            self.get_player_by_id(player).send((GameManager.ILLEGAL_MOVE_REPLY, None))
            return False
        self.get_player_by_id(player).send((GameManager.MOVE_ACCEPTED_REPLY, None))
        self.sendall((GameManager.INFO_REPLY, self.game.copy()))
        if not self.game.is_running():
            self.sendall((GameManager.GAME_ENDED_REPLY, None))
            return True
        return False
    
    def run(self):
        #main loop, on each iteration processes one request
        self.sendall((GameManager.INFO_REPLY, self.game.copy()))
        while True:
            sender, message, arguments = self.input_queue.get()
            if message == GameManager.DISCONNECT_REQUEST: #arguments = "disconnect" or "forfeit"
                self.game.forfeit(sender)
                self.sendall((GameManager.INFO_REPLY, self.game.copy()))
                self.sendall((GameManager.GAME_ENDED_REPLY, None))
                return
            elif message == GameManager.INFO_REQUEST:
                self.get_player_by_id(sender).send((GameManager.INFO_REPLY, self.game.copy()))
            elif message == GameManager.MOVE_REQUEST:
                if self.make_move(sender, arguments):
                    return
            else:
                raise Exception("unknown request")   

class GameRequest:
    # used to request a game from the matchmaker. Can be invalidated before the matchmaker processes it.
    def __init__(self, player):
        self.player = player
        self.valid = True
        self.lock = threading.Semaphore()
        self.resolved = False
    def invalidate(self):
        if self.is_resolved():
            return False
        self.valid = False
        return True
    def is_valid(self):
        return self.valid
    def resolve(self):
        self.got_resolved = True
    def is_resolved(self):
        return self.resolved

pending_GameRequests = queue.Queue()
def match_maker():
    #finds matches for all game requests. first come first serve, no ranking / other fancy features
    current_request = None
    while True: # might add players to game until the game decides that it is full? better for compat TODO
        if pending_GameRequests.qsize() >= 2:
            current_request = pending_GameRequests.get()
            with current_request.lock:
                if not current_request.is_valid():
                    continue
                next_request = pending_GameRequests.get()
                with next_request.lock:
                    if not next_request.is_valid():
                        pending_GameRequests.put(current_request)
                        continue
                    current_request.resolve()
                    next_request.resolve()
                    #randomized player1
                    if random.random() > 0.5:
                        current_request, next_request = next_request, current_request
                    players = (current_request.player, next_request.player)
                    new_game = GameManager(players)

class ClientHandler:
    #responsible for actually talking to a client
    GAME_REQUEST_TIMEOUT = 100
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

    def __init__(self, connection, address):
        self.thread = threading.Thread(target=self.init, args=(connection, address), daemon=True)
        self.thread.start()
    def init(self, connection, address):
        self.receive_buffer = []
        self.last_outgoing_message_time = time.time() # the client does not care about internal matters
        
        self.connection = connection
        self.address = address
        self.connection.setblocking(False)
        self.incoming_queue = queue.Queue()
        self.game_exists = False
        self.player = Player(threading.get_ident(), self.incoming_queue)
        self.game_request = GameRequest(self.player)
        self.game_state = "Searching Game"
        self.internal_queue = queue.Queue()

        self.last_incoming_message_time = time.time() #don't hold your fuck ups against the client
        self.handle_client()
    def receive(self):
        #parse and validate message
        try:
            self.receive_buffer += self.connection.recv(4 - len(self.receive_buffer))
            if len(self.receive_buffer) != 4:
                return None, None
            message_length = int.from_bytes(self.receive_buffer, "little")
            self.receive_buffer = []
            if message_length == 0:
                return None, None
            if message_length > config.MAX_MSG_LENGTH:
                self.disconnect()
                raise Exception(f"big message incoming {message_length}")
            message_bytes = self.connection.recv(message_length)
            if len(message_bytes) != message_length:
                self.disconnect()
                raise Exception("too few bytes sent")
            message, arguments = self.decode_message(message_bytes)
            if message is not None:
                self.last_incoming_message_time = time.time()
            return message, arguments
        except BlockingIOError:
            return None, None
    
    def send(self, message, arguments):
        #send reply to client
        message = self.encode_message(message, arguments)
        self.connection.send(message)
        self.last_outgoing_message_time = time.time()
    
    def decode_message(self, message):
        #decode client request
        message_string = message.decode(config.FORMAT)
        message_dict = json.loads(message_string)
        arguments = None
        try:
            request = message_dict["request"]
            if request == ClientHandler.MOVE_REQUEST:
                arguments = message_dict["move"]
        except KeyError:
            request = None
        return request, arguments
    
    def encode_message(self, message, arguments):
        #encode reply
        message = {"request": message}
        if arguments is not None:
            message = message | arguments
        message_string = json.dumps(message)
        message_bytes = message_string.encode(config.FORMAT)
        length_bytes = bytes(ctypes.c_uint32(len(message_bytes)))
        return length_bytes + message_bytes

    # bunch of helper functions
    def encode_game(self):
        return {"state": self.game_state.encode() | {"client_id": self.player.get_id()}}

    def client_timed_out(self):
        return time.time() - self.last_incoming_message_time >= ClientHandler.KEEP_ALIVE_TIMEOUT
    
    def is_time_to_send_keep_alive(self):
        return time.time() - self.last_outgoing_message_time >= ClientHandler.KEEP_ALIVE_TIMEOUT / 2
    
    def matchmaking_timed_out(self):
        return time.time() - self.game_request_time >= ClientHandler.GAME_REQUEST_TIMEOUT
    
    def send_to_game(self, message, arguments):
        #create message for sending to game
        if not self.game_exists:
            self.internal_queue.put((self.player.get_id(), message, arguments))
        else:
            try:
                while True:
                    message = self.internal_queue.get_nowait()
                    self.outgoing_queue.put(message)
            except queue.Empty:
                pass
        self.outgoing_queue.put((self.player.get_id(), message, arguments))
    
    def handle_client(self):
        """sanitizes commands sent by the client, also handles checking for lost connections via timeout"""
        pending_GameRequests.put(self.game_request)
        self.game_request_time = time.time()
        while True:
            #receive and handle incoming message
            request, arguments = self.receive()
            if request is None or request == ClientHandler.KEEP_ALIVE_REQUEST:
                pass
            elif request == ClientHandler.INFO_REQUEST:
                self.send(ClientHandler.INFO_REPLY, self.encode_game())
            elif request == ClientHandler.DISCONNECT_REQUEST:
                self.disconnect()
                return
            elif request == ClientHandler.MOVE_REQUEST:
                self.send_to_game(GameManager.MOVE_REQUEST, arguments)
            #check if the client has timed out
            if self.client_timed_out():
                print("client timed out")
                self.disconnect()
                return
            #handle outgoing messages
            try:
                reply = self.incoming_queue.get_nowait()
                if not self.game_exists:
                    self.game_exists = True
                    self.outgoing_queue = reply
                    self.game_state = "Game found"
                    self.send(ClientHandler.GAME_CREATED_REPLY, None)
                    reply = self.incoming_queue.get_nowait()
                if reply[0] == GameManager.INFO_REPLY:
                    self.game_state = reply[1]
                    self.send(ClientHandler.INFO_REPLY, self.encode_game())
                elif reply[0] == GameManager.ILLEGAL_MOVE_REPLY:
                    self.send(ClientHandler.ILLEGAL_MOVE_REPLY, None)
                elif reply[0] == GameManager.MOVE_ACCEPTED_REPLY:
                    self.send(ClientHandler.MOVE_ACCEPTED_REPLY, None)
                elif reply[0] == GameManager.GAME_ENDED_REPLY:
                    self.send(ClientHandler.GAME_ENDED_REPLY, None)
                    self.connection.close()
                    return

            except queue.Empty: #no messages to handle
                if not self.game_exists and self.matchmaking_timed_out():
                    self.disconnect()
                    return
            if self.is_time_to_send_keep_alive():
                self.send(ClientHandler.KEEP_ALIVE_REPLY, None)
                pass
    
    def disconnect(self):#TODO somwhow crashes after client timeout
        print("connection closed")
        self.connection.close()
        if not self.game_exists:
            with self.game_request.lock:
                if self.game_request.invalidate():
                    return
                self.outgoing_queue = self.incoming_queue.get()
        self.send_to_game(GameManager.DISCONNECT_REQUEST, None)

def server_frontend(): #listens to new clients, then spins up the necessary worker threads
    match_maker_worker = threading.Thread(target=match_maker, daemon=True)
    match_maker_worker.start()
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind(("", config.PORT))
    server.listen()
    print("running")
    while True:
        conn, addr = server.accept()
        new_handler = ClientHandler(conn, addr)
        print(threading.active_count() - 2)

if __name__ == "__main__":
    server_frontend()
