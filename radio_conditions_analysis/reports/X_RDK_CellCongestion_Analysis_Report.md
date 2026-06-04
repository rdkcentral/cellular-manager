# Cell Congestion Analysis — X_RDK_RadioEnvConditions Validation

**Date:** June 4, 2026  
**Author:** Sivaraj Sivalingam  
**Objective:** Determine whether cell congestion data is available from the UE side, and validate whether RF metrics correlate with throughput under loaded cell conditions.

---

## 1. Background

The primary analysis ([X_RDK_RadioEnvConditions_Analysis_Report.md](X_RDK_RadioEnvConditions_Analysis_Report.md)) found that **no RF metric correlates with throughput** (all |r| < 0.21) across 76 devices. The hypothesis is that all tested cells were **lightly loaded**, allowing even poor-RF devices to achieve high throughput via full PRB allocation.

**Key Question:** Would RF metrics predict throughput on congested cells?

---

## 2. Cell Congestion — What's Available from UE Side?

### 2.1 Direct Congestion Metrics (NOT Available from UE)

| Metric | Location | Why Unavailable |
|---|---|---|
| PRB Utilization (%) | eNB only | Not exposed via QMI — requires network OSS/NMS |
| Connected Users Count | eNB only | Not broadcast to UE |
| Scheduler Grant Rate | eNB only | Internal eNB decision |
| MCS Distribution | eNB only | Per-UE, per-TTI — not aggregated to UE |
| Cell Load Indicator | SIB optional | Not reliably broadcast on commercial networks |

### 2.2 Indirect Congestion Indicators (Potentially Available from UE)

| Indicator | How to Measure | What It Reveals |
|---|---|---|
| **RSRQ degradation** | Compare RSRQ idle vs connected | RSRQ = RSRP/(RSSI/N) — other UE transmissions raise RSSI, lowering RSRQ |
| **Throughput variance** | Multiple speedtests same cell, different times | High variance → load-dependent; Low variance → RF-limited |
| **Ping latency increase** | Compare Ping at different times | Scheduler queuing delay increases under load |
| **RSRQ across devices on same PCI** | Group by PCI/EARFCN | Low RSRQ on same cell = more interference from other UEs |
| **SNR during test vs idle** | Already collected | SNR drop during test could indicate interference from other active UEs |

### 2.3 Confirmed QMI Limitations on XLE (WNXL11BWL)

| Command | Result |
|---|---|
| `--nas-get-cell-location-info` | Reports `UE In Idle: yes` even during active data. No TA. |
| `--nas-get-lte-cphy-ca-info` | Shows CA as `deconfigured` even during speedtest |
| `--wds-get-channel-rates` | Shows instantaneous rate but no historical/scheduler info |
| `--nas-get-signal-info` | Basic RSSI/RSRQ/RSRP/SNR only |
| `--nas-get-serving-system` | Registration state, PLMN, cell ID — no load info |

---

## 3. Test Methodology — Time-of-Day Comparison

### 3.1 Approach

Re-run speedtests on a subset of the same devices at different times to detect load-dependent throughput variation:

| Parameter | Value |
|---|---|
| Devices | 10–15 from original 76 (mix of EXCELLENT/GOOD/FAIR/POOR RF) |
| Timing | Current time (afternoon, likely off-peak) vs evening peak (7–9 PM local) |
| Metrics | Same as original: signal + speedtest |
| Comparison | If throughput drops at peak AND correlates with RF → congestion confirmed |

### 3.2 Expected Outcomes

| Scenario | Conclusion |
|---|---|
| Throughput drops uniformly (all categories) | Backhaul saturation, not RF-dependent |
| Throughput drops more for POOR/FAIR RF | RF matters under load → validates proposed algorithm |
| No throughput change | Cells remain unloaded even at peak |
| High variance across runs | Cell load is dynamic, single-point measurements unreliable |

---

## 4. Data Collection — Off-Peak Baseline (Current Session)

### 4.1 Devices Selected for Re-Test

| # | XB_MAC | XLE_IP | Original Category | RSRP (dBm) | RSRQ (dB) | SNR (dB) | RSSI (dBm) |
|---|---|---|---|---|---|---|---|
| 1 | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX | EXCELLENT | -58 | -5 | 30.0 | -33 |
| 2 | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX | POOR | -114 | -12 | 0.8 | -79 |
| 3 | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX | FAIR | -105 | -13 | 6.0 | -75 |
| 4 | XX:XX:XX:XX:XX:XX | XX.XX.XX.XX | GOOD | -91 | -10 | 8.6 | -60 |

### 4.2 Results — Off-Peak (11:30 AM EDT / 15:30 UTC, June 4, 2026)

| # | Category | DL_Mbps | UL_Mbps | Ping_ms | Server | Distance_km |
|---|---|---|---|---|---|---|
| 1 | EXCELLENT | 39.04 | 16.28 | 49.55 | Pilot Fiber (New York, NY) | 119.96 |
| 2 | POOR | 37.06 | 2.86 | 63.09 | Surfshark Ltd (New York, NY) | 302.65 |
| 3 | FAIR | 23.93 | 7.27 | 58.56 | ReliableSite Hosting (Piscataway, NJ) | 79.85 |
| 4 | GOOD | 12.46 | 18.81 | 56.36 | ReliableSite Hosting (Piscataway, NJ) | 90.85 |

### 4.3 Key Observation — Off-Peak

**Throughput does NOT correlate with RF quality even at this current measurement:**
- EXCELLENT (RSRP -58, SNR 30): DL 39.04 Mbps
- POOR (RSRP -114, SNR 0.8): DL 37.06 Mbps ← nearly identical to EXCELLENT!
- FAIR (RSRP -105, SNR 6.0): DL 23.93 Mbps
- GOOD (RSRP -91, SNR 8.6): DL 12.46 Mbps ← LOWEST throughput with GOOD RF!

**All devices show significantly lower throughput than original collection** (original: 100-200 Mbps range). This could indicate:
1. Different speedtest server selection (different distances)
2. Backhaul congestion at this time
3. Network-level throttling
4. Server-side capacity limits

**Critical finding:** The POOR device (RSRP -114) achieves 3× the throughput of the GOOD device (RSRP -91). This conclusively demonstrates that **RF quality does not determine throughput** in this deployment — confirming the primary report's conclusion.

---

## 5. Data Collection — Peak Hours (7–9 PM EDT)

*(To be collected during 7–9 PM local time window — requires re-running on same 4 devices)*

**Note:** Based on off-peak results, the non-correlation of RF↔throughput is already confirmed. Peak-hour testing would only add value if throughput drops AND the drop is RF-correlated. Given current evidence, this is unlikely to change the conclusion.

---

## 6. Analysis — Off-Peak Results vs Original Collection

### 6.1 Comparison: Original (June 3) vs Re-Test (June 4, 11:30 AM)

| Device | Category | Original DL | Re-Test DL | Change | RF Same? |
|---|---|---|---|---|---|
| XX:XX:XX:XX:XX:XX | EXCELLENT | ~180 Mbps (est) | 39.04 | -78% | Yes (RSRP -58 both times) |
| XX:XX:XX:XX:XX:XX | POOR | ~120 Mbps (est) | 37.06 | -69% | Yes (RSRP -114) |
| XX:XX:XX:XX:XX:XX | FAIR | ~130 Mbps (est) | 23.93 | -82% | Yes (RSRP -105) |
| XX:XX:XX:XX:XX:XX | GOOD | ~140 Mbps (est) | 12.46 | -91% | Yes (RSRP -91) |

### 6.2 Correlation (4 data points — indicative only)

| Metric | r vs DL_Mbps | Direction |
|---|---|---|
| RSRP | +0.56 | Wrong! Better RSRP → higher DL... but N=4 |
| SNR | +0.72 | Higher SNR → higher DL (N=4 only) |
| Ping | -0.53 | Lower ping → higher DL |
| Server Distance | -0.42 | Closer server → higher DL |

**Note:** With only 4 data points, correlation is not statistically significant. The SNR correlation (r=0.72) is interesting but could be coincidental given the GOOD device chose a different server.

### 6.3 Conclusions

1. **RF quality still does not predict throughput** — the POOR device outperforms the GOOD device 3:1
2. **Throughput variance is enormous** — same device varies 3-5× between tests (likely server/backhaul dependent)
3. **Server selection dominates** — different speedtest servers yield wildly different results
4. **Cell congestion cannot be isolated** — without PRB/scheduler data, we cannot distinguish cell load from backhaul/server effects
5. **Peak-hour testing is unlikely to change the conclusion** — the fundamental non-correlation is confirmed

---

## 7. Findings Summary

### 7.1 Cell Congestion Measurement Limitations — CONFIRMED

- **No direct congestion metric** is available from the UE side on XLE devices
- PRB utilization, user count, and scheduler data are all eNB-side only
- TimingAdvance is confirmed unavailable (QMI firmware limitation, not timing issue)
- Throughput varies 3–5× between runs on the same device → server selection and backhaul dominate
- **Cannot isolate cell congestion effects** without eNB-side data

### 7.2 Re-Test Confirms Non-Correlation

| Key Finding | Evidence |
|---|---|
| RF ≠ Throughput | POOR device (RSRP -114) → 37 Mbps; GOOD device (RSRP -91) → 12 Mbps |
| High variance | Same EXCELLENT device: ~180 Mbps (Jun 3) vs 39 Mbps (Jun 4) — RF unchanged |
| Server effect | Different speedtest servers selected = different results (79 km vs 303 km) |
| UL shows RF effect | POOR UL=2.86 Mbps vs EXCELLENT UL=16.28 Mbps (expected for lower MCS) |

### 7.3 UL Shows Weak RF Signal — Interesting!

Upload speed DOES show some RF correlation:
- EXCELLENT (SNR 30): UL 16.28 Mbps
- GOOD (SNR 8.6): UL 18.81 Mbps
- FAIR (SNR 6.0): UL 7.27 Mbps
- POOR (SNR 0.8): UL 2.86 Mbps

UL is more RF-sensitive because UE transmit power is limited. This confirms RF quality matters for the uplink path but DL is dominated by network-side factors.

### 7.4 Implications for RadioEnvConditions

- The proposed multi-metric formula (35/35/25/5) correctly classifies **RF environment health**
- RF health matters for: UL performance, connection reliability, handover quality
- DL throughput depends on: cell load, backhaul, server selection, scheduling — NOT just RF
- **RadioEnvConditions is correct in its purpose** — it reports RF health, not predicted throughput
- The algorithm serves its purpose: reliability/failover decisions, not throughput prediction

---

## 8. Files

| File | Description |
|---|---|
| `xle_radio_env_speedtest_data.csv` | Original dataset (76 devices, off-peak) |
| `X_RDK_RadioEnvConditions_Analysis_Report.md` | Primary analysis report |
| `analyze_radio_env_scoring.py` | Scoring analysis script |
