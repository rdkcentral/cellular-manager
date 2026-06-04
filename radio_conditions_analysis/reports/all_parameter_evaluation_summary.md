# All Parameters Evaluation Summary — RadioEnvConditions Optimization

**Date:** June 4, 2026  
**Status:** COMPLETE — All investigations closed  
**Objective:** Exhaustive evaluation of all available parameters to optimize the RadioEnvConditions scoring algorithm. Started from RSRP-only baseline → step-table 35/35/25/5 → **final: linear interpolation 45/30/20/5**.

---

## Updated Recommended Formula

```
Linear interpolation ranges:
  RSRP_score = clamp((RSRP - (-130)) / (-70 - (-130)) × 100, 0, 100)
  RSRQ_score = clamp((RSRQ - (-20)) / (-5 - (-20)) × 100, 0, 100)
  SNR_score  = clamp((SNR - (-5)) / (25 - (-5)) × 100, 0, 100)
  RSSI_score = clamp((RSSI - (-100)) / (-50 - (-100)) × 100, 0, 100)

RADIO_SCORE = (RSRP_score × 45 + RSRQ_score × 30 + SNR_score × 20 + RSSI_score × 5) / 100
```

Classification: EXCELLENT (≥75), GOOD (≥60), FAIR (≥40), POOR (≥20), CRITICAL (<20)

### How Linear Interpolation Works

The formula maps each raw metric value to a 0–100 score using a straight line between a "worst" endpoint (score=0) and a "best" endpoint (score=100). `clamp` ensures the result stays within [0, 100].

**Simplified forms:**

| Metric | Worst (score=0) | Best (score=100) | Span | Simplified Formula |
|---|---|---|---|---|
| RSRP | -130 dBm | -70 dBm | 60 dB | `(RSRP + 130) / 60 × 100` |
| RSRQ | -20 dB | -5 dB | 15 dB | `(RSRQ + 20) / 15 × 100` |
| SNR | -5 dB | 25 dB | 30 dB | `(SNR + 5) / 30 × 100` |
| RSSI | -100 dBm | -50 dBm | 50 dB | `(RSSI + 100) / 50 × 100` |

**Worked examples (RSRP):**

```
RSRP = -85 dBm:  score = (-85 + 130) / 60 × 100 = 45/60 × 100 = 75.0  → EXCELLENT boundary
RSRP = -100 dBm: score = (-100 + 130) / 60 × 100 = 30/60 × 100 = 50.0 → FAIR
RSRP = -70 dBm:  score = (-70 + 130) / 60 × 100 = 60/60 × 100 = 100.0 → capped at 100
RSRP = -130 dBm: score = (-130 + 130) / 60 × 100 = 0/60 × 100 = 0.0   → floor
RSRP = -50 dBm:  score = 133.3 → clamped to 100
```

**Visual (RSRP score curve):**

```
Score
100 |                              ●━━━━━━━ (clamped at 100 for ≥ -70)
    |                           ╱
 75 |.......................╱......... ← EXCELLENT threshold (RSRP = -85)
    |                    ╱
 60 |................╱................ ← GOOD threshold (RSRP = -94)
    |             ╱
 40 |.........╱....................... ← FAIR threshold (RSRP = -106)
    |      ╱
 20 |...╱............................. ← POOR threshold (RSRP = -118)
    | ╱
  0 ●━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━ (clamped at 0 for ≤ -130)
    |_____|_____|_____|_____|_____|____
   -130  -118  -106   -94   -82   -70   RSRP (dBm)
```

**Why this is better than step tables:** A 2 dB difference (e.g., RSRP=-86 vs -84) produces a proportional 3.3-point score change, rather than a sudden 20-point jump at a boundary.

**How range endpoints were chosen:**

| Metric | Min (score=0) | Max (score=100) | Rationale |
|---|---|---|---|
| RSRP: -130 / -70 | Noise floor / undetectable | Very strong near tower | 3GPP range [-140, -44] narrowed to practical LTE |
| RSRQ: -20 / -5 | Extremely congested | Clean channel | Matches 3GPP reporting range [-19.5, -3] |
| SNR: -5 / 25 | Unusable link | 64QAM capable | Above 25 dB gives no modulation benefit |
| RSSI: -100 / -50 | Very weak total signal | Very strong total signal | Covers typical field range |

**C implementation:**

```c
int rsrp_score = ((rsrp_dbm + 130) * 100) / 60;
if (rsrp_score > 100) rsrp_score = 100;
if (rsrp_score < 0)   rsrp_score = 0;
```

### Previous Formula (Step Tables — superseded)

```
RADIO_SCORE = (RSRP_SCORE × 35 + RSRQ_SCORE × 35 + SNR_SCORE × 25 + RSSI_SCORE × 5) / 100
```

Classification: EXCELLENT (≥80), GOOD (≥60), FAIR (≥40), POOR (≥20), CRITICAL (<20)

---

## Parameters Fully Evaluated (No Improvement Found)

| Parameter | Source | Verdict | Reason for Exclusion |
|---|---|---|---|
| RSSI (high weight) | qmicli signal-info | ❌ Excluded at 25% | Systematic upward bias (+14.4 pts avg vs RSRP); 85% of devices score ≥80; redundant with RSRP (r=0.87) but inflates scores |
| Band/Frequency | qmicli rf-band-info | ❌ No differentiation | 88% of devices on 10 MHz BW; band score becomes a constant; only 9% of classifications change |
| Bandwidth (MHz) | qmicli cell-location | ❌ No differentiation | Same as above — nearly all devices on same BW |
| Carrier Aggregation | qmicli ca-info | ❌ Capacity metric | Not an RF quality indicator; most devices show "no CA" or "deconfigured" in idle |
| Tower Distance / TA | qmicli cell-location | ❌ Unavailable | Firmware limitation: `LTE Timing Advance: unavailable` even during active speedtest; only 3/76 devices had data |
| Throughput (DL/UL) | speedtest-cli | ❌ Wrong purpose | All |r| < 0.28; RF ≠ throughput in lightly-loaded cells; RadioEnvConditions should reflect RF health, not speed |
| Ping latency | speedtest-cli | ❌ Network-path metric | r=-0.47 with DL but measures backhaul/server, not radio environment |
| ECIO | qmicli signal-strength | ❌ Identical across devices | Same value (-2.5) on EXCELLENT and POOR devices |
| IO (interference+noise) | qmicli signal-strength | ❌ Identical across devices | Same value (-106) on EXCELLENT and POOR devices |
| SINR (QMI level 0-8) | qmicli signal-strength | ❌ Too coarse | Only 9 discrete levels; caps at level 8; same on EXCELLENT and POOR |
| TX Power | qmicli tx-rx-info | ❌ Always 0 | Proxy mode (qmicli -p) cannot observe active TX state; always shows idle/0 |
| RX Diversity Delta | qmicli tx-rx-info | ❌ Counterintuitive | POOR device had BETTER diversity (1.6 dB) than EXCELLENT (6.5 dB) |
| Channel Rates | qmicli wds | ❌ Always 0 | qmicli queries different WDS client than cellular-manager's active connection |
| Dormancy Status | qmicli wds | ❌ Always dormant | Same WDS client issue as channel rates |
| Neighbor Cell Count | qmicli cell-location | ❌ Weak correlation | |r| < 0.15 with throughput; count alone doesn't indicate quality |
| Packet Loss % | speedtest-cli | ❌ Weak, wrong domain | |r| = 0.27; network-path metric, not radio |
| Packet Statistics | qmicli wds | ❌ Stale/cached | Always shows initial 4 packets from cellular-manager setup |
| dmcli X_RDK_BandInfo | dmcli | ❌ Can be stale | Caches band from initial connection; doesn't update after handover |
| Balanced weights (25/25/25/25) | Analysis | ❌ Over-classifies | RSSI inflation causes 58% EXCELLENT; within-1 drops to 83.3% (vs 91%) |

---

## Remaining Avenues to Explore

### Investigated with Existing Data

| # | Investigation | Result | Details |
|---|---|---|---|
| 1 | **Linear Interpolation (Option B)** vs Step Tables | **Linear is significantly better** | See section below |
| 2 | **Step-table threshold tuning** | Not needed — Linear eliminates boundary artifacts | Thresholds 75/60/40/20 align naturally |

### Requires Device Access / Metric Availability Check (CLOSED)

| # | Investigation | Result | Details |
|---|---|---|---|
| 3 | **Neighbor cell RSRP quality** | ❌ Low value | Code exposes via `X_RDK_NeighborCell.{i}.ReceivedSignal` but measures handover potential, not current link quality. Neighbor count correlation was |r|<0.15. |
| 4 | **CQI (Channel Quality Indicator)** | ❌ Not accessible | CQI is UE→eNB feedback (not stored locally). Only available via Qualcomm DIAG interface — not exposed via qmicli or dmcli on production XLE. |
| 5 | **BLER (Block Error Rate)** | ❌ Not accessible | MAC-layer metric computed by modem firmware. Only available via Qualcomm DIAG or AT commands — neither interface available on production XLE. |

**Additional finding:** Serving RSRP during connected mode (test) vs idle differs by only 0.45 dB avg (±1.6 dB std) — effectively redundant with idle RSRP.

---

## Investigation 1 Results: Linear Interpolation vs Step Tables

### Controlled Comparison (Same weights, same thresholds — method only changes)

| Configuration | Method | Exact% | Within-1% | EXC | GOOD | FAIR | POOR | CRIT |
|---|---|---|---|---|---|---|---|---|
| 35/35/25/5, T=80/60/40/20 | Step | 33.3% | 91.0% | 38 | 31 | 6 | 2 | 1 |
| 35/35/25/5, T=80/60/40/20 | **Linear** | **73.1%** | **98.7%** | 18 | 22 | 27 | 10 | 1 |
| 45/30/20/5, T=80/60/40/20 | Step | 35.9% | 93.6% | 38 | 29 | 8 | 2 | 1 |
| 45/30/20/5, T=80/60/40/20 | **Linear** | **79.5%** | **98.7%** | 19 | 21 | 27 | 10 | 1 |
| 35/35/25/5, T=75/60/40/20 | Step | 33.3% | 83.3% | 44 | 25 | 6 | 2 | 1 |
| 35/35/25/5, T=75/60/40/20 | **Linear** | **74.4%** | **98.7%** | 23 | 17 | 27 | 10 | 1 |
| 45/30/20/5, T=75/60/40/20 | Step | 35.9% | 85.9% | 44 | 23 | 8 | 2 | 1 |
| 45/30/20/5, T=75/60/40/20 | **Linear** | **80.8%** | **98.7%** | 24 | 16 | 27 | 10 | 1 |

**Actual distribution:** EXCELLENT=23, GOOD=16, FAIR=27, POOR=12

### Isolated Effects

**Effect of Method (Step → Linear), holding everything else constant:**

| Weights | Thresholds | Step Exact% | Linear Exact% | Improvement |
|---|---|---|---|---|
| 35/35/25/5 | 80/60/40/20 | 33.3% | 73.1% | **+39.8%** |
| 45/30/20/5 | 80/60/40/20 | 35.9% | 79.5% | **+43.6%** |
| 35/35/25/5 | 75/60/40/20 | 33.3% | 74.4% | **+41.1%** |
| 45/30/20/5 | 75/60/40/20 | 35.9% | 80.8% | **+44.9%** |

Switching from Step to Linear **consistently adds ~40-45% exact accuracy** regardless of weights or thresholds.

**Effect of Weights (35/35/25/5 → 45/30/20/5), holding method constant:**

| Method | Thresholds | 35/35/25/5 | 45/30/20/5 | Improvement |
|---|---|---|---|---|
| Step | 80/60/40/20 | 33.3% | 35.9% | +2.6% |
| Linear | 80/60/40/20 | 73.1% | 79.5% | +6.4% |
| Step | 75/60/40/20 | 33.3% | 35.9% | +2.6% |
| Linear | 75/60/40/20 | 74.4% | 80.8% | +6.4% |

Weight change adds a modest 2-6% improvement.

### Why Step Tables Fail

Step tables create **discrete jumps** that misalign with classification boundaries:
- A device at RSRP=-86 gets step score 80, while RSRP=-84 gets 100 — a 20-point jump for 2 dB difference
- This quantization pushes many devices into EXCELLENT/GOOD that should be borderline
- Step tables produce 38-44 EXCELLENT (vs 23 actual) — massive over-classification

Linear interpolation produces **proportional scores** that naturally spread across the full range, matching the actual distribution much better (24 EXCELLENT vs 23 actual).

### Why Linear Favors Higher RSRP Weight (45% vs 35%)

Since ground truth IS RSRP-only, higher RSRP weight naturally aligns better. But the key question is: **does multi-metric still add value over pure RSRP?**

| Configuration | Exact% | W1% |
|---|---|---|
| Linear, pure RSRP only (100/0/0/0), T=75 | 94.9% | 100% |
| Linear, 45/30/20/5, T=75 | 80.8% | 98.7% |

Multi-metric is intentionally **less accurate** vs the RSRP-only ground truth because it detects 3 cases where RSRP disagrees with actual RF quality:
- Device #20: RSRP=-84 (EXCELLENT) but RSRQ=-16 (interference!)
- Device #26: RSRP=-87 (GOOD) but RSRQ=-16 (congestion risk)
- Device #62: RSRP=-95 (GOOD boundary) but RSRQ=-16 (congestion risk)

These RSRQ-degraded devices should NOT be rated as highly as their RSRP suggests. Under network load, they would experience significant quality degradation that RSRP alone cannot predict.

### Updated Recommendation

**Best configuration: Linear Interpolation, 45/30/20/5 weights, 75/60/40/20 thresholds**

```
Linear ranges:
  RSRP: [-130, -70] dBm → [0, 100]
  RSRQ: [-20, -5] dB   → [0, 100]
  SNR:  [-5, 25] dB    → [0, 100]
  RSSI: [-100, -50] dBm → [0, 100]

RADIO_SCORE = (RSRP_linear × 45 + RSRQ_linear × 30 + SNR_linear × 20 + RSSI_linear × 5) / 100

Classification:
  EXCELLENT: score ≥ 75
  GOOD:      score ≥ 60
  FAIR:      score ≥ 40
  POOR:      score ≥ 20
  CRITICAL:  score < 20
```

---

## Key Conclusions — FINAL

1. **Only 4 metrics provide meaningful RF quality differentiation**: RSRP, RSRQ, SNR, RSSI
2. **Linear interpolation is dramatically better than step tables** (+40-45% accuracy improvement)
3. **RSRP deserves highest weight (45%)** as the primary signal strength indicator
4. **RSRQ (30%) detects interference/congestion** that RSRP alone misses
5. **SNR (20%) adds operational quality** but is volatile
6. **RSSI (5%) is the least informative** — tiebreaker only
7. **No additional metrics are available or useful** — CQI/BLER not accessible, band/BW/CA/TA don't differentiate, neighbor count is too weak
8. **All investigations are CLOSED** — the formula is final

---

## Reports Index

| Report | Content |
|---|---|
| `X_RDK_BandInfo_Scoring_Analysis.md` | Band/BW scoring evaluation (concluded: no value) |
| `X_RDK_WeightComparison_35_vs_25_Analysis.md` | 35/35/25/5 vs 25/25/25/25 comparison |
| `X_RDK_RadioEnvConditions_Analysis_Report.md` | Main analysis report (executive summary) |
| `X_RDK_CellCongestion_Analysis_Report.md` | Cell congestion investigation |
| `X_RDK_RadioEnvConditions_speedtest_analysis.md` | Detailed speedtest & metrics analysis |
| `X_RDK_RadioEnvConditions_analysis.md` | Algorithm design document (Option A & B) |
