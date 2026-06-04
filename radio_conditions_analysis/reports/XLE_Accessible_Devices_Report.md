# XLE Accessible Devices Report

**Date:** June 3, 2026  
**Goal:** Identify 25 accessible XLE devices per RadioEnvConditions category for further debugging.

## Method

1. `jump ntid@<jump host ip>`
2. Select `1` (Reverse/Forward SSH) → `1` (COMCAST_DEVICE) → Enter XB MAC
3. On XB: `arp -n | grep 169.254` to find XLE link-local IP
4. `GetConfigFile /tmp/.dropbearXLE`
5. `ssh -y -i /tmp/.dropbearXLE root@<XLE_IP>`

**Skipped:** TG4482PC2 (CommScope XB7) — dropbear key auth fails on this platform.

## Summary

| Condition | Tested | Accessible | Skipped (TG4482) | No XLE in ARP | Password Prompt | XB Fail | Other |
|---|---|---|---|---|---|---|---|
| FAIR | 75 | 25 | 6 | 28 | 4 | 5 | 7 |
| POOR | 67 | 25 | 4 | 26 | 4 | 2 | 6 |
| EXCELLENT | 57 | 25 | 5 | 17 | 6 | 1 | 3 |
| UNAVAILABLE | 3 | 1 | 0 | 1 | 0 | 0 | 1 |
| GOOD | 90 | 25 | 7 | 39 | 7 | 3 | 9 |


## EXCELLENT — Accessible XLEs

| # | XLE MAC | XB MAC | XLE IP |
|---|---|---|---|
| 1 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 2 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 3 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 4 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 5 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 6 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 7 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 8 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 9 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 10 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 11 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 12 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 13 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 14 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 15 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 16 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 17 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 18 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 19 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 20 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 21 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 22 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 23 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 24 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 25 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |

## GOOD — Accessible XLEs

| # | XLE MAC | XB MAC | XLE IP |
|---|---|---|---|
| 1 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 2 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 3 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 4 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 5 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 6 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 7 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 8 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 9 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 10 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 11 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 12 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 13 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 14 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 15 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 16 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 17 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 18 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 19 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 20 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 21 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 22 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 23 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 24 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 25 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |

## FAIR — Accessible XLEs

| # | XLE MAC | XB MAC | XLE IP |
|---|---|---|---|
| 1 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 2 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 3 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 4 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 5 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 6 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 7 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 8 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 9 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 10 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 11 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 12 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 13 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 14 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 15 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 16 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 17 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 18 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 19 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 20 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 21 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 22 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 23 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 24 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 25 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |

## POOR — Accessible XLEs

| # | XLE MAC | XB MAC | XLE IP |
|---|---|---|---|
| 1 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 2 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 3 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 4 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 5 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 6 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 7 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 8 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 9 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 10 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 11 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 12 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 13 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 14 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 15 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 16 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 17 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 18 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 19 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 20 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 21 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 22 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 23 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 24 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |
| 25 | XX:XX:XX:XX:XX:XX | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX |

## Status Legend

| Status | Meaning |
|---|---|
| XLE_ACCESSIBLE | Full chain succeeded — got root shell on XLE |
| XLE_PASSWORD_PROMPT | SSH key rejected by XLE, fell back to password |
| NO_XLE_IN_ARP | XB reachable but no XLE in its ARP table |
| SKIPPED_TG4482PC2 | XB is CommScope XB7 — known to fail key auth |
| XB_INACCESSIBLE | Could not reach XB via jump |
| XLE_CONN_REFUSED | XLE link-local present but SSH refused |
| XLE_SSH_TIMEOUT | SSH to XLE timed out |
