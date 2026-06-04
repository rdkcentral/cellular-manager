# Band Info Scoring Analysis — Can Band/BW Improve RadioEnvConditions?

**Date:** June 4, 2026  
**Question:** Does adding Band Info (frequency band or channel bandwidth) as a weighted factor improve the RadioEnvConditions scoring algorithm?

---

## Dataset Summary

- 78 devices with valid band info parsed
- 76 successful speedtests (throughput excluded from this analysis)

## Band Distribution

| Band | Devices | Frequency | Typical Use |
|---|---|---|---|
| eutran-4 | 50 (64%) | 1700/2100 MHz (AWS-1) | Primary mid-band |
| eutran-2 | 15 (19%) | 1900 MHz (PCS) | Secondary mid-band |
| eutran-13 | 7 (9%) | 700 MHz | Coverage/propagation |
| eutran-66 | 6 (8%) | 1700/2100 MHz (AWS-3) | Capacity |

## Bandwidth Distribution

| BW | Devices | Percentage |
|---|---|---|
| 10 MHz | 69 | **88%** |
| 20 MHz | 8 | 10% |
| 15 MHz | 1 | 1% |

## RF Metrics by Band (Averages)

| Band | N | Avg RSRP | Avg RSRQ | Avg SNR | Avg RSSI |
|---|---|---|---|---|---|
| eutran-13 | 7 | -93.7 | -11.6 | 11.7 | -62.9 |
| eutran-2 | 15 | -82.9 | -10.7 | 15.2 | -55.3 |
| eutran-4 | 50 | -92.9 | -10.4 | 13.9 | -62.3 |
| eutran-66 | 6 | -97.2 | -12.2 | 11.4 | -70.7 |

## Band vs RSRP Category Crosstab

| Band | EXCELLENT | GOOD | FAIR | POOR | Total |
|---|---|---|---|---|---|
| eutran-13 | 2 | 1 | 2 | 2 | 7 |
| eutran-2 | 8 | 2 | 4 | 1 | 15 |
| eutran-4 | 13 | 12 | 16 | 9 | 50 |
| eutran-66 | 0 | 1 | 5 | 0 | 6 |

## Band Scoring Approaches Tested

Three band-scoring functions were evaluated:

1. **v1-propagation**: Rewards lower frequency (better propagation/coverage)
   - Band 13→90, Band 2→75, Band 4→70, Band 66→65 (+BW bonus)

2. **v2-capacity**: Rewards higher frequency (more capacity potential)
   - Band 66→90, Band 4→80, Band 2→75, Band 13→60 (+BW bonus)

3. **v3-bw-only**: Pure bandwidth-based (most objective)
   - BW≥20→100, BW≥15→80, BW≥10→60, else→40

## Results: Accuracy vs RSRP-Only Ground Truth

| Config (RSRP/RSRQ/SNR/RSSI/Band) | Band Scoring | Exact% | Within-1% |
|---|---|---|---|
| **35/35/25/5/0 (baseline, no band)** | none | **33.3%** | **91.0%** |
| 30/30/25/5/10 | v1-propagation | 34.6% | 91.0% |
| 30/30/25/5/10 | v2-capacity | 35.9% | 84.6% |
| 30/30/25/5/10 | **v3-bw-only** | **42.3%** | **91.0%** |
| 30/30/20/5/15 | v1-propagation | 35.9% | 91.0% |
| 30/30/20/5/15 | v2-capacity | 37.2% | 88.5% |
| 30/30/20/5/15 | v3-bw-only | 42.3% | 89.7% |
| 30/30/15/5/20 | v1-propagation | 39.7% | 91.0% |
| 30/30/15/5/20 | v2-capacity | 37.2% | 88.5% |
| 30/30/15/5/20 | v3-bw-only | 42.3% | 88.5% |
| 25/25/25/5/20 | v1-propagation | 39.7% | 91.0% |
| 25/25/25/5/20 | v2-capacity | 37.2% | 83.3% |
| 25/25/25/5/20 | v3-bw-only | 42.3% | 88.5% |
| 35/35/15/5/10 | v1-propagation | 34.6% | 91.0% |
| 35/35/15/5/10 | v2-capacity | 34.6% | 89.7% |
| 35/35/15/5/10 | v3-bw-only | 37.2% | 89.7% |

## Score Transitions (30/30/25/5/10, v3-bw-only)

| From (no band) | To (with band) | Count | Changed? |
|---|---|---|---|
| EXCELLENT | EXCELLENT | 33 | |
| EXCELLENT | GOOD | 5 | *CHANGED* |
| GOOD | GOOD | 31 | |
| FAIR | FAIR | 6 | |
| POOR | FAIR | 1 | *CHANGED* |
| POOR | POOR | 1 | |
| CRITICAL | POOR | 1 | *CHANGED* |

**Total changed: 7/78 (9.0%)**

## Why Band Info Does NOT Improve the Algorithm

### 1. Bandwidth is effectively constant
- 88% of devices operate on 10 MHz bandwidth
- Band score becomes a constant (60 for v3-bw-only) across nearly all devices
- Adding a constant dilutes real RF metric differentiation

### 2. Band identity doesn't correlate with RF quality
- All bands show devices across all RSRP categories (EXCELLENT through POOR)
- Average RSRP varies only 14 dB across bands (-82.9 to -97.2)
- No band is inherently "better RF environment"

### 3. Exact-match improvement is misleading
- The 42.3% (vs 33.3%) improvement comes from pulling borderline EXCELLENT scores down to GOOD
- This happens because adding a constant BW=60 score (below the 80 threshold) deflates otherwise-high composite scores
- It's not detecting a real signal — it's an artifact of dilution

### 4. Within-1 stays flat or degrades
- Best case: 91.0% within-1 (same as baseline)
- Many configs degrade within-1 to 83-89%
- v2-capacity approach actually HURTS accuracy

## Conclusion

**Band Info should NOT be included in the RadioEnvConditions algorithm** because:

1. In this deployment, bandwidth diversity is too low (88% on 10 MHz) to provide differentiation
2. Band/BW is a capacity metric, not an RF quality metric — it belongs in a separate `LinkCapacityScore` if needed
3. Adding it dilutes the meaningful RF signals (RSRP, RSRQ, SNR)
4. The 35/35/25/5 formula remains optimal for RF environment health assessment

## Recommendation

Keep RadioEnvConditions as a pure RF quality indicator:
```
RADIO_SCORE = (RSRP_SCORE × 35 + RSRQ_SCORE × 35 + SNR_SCORE × 25 + RSSI_SCORE × 5) / 100
```

If band-aware throughput prediction is needed in the future, introduce a separate TR-181 parameter (e.g., `X_RDK_LinkCapacityScore`).
