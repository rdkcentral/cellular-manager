#!/usr/bin/env python3
"""
Analysis script for X_RDK_RadioEnvConditions scoring optimization.

Uses the existing RadioEnvConditions (RSRP-only) from the device as baseline.
Applies the proposed weighted formula from X_RDK_RadioEnvConditions_analysis.md.
Then evaluates other parameter combinations to find the best throughput predictor.
"""

import csv
import statistics
from collections import defaultdict

CSV_FILE = "xle_radio_env_speedtest_data.csv"
OUTPUT_CSV = "xle_radio_env_scoring_analysis.csv"


# ===========================================================================
# SCORING ALGORITHMS
# ===========================================================================

# --- Proposed Algorithm: Option A (Step Tables) from analysis doc ---
def step_rsrp_score(rsrp):
    if rsrp > -85: return 100
    if rsrp >= -95: return 80
    if rsrp >= -105: return 60
    if rsrp >= -115: return 30
    return 10

def step_rsrq_score(rsrq):
    if rsrq > -10: return 100
    if rsrq >= -12: return 85
    if rsrq >= -15: return 70
    if rsrq >= -17: return 40
    return 10

def step_snr_score(snr):
    if snr > 13: return 100
    if snr >= 5: return 75
    if snr >= 1: return 50
    if snr >= -3: return 25
    return 5

def step_rssi_score(rssi):
    if rssi > -65: return 100
    if rssi >= -75: return 80
    if rssi >= -85: return 60
    if rssi >= -95: return 40
    return 20


def weighted_score_option_a(rsrp, rsrq, snr, rssi):
    """Proposed weighted formula with Option A step tables.
    RSRP*35 + RSRQ*35 + SNR*25 + RSSI*5"""
    if rsrp is None:
        return None, None
    s_rsrp = step_rsrp_score(rsrp)
    s_rsrq = step_rsrq_score(rsrq) if rsrq is not None else 50
    s_snr = step_snr_score(snr) if snr is not None else 50
    s_rssi = step_rssi_score(rssi) if rssi is not None else 50
    score = (s_rsrp * 35 + s_rsrq * 35 + s_snr * 25 + s_rssi * 5) / 100
    score = max(0, min(100, score))
    if score >= 80: condition = "EXCELLENT"
    elif score >= 60: condition = "GOOD"
    elif score >= 40: condition = "FAIR"
    elif score >= 20: condition = "POOR"
    else: condition = "CRITICAL"
    return score, condition


# --- Proposed Algorithm: Option B (Linear Interpolation) from analysis doc ---
def linear_score(value, min_val, max_val):
    if value >= max_val: return 100
    if value <= min_val: return 0
    return ((value - min_val) * 100) / (max_val - min_val)


def weighted_score_option_b(rsrp, rsrq, snr, rssi):
    """Proposed weighted formula with Option B linear interpolation.
    RSRP*35 + RSRQ*35 + SNR*25 + RSSI*5"""
    if rsrp is None:
        return None, None
    s_rsrp = linear_score(rsrp, -120, -70)
    s_rsrq = linear_score(rsrq, -20, -5) if rsrq is not None else 50
    s_snr = linear_score(snr, -10, 25) if snr is not None else 50
    s_rssi = linear_score(rssi, -100, -50) if rssi is not None else 50
    score = (s_rsrp * 35 + s_rsrq * 35 + s_snr * 25 + s_rssi * 5) / 100
    score = max(0, min(100, score))
    if score >= 80: condition = "EXCELLENT"
    elif score >= 60: condition = "GOOD"
    elif score >= 40: condition = "FAIR"
    elif score >= 20: condition = "POOR"
    else: condition = "CRITICAL"
    return score, condition


# --- Alternative: SNR-heavy weighting ---
def weighted_snr_heavy(rsrp, rsrq, snr, rssi):
    """SNR-heavy: RSRP*20 + RSRQ*20 + SNR*50 + RSSI*10"""
    if rsrp is None:
        return None, None
    s_rsrp = step_rsrp_score(rsrp)
    s_rsrq = step_rsrq_score(rsrq) if rsrq is not None else 50
    s_snr = step_snr_score(snr) if snr is not None else 50
    s_rssi = step_rssi_score(rssi) if rssi is not None else 50
    score = (s_rsrp * 20 + s_rsrq * 20 + s_snr * 50 + s_rssi * 10) / 100
    score = max(0, min(100, score))
    if score >= 80: condition = "EXCELLENT"
    elif score >= 60: condition = "GOOD"
    elif score >= 40: condition = "FAIR"
    elif score >= 20: condition = "POOR"
    else: condition = "CRITICAL"
    return score, condition


# --- Alternative: SNR-only ---
def snr_only_score(snr):
    if snr is None: return None, None
    s = step_snr_score(snr)
    if s >= 80: condition = "EXCELLENT"
    elif s >= 60: condition = "GOOD"
    elif s >= 40: condition = "FAIR"
    elif s >= 20: condition = "POOR"
    else: condition = "CRITICAL"
    return s, condition


# --- Alternative: RSRQ-heavy ---
def weighted_rsrq_heavy(rsrp, rsrq, snr, rssi):
    """RSRQ-heavy: RSRP*20 + RSRQ*50 + SNR*25 + RSSI*5"""
    if rsrp is None:
        return None, None
    s_rsrp = step_rsrp_score(rsrp)
    s_rsrq = step_rsrq_score(rsrq) if rsrq is not None else 50
    s_snr = step_snr_score(snr) if snr is not None else 50
    s_rssi = step_rssi_score(rssi) if rssi is not None else 50
    score = (s_rsrp * 20 + s_rsrq * 50 + s_snr * 25 + s_rssi * 5) / 100
    score = max(0, min(100, score))
    if score >= 80: condition = "EXCELLENT"
    elif score >= 60: condition = "GOOD"
    elif score >= 40: condition = "FAIR"
    elif score >= 20: condition = "POOR"
    else: condition = "CRITICAL"
    return score, condition


# --- Alternative: Equal weight RSRP+SNR+RSRQ (no RSSI) ---
def equal_three(rsrp, rsrq, snr, rssi):
    """Equal: RSRP*33 + RSRQ*33 + SNR*34"""
    if rsrp is None:
        return None, None
    s_rsrp = step_rsrp_score(rsrp)
    s_rsrq = step_rsrq_score(rsrq) if rsrq is not None else 50
    s_snr = step_snr_score(snr) if snr is not None else 50
    score = (s_rsrp * 33 + s_rsrq * 33 + s_snr * 34) / 100
    score = max(0, min(100, score))
    if score >= 80: condition = "EXCELLENT"
    elif score >= 60: condition = "GOOD"
    elif score >= 40: condition = "FAIR"
    elif score >= 20: condition = "POOR"
    else: condition = "CRITICAL"
    return score, condition


# --- Throughput-based "truth" score ---
def throughput_score(dl_mbps):
    """What the score SHOULD be based on actual throughput."""
    if dl_mbps is None: return None
    if dl_mbps >= 100: return "EXCELLENT"
    if dl_mbps >= 50: return "GOOD"
    if dl_mbps >= 25: return "FAIR"
    if dl_mbps >= 10: return "POOR"
    return "CRITICAL"


# ===========================================================================
# UTILITIES
# ===========================================================================

SCORE_ORDER = {"EXCELLENT": 4, "GOOD": 3, "FAIR": 2, "POOR": 1, "CRITICAL": 0}

def score_to_num(s):
    return SCORE_ORDER.get(s, -1)

def safe_float(val):
    try:
        v = float(val)
        return v
    except (ValueError, TypeError):
        return None


def pearson_r(x, y):
    n = len(x)
    if n < 3: return 0
    mx, my = sum(x)/n, sum(y)/n
    num = sum((xi-mx)*(yi-my) for xi, yi in zip(x, y))
    dx = sum((xi-mx)**2 for xi in x) ** 0.5
    dy = sum((yi-my)**2 for yi in y) ** 0.5
    if dx*dy == 0: return 0
    return num / (dx*dy)


def accuracy(predicted, actual):
    """Exact match % and within-1-level %."""
    pairs = [(p, a) for p, a in zip(predicted, actual) if p and a]
    if not pairs: return 0, 0, 0
    exact = sum(1 for p, a in pairs if p == a)
    within1 = sum(1 for p, a in pairs if abs(score_to_num(p) - score_to_num(a)) <= 1)
    n = len(pairs)
    return exact/n*100, within1/n*100, n


# ===========================================================================
# MAIN
# ===========================================================================

def parse_csv_smart():
    """Parse CSV handling column misalignment (rows have 40-42 cols due to extra commas).
    Uses eutran-* as anchor for signal fields and counts from end for speedtest fields."""
    parsed = []
    with open(CSV_FILE, 'r') as f:
        reader = csv.reader(f)
        header = next(reader)  # 40 cols
        for row in reader:
            n = len(row)
            # Find Band_actual (eutran-*) as anchor
            band_pos = next((j for j, val in enumerate(row) if val.startswith('eutran-')), -1)
            if band_pos == -1:
                continue  # Skip rows without band info

            d = {}
            d['DeviceNum'] = row[0]
            d['Category'] = row[1]
            # Signal fields relative to band position (band is at header pos 13)
            d['RSRP_dBm'] = row[band_pos - 6] if band_pos >= 6 else ''
            d['RSRQ_dB'] = row[band_pos - 5] if band_pos >= 5 else ''
            d['SNR_dB'] = row[band_pos - 4] if band_pos >= 4 else ''
            d['RSSI_dBm'] = row[band_pos - 3] if band_pos >= 3 else ''
            d['RadioEnvConditions'] = row[band_pos - 2] if band_pos >= 2 else ''
            d['BandInfo_dmcli'] = row[band_pos - 1] if band_pos >= 1 else ''
            d['Band_actual'] = row[band_pos]
            d['EARFCN'] = row[band_pos + 1] if band_pos + 1 < n else ''
            d['BandBW_MHz'] = row[band_pos + 2] if band_pos + 2 < n else ''
            d['CA_Active'] = row[band_pos + 3] if band_pos + 3 < n else ''
            d['CA_TotalBW_MHz'] = row[band_pos + 4] if band_pos + 4 < n else ''

            # Speedtest fields counted from end (always stable from end)
            # Timestamp=n-1, SNR_test=n-2, RSRQ_conn=n-3, RSRP_conn=n-4,
            # UE_Idle=n-5, PLMN=n-6, Neigh_Inter=n-7, Neigh_Intra=n-8,
            # ServerDist=n-9, Server=n-10, PktLoss=n-11, Lat_max=n-12,
            # Lat_min=n-13, Lat_avg=n-14, Ping=n-15, UL=n-16, DL=n-17
            d['DL_Mbps'] = row[n - 17] if n >= 17 else ''
            d['UL_Mbps'] = row[n - 16] if n >= 16 else ''
            d['Ping_ms'] = row[n - 15] if n >= 15 else ''
            d['Latency_avg_ms'] = row[n - 14] if n >= 14 else ''
            d['PacketLoss_pct'] = row[n - 11] if n >= 11 else ''
            d['SpeedtestServer'] = row[n - 10] if n >= 10 else ''
            d['ServerDistance_km'] = row[n - 9] if n >= 9 else ''
            d['Neighbors_IntraFreq'] = row[n - 8] if n >= 8 else ''
            d['Neighbors_InterFreq'] = row[n - 7] if n >= 7 else ''
            d['PLMN'] = row[n - 6] if n >= 6 else ''
            d['UE_InIdle_During_Test'] = row[n - 5] if n >= 5 else ''
            d['Serving_RSRP_Connected_dBm'] = row[n - 4] if n >= 4 else ''
            d['Serving_RSRQ_Connected_dB'] = row[n - 3] if n >= 3 else ''
            d['SNR_During_Test_dB'] = row[n - 2] if n >= 2 else ''
            d['Timestamp'] = row[n - 1] if n >= 1 else ''
            parsed.append(d)
    return parsed


def main():
    rows = parse_csv_smart()

    # Filter successful rows (have valid DL speed)
    success = [r for r in rows if safe_float(r.get('DL_Mbps')) is not None]

    print(f"Total rows: {len(rows)}")
    print(f"Successful (with speedtest): {len(success)}")
    print(f"Failed/unreachable: {len(rows) - len(success)}")

    # ===================================================================
    print("\n" + "="*80)
    print("EXISTING RSRP-ONLY (RadioEnvConditions from device) vs THROUGHPUT")
    print("="*80)

    # Group by existing RadioEnvConditions
    env_groups = defaultdict(list)
    for r in success:
        env = r.get('RadioEnvConditions', '').strip()
        dl = safe_float(r['DL_Mbps'])
        if env and dl is not None:
            env_groups[env].append(dl)

    print(f"\n{'Existing Score':<12} {'N':<5} {'Avg DL':<9} {'Min':<8} {'Max':<8} {'StdDev':<8} {'Throughput Distribution'}")
    print("-"*90)
    for cat in ['EXCELLENT', 'GOOD', 'FAIR', 'POOR']:
        if cat in env_groups:
            v = env_groups[cat]
            tp = [throughput_score(x) for x in v]
            dist = {s: tp.count(s) for s in ['EXCELLENT','GOOD','FAIR','POOR','CRITICAL'] if tp.count(s)>0}
            std = statistics.stdev(v) if len(v)>1 else 0
            print(f"{cat:<12} {len(v):<5} {statistics.mean(v):<9.1f} {min(v):<8.1f} {max(v):<8.1f} {std:<8.1f} {dist}")

    # Accuracy of existing RSRP-only
    pred_existing = [r.get('RadioEnvConditions','').strip() for r in success]
    actual_tp = [throughput_score(safe_float(r['DL_Mbps'])) for r in success]
    ex, w1, n = accuracy(pred_existing, actual_tp)
    print(f"\nRSRP-Only Accuracy: {ex:.1f}% exact, {w1:.1f}% within-1-level (n={n})")

    # ===================================================================
    print("\n" + "="*80)
    print("PROPOSED WEIGHTED FORMULA (from X_RDK_RadioEnvConditions_analysis.md)")
    print("="*80)

    # Apply Option A and Option B
    for r in success:
        rsrp = safe_float(r['RSRP_dBm'])
        rsrq = safe_float(r['RSRQ_dB'])
        snr = safe_float(r['SNR_dB'])
        rssi = safe_float(r['RSSI_dBm'])
        r['score_a'], r['cond_a'] = weighted_score_option_a(rsrp, rsrq, snr, rssi)
        r['score_b'], r['cond_b'] = weighted_score_option_b(rsrp, rsrq, snr, rssi)

    # Show Option A distribution
    print("\n--- Option A (Step Tables): RSRP*35 + RSRQ*35 + SNR*25 + RSSI*5 ---")
    grp_a = defaultdict(list)
    for r in success:
        if r['cond_a']:
            grp_a[r['cond_a']].append(safe_float(r['DL_Mbps']))
    print(f"{'Weighted Score':<12} {'N':<5} {'Avg DL':<9} {'Min':<8} {'Max':<8} {'Throughput Distribution'}")
    print("-"*80)
    for cat in ['EXCELLENT','GOOD','FAIR','POOR','CRITICAL']:
        if cat in grp_a:
            v = grp_a[cat]
            tp = [throughput_score(x) for x in v]
            dist = {s: tp.count(s) for s in ['EXCELLENT','GOOD','FAIR','POOR','CRITICAL'] if tp.count(s)>0}
            print(f"{cat:<12} {len(v):<5} {statistics.mean(v):<9.1f} {min(v):<8.1f} {max(v):<8.1f} {dist}")

    pred_a = [r['cond_a'] for r in success]
    ex, w1, n = accuracy(pred_a, actual_tp)
    print(f"\nOption A Accuracy: {ex:.1f}% exact, {w1:.1f}% within-1-level (n={n})")

    # Show Option B distribution
    print("\n--- Option B (Linear Interp): RSRP*35 + RSRQ*35 + SNR*25 + RSSI*5 ---")
    grp_b = defaultdict(list)
    for r in success:
        if r['cond_b']:
            grp_b[r['cond_b']].append(safe_float(r['DL_Mbps']))
    print(f"{'Weighted Score':<12} {'N':<5} {'Avg DL':<9} {'Min':<8} {'Max':<8} {'Throughput Distribution'}")
    print("-"*80)
    for cat in ['EXCELLENT','GOOD','FAIR','POOR','CRITICAL']:
        if cat in grp_b:
            v = grp_b[cat]
            tp = [throughput_score(x) for x in v]
            dist = {s: tp.count(s) for s in ['EXCELLENT','GOOD','FAIR','POOR','CRITICAL'] if tp.count(s)>0}
            print(f"{cat:<12} {len(v):<5} {statistics.mean(v):<9.1f} {min(v):<8.1f} {max(v):<8.1f} {dist}")

    pred_b = [r['cond_b'] for r in success]
    ex, w1, n = accuracy(pred_b, actual_tp)
    print(f"\nOption B Accuracy: {ex:.1f}% exact, {w1:.1f}% within-1-level (n={n})")

    # ===================================================================
    print("\n" + "="*80)
    print("ALTERNATIVE PARAMETER COMBINATIONS")
    print("="*80)

    algos = {
        "RSRP-Only (existing)": lambda r: (None, r.get('RadioEnvConditions','').strip()),
        "Proposed A (35/35/25/5)": lambda r: weighted_score_option_a(
            safe_float(r['RSRP_dBm']), safe_float(r['RSRQ_dB']),
            safe_float(r['SNR_dB']), safe_float(r['RSSI_dBm'])),
        "Proposed B (linear 35/35/25/5)": lambda r: weighted_score_option_b(
            safe_float(r['RSRP_dBm']), safe_float(r['RSRQ_dB']),
            safe_float(r['SNR_dB']), safe_float(r['RSSI_dBm'])),
        "SNR-Heavy (20/20/50/10)": lambda r: weighted_snr_heavy(
            safe_float(r['RSRP_dBm']), safe_float(r['RSRQ_dB']),
            safe_float(r['SNR_dB']), safe_float(r['RSSI_dBm'])),
        "RSRQ-Heavy (20/50/25/5)": lambda r: weighted_rsrq_heavy(
            safe_float(r['RSRP_dBm']), safe_float(r['RSRQ_dB']),
            safe_float(r['SNR_dB']), safe_float(r['RSSI_dBm'])),
        "Equal 3-way (33/33/34/0)": lambda r: equal_three(
            safe_float(r['RSRP_dBm']), safe_float(r['RSRQ_dB']),
            safe_float(r['SNR_dB']), safe_float(r['RSSI_dBm'])),
        "SNR-Only": lambda r: snr_only_score(safe_float(r['SNR_dB'])),
    }

    print(f"\n{'Algorithm':<32} {'Exact%':<9} {'Within1%':<10} {'N':<5} {'Corr(score,DL)'}")
    print("-"*75)

    for name, fn in algos.items():
        preds = []
        scores_num = []
        dls = []
        for r in success:
            sc, cond = fn(r)
            tp = throughput_score(safe_float(r['DL_Mbps']))
            if cond and tp:
                preds.append(cond)
                dls.append(safe_float(r['DL_Mbps']))
                scores_num.append(sc if sc is not None else score_to_num(cond)*25)
        ex, w1, n = accuracy(preds, [throughput_score(d) for d in dls])
        # Correlation of numeric score with DL
        r_val = pearson_r(scores_num, dls) if len(scores_num) >= 5 else 0
        print(f"{name:<32} {ex:<9.1f} {w1:<10.1f} {n:<5} {r_val:.3f}")

    # ===================================================================
    print("\n" + "="*80)
    print("PARAMETER CORRELATION WITH THROUGHPUT (Pearson r)")
    print("="*80)

    params = {
        "RSRP (idle)": 'RSRP_dBm',
        "RSRQ (idle)": 'RSRQ_dB',
        "SNR (idle)": 'SNR_dB',
        "RSSI (idle)": 'RSSI_dBm',
        "BandBW MHz": 'BandBW_MHz',
        "RSRP (during test)": 'Serving_RSRP_Connected_dBm',
        "RSRQ (during test)": 'Serving_RSRQ_Connected_dB',
        "SNR (during test)": 'SNR_During_Test_dB',
    }

    print(f"\n{'Parameter':<25} {'r vs DL_Mbps':<14} {'N'}")
    print("-"*50)
    for label, col in params.items():
        pairs = [(safe_float(r[col]), safe_float(r['DL_Mbps']))
                 for r in success if safe_float(r.get(col)) is not None and safe_float(r['DL_Mbps']) is not None]
        if len(pairs) >= 5:
            x = [p[0] for p in pairs]
            y = [p[1] for p in pairs]
            r_val = pearson_r(x, y)
            print(f"{label:<25} {r_val:<14.4f} {len(pairs)}")
        else:
            print(f"{label:<25} {'N/A':<14} {len(pairs)}")

    # ===================================================================
    print("\n" + "="*80)
    print("THROUGHPUT BY BAND")
    print("="*80)

    band_data = defaultdict(list)
    for r in success:
        dl = safe_float(r['DL_Mbps'])
        band = r.get('Band_actual','')
        if dl is not None and band:
            band_data[band].append(dl)

    print(f"\n{'Band':<14} {'N':<5} {'Avg DL':<9} {'Min':<8} {'Max':<8} {'StdDev'}")
    print("-"*55)
    for band in sorted(band_data.keys(), key=lambda b: -statistics.mean(band_data[b])):
        v = band_data[band]
        std = statistics.stdev(v) if len(v)>1 else 0
        print(f"{band:<14} {len(v):<5} {statistics.mean(v):<9.1f} {min(v):<8.1f} {max(v):<8.1f} {std:.1f}")

    # ===================================================================
    print("\n" + "="*80)
    print("KEY MISMATCHES: RSRP=POOR/FAIR but throughput=EXCELLENT")
    print("="*80)
    print(f"\n{'#':<4} {'RSRP':<6} {'RSRQ':<6} {'SNR':<6} {'RSSI':<6} {'Band':<10} {'BW':<4} {'DL Mbps':<9} {'Existing':<10} {'OptA':<10} {'TrueTP'}")
    print("-"*85)

    for r in sorted(success, key=lambda x: safe_float(x['DL_Mbps']) or 0, reverse=True):
        existing = r.get('RadioEnvConditions','').strip()
        true_tp = throughput_score(safe_float(r['DL_Mbps']))
        if existing in ('POOR','FAIR') and true_tp == 'EXCELLENT':
            print(f"{r['DeviceNum']:<4} {r['RSRP_dBm']:<6} {r['RSRQ_dB']:<6} {r['SNR_dB']:<6} "
                  f"{r['RSSI_dBm']:<6} {r['Band_actual']:<10} {r['BandBW_MHz']:<4} "
                  f"{r['DL_Mbps']:<9} {existing:<10} {r.get('cond_a',''):<10} {true_tp}")

    # ===================================================================
    print("\n" + "="*80)
    print("WEIGHT COMBINATION SWEEP (RSRP/RSRQ/SNR/RSSI)")
    print("="*80)
    print("\nSweeping weight combinations in steps of 5, total=100...")
    print("Filter: must have >=2 devices in at least 3 categories (meaningful spread)")
    print(f"\n{'Weights (P/Q/S/I)':<22} {'Exact%':<9} {'Within1%':<10} {'Corr':<8} {'Monotonic':<10} {'Spread'}")
    print("-"*75)

    best_exact = (0, 0, "", 0)
    best_w1 = (0, 0, "", 0)
    best_mono = (0, 0, "", 0, 0)
    results = []

    # Sweep weights in increments of 5, must sum to 100
    for w_rsrp in range(0, 65, 5):
        for w_rsrq in range(0, 101 - w_rsrp, 5):
            for w_snr in range(0, 101 - w_rsrp - w_rsrq, 5):
                w_rssi = 100 - w_rsrp - w_rsrq - w_snr
                if w_rssi < 0:
                    continue

                preds = []
                dls = []
                scores_num = []
                for r in success:
                    rsrp = safe_float(r['RSRP_dBm'])
                    rsrq = safe_float(r['RSRQ_dB'])
                    snr = safe_float(r['SNR_dB'])
                    rssi = safe_float(r['RSSI_dBm'])
                    dl = safe_float(r['DL_Mbps'])
                    if rsrp is None or dl is None:
                        continue
                    s_rsrp = step_rsrp_score(rsrp)
                    s_rsrq = step_rsrq_score(rsrq) if rsrq is not None else 50
                    s_snr = step_snr_score(snr) if snr is not None else 50
                    s_rssi = step_rssi_score(rssi) if rssi is not None else 50
                    score = (s_rsrp*w_rsrp + s_rsrq*w_rsrq + s_snr*w_snr + s_rssi*w_rssi) / 100
                    score = max(0, min(100, score))
                    if score >= 80: cond = "EXCELLENT"
                    elif score >= 60: cond = "GOOD"
                    elif score >= 40: cond = "FAIR"
                    elif score >= 20: cond = "POOR"
                    else: cond = "CRITICAL"
                    preds.append(cond)
                    dls.append(dl)
                    scores_num.append(score)

                # Check distribution spread
                cat_counts = defaultdict(int)
                for p in preds:
                    cat_counts[p] += 1
                cats_with_2plus = sum(1 for c in cat_counts.values() if c >= 2)

                # Skip trivial distributions (everything in 1-2 buckets)
                if cats_with_2plus < 3:
                    continue

                tp_actual = [throughput_score(d) for d in dls]
                ex, w1, n = accuracy(preds, tp_actual)
                r_val = pearson_r(scores_num, dls) if len(scores_num) >= 5 else 0

                # Check monotonicity: avg DL should increase with score category
                cat_avg = {}
                for p, d in zip(preds, dls):
                    if p not in cat_avg:
                        cat_avg[p] = []
                    cat_avg[p].append(d)
                cat_avg = {k: statistics.mean(v) for k, v in cat_avg.items()}
                order = ['CRITICAL','POOR','FAIR','GOOD','EXCELLENT']
                avgs_in_order = [cat_avg.get(c) for c in order if c in cat_avg]
                # Count monotonic pairs
                mono_pairs = sum(1 for i in range(len(avgs_in_order)-1) if avgs_in_order[i] < avgs_in_order[i+1])
                total_pairs = max(1, len(avgs_in_order) - 1)
                mono_pct = mono_pairs / total_pairs * 100

                spread = f"{cats_with_2plus}cat"
                results.append((w_rsrp, w_rsrq, w_snr, w_rssi, ex, w1, r_val, mono_pct, spread))

                if ex > best_exact[0] or (ex == best_exact[0] and w1 > best_exact[1]):
                    best_exact = (ex, w1, f"{w_rsrp}/{w_rsrq}/{w_snr}/{w_rssi}", mono_pct)
                if w1 > best_w1[0] or (w1 == best_w1[0] and ex > best_w1[1]):
                    best_w1 = (w1, ex, f"{w_rsrp}/{w_rsrq}/{w_snr}/{w_rssi}", mono_pct)
                if mono_pct > best_mono[0] or (mono_pct == best_mono[0] and ex > best_mono[1]):
                    best_mono = (mono_pct, ex, f"{w_rsrp}/{w_rsrq}/{w_snr}/{w_rssi}", w1, r_val)

    # Sort by: monotonicity first, then exact match
    results.sort(key=lambda x: (-x[7], -x[4], -x[5]))

    # Print top 25 (monotonic + accurate)
    for w_rsrp, w_rsrq, w_snr, w_rssi, ex, w1, r_val, mono, spread in results[:25]:
        label = f"{w_rsrp}/{w_rsrq}/{w_snr}/{w_rssi}"
        print(f"{label:<22} {ex:<9.1f} {w1:<10.1f} {r_val:<8.3f} {mono:<10.0f} {spread}")

    print(f"\n  BEST Exact Match (3+cat): {best_exact[2]} → {best_exact[0]:.1f}% exact, {best_exact[1]:.1f}% within-1, mono={best_exact[3]:.0f}%")
    print(f"  BEST Within-1 (3+cat):    {best_w1[2]} → {best_w1[1]:.1f}% exact, {best_w1[0]:.1f}% within-1, mono={best_w1[3]:.0f}%")
    print(f"  BEST Monotonic (3+cat):   {best_mono[2]} → {best_mono[1]:.1f}% exact, {best_mono[3]:.1f}% within-1, mono={best_mono[0]:.0f}%")

    # ===================================================================
    # Show detailed breakdown for top 5 combos
    print("\n" + "="*80)
    print("DETAILED BREAKDOWN: TOP 5 WEIGHT COMBINATIONS")
    print("="*80)

    for idx, (w_rsrp, w_rsrq, w_snr, w_rssi, ex, w1, r_val, mono, spread) in enumerate(results[:5]):
        print(f"\n--- #{idx+1}: Weights RSRP={w_rsrp} RSRQ={w_rsrq} SNR={w_snr} RSSI={w_rssi} ---")
        print(f"    Exact={ex:.1f}%, Within1={w1:.1f}%, Monotonic={mono:.0f}%")
        cat_data = defaultdict(list)
        for r in success:
            rsrp_v = safe_float(r['RSRP_dBm'])
            rsrq_v = safe_float(r['RSRQ_dB'])
            snr_v = safe_float(r['SNR_dB'])
            rssi_v = safe_float(r['RSSI_dBm'])
            dl = safe_float(r['DL_Mbps'])
            if rsrp_v is None or dl is None:
                continue
            s_rsrp = step_rsrp_score(rsrp_v)
            s_rsrq = step_rsrq_score(rsrq_v) if rsrq_v is not None else 50
            s_snr = step_snr_score(snr_v) if snr_v is not None else 50
            s_rssi = step_rssi_score(rssi_v) if rssi_v is not None else 50
            score = (s_rsrp*w_rsrp + s_rsrq*w_rsrq + s_snr*w_snr + s_rssi*w_rssi) / 100
            if score >= 80: cond = "EXCELLENT"
            elif score >= 60: cond = "GOOD"
            elif score >= 40: cond = "FAIR"
            elif score >= 20: cond = "POOR"
            else: cond = "CRITICAL"
            cat_data[cond].append(dl)
        for cat in ['EXCELLENT','GOOD','FAIR','POOR','CRITICAL']:
            if cat in cat_data:
                v = cat_data[cat]
                tp = [throughput_score(x) for x in v]
                dist = {s: tp.count(s) for s in ['EXCELLENT','GOOD','FAIR','POOR','CRITICAL'] if tp.count(s)>0}
                print(f"    {cat:<12} N={len(v):<3} AvgDL={statistics.mean(v):>7.1f}  MinDL={min(v):>6.1f}  {dist}")

    # ===================================================================
    # Threshold sweep on best monotonic combo
    print("\n" + "="*80)
    print("THRESHOLD SWEEP (using best monotonic weight combo)")
    print("="*80)

    best_w = results[0]  # best monotonic
    bw_rsrp, bw_rsrq, bw_snr, bw_rssi = best_w[0], best_w[1], best_w[2], best_w[3]
    print(f"Weights: RSRP={bw_rsrp}, RSRQ={bw_rsrq}, SNR={bw_snr}, RSSI={bw_rssi}")

    # Compute raw scores for all devices with best weights
    raw_scores = []
    raw_dls = []
    for r in success:
        rsrp = safe_float(r['RSRP_dBm'])
        rsrq = safe_float(r['RSRQ_dB'])
        snr = safe_float(r['SNR_dB'])
        rssi = safe_float(r['RSSI_dBm'])
        dl = safe_float(r['DL_Mbps'])
        if rsrp is None or dl is None:
            continue
        s_rsrp = step_rsrp_score(rsrp)
        s_rsrq = step_rsrq_score(rsrq) if rsrq is not None else 50
        s_snr = step_snr_score(snr) if snr is not None else 50
        s_rssi = step_rssi_score(rssi) if rssi is not None else 50
        score = (s_rsrp*bw_rsrp + s_rsrq*bw_rsrq + s_snr*bw_snr + s_rssi*bw_rssi) / 100
        raw_scores.append(score)
        raw_dls.append(dl)

    print(f"\nScore range: {min(raw_scores):.1f} - {max(raw_scores):.1f}")
    print(f"Score distribution: mean={statistics.mean(raw_scores):.1f}, median={sorted(raw_scores)[len(raw_scores)//2]:.1f}")

    print(f"\n{'Thresholds (E/G/F/P)':<25} {'Exact%':<9} {'Within1%':<10} {'Mono%'}")
    print("-"*55)

    thresh_results = []
    for t_exc in range(65, 96, 5):
        for t_good in range(45, t_exc, 5):
            for t_fair in range(25, t_good, 5):
                for t_poor in range(10, t_fair, 5):
                    preds = []
                    cat_dls = defaultdict(list)
                    for sc, dl in zip(raw_scores, raw_dls):
                        if sc >= t_exc: c = "EXCELLENT"
                        elif sc >= t_good: c = "GOOD"
                        elif sc >= t_fair: c = "FAIR"
                        elif sc >= t_poor: c = "POOR"
                        else: c = "CRITICAL"
                        preds.append(c)
                        cat_dls[c].append(dl)
                    # Need 3+ categories with 2+ devices
                    cats_ok = sum(1 for v in cat_dls.values() if len(v) >= 2)
                    if cats_ok < 3:
                        continue
                    tp_actual = [throughput_score(d) for d in raw_dls]
                    ex, w1, _ = accuracy(preds, tp_actual)
                    # Monotonicity
                    cat_avg = {k: statistics.mean(v) for k, v in cat_dls.items()}
                    order = ['CRITICAL','POOR','FAIR','GOOD','EXCELLENT']
                    avgs_in_order = [cat_avg[c] for c in order if c in cat_avg]
                    mono_pairs = sum(1 for i in range(len(avgs_in_order)-1) if avgs_in_order[i] < avgs_in_order[i+1])
                    total_pairs = max(1, len(avgs_in_order) - 1)
                    mono_pct = mono_pairs / total_pairs * 100
                    thresh_results.append((t_exc, t_good, t_fair, t_poor, ex, w1, mono_pct))

    thresh_results.sort(key=lambda x: (-x[6], -x[4], -x[5]))
    for t_exc, t_good, t_fair, t_poor, ex, w1, mono in thresh_results[:10]:
        print(f"{t_exc:>2}/{t_good:>2}/{t_fair:>2}/{t_poor:>2}                 {ex:<9.1f} {w1:<10.1f} {mono:.0f}")

    if thresh_results:
        bt = thresh_results[0]
        print(f"\n  BEST: Thresholds {bt[0]}/{bt[1]}/{bt[2]}/{bt[3]} → {bt[4]:.1f}% exact, {bt[5]:.1f}% within-1, mono={bt[6]:.0f}%")

        # Show final distribution
        t_exc, t_good, t_fair, t_poor = bt[0], bt[1], bt[2], bt[3]
        print(f"\n  Final distribution with weights {bw_rsrp}/{bw_rsrq}/{bw_snr}/{bw_rssi} + thresholds {t_exc}/{t_good}/{t_fair}/{t_poor}:")
        dist_final = defaultdict(list)
        for sc, dl in zip(raw_scores, raw_dls):
            if sc >= t_exc: cond = "EXCELLENT"
            elif sc >= t_good: cond = "GOOD"
            elif sc >= t_fair: cond = "FAIR"
            elif sc >= t_poor: cond = "POOR"
            else: cond = "CRITICAL"
            dist_final[cond].append(dl)
        for cat in ['EXCELLENT','GOOD','FAIR','POOR','CRITICAL']:
            if cat in dist_final:
                v = dist_final[cat]
                tp = [throughput_score(x) for x in v]
                dist = {s: tp.count(s) for s in ['EXCELLENT','GOOD','FAIR','POOR','CRITICAL'] if tp.count(s)>0}
                print(f"    {cat:<12} N={len(v):<3} AvgDL={statistics.mean(v):>7.1f}  MinDL={min(v):>6.1f}  MaxDL={max(v):>6.1f}  {dist}")

    print("\n\nDone.")


if __name__ == "__main__":
    main()
