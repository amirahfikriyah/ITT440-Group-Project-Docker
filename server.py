import os, time, threading, socket
import mysql.connector

DB_HOST = os.getenv("DB_HOST", "db")
DB_USER = os.getenv("DB_USER", "root")
DB_PASS = os.getenv("DB_PASS", "rootpass")
DB_NAME = os.getenv("DB_NAME", "itt440")
USER_ID = os.getenv("USER_ID", "python_user")
PORT = int(os.getenv("PORT", "5001"))

def db_conn():
    while True:
        try:
            return mysql.connector.connect(
                host=DB_HOST, user=DB_USER, password=DB_PASS, database=DB_NAME
            )
        except Exception as e:
            print("[PY SERVER] waiting for DB...", e, flush=True)
            time.sleep(2)

def updater_loop():
    while True:
        try:
            conn = db_conn()
            cur = conn.cursor()
            cur.execute(
                "UPDATE scores SET points=points+1, datetime_stamp=NOW() WHERE user=%s",
                (USER_ID,)
            )
            conn.commit()
            cur.close()
            conn.close()
            print(f"[PY SERVER] updated {USER_ID}", flush=True)
        except Exception as e:
            print("[PY SERVER] update error:", e, flush=True)
        time.sleep(30)

def handle_client(conn_sock):
    try:
        req = conn_sock.recv(1024).decode(errors="ignore").strip()
        if req.upper() != "GET":
            conn_sock.sendall(b"Send GET\n")
            return

        conn = db_conn()
        cur = conn.cursor()
        cur.execute("SELECT points, datetime_stamp FROM scores WHERE user=%s", (USER_ID,))
        row = cur.fetchone()
        cur.close()
        conn.close()

        if not row:
            conn_sock.sendall(b"NO_RECORD\n")
            return

        points, ts = row
        msg = f"user={USER_ID} points={points} updated={ts}\n"
        conn_sock.sendall(msg.encode())
    except Exception as e:
        print("[PY SERVER] client error:", e, flush=True)

def main():
    print("[PY SERVER] starting...", flush=True)
    t = threading.Thread(target=updater_loop, daemon=True)
    t.start()

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind(("0.0.0.0", PORT))
    s.listen(10)
    print(f"[PY SERVER] listening on {PORT}", flush=True)

    while True:
        c, addr = s.accept()
        with c:
            handle_client(c)

if __name__ == "__main__":
    main()
