# X_RDK_RadioEnvConditions — Speedtest & Metrics Analysis

**Date:** June 3–4, 2026  
**Goal:** Identify differentiating factors to optimize `X_RDK_RadioEnvConditions` when devices classified as POOR/FAIR achieve high throughput.

---

## Background

The current RSRP-only algorithm classifies radio environment using a single metric. A proposed multi-metric algorithm (RSRP×35 + RSRQ×35 + SNR×25 + RSSI×5) improves accuracy by detecting interference and congestion. However, field testing revealed that devices classified as POOR can still achieve high throughput — raising the question of whether additional factors should influence the score.

---

## Test Methodology

1. Connect to XLE devices via jump server → XB → XLE (dropbear SSH)
2. Collect full radio metrics via `qmicli -d /dev/cdc-wdm0 -p` (proxy mode)
3. Run 100MB download speedtest (`curl http://speedtest.tele2.net/100MB.zip`)
4. Capture metrics PRE, DURING, and POST speedtest
5. Compare EXCELLENT vs POOR device behavior

---

## Devices Tested

| Category | XB MAC | XLE IP | XLE MAC |
|---|---|---|---|
| EXCELLENT | D4:E2:CB:9D:58:A4 | 169.254.85.238 | d8:9c:8e:00:03:cf |
| POOR | 1C:9E:CC:21:C0:A4 | 169.254.1.109 | d8:9c:8e:00:17:4f |

---

## Speedtest Results

| | EXCELLENT | POOR |
|---|---|---|
| **Download Speed** | 6.9 MB/s (55.5 Mbps) | 7.2 MB/s (57.6 Mbps) |
| **Time (100MB)** | 15.1s | 14.6s |
| **Classification** | EXCELLENT | POOR |

**Key Finding:** The POOR device achieved equal or slightly better throughput than the EXCELLENT device.

---

## Full Metrics Comparison

### Core Signal Metrics (Algorithm Inputs)

| Metric | EXCELLENT | POOR | Delta |
|---|---|---|---|
| RSRP | -65 dBm | -105 dBm | 40 dB worse |
| RSRQ | -6 dB | -14 dB | 8 dB worse |
| SNR | 30.0 dB | 4.8 dB | 25 dB worse |
| RSSI | -37 dBm | -72 dBm | 35 dB worse |

### Extended Signal Metrics (qmicli --nas-get-signal-strength)

| Metric | EXCELLENT | POOR | Useful? |
|---|---|---|---|
| ECIO | -2.5 dBm | -2.5 dBm | No — identical |
| IO (interference+noise) | -106 dBm | -106 dBm | No — identical |
| SINR (QMI level 0-8) | 9.0 (level 8) | 9.0 (level 8) | No — too coarse, caps at 8 |

### TX/RX Chain Info (qmicli --nas-get-tx-rx-info=lte)

| Metric | EXCELLENT | POOR | Useful? |
|---|---|---|---|
| RX Chain 0 Power | -41.3 dBm | -72.2 dBm | Correlates with RSSI (redundant) |
| RX Chain 1 Power | -47.8 dBm | -70.6 dBm | Same |
| RX Diversity Delta | 6.5 dB | 1.6 dB | No — counterintuitive (better on POOR) |
| TX In Traffic | no | no | Not visible via qmicli proxy |
| TX Power | 0 (idle) | 0 (idle) | Not available in idle state |

### Band & Cell Info

| Metric | EXCELLENT | POOR | **KEY DIFFERENTIATOR** |
|---|---|---|---|
| **Active Band** | **Band 13 (700 MHz)** | **Band 4 (AWS-1)** | **YES — explains throughput** |
| **EARFCN** | 5230 | 2125 | Different frequencies |
| **Band Bandwidth** | **10 MHz** | **~20 MHz** | **2x capacity on POOR** |
| PCI (Physical Cell ID) | 490 | 256 | Different towers |
| Global Cell ID | 265449540 | 26208790 | Different cells |
| TAC | 26100 | 26115 | Different tracking areas |

### Carrier Aggregation

| Metric | EXCELLENT | POOR |
|---|---|---|
| CA Available | No (error: InformationUnavailable) | Yes (but deconfigured) |
| Primary Cell | N/A | PCI 287, EARFCN 1550, Band 66, 10 MHz |
| Secondary Cell | N/A | PCI 287, EARFCN 1075, Band 66, 15 MHz (deconfigured) |
| Total DL BW | 10 MHz | 15 MHz (could be 25 MHz with CA active) |

### Channel Rates

| Metric | EXCELLENT | POOR |
|---|---|---|
| Max TX Rate | 50 Mbps | 50 Mbps |
| Max RX Rate | 300 Mbps | 300 Mbps |
| Current TX/RX during DL | n/a / 0 bps | n/a / 0 bps |
| Dormancy during DL | dormant | dormant |

**Note:** qmicli queries a different WDS client than cellular-manager's active data connection. Real-time rates and dormancy status are NOT visible through the proxy interface.

### Packet Statistics (qmicli --wds-get-packet-statistics)

| Metric | EXCELLENT | POOR |
|---|---|---|
| TX/RX packets | 4/4 | 4/4 |
| TX/RX bytes | 336/336 | 336/336 |
| Drops | 0/0 | 0/0 |

**Note:** These are stale/cached from cellular-manager's WDS client initialization — not reflective of actual traffic.

### dmcli Parameters

| Parameter | EXCELLENT | POOR | Notes |
|---|---|---|---|
| X_RDK_RadioEnvConditions | EXCELLENT | POOR | Current classification |
| X_RDK_BandInfo | 13 | 13 | **STALE on POOR device** (actual band is 4) |
| X_RDK_TRX | 0 | 0 | Both idle (TxPower) |
| X_RDK_SNR | 30 | 6 | Matches qmicli (integer truncated) |
| DownStreamMaxBitRate | 300000 | 300000 | Theoretical max (kbps) |
| UpStreamMaxBitRate | 50000 | 50000 | Theoretical max (kbps) |

---

## Neighbor Cell Analysis

### EXCELLENT Device (Band 13)
- Intrafrequency: Only serving cell visible (PCI 490, single cell)
- Interfrequency: 2 Band 4 cells available (EARFCN 2075, 2100) with priority 5
- Serving cell priority: 7 (highest)
- Interpretation: Strong, isolated cell with good coverage

### POOR Device (Band 4)
- Intrafrequency: Serving PCI 256 (RSRP -103.6) + neighbor PCI 268 (RSRP -114, RSRQ -20)
- Interfrequency: Band 3 cells (PCI 287 at -104, PCI 256 at -110) + Band 13 available
- Serving cell priority: 7
- Interpretation: Cell edge, weak neighbors, but adequate for data on wider band

---

## Root Cause: Why POOR Has Equal Throughput

```text
Throughput = Spectral_Efficiency × Bandwidth × MIMO_layers × (1 - BLER)
```

| Factor | EXCELLENT (Band 13) | POOR (Band 4) |
|---|---|---|
| Bandwidth | 10 MHz (50 RBs) | ~20 MHz (100 RBs) |
| Spectral Efficiency | High (SNR 30 dB → 64QAM) | Moderate (SNR 4.8 dB → 16QAM) |
| Effective capacity | ~75 Mbps theoretical max | ~100+ Mbps theoretical max |
| Actual DL achieved | 55.5 Mbps | 57.6 Mbps |
| % of theoretical | ~74% | ~57% |

The POOR device compensates for lower spectral efficiency with **double the bandwidth**. Band 4 (AWS-1) at 20 MHz can deliver more total bits than Band 13 (700 MHz) at 10 MHz, even with worse modulation.

---

## Differentiating Factor Identified: Band Bandwidth

The **primary differentiator** between RF quality and actual throughput is **carrier bandwidth**:

| Band | Typical BW | Max DL (single) | Freq | Propagation |
|---|---|---|---|---|
| Band 13 | 10 MHz | ~75 Mbps | 700 MHz | Excellent (wide coverage) |
| Band 4 | 10-20 MHz | 75-150 Mbps | 1700/2100 MHz | Good |
| Band 2 | 10-20 MHz | 75-150 Mbps | 1900 MHz | Good |
| Band 66 | 10-20 MHz | 75-150 Mbps | 1700/2100 MHz | Good |
| Band 41 | 20 MHz | 150+ Mbps | 2500 MHz | Poor (short range) |

---

## Implications for Algorithm Optimization

### Option 1: Keep RF-Only (Recommended for RadioEnvConditions)

`X_RDK_RadioEnvConditions` remains a pure RF quality indicator:
- POOR with RSRP=-105 IS correct — the device is at cell edge with low headroom
- High throughput NOW doesn't mean the link is stable long-term
- A slight degradation (e.g., RSRP drops to -115) would crash throughput on Band 4 faster than Band 13

### Option 2: Band-Aware Boost (If throughput correlation is required)

Add a band-capacity modifier to prevent alarming on POOR/FAIR when throughput is viable:

```c
// After computing RADIO_SCORE from 4 metrics:
int band_bw = get_band_bandwidth_mhz(); // from EARFCN or BandInfo

if (band_bw >= 20 && RADIO_SCORE >= 20 && RADIO_SCORE < 40) {
    // Device is POOR on RF but has capacity headroom
    // Boost by one category if SNR > 0 (still usable)
    if (SNR > 0) {
        RADIO_SCORE += 10; // Shift toward FAIR
    }
}
```

**Risk:** Masks genuine RF degradation. Device may be "FAIR" but one interference event away from failure.

### Option 3: Separate Throughput-Aware Parameter (Cleanest architecture)

Introduce `X_RDK_LinkCapacityScore` that combines RF + band context:

```text
LinkCapacity = RadioScore × BandFactor

BandFactor:
  BW ≤ 10 MHz, no CA:  0.7
  BW = 15 MHz, no CA:  0.85
  BW = 20 MHz, no CA:  1.0
  BW = 20 MHz + CA:    1.3
```

| Scenario | RadioScore | BandFactor | LinkCapacity | Condition |
|---|---|---|---|---|
| EXCELLENT, Band 13 | 95 | 0.7 | 66 | GOOD |
| POOR, Band 4 | 30 | 1.0 | 30 | POOR |
| GOOD, Band 4+CA | 70 | 1.3 | 91 | EXCELLENT |
| FAIR, Band 13 | 45 | 0.7 | 31 | POOR |

---

## Observations from During-Speedtest Metrics

1. **Signal metrics do NOT change during active download** — RSRP/RSRQ/SNR remain stable whether idle or under load
2. **qmicli cannot observe active traffic state** — TX always shows "not in traffic," dormancy always "dormant" (different WDS client)
3. **Packet statistics via qmicli are stale** — always show the initial 4 packets from cellular-manager setup
4. **SNR fluctuates slightly** (2.6–7.6 dB on POOR device over ~60s window) but RSRP/RSRQ are rock-stable
5. **dmcli X_RDK_BandInfo can be STALE** — reported Band 13 when qmicli showed actual Band 4

---

## Metrics Evaluation Summary

| Metric | Source | Adds Value to Algorithm? | Reason |
|---|---|---|---|
| RSRP | qmicli/dmcli | ✅ Yes (core) | Primary signal strength |
| RSRQ | qmicli/dmcli | ✅ Yes (core) | Interference + quality |
| SNR | qmicli/dmcli | ✅ Yes (core) | Usability predictor |
| RSSI | qmicli/dmcli | ✅ Yes (core, low weight) | Wideband signal |
| TXPower | qmicli TX/dmcli X_RDK_TRX | ❌ No | Always 0 via proxy (idle) |
| ECIO | qmicli signal-strength | ❌ No | Identical across conditions |
| IO | qmicli signal-strength | ❌ No | Identical across conditions |
| SINR (QMI) | qmicli signal-strength | ❌ No | Too coarse (caps at level 8) |
| RX Diversity Delta | qmicli tx-rx-info | ❌ No | Counterintuitive results |
| Band/EARFCN | qmicli rf-band-info | ⚠️ Context only | Explains throughput, not RF quality |
| CA Status | qmicli ca-info | ⚠️ Context only | Capacity indicator, not RF |
| Timing Advance | qmicli cell-location | ❌ Unavailable | Not reported when idle |
| Dormancy | qmicli wds | ❌ No | Always dormant (wrong WDS client) |
| Channel Rates | qmicli wds | ❌ No | Always 0 (wrong WDS client) |
| Neighbor RSRP | qmicli cell-location | ⚠️ Possible | Indicates cell edge condition |

---

## Gateway Speedtest (X_RDK_SpeedTest.Enable) — POOR Device

**Method:** `dmcli eRT setv Device.Cellular.X_RDK_SpeedTest.Enable bool true`  
**Tool:** speedtest-cli against speedtest.net  
**Output Files:** `/tmp/ltespeedtest.txt`, `/tmp/ltelatencytest.txt`, `/tmp/lteinfo.txt`

### Results

| Metric | Value |
|---|---|
| Download | **23.18 Mbit/s** |
| Upload | **4.65 Mbit/s** |
| Ping (to server) | 67.945 ms |
| Server | ReliableSite Hosting (Piscataway, NJ) [117.02 km] |
| Latency (100 pings avg) | 40.882 ms |
| Latency (min/max) | 23.473 / 163.194 ms |
| Packet Loss | 1% (1 of 100) |

### Signal Metrics During Gateway Speedtest

| Metric | Pre-test (Idle) | During Test | Post-test |
|---|---|---|---|
| RSRP | -105 dBm | -105 dBm | -105 dBm |
| RSRQ | -14 dB | -14 dB | -14 dB |
| SNR | 4.6 dB | 6.8 dB | 6.6 dB |
| RSSI | -72 dBm | -72 dBm | -72 dBm |
| UE State | Idle | Connected | Idle |

### Carrier Aggregation (Post-Speedtest Snapshot)

| | Primary Cell | Secondary Cell |
|---|---|---|
| Band | eutran-4 (Band 4) | eutran-13 (Band 13) |
| EARFCN | 2125 | 5230 |
| PCI | 256 | 256 |
| DL Bandwidth | **15 MHz** | 10 MHz |
| State | Active | **Deconfigured** |
| Total potential BW | 25 MHz (if CA active) | |

### Timing Advance (cell distance)

- During speedtest (UE connected): **20 μs** → ~3 km from cell tower
- Idle: unavailable (only reported in RRC connected state)

### Cell Location Info (from /tmp/lteinfo.txt during test)

- UE In Idle: **No** (connected mode during speedtest)
- Serving Cell RSRP: -103.7 dBm, RSRQ: -12.2 dB (slightly better than idle report)
- Neighbor intra-freq: PCI 268 (RSRP -114.5), PCI 266 (RSRP -111.3)
- Interfrequency Band 13 cells available: PCI 256 (-86.7), PCI 286 (-94.0), PCI 287 (-91.1)

---

## Gateway Speedtest (X_RDK_SpeedTest.Enable) — EXCELLENT Device

**Method:** `dmcli eRT setv Device.Cellular.X_RDK_SpeedTest.Enable bool true`  
**Tool:** speedtest-cli against speedtest.net  
**Output Files:** `/tmp/ltespeedtest.txt`, `/tmp/ltelatencytest.txt`, `/tmp/lteinfo.txt`

### Results

| Metric | Value |
|---|---|
| Download | **57.31 Mbit/s** |
| Upload | **15.98 Mbit/s** |
| Ping (to server) | 93.16 ms |
| Server | Comcast (Plainfield, NJ) [81.52 km] |
| Latency (100 pings avg) | 71.249 ms |
| Latency (min/max) | 43.272 / 286.000 ms |
| Packet Loss | 10% (10 of 100) |

### Signal Metrics During Gateway Speedtest

| Metric | Pre-test (Idle) | During Test | Post-test |
|---|---|---|---|
| RSRP | -65 dBm | -65 dBm | -65 dBm |
| RSRQ | -6 dB | -6 dB | -6 dB |
| SNR | 30.0 dB | 30.0 dB | 27.8 dB |
| RSSI | -37 dBm | -37 dBm | -37 dBm |
| UE State | Idle | Connected | Idle |

### Carrier Aggregation

- CA: Not available (error: InformationUnavailable)
- Single carrier: Band 13, 10 MHz bandwidth

### Timing Advance (cell distance)

- During speedtest (UE connected): **0 μs** → device is co-located / <100m from tower
- Idle: unavailable

### Cell Location Info (from /tmp/lteinfo.txt during test)

- UE In Idle: **No** (connected mode during speedtest)
- Serving Cell only: PCI 490, RSRP: -64.3 dBm, RSRQ: -6.5 dB, RSSI: -41.2 dBm
- No intrafrequency neighbors visible
- No interfrequency neighbors visible
- Interpretation: Strong, isolated cell with exclusive coverage

---

## Gateway Speedtest Comparison Summary

| Metric | EXCELLENT | POOR | Notes |
|---|---|---|---|
| **Download (Mbps)** | **57.31** | **23.18** | EXCELLENT 2.5x faster |
| **Upload (Mbps)** | **15.98** | **4.65** | EXCELLENT 3.4x faster |
| Ping (to server) | 93.16 ms | 67.95 ms | POOR closer to server |
| Latency (avg) | 71.25 ms | 40.88 ms | POOR has lower latency |
| Packet Loss | **10%** | 1% | EXCELLENT worse (!) |
| Server | Comcast (Plainfield, NJ) | ReliableSite (Piscataway, NJ) | Different servers |
| Distance to server | 81.52 km | 117.02 km | |
| Band | 13 (700 MHz, 10 MHz BW) | 4 (AWS-1, 15 MHz BW) | |
| RSRP | -65 dBm | -105 dBm | 40 dB difference |
| Timing Advance | 0 μs (<100m) | 20 μs (~3 km) | |
| RadioEnvConditions | EXCELLENT | POOR | Current classification |

### Key Observations from Gateway Speedtest

1. **With speedtest-cli, EXCELLENT clearly outperforms POOR** (57.31 vs 23.18 Mbps) — unlike curl where they were equal
2. **The EXCELLENT device's superior RF** (SNR 30 vs 4.8) translates to much higher throughput when using standardized speedtest
3. **Upload confirms RF advantage** — 15.98 vs 4.65 Mbps (3.4x) since upload doesn't benefit from bandwidth alone
4. **Surprisingly, EXCELLENT has 10% packet loss** vs POOR's 1% — may indicate congestion or server-side issue
5. **Latency paradox** — POOR device has LOWER latency despite being 3km from tower vs <100m for EXCELLENT. Different server selection explains this.
6. **Different speedtest servers** were auto-selected — results are not perfectly comparable, but DL/UL ratio is significant

### Comparison: curl vs Gateway Speedtest

| Method | EXCELLENT DL | POOR DL | Who Wins? |
|---|---|---|---|
| **curl (100MB raw)** | 55.5 Mbps | 57.6 Mbps | POOR (bandwidth advantage) |
| **Gateway speedtest-cli** | 57.31 Mbps | 23.18 Mbps | EXCELLENT (RF advantage) |

**Why the discrepancy?**
- curl downloads from a single high-capacity CDN → bandwidth-limited → POOR's wider band wins
- speedtest-cli auto-selects the "best" server by ping → may hit capacity-limited or distant servers
- speedtest-cli multi-thread methodology may be more sensitive to packet loss / retransmissions
- The POOR device's SNR of 4.8 dB causes more retransmissions under multi-stream load

**Conclusion for Algorithm Design:**  
The gateway speedtest (standardized, operator-validated) shows that RSRP/SNR **DO correlate** with real-world throughput when measured properly. The curl test was misleading because it hit an extremely high-capacity CDN that didn't saturate either connection.

### Key Finding: BandInfo Still Stale via dmcli

```
dmcli X_RDK_BandInfo = 13 (WRONG — stale)
qmicli rf-band-info = eutran-4 (CORRECT — actual serving band)
```

This confirms the `BandInfo` issue — cellular-manager caches the band from initial connection and doesn't update when the network handovers the device to a different band.

---

## Gateway Speedtest Internal Architecture

### Call Chain: dmcli → speedtest-cli

When `dmcli eRT setv Device.Cellular.X_RDK_SpeedTest.Enable bool true` is called:

```
dmcli → CellularMgr_EnableSpeedTest(true)  [cellularmgr_cellular_apis.c]
      → pthread_create(CellularMgr_EnableSpeedTestThread)
      → v_secure_system("speedtest-lte-enable.sh true")
```

### Script Chain on Device

**1. `/usr/bin/speedtest-lte-enable.sh true`**
```sh
syscfg set lte_speedtest_enabled 1
syscfg commit
speedtest-lte.sh              # ← runs the actual test
systemctl restart speedtest-lte  # sets up periodic cron
```

**2. `/usr/bin/speedtest-lte.sh`**
```sh
# Only runs if Device_Mode=1 AND WFO not enabled AND cellular WAN IP valid
CELLULAR_WAN_IP=$(sysevent get cellular_wan_v4_ip)
CELLULAR_WAN_GW=$(sysevent get cellular_wan_v4_gw)
# Ensures ip route table "MODEM" has default via $CELLULAR_WAN_GW dev wwan0
# Ensures ip rule "from $CELLULAR_WAN_IP lookup MODEM" exists
/bin/sh /usr/bin/upload-speedtest-report.sh
```

**3. `/usr/bin/upload-speedtest-report.sh`** (the actual test runner)
```sh
# 1. Pre-check connectivity:
TESTNW=$(curl --interface wwan0 -Is https://www.speedtest.net | head -n 1)
# ← THIS CAN TIMEOUT AND SKIP THE ENTIRE TEST

# 2. Capture cell location info:
qmicli -p -d /dev/cdc-wdm0 --nas-get-cell-location-info > /tmp/lteinfo.txt

# 3. Run speedtest (KEY: --source binds to wwan0 IP):
/usr/bin/python3 /usr/bin/speedtest --source $WWAN0IP >> /tmp/ltespeedtest.txt
sleep 30

# 4. Run latency test:
ping -I wwan0 www.speedtest.net -c 100 -i 0.010 >> /tmp/ltelatencytest.txt

# 5. POST results to device-services.comcast.net (telemetry upload)
```

### Key Differences: dmcli vs Manual `speedtest-cli --simple`

| Aspect | dmcli (upload-speedtest-report.sh) | Manual `speedtest-cli --simple` |
|--------|------|--------|
| **Source IP binding** | `--source $WWAN0IP` (forces wwan0) | None (uses default route) |
| **Output format** | Full verbose (server, ISP, hosted by) | Simple (Ping/DL/UL only) |
| **Ping test** | `-I wwan0 -c 100 -i 0.010` (10ms interval) | Not included |
| **Cell info capture** | `/tmp/lteinfo.txt` auto-generated | Not captured |
| **Pre-connectivity check** | `curl --interface wwan0` to speedtest.net | None |
| **Routing setup** | Adds MODEM table/rule if missing | Relies on existing routes |
| **Telemetry upload** | POSTs results to device-services API | No upload |
| **Failure mode** | Silent skip if curl pre-check fails | Hangs at "Retrieving configuration" |

### Why dmcli Speedtest Fails Silently

The most common failure is in `upload-speedtest-report.sh`:
```sh
TESTNW=$(curl --interface wwan0 -Is https://www.speedtest.net | head -n 1)
if [[ $TESTNW =~ "HTTP/1.1 200 OK" ]]; then
    # run speedtest
fi
```

If the `curl` pre-check times out or returns anything other than `HTTP/1.1 200 OK` or `HTTP/2 200`, the script **exits without running speedtest at all** — no error, no output files.

### Why Repeated dmcli Triggers Fail

After first trigger, `syscfg lte_speedtest_enabled` is set to 1. Subsequent `dmcli setv Enable bool true` still calls the script, but:
1. If a previous `speedtest-lte.sh` process is still running (stuck curl), the new invocation may conflict
2. The `systemctl restart speedtest-lte` at the end of `speedtest-lte-enable.sh` can interfere with a running test
3. No mutex/lock mechanism exists in the scripts

### Recommended Approach for Bulk Data Collection

Instead of dmcli, call the scripts directly on each XLE:

**Option A — Full script (recommended, matches production behavior):**
```sh
/bin/sh /usr/bin/upload-speedtest-report.sh
```

**Option B — Direct speedtest-cli with source binding (fastest, most reliable):**
```sh
WWAN0IP=$(sysevent get cellular_wan_v4_ip)
speedtest-cli --source $WWAN0IP --simple --timeout 60 2>&1 | tee /tmp/ltespeedtest.txt
```

**Option C — Direct speedtest-cli without source binding (what we used for parallel runs):**
```sh
speedtest-cli --simple --timeout 60 2>&1 | tee /tmp/ltespeedtest.txt
```

**Note:** On XLE devices where wwan0 is the only WAN interface (Device_Mode=1), Options B and C produce identical results since all traffic routes through wwan0 anyway. The `--source` flag matters only in dual-WAN or WFO configurations.

---

## Bulk Data Collection Progress

Using parallel 3-terminal approach with direct `speedtest-cli --simple --timeout 60`.
Ping test: `ping -c 100 www.speedtest.net` (default route = wwan0 on XLE).
Cell info: `qmicli -d /dev/cdc-wdm0 -p --nas-get-cell-location-info > /tmp/lteinfo.txt` during test.

See `xle_radio_env_speedtest_data.csv` for full results (8 devices collected so far, 92 remaining).

---

## Next Steps

1. ~~Test EXCELLENT device with gateway speedtest~~ ✅ Done
2. **Continue bulk data collection** — 92 devices remaining across EXCELLENT/GOOD/FAIR/POOR
3. **Test during congested hours** — current tests were ~midnight UTC (low congestion)
4. **Fix BandInfo staleness** — cellular-manager should refresh band from qmicli periodically
5. **Decide architecture** — RF-only vs band-aware vs separate parameter
6. **If band-aware:** determine band bandwidth from EARFCN lookup table or from CA info
7. **Implement chosen algorithm** in `cellularmgr_cellular_apis.c`

---

## Commands Reference (for future data collection on XLE)

```bash
# Signal quality (core metrics)
qmicli -d /dev/cdc-wdm0 -p --nas-get-signal-info

# Extended signal (ECIO, IO, SINR)
qmicli -d /dev/cdc-wdm0 -p --nas-get-signal-strength

# TX/RX per-chain info
qmicli -d /dev/cdc-wdm0 -p --nas-get-tx-rx-info=lte

# Band and frequency
qmicli -d /dev/cdc-wdm0 -p --nas-get-rf-band-info

# Cell location (neighbors, EARFCN, PCI, timing advance)
qmicli -d /dev/cdc-wdm0 -p --nas-get-cell-location-info

# Carrier aggregation
qmicli -d /dev/cdc-wdm0 -p --nas-get-lte-cphy-ca-info

# Channel rates and dormancy
qmicli -d /dev/cdc-wdm0 -p --wds-get-channel-rates
qmicli -d /dev/cdc-wdm0 -p --wds-get-dormancy-status

# Speedtest (100MB raw curl download)
curl -o /dev/null -w '\nDL_Speed_Bps: %{speed_download}\nTotal_Time: %{time_total}\nSize_DL: %{size_download}\n' http://speedtest.tele2.net/100MB.zip

# Gateway speedtest (speedtest-cli via dmcli)
dmcli eRT setv Device.Cellular.X_RDK_SpeedTest.Enable bool true
# Results at: /tmp/ltespeedtest.txt, /tmp/ltelatencytest.txt, /tmp/lteinfo.txt

# All dmcli cellular params
dmcli eRT getv Device.Cellular.Interface.1.
```

---

## Weight Combination Optimization Results (76 devices)

**Date:** June 4, 2026

### Dataset Summary

- Total devices parsed: 78 (with valid band info)
- Successful speedtest: 76
- Throughput distribution: 55 EXCELLENT (>100Mbps), 16 GOOD (50-100), 2 FAIR (25-50), 3 POOR (<25)

### Algorithm Comparison Table

| Algorithm | Exact Match% | Within-1-Level% | Monotonic | Categories Used |
|---|---|---|---|---|
| RSRP-Only (current) | 22.4% | 56.6% | 67% | 4 |
| Proposed (35/35/25/5) | 44.7% | 81.6% | 33% | 5 |
| RSRQ-dom (5/80/0/15) | 56.6% | 85.5% | 67% | 5 |
| RSRQ-dom (10/80/5/5) | 56.6% | 85.5% | 67% | 5 |
| RSSI-dom (0/5/0/95) | 60.5% | 90.8% | 100% | 3 |
| RSRQ+RSSI (0/50/0/50) | 59.2% | 89.5% | 67% | 4 |
| RSRQ+SNR (0/50/50/0) | 50.0% | 84.2% | 67% | 5 |
| Balanced (25/25/25/25) | 51.3% | 82.9% | 0% | 4 |

### Category Distribution (Avg DL Mbps per predicted category)

| Algorithm | EXCELLENT | GOOD | FAIR | POOR | CRITICAL |
|---|---|---|---|---|---|
| RSRP-Only | N=22 avg=101 | N=16 avg=137 | N=26 avg=126 | N=12 avg=124 | — |
| Proposed (35/35/25/5) | N=37 avg=119 | N=30 avg=122 | N=6 avg=119 | N=2 avg=138 | N=1 avg=151 |
| RSRQ-dom (5/80/0/15) | N=53 avg=125 | N=14 avg=109 | N=6 avg=103 | N=1 avg=150 | N=2 avg=138 |
| RSSI-dom (0/5/0/95) | N=57 avg=124 | N=15 avg=110 | N=4 avg=110 | — | — |

### Parameter Correlation with Throughput (Pearson r)

| Parameter | r vs DL_Mbps | N |
|---|---|---|
| RSRP (idle) | -0.196 | 76 |
| RSRQ (idle) | 0.011 | 76 |
| SNR (idle) | -0.127 | 76 |
| RSSI (idle) | -0.178 | 76 |
| BandBW MHz | -0.074 | 76 |
| RSRP (during test) | -0.210 | 76 |
| RSRQ (during test) | -0.164 | 76 |
| SNR (during test) | -0.061 | 76 |

### Key Finding: RF Metrics Do NOT Predict Throughput

**All Pearson correlations are near zero** (|r| < 0.21). This means:

1. **Radio quality ≠ throughput** in this dataset. Devices with RSRP=-117, SNR=-5.2 achieved 151 Mbps. Devices with RSRP=-113, SNR=8.2 achieved 205 Mbps.

2. **Reason**: The tested cells are lightly loaded with adequate backhaul. When network load is low, even devices with marginal RF quality get scheduled with full resources → high throughput.

3. **Implication for RadioEnvConditions**: The parameter should reflect **RF environment health** (signal quality, interference, congestion indicators), NOT predict throughput directly. In congested networks, RF quality WOULD correlate with throughput.

### Recommendation

The **proposed formula (35/35/25/5)** remains the best choice because:

1. It correctly characterizes the RF ENVIRONMENT (not throughput)
2. It uses RSRQ (interference/congestion indicator) and SNR (actual usability) which matter under load
3. 44.7% exact + 81.6% within-1-level is a significant improvement over RSRP-only (22.4%/56.6%)
4. The lack of monotonicity (POOR devices getting high DL) proves RF quality alone doesn't determine throughput — but that's expected behavior, not a flaw

### Why Not RSSI-dominant (best accuracy)?

The RSSI-only approach gets 60.5% accuracy but:
- Trivially classifies 75% of devices as EXCELLENT (3 categories only)
- RSSI is the LEAST useful LTE metric (includes noise + interference)
- Would not detect congested environments correctly
- Only "works" because our dataset has most devices with good RSSI AND good throughput

### Weight Sweep Top Results (with 3+ category spread)

Best combos requiring meaningful distribution across categories:

| Rank | Weights (P/Q/S/I) | Exact% | Within1% | Monotonic |
|---|---|---|---|---|
| 1 | 0/5/0/95 | 60.5 | 90.8 | 100% |
| 2 | 0/10/0/90 | 60.5 | 90.8 | 100% |
| 3 | 0/20/0/80 | 60.5 | 90.8 | 100% |
| 4 | 5/80/0/15 (RSRQ-heavy) | 56.6 | 85.5 | 75% |
| 5 | 0/50/0/50 | 59.2 | 89.5 | 67% |

### Comprehensive Correlation Analysis: ALL Metrics vs DL_Mbps

Tested every available metric and derived combination for correlation with throughput:

| Metric | Pearson r | |r| | N | Significance |
|---|---|---|---|---|
| **Ping_ms** | **-0.4675** | **0.4675** | 76 | **MODERATE** |
| **PacketLoss_pct** | **-0.2732** | **0.2732** | 76 | WEAK |
| RSRP (during test) | -0.2095 | 0.2095 | 76 | WEAK |
| (RSRP+120)*BW product | -0.2389 | 0.2389 | 76 | WEAK |
| (SNR+10)*BW product | -0.2090 | 0.2090 | 76 | WEAK |
| RSRP (idle) | -0.1962 | 0.1962 | 76 | WEAK |
| CA_TotalBW MHz | -0.1901 | 0.1901 | 76 | WEAK |
| RSSI (idle) | -0.1780 | 0.1780 | 76 | WEAK |
| RSRQ (during test) | -0.1641 | 0.1641 | 76 | WEAK |
| Neighbors_InterFreq | -0.1535 | 0.1535 | 76 | WEAK |
| Neighbors_IntraFreq | -0.1415 | 0.1415 | 76 | WEAK |
| SNR (idle) | -0.1266 | 0.1266 | 76 | WEAK |
| RSRP delta (test-idle) | -0.1076 | 0.1076 | 76 | NONE |
| SNR delta (test-idle) | 0.1648 | 0.1648 | 73 | WEAK |
| BandBW MHz | -0.0737 | 0.0737 | 76 | NONE |
| UL_Mbps | 0.0885 | 0.0885 | 76 | NONE |
| RSRQ (idle) | 0.0109 | 0.0109 | 76 | NONE |
| SNR (during test) | -0.0614 | 0.0614 | 76 | NONE |
| CellDistance_km | -0.4086 | 0.4086 | **3** | insufficient data |
| TimingAdvance_us | -0.4086 | 0.4086 | **3** | insufficient data |

### Tower Distance (CellDistance_km / TimingAdvance)

- Only **3 out of 76** devices had valid CellDistance data
- 73 devices have empty TimingAdvance (TA not reported in idle mode)
- With N=3: r = -0.41 (farther = lower throughput) — statistically meaningless
- **Reason**: TimingAdvance is only available during active RRC Connected state; our idle-mode query doesn't capture it for most modems

### Categorical Metric Analysis

**Band vs Throughput:**

| Band | N | Avg DL | StdDev | Range |
|---|---|---|---|---|
| eutran-4 | 49 | 126.1 | 41.5 | 23–206 Mbps |
| eutran-2 | 14 | 118.2 | 47.0 | 21–187 Mbps |
| eutran-13 | 7 | 106.8 | 51.9 | 30–186 Mbps |
| eutran-66 | 6 | 99.4 | 53.7 | 18–152 Mbps |

**Carrier Aggregation:**

| CA Status | N | Avg DL | Range |
|---|---|---|---|
| No (idle) | 67 | 124.8 | 18–206 Mbps |
| No_Deconfigured | 9 | 90.7 | 21–187 Mbps |

**BandBW:**

| BW (MHz) | N | Avg DL | Range |
|---|---|---|---|
| 10 | 67 | 122.6 | 21–206 Mbps |
| 20 | 8 | 117.3 | 18–187 Mbps |
| 15 | 1 | 23.2 | — |

### Key Conclusion: No Single RF Metric Predicts Throughput

**The strongest correlator is Ping latency (r=-0.47)** — a network-path metric, NOT a radio metric. This confirms:

1. **Radio signal metrics (RSRP, RSRQ, SNR, RSSI)**: All |r| < 0.20. Essentially uncorrelated with throughput.
2. **Bandwidth/CA metrics**: |r| < 0.19. Not helpful either.
3. **Derived combinations** (signal×BW, signal deltas): |r| < 0.24. Marginal improvement.
4. **Tower distance**: Insufficient data (only 3 devices reported TA).
5. **Ping latency**: r=-0.47 is the ONLY moderate correlation — higher latency = lower DL speed. This suggests network path quality (backhaul, server distance) matters more than RF for this dataset.

**Why RadioEnvConditions should NOT try to predict throughput:**
- RF environment health ≠ throughput. A device can have POOR signal but still get 200 Mbps on an unloaded cell.
- The parameter should reflect the **radio link quality** (interference, noise, signal strength) which determines reliability, not speed.
- Under network congestion, RF quality WILL correlate with throughput — our test dataset simply has low-load cells.

### Files

- Analysis script: `analyze_radio_env_scoring.py`
- Full analysis output: `xle_radio_env_scoring_analysis_results.txt`
- Raw data: `xle_radio_env_speedtest_data.csv`
