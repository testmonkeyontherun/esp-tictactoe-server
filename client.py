import socket
import common
import json
PORT = 12345
SERVER = "localhost"
ADDR = (SERVER, PORT)
MSG_LENGTH = 100
FORMAT = "utf-8"

def pad(msg, target_length):
    if len(msg) > target_length:
        return msg[:target_length]
    else:
        return msg + b"\0" * (target_length - len(msg))



if __name__ == "__main__":
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect(ADDR)
    common.send(json.dumps({"command" : "register player"}), client, False, False)
    playerid = json.loads(common.recieve(client, False, False))["reply"]
    print("playerid:", playerid)
    common.send(json.dumps({"command" : "create game", "playerid" : playerid, "public": "true"}), client, False, False)
    gameid = json.loads(common.recieve(client, False, False))["reply"]
    print("gameid:", gameid)
    common.send(json.dumps({"command" : "deregister player", "playerid" : playerid}), client, False, False)
    reply = json.loads(common.recieve(client, False, False))["reply"]
    print(reply)

