# Weight Comparison: 35/35/25/5 vs 25/25/25/25 (Balanced)

**Date:** June 4, 2026  
**Question:** Why is the proposed 35/35/25/5 weighting better than a balanced 25/25/25/25?

---

## Head-to-Head Accuracy

| Metric | 35/35/25/5 | 25/25/25/25 |
|---|---|---|
| Exact Match % | 33.3% | 33.3% |
| **Within-1-Level %** | **91.0%** | **83.3%** |

Both have the same exact-match rate, but **35/35/25/5 has significantly better within-1 accuracy** (91% vs 83%), meaning balanced weights produce more severe misclassifications (off by 2+ levels).

### What "Within-1-Level" Means

Categories are assigned numeric levels: EXCELLENT(4) > GOOD(3) > FAIR(2) > POOR(1) > CRITICAL(0).

- **Exact match**: predicted = actual (e.g., predicted GOOD, actual GOOD)
- **Within-1**: |predicted_level − actual_level| ≤ 1 (e.g., predicted GOOD, actual FAIR — off by 1, acceptable)
- **Outside within-1**: off by 2+ levels (e.g., predicted EXCELLENT, actual FAIR — unacceptable)

A high within-1 score means the algorithm rarely produces "wildly wrong" classifications. The 91% vs 83.3% gap shows that 25/25/25/25 produces nearly **twice as many** 2+ level errors (17% vs 9% of devices).

---

## Category Distribution

| Category | Actual (RSRP-only) | 35/35/25/5 | 25/25/25/25 |
|---|---|---|---|
| EXCELLENT | 23 | 38 | **45** |
| GOOD | 16 | 31 | 25 |
| FAIR | 27 | 6 | 6 |
| POOR | 12 | 2 | 2 |
| CRITICAL | 0 | 1 | 0 |

**25/25/25/25 classifies 58% of devices as EXCELLENT** — a clear over-classification problem.

---

## Score Distribution (Numeric)

| Stat | 35/35/25/5 | 25/25/25/25 |
|---|---|---|
| Mean | 78.5 | 80.4 |
| Median | 79.8 | 81.2 |
| Std Dev | 19.3 | 17.6 |
| Min | 11.2 | 21.2 |
| Max | 100.0 | 100.0 |
| Range | 88.8 | 78.8 |

25/25/25/25 compresses the score range (78.8 vs 88.8) and shifts the distribution upward, reducing the algorithm's ability to differentiate device conditions.

---

## Disagreements: Where They Classify Differently

10 out of 78 devices (12.8%) are classified differently:

| # | RSRP | RSRQ | SNR | RSSI | Actual | 35/35/25/5 | 25/25/25/25 | Closer to Actual |
|---|---|---|---|---|---|---|---|---|
| 26 | -87 | -16.0 | 19.6 | -56 | GOOD | GOOD | EXCELLENT | **35/35/25/5** |
| 41 | -99 | -11.0 | 18.0 | -70 | FAIR | GOOD | EXCELLENT | **35/35/25/5** |
| 52 | -97 | -11.0 | 17.2 | -74 | FAIR | GOOD | EXCELLENT | **35/35/25/5** |
| 54 | -98 | -12.0 | 6.6 | -63 | FAIR | GOOD | EXCELLENT | **35/35/25/5** |
| 58 | -99 | -11.0 | 15.4 | -70 | FAIR | GOOD | EXCELLENT | **35/35/25/5** |
| 60 | -101 | -16.0 | 5.4 | -71 | FAIR | FAIR | GOOD | **35/35/25/5** |
| 61 | -99 | -19.0 | -2.2 | -63 | FAIR | POOR | FAIR | 25/25/25/25 |
| 63 | -101 | -10.0 | 16.4 | -70 | FAIR | GOOD | EXCELLENT | **35/35/25/5** |
| 71 | -117 | -20.0 | -5.2 | -77 | POOR | CRITICAL | POOR | 25/25/25/25 |
| 73 | -103 | -11.0 | 14.8 | -66 | FAIR | GOOD | EXCELLENT | **35/35/25/5** |

**Score: 35/35/25/5 wins 8, 25/25/25/25 wins 2**

The pattern is clear: 25/25/25/25 over-promotes FAIR devices to EXCELLENT (7 of 10 disagreements).

---

## Root Cause: RSSI Inflation

### Individual Metric Score Distributions

| Metric | Avg Score | Scoring ≥80 | Scoring ≤40 |
|---|---|---|---|
| RSRP | 72.6 | 54% | 13% |
| RSRQ | 82.1 | 73% | 12% |
| SNR | 80.2 | 51% | 5% |
| **RSSI** | **86.9** | **85%** | **0%** |

RSSI scores are **heavily right-skewed**: 85% of devices score ≥80 on RSSI, and **zero** devices score ≤40. This makes RSSI an upward-biasing constant when given significant weight.

### RSSI vs RSRP Score Comparison (Per-Device)

| Comparison | Count | Percentage |
|---|---|---|
| RSSI score > RSRP score | 47/78 | 60% |
| RSSI score = RSRP score | 31/78 | 40% |
| RSSI score < RSRP score | 0/78 | **0%** |
| **Average delta** | **+14.4 points** | (RSSI always inflates) |

RSSI **never** scores lower than RSRP on the same device, and averages 14.4 points higher. At 25% weight, this adds ~3.6 points of systematic upward bias to every score.

### Correlation Between RSSI and RSRP Scores

- Pearson r = 0.871 (highly correlated)
- RSSI is **partially redundant** with RSRP but with a constant positive offset
- Giving both equal weight effectively double-counts signal strength with an upward bias

---

## Worked Example

**Device: RSRP=-99, RSRQ=-11, SNR=18, RSSI=-70 (Actual: FAIR)**

| Metric | Score | Meaning |
|---|---|---|
| RSRP | 60 | FAIR-range signal |
| RSRQ | 85 | Decent quality |
| SNR | 100 | Good SNR |
| RSSI | 80 | High (RSSI is less sensitive to degradation) |

**35/35/25/5:** (60×35 + 85×35 + 100×25 + 80×5) / 100 = **79.8 → GOOD** (off by 1)  
**25/25/25/25:** (60×25 + 85×25 + 100×25 + 80×25) / 100 = **81.2 → EXCELLENT** (off by 2)

The +20 point RSSI-RSRP delta, amplified by 25% weight, pushes the balanced score over the EXCELLENT threshold.

---

## Why RSSI Should Have Minimal Weight

**RSSI** (Received Signal Strength Indicator) = total wideband received power, **including** interference and noise.

| Scenario | RSRP | RSSI | RSRQ | SNR | True Quality |
|---|---|---|---|---|---|
| Strong signal, no interference | -70 | -40 | -6 | 25 | EXCELLENT |
| Strong interference + signal | -100 | -45 | -18 | -2 | POOR |
| Weak signal, no interference | -110 | -80 | -10 | 8 | FAIR |

In the second scenario, RSSI is **high** (-45) because it includes interference energy, but actual quality is POOR. RSSI cannot distinguish signal from noise/interference — it's a crude legacy metric from 2G/3G.

**Proper role of RSSI:** Tiebreaker only (5% weight). Useful when RSRP/RSRQ/SNR are identical between two devices — the one with higher RSSI has slightly more energy budget.

---

## Why RSRP and RSRQ Deserve Equal Top Weight (35% each)

| Metric | What It Measures | Why Important |
|---|---|---|
| RSRP | Per-subcarrier reference signal power | Direct measure of path loss / cell distance |
| RSRQ | RSRP / (N × RSSI) — quality ratio | Detects interference, congestion, resource loading |

- **RSRP alone** misses interference (a device near tower but in heavy interference shows good RSRP, bad RSRQ)
- **RSRQ alone** misses signal strength (a weak signal with no interference shows bad RSRP, good RSRQ)
- **Together at equal weight** they cover both failure modes: distance degradation AND interference degradation

## Why SNR Gets 25%

SNR (Signal-to-Noise Ratio) is the **operational quality** metric — it directly determines modulation order (and thus spectral efficiency). However:
- It's partially derived from RSRP and noise floor (correlated with RSRP/RSRQ)
- It's the most volatile metric (fluctuates ±5 dB over seconds)
- 25% weight gives it influence without letting short-term variance dominate

---

## Conclusion

| Factor | 35/35/25/5 | 25/25/25/25 |
|---|---|---|
| Within-1 accuracy | **91.0%** | 83.3% |
| EXCELLENT over-classification | 38/78 (49%) | **45/78 (58%)** |
| Score range (discrimination) | **88.8** | 78.8 |
| Wins on disagreements | **8/10** | 2/10 |
| RSSI inflation effect | Negligible (5%) | **Significant (25%)** |

**35/35/25/5 is the better choice** because it:
1. Limits RSSI's systematic upward bias to a negligible 5%
2. Gives proper emphasis to the two most informative LTE metrics (RSRP + RSRQ)
3. Maintains wider score spread for better category separation
4. Produces fewer severe misclassifications (2+ levels off)

---

## Epilogue: Evolution to Final Algorithm

This analysis was performed using **step tables** (discrete score buckets). Subsequent investigation showed that **linear interpolation** eliminates the boundary artifacts that cause step-table over-classification, improving accuracy by +40-45%.

The final recommended algorithm is:

```
Linear interpolation:
  RSRP_score = clamp((RSRP + 130) / 60 × 100, 0, 100)   range: [-130, -70] dBm
  RSRQ_score = clamp((RSRQ + 20) / 15 × 100, 0, 100)    range: [-20, -5] dB
  SNR_score  = clamp((SNR + 5) / 30 × 100, 0, 100)      range: [-5, 25] dB
  RSSI_score = clamp((RSSI + 100) / 50 × 100, 0, 100)   range: [-100, -50] dBm

RADIO_SCORE = (RSRP_score × 45 + RSRQ_score × 30 + SNR_score × 20 + RSSI_score × 5) / 100

Classification: EXCELLENT(≥75), GOOD(≥60), FAIR(≥40), POOR(≥20), CRITICAL(<20)
```

**Key changes from this report's 35/35/25/5 step-table formula:**

| Aspect | This Report (Step) | Final (Linear) |
|---|---|---|
| Scoring method | 5-level step table | Continuous linear interpolation |
| Weights | 35/35/25/5 | 45/30/20/5 |
| EXCELLENT threshold | ≥80 | ≥75 |
| Exact accuracy | 33.3% | **80.8%** |
| Within-1 accuracy | 91.0% | **98.7%** |

The RSSI inflation problem identified here (§Root Cause) is even less impactful with the final algorithm because linear scoring produces more proportional RSSI values and the weight remains minimal at 5%.

See `all_parameter_evaluation_summary.md` for the complete final recommendation.
