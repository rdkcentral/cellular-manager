#!/bin/sh
#
# collect_xle_metrics.sh — Run on XLE device to collect radio metrics + gateway speedtest
# Usage: source this on XLE after SSH, or run as: sh collect_xle_metrics.sh
#
# Output: prints CSV-formatted line to stdout
# Prerequisite: dmcli and qmicli available, /dev/cdc-wdm0 accessible
#

# Step 1: Get XLE software version
XLE_VERSION=$(dmcli eRT getv Device.DeviceInfo.SoftwareVersion 2>/dev/null | grep "value:" | awk '{print $NF}')

# Step 2: Collect pre-speedtest signal metrics
SIGNAL_INFO=$(qmicli -d /dev/cdc-wdm0 -p --nas-get-signal-info 2>/dev/null)
RSRP=$(echo "$SIGNAL_INFO" | grep "RSRP:" | awk -F"'" '{print $2}' | awk '{print $1}')
RSRQ=$(echo "$SIGNAL_INFO" | grep "RSRQ:" | awk -F"'" '{print $2}' | awk '{print $1}')
SNR=$(echo "$SIGNAL_INFO" | grep "SNR:" | awk -F"'" '{print $2}' | awk '{print $1}')
RSSI=$(echo "$SIGNAL_INFO" | grep "RSSI:" | awk -F"'" '{print $2}' | awk '{print $1}')

# Step 2: Band info
BAND_INFO=$(qmicli -d /dev/cdc-wdm0 -p --nas-get-rf-band-info 2>/dev/null)
BAND_ACTUAL=$(echo "$BAND_INFO" | grep "Active Band Class:" | head -n 1 | awk -F"'" '{print $2}')
EARFCN=$(echo "$BAND_INFO" | grep "Active Channel:" | head -n 1 | awk -F"'" '{print $2}')

# Step 3: CA info
CA_INFO=$(qmicli -d /dev/cdc-wdm0 -p --nas-get-lte-cphy-ca-info 2>/dev/null)
if echo "$CA_INFO" | grep -q "error"; then
    CA_ACTIVE="No"
    CA_TOTAL_BW=""
    CA_PRI_BW=$(echo "$BAND_INFO" | grep -c "Band") # fallback
    PCI=$(echo "" | head -n 1)
else
    CA_PRI_BW=$(echo "$CA_INFO" | grep "DL Bandwidth:" | head -n 1 | awk -F"'" '{print $2}')
    CA_SEC_STATE=$(echo "$CA_INFO" | grep "State:" | awk -F"'" '{print $2}')
    if echo "$CA_SEC_STATE" | grep -qi "deconfig"; then
        CA_ACTIVE="No_Deconfigured"
    else
        CA_ACTIVE="Yes"
    fi
    CA_SEC_BW=$(echo "$CA_INFO" | sed -n '/Secondary Cell/,/State/p' | grep "DL Bandwidth:" | awk -F"'" '{print $2}')
    if [ -n "$CA_PRI_BW" ] && [ -n "$CA_SEC_BW" ]; then
        CA_TOTAL_BW=$((CA_PRI_BW + CA_SEC_BW))
    else
        CA_TOTAL_BW="$CA_PRI_BW"
    fi
fi

# Step 4: Cell location (for PCI, CellId, TAC, neighbors)
CELL_LOC=$(qmicli -d /dev/cdc-wdm0 -p --nas-get-cell-location-info 2>/dev/null)
PCI=$(echo "$CELL_LOC" | grep "Serving Cell ID:" | awk -F"'" '{print $2}')
GLOBAL_CELL_ID=$(echo "$CELL_LOC" | grep "Global Cell ID:" | awk -F"'" '{print $2}')
TAC=$(echo "$CELL_LOC" | grep "Tracking Area Code:" | awk -F"'" '{print $2}')
PLMN=$(echo "$CELL_LOC" | grep "PLMN:" | head -n 1 | awk -F"'" '{print $2}')
TA=$(echo "$CELL_LOC" | grep "LTE Timing Advance:" | awk -F"'" '{print $2}')

# Count neighbors
INTRA_NEIGHBORS=$(echo "$CELL_LOC" | sed -n '/Intrafrequency/,/Interfrequency/p' | grep -c "Physical Cell ID:")
INTRA_NEIGHBORS=$((INTRA_NEIGHBORS - 1)) # subtract serving cell
if [ "$INTRA_NEIGHBORS" -lt 0 ]; then INTRA_NEIGHBORS=0; fi
INTER_NEIGHBORS=$(echo "$CELL_LOC" | sed -n '/Interfrequency/,/GSM/p' | grep -c "Physical Cell ID:")

# Step 5: dmcli params
RADIO_COND=$(dmcli eRT getv Device.Cellular.Interface.1.X_RDK_RadioEnvConditions 2>/dev/null | grep "value:" | awk '{print $NF}')
BAND_DMCLI=$(dmcli eRT getv Device.Cellular.Interface.1.X_RDK_BandInfo 2>/dev/null | grep "value:" | awk '{print $NF}')

# Step 6: Trigger gateway speedtest
dmcli eRT setv Device.Cellular.X_RDK_SpeedTest.Enable bool true >/dev/null 2>&1

# Step 7: Wait for speedtest to complete (check for ltespeedtest.txt with upload line)
WAIT_COUNT=0
MAX_WAIT=180  # 3 minutes max
while [ $WAIT_COUNT -lt $MAX_WAIT ]; do
    if [ -f /tmp/ltespeedtest.txt ] && grep -q "Upload:" /tmp/ltespeedtest.txt 2>/dev/null; then
        break
    fi
    WAIT_COUNT=$((WAIT_COUNT + 1))
    # busybox sleep
    sleep 1 2>/dev/null || true
done

# Step 8: Parse speedtest results
if [ -f /tmp/ltespeedtest.txt ]; then
    DL_MBPS=$(grep "Download:" /tmp/ltespeedtest.txt | awk '{print $2}')
    UL_MBPS=$(grep "Upload:" /tmp/ltespeedtest.txt | awk '{print $2}')
    PING=$(grep "km]:" /tmp/ltespeedtest.txt | awk '{print $NF}' | sed 's/ms//')
    SERVER=$(grep "Hosted by" /tmp/ltespeedtest.txt | sed 's/Hosted by //' | sed 's/ \[.*//')
    SERVER_DIST=$(grep "Hosted by" /tmp/ltespeedtest.txt | grep -o '\[.*km\]' | tr -d '[]' | sed 's/ km//')
else
    DL_MBPS="TIMEOUT"
    UL_MBPS="TIMEOUT"
    PING=""
    SERVER=""
    SERVER_DIST=""
fi

# Step 9: Wait for latency test
WAIT_COUNT=0
while [ $WAIT_COUNT -lt 120 ]; do
    if [ -f /tmp/ltelatencytest.txt ] && grep -q "packets transmitted" /tmp/ltelatencytest.txt 2>/dev/null; then
        break
    fi
    WAIT_COUNT=$((WAIT_COUNT + 1))
    sleep 1 2>/dev/null || true
done

# Step 10: Parse latency results
if [ -f /tmp/ltelatencytest.txt ]; then
    LATENCY_STATS=$(grep "round-trip" /tmp/ltelatencytest.txt | awk -F'=' '{print $2}')
    LATENCY_MIN=$(echo "$LATENCY_STATS" | awk -F'/' '{print $1}' | tr -d ' ')
    LATENCY_AVG=$(echo "$LATENCY_STATS" | awk -F'/' '{print $2}')
    LATENCY_MAX=$(echo "$LATENCY_STATS" | awk -F'/' '{print $3}')
    PKT_LOSS=$(grep "packet loss" /tmp/ltelatencytest.txt | grep -o '[0-9]*%' | tr -d '%')
else
    LATENCY_MIN=""
    LATENCY_AVG=""
    LATENCY_MAX=""
    PKT_LOSS=""
fi

# Step 11: Get during-test metrics from lteinfo.txt
if [ -f /tmp/lteinfo.txt ]; then
    UE_IDLE=$(grep "UE In Idle:" /tmp/lteinfo.txt | head -n 1 | awk -F"'" '{print $2}')
    SERVING_RSRP_CONN=$(grep -A3 "Cell \[0\]:" /tmp/lteinfo.txt | grep "RSRP:" | head -n 1 | awk -F"'" '{print $2}')
    SERVING_RSRQ_CONN=$(grep -A3 "Cell \[0\]:" /tmp/lteinfo.txt | grep "RSRQ:" | head -n 1 | awk -F"'" '{print $2}')
    TA_DURING=$(grep "LTE Timing Advance:" /tmp/lteinfo.txt | awk -F"'" '{print $2}')
else
    UE_IDLE=""
    SERVING_RSRP_CONN=""
    SERVING_RSRQ_CONN=""
    TA_DURING=""
fi

# Step 12: Get during-test SNR
SIGNAL_DURING=$(qmicli -d /dev/cdc-wdm0 -p --nas-get-signal-info 2>/dev/null)
SNR_DURING=$(echo "$SIGNAL_DURING" | grep "SNR:" | awk -F"'" '{print $2}' | awk '{print $1}')

# Calculate cell distance from timing advance (TA in μs, distance = TA * 150 m)
# TA = round-trip time, so distance = TA * c / 2 = TA(μs) * 300/2 = TA * 150 meters
if [ -n "$TA_DURING" ] && [ "$TA_DURING" != "unavailable" ]; then
    CELL_DIST_KM=$(awk "BEGIN {printf \"%.1f\", $TA_DURING * 0.15}")
else
    TA_DURING="unavailable"
    CELL_DIST_KM=""
fi

# Get bandwidth from CA info or estimate from band
if [ -n "$CA_PRI_BW" ] && [ "$CA_PRI_BW" != "0" ]; then
    BAND_BW="$CA_PRI_BW"
elif echo "$BAND_ACTUAL" | grep -q "eutran-13"; then
    BAND_BW="10"
elif echo "$BAND_ACTUAL" | grep -q "eutran-4"; then
    BAND_BW="15"
elif echo "$BAND_ACTUAL" | grep -q "eutran-66"; then
    BAND_BW="15"
elif echo "$BAND_ACTUAL" | grep -q "eutran-2"; then
    BAND_BW="15"
else
    BAND_BW=""
fi

TIMESTAMP=$(date -u +%Y-%m-%dT%H:%MZ 2>/dev/null || date +%Y-%m-%dT%H:%MZ)

# UE idle: convert 'yes'/'no' to more readable 'Yes'/'No'
if [ "$UE_IDLE" = "no" ]; then
    UE_IDLE_FMT="No"
elif [ "$UE_IDLE" = "yes" ]; then
    UE_IDLE_FMT="Yes"
else
    UE_IDLE_FMT="$UE_IDLE"
fi

# Output CSV line (no header - caller adds header)
# Note: XB_VERSION must be passed as $1 argument (collected on XB before SSH to XLE)
XB_VERSION="${1:-}"
echo "${XLE_VERSION},${XB_VERSION},${RSRP},${RSRQ},${SNR},${RSSI},${RADIO_COND},${BAND_DMCLI},${BAND_ACTUAL},${EARFCN},${BAND_BW},${CA_ACTIVE},${CA_TOTAL_BW},${PCI},${GLOBAL_CELL_ID},${TAC},${TA_DURING},${CELL_DIST_KM},${DL_MBPS},${UL_MBPS},${PING},${LATENCY_AVG},${LATENCY_MIN},${LATENCY_MAX},${PKT_LOSS},${SERVER},${SERVER_DIST},${INTRA_NEIGHBORS},${INTER_NEIGHBORS},${PLMN},${UE_IDLE_FMT},${SERVING_RSRP_CONN},${SERVING_RSRQ_CONN},${SNR_DURING},${TIMESTAMP}"
