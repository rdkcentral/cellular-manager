---
applyTo: "**/*.sh"
---

# Shell Script Standards for Cellular Manager

## Platform Independence

### Use POSIX Shell
- Use `#!/bin/sh` not `#!/bin/bash`
- Avoid bashisms (use shellcheck to verify)
- Test on busybox ash (common in embedded devices)

```bash
#!/bin/sh
# GOOD: POSIX compliant

# BAD: Bash-specific
if [[ $var == "value" ]]; then  # Use [ ] instead
    array=(1 2 3)  # Arrays not in POSIX
fi

# GOOD: POSIX compliant
if [ "$var" = "value" ]; then
    set -- 1 2 3  # Use positional parameters
fi
```

## Resource Awareness

### Minimize Process Spawning
- Use shell builtins when possible
- Avoid pipes when not necessary
- Batch operations to reduce forks

```bash
# BAD: Multiple processes
cat /tmp/cellular_log | grep "REGISTRATION" | wc -l

# GOOD: Fewer processes
grep -c "REGISTRATION" /tmp/cellular_log
```

### Memory Usage
- Avoid reading entire files into variables
- Process streams line by line
- Clean up temporary files

```bash
# BAD: Loads entire file into memory
content=$(cat /tmp/modem_trace.log)
echo "$content" | grep ERROR

# GOOD: Stream processing
grep ERROR /tmp/modem_trace.log
```

## Error Handling

### Always Check Exit Codes
```bash
# GOOD: Check critical operations
if ! mkdir -p /tmp/cellularmgr; then
    logger -t cellularmgr "ERROR: Failed to create directory"
    exit 1
fi

# GOOD: Use set -e for fail-fast
set -e  # Exit on any error
set -u  # Exit on undefined variable

# GOOD: Trap for cleanup
cleanup() {
    rm -f "$TEMP_FILE"
}
trap cleanup EXIT INT TERM

TEMP_FILE=$(mktemp)
```

## Script Quality

### Defensive Programming
```bash
# GOOD: Quote all variables
rm -f "$file_path"  # Not: rm -f $file_path

# GOOD: Use -- to separate options from arguments
grep -r -- "$pattern" "$directory"

# GOOD: Validate inputs
if [ -z "$1" ]; then
    echo "Usage: $0 <modem_device>" >&2
    exit 1
fi
```

### Logging
```bash
log_info() {
    logger -t cellularmgr -p user.info "$*"
}

log_error() {
    logger -t cellularmgr -p user.error "$*"
    echo "ERROR: $*" >&2
}

# Usage
log_info "Starting modem diagnostics"
if ! check_modem_state; then
    log_error "Modem not responding"
    exit 1
fi
```

## Cellular-Specific Guidelines

### Modem State Scripts
- Scripts that trigger modem state transitions must log start/end status
- Always check modem availability before sending commands
- Parse AT responses defensively (check for OK/ERROR)

```bash
# GOOD: Check modem before sending AT command
if [ ! -c "$MODEM_DEV" ]; then
    log_error "Modem device $MODEM_DEV not found"
    exit 1
fi

# GOOD: Parse AT response safely
response=$(echo "AT+CEREG?" | socat - "$MODEM_DEV",crnl,b115200)
if echo "$response" | grep -q "^+CEREG:"; then
    reg_status=$(echo "$response" | sed -n 's/+CEREG: [0-9],\([0-9]\).*/\1/p')
    log_info "Registration status: $reg_status"
else
    log_error "Unexpected AT response: $response"
fi
```

### Log Parsing
- Parse logs with stable patterns for SIM, registration, PDP, and crash loops
- Avoid assumptions about log format changes across firmware versions

## Testing Scripts

### Use shellcheck
```bash
# Run shellcheck on all scripts
find . -name "*.sh" -exec shellcheck {} +
```

## Anti-Patterns

```bash
# BAD: Unquoted variables
for file in $FILES; do  # Word splitting!

# GOOD: Quoted
for file in "$FILES"; do

# GOOD: Iterate over positional parameters safely
set -- *.log
for file in "$@"; do

# GOOD: Use glob
for file in *.log; do

# BAD: Not checking if file exists
rm /tmp/modem_dump  # Error if doesn't exist

# GOOD: Check or use -f
rm -f /tmp/modem_dump
```
