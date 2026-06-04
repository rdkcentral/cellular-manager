# X_RDK_RadioEnvConditions Optimization — Analysis Report

**Date:** June 4, 2026  
**Author:** Sivaraj Sivalingam  
**Objective:** Evaluate whether the current RSRP-only `X_RDK_RadioEnvConditions` algorithm can be improved using additional LTE radio metrics, and determine which combination best reflects radio environment quality.

---

## 1. Executive Summary

The current `X_RDK_RadioEnvConditions` TR-181 parameter uses **RSRP-only** thresholds to classify the cellular radio environment. Field testing across **76 devices** with successful speedtests revealed that this single-metric approach has only **22.4% accuracy** at predicting throughput-based quality and produces **non-monotonic** results (devices classified as POOR achieve higher average throughput than those classified as EXCELLENT).

A proposed [**multi-metric weighted formula**](#4-proposed-algorithm) (RSRP×35% + RSRQ×35% + SNR×25% + RSSI×5%) using step-table scoring improves accuracy to **44.7%** with **81.6% within-1-level** accuracy. However, analysis shows that [**no RF metric meaningfully correlates with throughput**](#6-correlation-analysis--all-available-metrics) (all |r| < 0.21) in this dataset due to [lightly-loaded cells](#8-why-rf-metrics-dont-predict-throughput-in-this-dataset).

**Recommendation:** Adopt the final [**linear interpolation algorithm**](#91-adopt-linear-multi-metric-algorithm-final) (45/30/20/5 weights, 75/60/40/20 classification thresholds) for `X_RDK_RadioEnvConditions`. It achieves 80.8% exact accuracy (vs [22.4% baseline](#32-performance)), correctly characterizes RF environment health, and detects [interference/congestion cases](#8-why-rf-metrics-dont-predict-throughput-in-this-dataset) that RSRP alone misses.

---

## 2. Data Collection

### 2.1 Methodology

| Item | Detail |
|---|---|
| Platform | XLE (WNXL11BWL), ARM, BusyBox v1.31.1 |
| Access Path | Jump server → XB (CGM4981COM) → XLE via dropbear SSH (port 22) |
| Signal Metrics | `qmicli -d /dev/cdc-wdm0 -p --nas-get-signal-strength` (idle mode) |
| Band/Cell Info | `qmicli -d /dev/cdc-wdm0 -p --nas-get-cell-location-info` |
| Speedtest | `dmcli eRT setv Device.Cellular.X_RDK_SpeedTest.Enable bool true` |
| Post-Test Metrics | Signal re-queried after speedtest for connected-mode comparison |

### 2.2 Data Collected Per Device (39 fields)

| Category | Fields |
|---|---|
| Device Identification | DeviceNum, Category, XLE_MAC, XB_MAC, XLE_IP, XLE_Version, XB_Version |
| Signal Quality (idle) | RSRP_dBm, RSRQ_dB, SNR_dB, RSSI_dBm, RadioEnvConditions |
| Cell/Band Info | BandInfo_dmcli, Band_actual, EARFCN, BandBW_MHz, CA_Active, CA_TotalBW_MHz |
| Cell Identity | PCI, GlobalCellId, TAC |
| Distance | TimingAdvance_us, CellDistance_km |
| Speedtest Results | DL_Mbps, UL_Mbps, Ping_ms, Latency_avg/min/max_ms, PacketLoss_pct |
| Server Info | SpeedtestServer, ServerDistance_km |
| Neighbors | Neighbors_IntraFreq, Neighbors_InterFreq |
| Network | PLMN, UE_InIdle_During_Test |
| Signal (connected) | Serving_RSRP_Connected_dBm, Serving_RSRQ_Connected_dB, SNR_During_Test_dB |
| Metadata | Timestamp |

### 2.3 Dataset Summary

| Metric | Value |
|---|---|
| Total devices targeted | 100 (25 per RSRP category) |
| Devices with band info (parseable) | 78 |
| Successful speedtests | 76 |
| Failed (unreachable/timeout) | 22 |
| Collection period | June 3–4, 2026 |

### 2.4 Device Distribution by Current Algorithm (RSRP-Only)

| RadioEnvConditions | Devices (with speedtest) | Avg DL (Mbps) | Min DL | Max DL |
|---|---|---|---|---|
| EXCELLENT (RSRP > -85) | 22 | 101.0 | 21.4 | 186.9 |
| GOOD (-85 to -95) | 16 | 137.2 | 64.5 | 197.6 |
| FAIR (-95 to -105) | 26 | 125.9 | 17.5 | 201.4 |
| POOR (-105 to -115) | 12 | 123.8 | 23.2 | 205.5 |

**Key Observation:** Average throughput is HIGHER for GOOD/FAIR/POOR than EXCELLENT. The current algorithm has no predictive power for throughput (see [§8 — Why RF Metrics Don't Predict Throughput](#8-why-rf-metrics-dont-predict-throughput-in-this-dataset)).

**Upload Speed by RSRP-Only Category:**

| RadioEnvConditions | Avg UL (Mbps) | Min UL | Max UL |
|---|---|---|---|
| EXCELLENT | 44.1 | 10.0 | 163.7 |
| GOOD | 61.9 | 22.8 | 150.0 |
| FAIR | 62.7 | 2.6 | 158.5 |
| POOR | 78.4 | 4.7 | 170.3 |

**Same non-monotonic pattern for UL** — POOR-RF devices achieve higher average upload speed than EXCELLENT. RSRP vs UL: r = -0.28 (wrong direction).

---

## 3. Current Algorithm (Baseline)

### 3.1 Implementation

Source: `source/CellularManager/cellularmgr_cellular_apis.h`

```c
// RSRP-only thresholds:
EXCELLENT: RSRP > -85 dBm
GOOD:      -85 to -95 dBm
FAIR:      -95 to -105 dBm
POOR:      -105 to -115 dBm
```

### 3.2 Performance

| Metric | Value |
|---|---|
| Exact match with throughput-based truth | 22.4% |
| Within 1 level | 56.6% |
| Monotonicity (avg DL increases with score) | 67% (non-monotonic) |
| Pearson r (RSRP vs DL_Mbps) | -0.196 (weak, wrong direction) |

---

## 4. Proposed Algorithm

### 4.1 Multi-Metric Weighted Formula (Option A — Step Tables)

```
RADIO_SCORE = (RSRP_SCORE×35 + RSRQ_SCORE×35 + SNR_SCORE×25 + RSSI_SCORE×5) / 100
```

**Metric-to-Score Conversion (Step Tables):**

| RSRP (dBm) | Score | | RSRQ (dB) | Score | | SNR (dB) | Score | | RSSI (dBm) | Score |
|---|---|---|---|---|---|---|---|---|---|---|
| > -85 | 100 | | > -10 | 100 | | > 13 | 100 | | > -65 | 100 |
| -85 to -95 | 80 | | -10 to -12 | 85 | | 5 to 13 | 75 | | -65 to -75 | 80 |
| -95 to -105 | 60 | | -12 to -15 | 70 | | 1 to 5 | 50 | | -75 to -85 | 60 |
| -105 to -115 | 30 | | -15 to -17 | 40 | | -3 to 1 | 25 | | -85 to -95 | 40 |
| < -115 | 10 | | < -17 | 10 | | < -3 | 5 | | < -95 | 20 |

**Score-to-Condition Mapping:**

| Final Score | Condition |
|---|---|
| ≥ 80 | EXCELLENT |
| ≥ 60 | GOOD |
| ≥ 40 | FAIR |
| ≥ 20 | POOR |
| < 20 | CRITICAL |

### 4.2 Performance

| Metric | Value |
|---|---|
| Exact match with throughput-based truth | 44.7% (+22.3% improvement) |
| Within 1 level | 81.6% (+25% improvement) |
| Categories used | 5 (adds CRITICAL) |
| Pearson r (composite score vs DL_Mbps) | -0.083 |

### 4.3 Category Distribution Under Proposed Algorithm

| Predicted Category | N | Avg DL (Mbps) | Min | Max | Throughput Truth Distribution |
|---|---|---|---|---|---|
| EXCELLENT | 37 | 118.7 | 21.4 | 197.6 | 26 EXC, 8 GOOD, 2 FAIR, 1 POOR |
| GOOD | 30 | 121.5 | 17.5 | 205.5 | 20 EXC, 8 GOOD, 2 POOR |
| FAIR | 6 | 119.2 | 103.1 | 143.8 | 6 EXC |
| POOR | 2 | 137.7 | 125.4 | 150.0 | 2 EXC |
| CRITICAL | 1 | 151.4 | 151.4 | 151.4 | 1 EXC |

---

## 5. Alternative Weight Combinations Tested

A comprehensive sweep of all weight combinations (step=5%, sum=100%) was performed:

| Algorithm | Weights (P/Q/S/I) | Exact% | Within-1% | Monotonic | Notes |
|---|---|---|---|---|---|
| RSRP-Only (current) | 100/0/0/0 | 22.4 | 56.6 | 67% | Baseline |
| **Proposed (Option A)** | **35/35/25/5** | **44.7** | **81.6** | 33% | **Recommended** |
| RSRQ-Heavy | 20/50/25/5 | 48.7 | 80.3 | — | Higher exact but less spread |
| RSRQ-Dominant | 5/80/0/15 | 56.6 | 85.5 | 75% | 4+ categories |
| RSSI-Dominant | 0/5/0/95 | 60.5 | 90.8 | 100% | Trivial — 75% in EXCELLENT |
| RSRQ+RSSI | 0/50/0/50 | 59.2 | 89.5 | 67% | Only 4 categories |
| Equal 3-way | 33/33/34/0 | 42.1 | 77.6 | 0% | Poor monotonicity |
| SNR-Only | 0/0/100/0 | 42.1 | 72.4 | — | Missing congestion info |

**Why RSSI-dominant (60.5%) was rejected:** It classifies 75% of devices as EXCELLENT trivially. RSSI includes noise+interference and is the least useful LTE metric. It would not detect congested environments. See also [§9.2 Rationale](#92-rationale) for the final weight selection.

---

## 6. Correlation Analysis — ALL Available Metrics

### 6.1 Pearson Correlation with DL_Mbps (76 devices)

| Metric | Pearson r | |r| | Significance |
|---|---|---|---|
| **Ping_ms (latency)** | **-0.468** | **0.468** | **MODERATE — only useful correlator** |
| PacketLoss_pct | -0.273 | 0.273 | Weak |
| (RSRP+120)×BW (derived) | -0.239 | 0.239 | Weak |
| RSRP (during test) | -0.210 | 0.210 | Weak |
| (SNR+10)×BW (derived) | -0.209 | 0.209 | Weak |
| RSRP (idle) | -0.196 | 0.196 | Weak |
| CA_TotalBW_MHz | -0.190 | 0.190 | Weak |
| RSSI (idle) | -0.178 | 0.178 | Weak |
| SNR delta (test-idle) | 0.165 | 0.165 | Weak |
| RSRQ (during test) | -0.164 | 0.164 | Weak |
| Neighbors_InterFreq | -0.154 | 0.154 | Weak |
| PCI | -0.150 | 0.150 | Weak |
| Neighbors_IntraFreq | -0.142 | 0.142 | Weak |
| SNR (idle) | -0.127 | 0.127 | Weak |
| RSRP delta (test-idle) | -0.108 | 0.108 | None |
| UL_Mbps | 0.089 | 0.089 | None |
| BandBW_MHz | -0.074 | 0.074 | None |
| SNR (during test) | -0.061 | 0.061 | None |
| RSRQ (idle) | 0.011 | 0.011 | None |

### 6.2 Upload Speed Correlation

| Metric | r vs UL_Mbps | r vs DL_Mbps | Notes |
|---|---|---|---|
| RSRP (idle) | -0.280 | -0.196 | Slightly stronger for UL, still wrong direction |
| SNR (idle) | -0.173 | -0.127 | Weak for both |
| RSRQ (idle) | -0.178 | 0.011 | Weak for both |

**UL is also non-monotonic** — POOR category averages 78.4 Mbps UL vs EXCELLENT at 44.1 Mbps. Same lightly-loaded cell effect applies to both directions.

### 6.3 Interpretation

**No RF metric has meaningful correlation with throughput (DL or UL).** The strongest correlator is **Ping latency (r=-0.47)** — a network-path metric reflecting backhaul quality and server distance, not radio conditions.

All radio signal metrics (RSRP, RSRQ, SNR, RSSI) individually and in combination show |r| < 0.28 for both DL and UL.

---

## 7. Data Gaps & Unavailable Metrics

| Metric | Status | Issue |
|---|---|---|
| **Tower Distance (CellDistance_km)** | ❌ Confirmed unavailable | QMI firmware does not expose TA even during active data sessions |
| **TimingAdvance_us** | ❌ Confirmed unavailable | `nas-get-cell-location-info` reports `UE In Idle: yes` and `LTE Timing Advance: unavailable` even while speedtest is actively running. Modem firmware limitation — not a collection timing issue. |
| **Carrier Aggregation (active)** | ❌ Not observed | All devices showed CA=No or No_Deconfigured during idle query |
| **Network Load / PRB Utilization** | ❌ Not available | Not exposed via QMI/HAL — would require eNB-side data |
| **BLER (Block Error Rate)** | ❌ Not accessible | Only available via Qualcomm DIAG interface — not exposed via qmicli or AT commands on production XLE |
| **CQI (Channel Quality Indicator)** | ❌ Not accessible | UE→eNB feedback only; requires Qualcomm DIAG interface — not available on production XLE |
| **MIMO Rank/Layers** | ❌ Not collected | Would indicate spatial multiplexing capability |
| **Scheduler Type / MCS** | ❌ Not available | eNB-side information |
| **Backhaul Quality** | ❌ Not measurable | Would explain the Ping correlation |
| **Cell Load (connected users)** | ❌ Not available | Network-side metric |

### 7.1 Items That Could Improve Analysis (Future Work)

1. ~~**Tower Distance** — Re-collect during active state~~ → **Closed.** QMI firmware reports TA as unavailable even during active speedtest (verified June 4, 2026)
2. ~~**BLER / Retransmission rate**~~ → **Closed.** Not accessible via qmicli — only via Qualcomm DIAG interface (not available on production XLE)
3. ~~**CQI**~~ → **Closed.** Not accessible — UE→eNB feedback only, requires Qualcomm DIAG interface
4. **Test on congested cells** — Current dataset is all low-load; RF metrics would matter more under congestion
5. **Time-of-day variation** — All tests within short window; peak-hour data may show different correlations

---

## 8. Why RF Metrics Don't Predict Throughput (In This Dataset)

### 8.1 Evidence

| Device Example | RSRP | SNR | RadioEnvConditions | Actual DL |
|---|---|---|---|---|
| #93 (POOR) | -113 dBm | 8.2 dB | POOR | **205.5 Mbps** |
| #62 (FAIR) | -102 dBm | 9.0 dB | FAIR | **201.4 Mbps** |
| #88 (POOR) | -117 dBm | -5.2 dB | POOR | **151.4 Mbps** |
| #84 (POOR) | -105 dBm | 6.8 dB | POOR | **186.7 Mbps** |
| #1 (EXCELLENT) | -65 dBm | 30.0 dB | EXCELLENT | 57.3 Mbps |

### 8.2 Explanation

In **lightly-loaded cells** (few simultaneous users), the eNB scheduler allocates maximum resources to the requesting UE regardless of RF quality. The modem can still decode at lower MCS with more retransmissions but still achieves high throughput because:

- Full PRB (Physical Resource Block) allocation available
- No competition for scheduling
- Backhaul capacity is not saturated
- HARQ retransmissions succeed quickly

### 8.3 Congestion Re-Test Validation (June 4, 2026)

A follow-up re-test of 4 devices (one per RF category) at a different time confirmed the finding:

| Category | RSRP | SNR | Re-Test DL (Mbps) | Re-Test UL (Mbps) | Ping (ms) |
|---|---|---|---|---|---|
| EXCELLENT | -58 | 30.0 | 39.04 | 16.28 | 49.55 |
| GOOD | -91 | 8.6 | **12.46** | 18.81 | 56.36 |
| FAIR | -105 | 6.0 | 23.93 | 7.27 | 58.56 |
| POOR | -114 | 0.8 | **37.06** | 2.86 | 63.09 |

**Key finding:** The POOR device (RSRP -114) achieved 3× the DL throughput of the GOOD device (RSRP -91). All 4 devices showed significantly lower throughput than original collection (39 vs 180 Mbps for EXCELLENT), likely due to different speedtest server selection — confirming that **network-path factors dominate over RF quality**.

**Cell congestion cannot be measured from UE side** — PRB utilization, user count, and scheduler data are all eNB-side only. No QMI command exposes these metrics.

### 8.4 Conclusion

`RadioEnvConditions` should classify **RF environment health** (for reliability, handover decisions, failover triggers), NOT predict throughput. The two are only correlated under congestion, which cannot be measured from the UE. See [§9.3](#93-what-radioEnvconditions-tells-the-system) and [§9.4](#94-what-it-does-not-tell) for intended semantics.

---

## 9. Final Recommendation

### 9.1 Adopt Linear Multi-Metric Algorithm (Final)

```c
// Linear interpolation scoring (continuous 0-100)
int rsrp_score = ((rsrp_dbm + 130) * 100) / 60;   // [-130, -70] → [0, 100]
int rsrq_score = ((rsrq_db + 20) * 100) / 15;     // [-20, -5]   → [0, 100]
int snr_score  = ((snr_db + 5) * 100) / 30;       // [-5, 25]    → [0, 100]
int rssi_score = ((rssi_dbm + 100) * 100) / 50;   // [-100, -50] → [0, 100]
// clamp each to [0, 100]

int radio_score = (rsrp_score * 45 + rsrq_score * 30 + snr_score * 20 + rssi_score * 5) / 100;

// Map to condition:
if (radio_score >= 75) return EXCELLENT;
if (radio_score >= 60) return GOOD;
if (radio_score >= 40) return FAIR;
if (radio_score >= 20) return POOR;
return CRITICAL;
```

### 9.2 Rationale

| Criterion | RSRP-Only (Current) | Step Tables (35/35/25/5) | **Linear (45/30/20/5)** |
|---|---|---|---|
| Exact accuracy | 22.4% | 44.7% | **80.8%** |
| Within-1-level | 56.6% | 81.6% | **98.7%** |
| Detects interference (RSRQ) | ❌ No | ✅ Yes (35%) | ✅ Yes (30%) |
| Detects noise/usability (SNR) | ❌ No | ✅ Yes (25%) | ✅ Yes (20%) |
| Cliff-edge behavior | Single threshold jumps | 20-pt jumps at boundaries | **Proportional — no jumps** |
| Backward compatible | — | Same output states (adds CRITICAL) | Same output states (adds CRITICAL) |
| Implementation complexity | 4 if-else | ~30 lines C code | ~30 lines C code |

### 9.3 What RadioEnvConditions Tells the System

| Condition | Meaning | Recommended Action |
|---|---|---|
| EXCELLENT | Strong signal, low interference, low noise | Normal operation |
| GOOD | Reliable LTE, moderate conditions | Normal operation |
| FAIR | Usable but degraded RF environment | Consider monitoring, pre-failover check |
| POOR | Weak signal + interference, high retransmissions expected | Alert, prepare failover |
| CRITICAL | Near radio link failure | Trigger failover if available |

### 9.4 What It Does NOT Tell

- Actual achievable throughput (depends on cell load, backhaul, scheduling)
- Network congestion level (would need PRB utilization or CQI data)
- Whether the connection will drop (depends on mobility/handover state)

---

## 10. Files & Artifacts

| File | Description |
|---|---|
| `xle_radio_env_speedtest_data.csv` | Raw collected data (100 rows, 39 columns) |
| `analyze_radio_env_scoring.py` | Analysis script (weight sweep, correlations, comparisons) |
| `xle_radio_env_scoring_analysis_results.txt` | Full script output |
| `X_RDK_RadioEnvConditions_analysis.md` | Algorithm design document (Option A & B) |
| `X_RDK_RadioEnvConditions_speedtest_analysis.md` | Detailed analysis notes |
| `X_RDK_CellCongestion_Analysis_Report.md` | Cell congestion investigation and re-test results |
| `source/CellularManager/cellularmgr_cellular_apis.h` | Current RSRP-only thresholds |

---

## 11. Next Steps

1. ~~**Tower Distance collection**~~ — **Closed.** QMI firmware confirmed unable to report TA regardless of RRC state
2. ~~**Cell congestion testing**~~ — **Closed.** Re-test of 4 devices confirmed non-correlation persists at different times. Direct congestion metrics (PRB utilization, user count) are not available from UE side.
3. **Implementation** — Apply [final linear algorithm](#91-adopt-linear-multi-metric-algorithm-final) in `cellularmgr_cellular_apis.c`
4. **Optional: Peak-hour validation** — Test during 7–9 PM peak hours on known busy cells if congestion correlation evidence is still desired (low priority — unlikely to change recommendation)
