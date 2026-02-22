#!/bin/bash
# Download a free HDR environment map from Poly Haven (https://polyhaven.com)
# All assets are CC0 licensed.

set -e

ASSET="${1:-autumn_park}"
RES="${2:-8k}"
OUT_DIR="data"

mkdir -p "$OUT_DIR"

URL="https://dl.polyhaven.org/file/ph-assets/HDRIs/hdr/${RES}/${ASSET}_${RES}.hdr"
OUT_FILE="${OUT_DIR}/${ASSET}_${RES}.hdr"

if [ -f "$OUT_FILE" ]; then
    echo "Already exists: $OUT_FILE"
else
    echo "Downloading: $URL"
    curl -L -o "$OUT_FILE" "$URL"
    echo "Saved to: $OUT_FILE"
fi

echo ""
echo "Usage: ./build/Release/src/FractalTracer --formula amazingbox --hdrenv $OUT_FILE"
