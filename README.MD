# 42 Wolfsburg — ft_irc (IRC Server)

Internet Relay Chat (IRC) is a text-based communication protocol that enables real-time messaging in public channels or private messages.  
This project is a **42 School `ft_irc`** implementation: an IRC server written in **C++98**.

---

## Table of Contents

- [Features](#features)
- [Requirements](#requirements)
- [Build](#build)
- [Run](#run)
- [Connect to the Server](#connect-to-the-server)
  - [Using netcat (`nc`)](#using-netcat-nc)
  - [Using HexChat](#using-hexchat)
- [Supported Commands](#supported-commands)
- [Docker](#docker)
- [Testing](#testing)
- [Troubleshooting](#troubleshooting)
- [Project Structure](#project-structure)
- [Collaborators](#collaborators)

---

## Features

- IRC server executable: **`ircserv`**
- Connect via terminal (`nc`) or IRC clients (e.g., **HexChat**)
- Basic IRC workflow: authenticate, set nick/user, join channels, chat, manage channels, etc.


---

## Requirements

- **C++ compiler** supporting C++98 (project Makefile uses `c++` / `g++`)
- **make**
- (Optional) `nc` (netcat) for terminal testing
- (Optional) **HexChat** for GUI client testing
- (Optional) Docker / Docker Compose (if you want to use containers)

---

## Build

```bash
make
```

This generates the executable:

- `./ircserv`

Useful clean targets:

```bash
make clean
make fclean
make re
```

---

## Run

### Run with explicit arguments

```bash
./ircserv <port> <password>
```

Example:

```bash
./ircserv 6667 42
```

### Run using Makefile helper

The Makefile provides a `server` target that runs the server on `6667` with password `42` by default:

```bash
make server
```

---

## Connect to the Server

### Using netcat (`nc`)

1. Start the server (see [Run](#run))
2. In another terminal:

```bash
nc -C localhost 6667
PASS <password>
NICK <nickname>
USER <username> 0 * <realname>
```

After that you can `JOIN #channel` and use `PRIVMSG`.

### Using HexChat

1. Install HexChat: https://hexchat.github.io/downloads.html
2. Create a new network (e.g. `ft_irc`)
3. Add server: `localhost/6667`
4. Configure your server password (PASS) in HexChat
5. Connect

The Makefile also includes helper targets for HexChat/testing scripts (see [Testing](#testing)).

---

## Supported Commands

| Command  | Terminal Usage                                | HexChat Usage                         | Description |
|---------:|-----------------------------------------------|----------------------------------------|------------|
| PASS     | `PASS <password>`                             | Entered on connection                  | Authenticate with server |
| NICK     | `NICK <nickname>`                             | `/nick <nickname>`                     | Set or change nickname |
| USER     | `USER <username> 0 * <realname>`              | Set on connection                      | Set username and realname |
| JOIN     | `JOIN <channel>`                              | `/join <channel>`                      | Join a channel |
| PRIVMSG  | `PRIVMSG <nickname/#channel> <message>`       | `/msg <nickname/#channel> <message>`   | Message user or channel |
| PART     | `PART #<channel>`                             | `/part #<channel>`                     | Leave a channel |
| QUIT     | `QUIT [message]`                              | `/quit [message]`                      | Disconnect |
| KICK     | `KICK #<channel> <nickname> [message]`        | `/kick #<channel> <nickname> [message]`| Remove a user from a channel |
| MODE     | `MODE #<channel> <mode> [parameters]`         | `/mode #<channel> <mode> [parameters]` | Change channel/user modes |
| TOPIC    | `TOPIC #<channel> [new topic]`                | `/topic #<channel> [new topic]`        | Get/set channel topic |
| WHO      | `WHO <nickname>`                              | `/who <nickname>`                      | Query users |
| INVITE   | `INVITE <nickname> <channel>`                 | `/invite <nickname> <channel>`         | Invite a user |
| PING     | `PING <message>`                              | `/ping <message>`                      | Connection test |
| PONG     | `PONG <ping_message>`                         | `/pong <ping_message>`                 | Reply to server PING |

---

## Docker

This repository contains a `Dockerfile` and `docker-compose.yml`.

Typical usage (adjust to your compose/service names if needed):

```bash
docker compose up --build
```

---

## Testing

Your Makefile includes helpful targets:

- Run server:  
  ```bash
  make server
  ```

- Run a netcat client:  
  ```bash
  make client
  ```

- Run test scripts:
  ```bash
  make test
  ```

There are additional debugging/memory targets (valgrind/helgrind/leaks) in the Makefile, but they may depend on platform/tool availability.

---

## Troubleshooting

- **“Command not found” / not executable**  
  Make sure you built the project (`make`) and run `./ircserv ...`.

- **Port already in use (e.g. 6667)**  
  The Makefile includes:
  ```bash
  make freePort
  ```

- **Can’t connect from client**  
  Confirm the server is listening on the same port you connect to, and that you are sending `PASS`, `NICK`, `USER` in the expected order.

---

## Project Structure

Common directories in this repository:

- `src/` — implementation files
- `include/` — headers
- `testScripts/` — local testing utilities
- `docs/` — documentation
- `.settings/` — local dev tooling (hooks, colors, etc.)

---

## Collaborators

| Name | GitHub |
|:--|:--|
| Vitoria Lizarraga (42: `@lilizarr`) | `@VictoriaLizCor` |
| Manuel Guillermo Cruz (42: `@mcruz-sa`) | `@MemoCSales` |
| Marian Sacaliuc (42: `@msacaliu`) | `@Gaspar24` |
