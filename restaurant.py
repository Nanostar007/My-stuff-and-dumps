import sqlite3
from datetime import datetime, timedelta

DB = "restaurant.db"
RES_DURATION_MIN = 90

def db():
    return sqlite3.connect(DB)

def setup():
    with db() as con:
        con.execute("""
        CREATE TABLE IF NOT EXISTS tables(
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            seats INTEGER NOT NULL
        )""")
        con.execute("""
        CREATE TABLE IF NOT EXISTS reservations(
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            guests INTEGER NOT NULL,
            table_id INTEGER NOT NULL,
            start DATETIME NOT NULL,
            end DATETIME NOT NULL
        )""")

def add_table():
    seats = int(input("Seats: "))
    with db() as con:
        con.execute("INSERT INTO tables(seats) VALUES(?)", (seats,))
    print("Table added.")

def available(table_id, start, end):
    with db() as con:
        cur = con.execute("""
        SELECT 1 FROM reservations
        WHERE table_id = ?
        AND NOT (end <= ? OR start >= ?)
        """, (table_id, start, end))
        return cur.fetchone() is None

def find_table(guests, start, end):
    with db() as con:
        cur = con.execute(
            "SELECT id FROM tables WHERE seats >= ? ORDER BY seats", (guests,)
        )
        for (table_id,) in cur.fetchall():
            if available(table_id, start, end):
                return table_id
    return None

def reserve():
    name = input("Name: ")
    guests = int(input("Guests: "))
    start = datetime.fromisoformat(input("Start (YYYY-MM-DD HH:MM): "))
    end = start + timedelta(minutes=RES_DURATION_MIN)

    table_id = find_table(guests, start, end)
    if not table_id:
        print("No table available.")
        return

    with db() as con:
        con.execute("""
        INSERT INTO reservations(name, guests, table_id, start, end)
        VALUES(?,?,?,?,?)
        """, (name, guests, table_id, start, end))

    print(f"Reserved table {table_id} for {name}.")

def list_reservations():
    with db() as con:
        rows = con.execute("""
        SELECT id, name, guests, table_id, start, end
        FROM reservations
        ORDER BY start
        """).fetchall()

    if not rows:
        print("No reservations.")
        return

    for r in rows:
        print(f"[{r[0]}] {r[1]} | {r[2]} guests | Table {r[3]} | {r[4]} → {r[5]}")

def delete_reservation():
    rid = input("Reservation ID: ")
    with db() as con:
        con.execute("DELETE FROM reservations WHERE id = ?", (rid,))
    print("Deleted.")

def menu():
    print("\n--- Lil caesars ---")
    print("1) Add table")
    print("2) New reservation")
    print("3) List reservations")
    print("4) Delete reservation")
    print("0) Exit")

def main():
    setup()
    while True:
        menu()
        c = input("> ")
        if c == "1":
            add_table()
        elif c == "2":
            reserve()
        elif c == "3":
            list_reservations()
        elif c == "4":
            delete_reservation()
        elif c == "0":
            break

if __name__ == "__main__":
    main()

