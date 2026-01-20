import os, socket, time

HOST = os.getenv("SERVER_HOST", "server_py")
PORT = int(os.getenv("SERVER_PORT", "5001"))

while True:
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((HOST, PORT))
        s.sendall(b"GET")
        data = s.recv(1024).decode(errors="ignore").strip()
        print("[PY CLIENT]", data)
        s.close()
    except Exception as e:
        print("[PY CLIENT] error:", e)

    time.sleep(10)
