############## config #############
PORT = 12345
SERVER = "localhost" #socket.gethostbyname(socket.gethostname())
ADDR = (SERVER, PORT)
MSG_LENGTH = 100
MAX_MSG_LENGTH = 10000
FORMAT = "utf-8"
TIMEOUT = 200
PLAYER_ID_LENGTH = 100
##################################
boolstring = ("true", "false")

def pad(msg, target_length):
    if len(msg) > target_length:
        return msg[:target_length]
    else:
        return msg + b"\0" * (target_length - len(msg))

def send(msg, conn, is_blocking, shall_block):
    if shall_block:
        conn.setblocking(True)
    else:
        conn.settimeout(TIMEOUT)
    try:
        message = str(len(msg)).encode(FORMAT)
        message = pad(message, MSG_LENGTH)
        conn.send(message)
        conn.send(msg.encode(FORMAT))
    except TimeoutError:
        conn.close()
        raise IOError
    if is_blocking:
        conn.setblocking(True)

def recieve(conn, is_blocking, shall_block):
    if shall_block:
        conn.setblocking(True)
    else:
        conn.settimeout(TIMEOUT)
    msg = conn.recv(MSG_LENGTH).decode(FORMAT).strip("\0")
    try:
        message_length = int(msg)
        if message_length > MAX_MSG_LENGTH:
            raise ValueError
    except ValueError:
        conn.close()
        raise IOError
    conn.settimeout(TIMEOUT)
    try:
        msg = conn.recv(message_length).decode(FORMAT).strip("\0")
    except TimeoutError:
        conn.close()
        raise IOError
    if is_blocking:
        conn.setblocking(True)
    return msg