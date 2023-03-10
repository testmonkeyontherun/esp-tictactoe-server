import threading
import queue
import socket
import json
import common
import secrets
import string

class Game:
    def __init__(self, playerid, gameid, public, games, players):
        self.players = players
        self.games = games
        self.gameid = gameid
        self.player1 = self.players[playerid]
        self.games[self.gameid] = self
        self.player2 = None
        self.public = public
    def join(self, playerid):
        if self.player2 is not None:
            return "game is full"
        if playerid == self.player1.playerid:
            return "no self play allowed"
        else:
            self.player2 = self.players[playerid]
            return "true"
    def destroy(self):
        self.player1.deregister_game(self.gameid)
        if self.player2 is not None:
            self.player2.deregister_game(self.gameid)
        del self.games[self.gameid]
        
        
class Player:
    def __init__(self, playerid, games, players):
        self.playerid = playerid
        self.players = players
        self.players[self.playerid] = self
        self.owngames = {}
        self.games = games
    def destroy(self):
        keys = list(self.owngames.keys())
        for game in keys:
            self.owngames[game].destroy()
        del self.players[self.playerid]
    def register_game(self, gameid):
        self.owngames[gameid] = self.games[gameid]
    def deregister_game(self, gameid): # is only called by a game
        del self.owngames[gameid]

known_messages = ()
message_queue = queue.Queue()

players = dict()
games = dict()

def handle_client(conn, addr):
    while True:
        try:
            msg = common.recieve(conn, True, True)
            try:
                command = json.loads(msg)
            except json.decoder.JSONDecodeError:
                conn.close()
                break
            message_queue.put_nowait((command, conn.dup(), addr))
        except IOError as e:
            break

def server_frontend():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind(common.ADDR)
    server.listen()
    print("running")
    while True:
        conn, addr = server.accept()
        message_worker = threading.Thread(target=handle_client, args=(conn, addr))
        message_worker.start()
        print(threading.active_count() - 1)

def generate_id(N):
    return ''.join(secrets.choice(string.ascii_uppercase + string.digits) for _ in range(N))
def game_master():
    while True:
        msg, conn, addr = message_queue.get()
        #####gamelogic####
        try:
            if msg["command"] == "register player":
                playerid = generate_id(common.PLAYER_ID_LENGTH)
                while playerid in players:
                    playerid = generate_id(common.PLAYER_ID_LENGTH)
                Player(playerid, games, players)
                reply = playerid
            elif msg["command"] == "deregister player":
                playerid = msg["playerid"]
                if playerid not in players:
                    reply = "false"
                else:
                    players[playerid].destroy()
                    reply = "true"
                    print(games)#debug
                    print(players)#debug
            elif msg["command"] == "create game":
                playerid = msg["playerid"]
                public = msg["public"]
                if public not in common.boolstring:
                    reply = "invalid option for public"
                elif playerid not in players:
                    reply = "unknown playerid"
                else:
                    gameid = generate_id(common.PLAYER_ID_LENGTH)
                    while gameid in games:
                        gameid = generate_id(common.PLAYER_ID_LENGTH)
                    Game(playerid, gameid, public, games, players)
                    players[playerid].register_game(gameid)
                    reply = gameid                             
            ##################
            else:
                reply = "unknown command"
        except (KeyError, TypeError):
            reply = "unknown command"
        common.send("{\"reply\":\"" + reply + "\"}", conn, False, False)

if __name__ == "__main__":
    game_master_thread = threading.Thread(target=game_master)
    game_master_thread.start()
    server_frontend_thread = threading.Thread(target=server_frontend)
    server_frontend_thread.start()
    server_frontend_thread.join()