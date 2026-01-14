# Matt_daemon 🛡️

A system daemon written in C++ that implements the complete lifecycle of a Unix background process. Developed as part of the 42 School curriculum, this project focuses on low-level system programming, socket communication, and robust signal handling.

The daemon listens for incoming network requests on a specific port, logs all activity with a custom logging, and implements advanced features like log rotation and runtime configuration reloading.

## ✨ Features

* **Custom Daemonization**: Implements the "double-fork" mechanism from scratch to ensure the process is correctly detached from the controlling terminal and inherited by `init` (PID 1).
* **Singleton Enforcement**: Uses a lock file (`/var/lock/matt_daemon.lock`) and PID file to ensure only one instance of the daemon runs at any time.
* **Logger**: A reusable logging class that generates timestamped logs in `/var/log/matt_daemon/matt_daemon.log`.
* **Advanced Log Management**:
   * **Rotation**: Automatically rotates logs based on configurable time intervals or file size.
   * **Retention**: Manages the total number of archived log files to save disk space.
* **Network Server**: Handles client connections using non-blocking sockets.
* **Signal Interception**: Gracefully handles termination signals (like `SIGTERM` or `SIGINT`), ensuring the lock file is cleaned up and the state is saved before exiting.
* **Hot Reloading**: Supports `.ini` configuration files that can be reloaded at runtime to update some paramaters without restarting the service.

## 🏗️ Architecture

To transform a standard process into a "True Daemon," the following steps are performed:

1. **First Fork**: The parent exits, allowing the shell to think the command is done.
2. **setsid()**: The child becomes the session leader of a new session and has no controlling terminal.
3. **Second Fork**: Ensures the daemon cannot inadvertently acquire a terminal again.
4. **Umask & Chdir**: Resets file permissions and moves to the root directory to avoid blocking filesystem unmounting.
5. **Standard Stream Redirection**: Redirects `stdin`, `stdout`, and `stderr` to `/dev/null`.

## ⚙️ Configuration

The daemon is highly configurable via an initialization file. Some fields (like rotation intervals) can be reloaded dynamically.

```ini
[daemon]
; Lock file path to prevent multiple instances (requires daemon restart) [default: /var/lock/matt_daemon.lock]
lock_file=/var/lock/matt_daemon.lock

; PID file path for process management (requires daemon restart) [default: /var/run/matt_daemon.pid]
pid_file=/var/run/matt_daemon.pid

[server]
; TCP port for the daemon server [1024-65535] (requires daemon restart) [default: 4242]
port=4242

; Maximum number of concurrent connections to the daemon server [1-65535] (requires daemon restart) [default: 4096]
max_connections=3

[logger]
; Log file path [default: /var/log/matt_daemon/matt_daemon.log]
log_file=/var/log/matt_daemon/matt_daemon.log

; Logging level: DEBUG (least verbose), INFO, WARNING, ERROR (most verbose) [default: ERROR]
log_level=ERROR

; Enable automatic log rotation [default: true]
auto_rotate=true

; Log rotation interval in hours [1-8760] [default: 24]
rotate_interval=24

; Maximum log file size in kilobytes (KB) [1-1048576] [default: 1024]
rotate_size=1024

; Number of rotated log files to retain [1-100] [default: 5]
rotate_count=5
```

## 🖥️ Usage

### Installation

The program must be run with root privileges.

```bash
make
sudo ./MattDaemon
```

### Interacting with the Daemon

You can send messages to the daemon using `netcat` or any socket client.

```bash
# Connect to the daemon
nc localhost 4242

# Send messages (will be logged)
> Hello System!
> Status report.

# Shutdown the daemon
> quit
```

### Monitoring Logs

```bash
tail -f /var/log/matt_daemon/matt_daemon.log
```

### Example Log Output:

```
[11/01/2026-14:34:58] [ INFO ] - Matt_daemon: Started.
[11/01/2026-14:34:58] [ INFO ] - Matt_daemon: Entering Daemon mode.
[11/01/2026-14:34:58] [ INFO ] - Matt_daemon: started. PID: 6498.
[11/01/2026-14:36:43] [ LOG ]  - Matt_daemon: User input: Hello System!
```