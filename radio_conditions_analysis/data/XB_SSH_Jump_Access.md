# SSH Access to XB / XLE Devices

Use the jump server below to access XB devices over SSH:

```text
jump ssival030@96.118.217.151
```

After logging in:

1. Enter `1` when prompted (Reverse/Forward SSH).
2. Enter `1` again when prompted (COMCAST_DEVICE).
3. Enter the XB CM MAC address when prompted to reach the XB device.

## Accessing XLE from an XB Device

XLE devices cannot be reached directly from the jump server. Instead, SSH into the associated XB first (using its MAC above), then run:

```bash
arp -n | grep "169.254"
GetConfigFile /tmp/.dropbearXLE
ssh -y -i /tmp/.dropbearXLE root@<XLE_LINK_LOCAL_IP>
```

### Notes

- The XLE link-local IP is **not** always `169.254.1.25` — parse the actual IP from the `arp -n` output (pick the entry with a real MAC, ignore `<incomplete>` entries).
- Use `ssh -y` to auto-accept the host key (dropbear's equivalent of `StrictHostKeyChecking=no`).
- **Skip TG4482PC2 (CommScope XB7)** devices — dropbear key auth to XLE fails on this platform. Works on **CGM4981COM (Technicolor XB8)** and similar.