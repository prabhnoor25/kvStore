# KVStore — Redis-like In-Memory Key Value Store (C++)

A simple Redis-like key-value store implemented in C++.

This project supports TTL expiration, list operations, persistence, and atomic counters.  
It was built as a systems programming exercise to demonstrate data structure design, command parsing, and state management.

---

# Features

## Core Commands

| Command | Description |
|------|-------------|
| SET key value [EX seconds] | Set a key with optional expiration |
| GET key | Retrieve a value |
| DEL key | Delete a key |
| TTL key | Get remaining TTL |
| KEYS * | List keys |
| STATS | Show database stats |

Example:

```

SET user Alice
GET user
DEL user

```

---

# Expiration (TTL)

Keys can expire automatically.

Example:

```

SET session abc EX 10
TTL session

```

Output:

```

9

```

Expired keys are cleaned using **lazy expiration** before every command execution.

---

# Atomic Counter Operations

### INCR

```

SET counter 10
INCR counter

```

Output:

```

11

```

### DECR

```

DECR counter

```

Output:

```

10

```

These operations behave like Redis atomic counters.

---

# List Operations

List operations are implemented using `std::list`.

### LPUSH

```

LPUSH mylist a
LPUSH mylist b

```

List becomes:

```

b a

```

### RPUSH

```

RPUSH mylist c

```

List becomes:

```

b a c

```

### LPOP

```

LPOP mylist

```

Output:

```

b

```

### RPOP

```

RPOP mylist

```

Output:

```

c

```

---

# Persistence

The store can be saved to disk and restored later.

### SAVE

```

SAVE dump.txt

```

Writes database snapshot to file.

### LOAD

```

LOAD dump.txt

```

Restores saved keys.

Example snapshot file:

```

user string Alice
counter string 10

```

---

# Memory Management

Basic eviction logic is implemented.

If the key limit is exceeded, the store evicts entries automatically.

---

# Statistics

Command:

```

STATS

```

Output:

```

total_keys=5
expired_cleaned=2

```

This shows:

- number of keys currently stored
- number of expired keys cleaned

---

# Architecture

The store uses:

```

unordered_map<string, Entry>

```

Entry structure:

```

Entry
├── type (string or list)
├── value (string)
├── list_value (std::list<string>)
└── expiry timestamp

```

Expiration timestamps are stored as **epoch seconds**.

---

# Command Processing

The program reads commands sequentially from **stdin**.

Example:

```

SET user Alice
GET user
INCR counter

```

Commands are parsed using:

```

std::stringstream

```

---

# Build Instructions

This project compiles using **GCC 6 (MinGW)**.

Compile:

```

g++ -std=c++11 src/kvstore.cpp -o kvstore.exe

```

Run:

```

kvstore.exe

```

---

# Example Session

```

SET user Alice
OK

GET user
Alice

SET counter 0
OK

INCR counter
1

LPUSH list a
1

LPUSH list b
2

LPOP list
b

```

---

# Project Structure

```

kvstore-project
│
├── src
│   └── kvstore.cpp
│
├── tests
│
├── scripts
│
├── README.md
└── .gitignore

```

---

# Design Decisions

### Why unordered_map?

Provides **O(1) average lookup time** for key-value access.

### Why lazy expiration?

Simplifies design without needing background threads.

Expired keys are cleaned before command execution.

### Why std::list for lists?

Efficient push/pop operations on both ends.

---

# Complexity

| Operation | Complexity |
|------|------|
SET | O(1)
GET | O(1)
DEL | O(1)
INCR / DECR | O(1)
LPUSH / RPUSH | O(1)
LPOP / RPOP | O(1)
TTL | O(1)

---

# Known Limitations

Current version:

• Single-threaded  
• Uses simple file persistence  
• No network protocol yet  

---

# Future Improvements

Planned enhancements:

• TCP server for multiple clients  
• Pub/Sub messaging system  
• True LRU eviction  
• Pattern matching for KEYS  
• Snapshot compression  

---
