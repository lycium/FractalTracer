#!/bin/bash
# Build RelWithDebInfo and profile FractalTracer with perf.
# Usage: ./profile.sh [extra args to FractalTracer, e.g. --spp 8 --formula mandelbulb]
set -e

BINARY=build/RelWithDebInfo/src/FractalTracer
PERF_DATA=/tmp/fractal_perf.data

if [ ! -f "$BINARY" ] || [ src/renderer/Renderer.h -nt "$BINARY" ]; then
    echo "=== Configuring RelWithDebInfo ==="
    cmake --preset RelWithDebInfo
    echo "=== Building ==="
    cmake --build build/RelWithDebInfo --target FractalTracer -j$(nproc)
fi

echo "=== Profiling: $@ ==="
perf record -F 500 -g --call-graph fp -o "$PERF_DATA" -- "$BINARY" "$@"

echo ""
echo "=== Hotspot summary (>= 0.5%) ==="
perf report -i "$PERF_DATA" --stdio --no-children -n --sort symbol --percent-limit 0.5 2>/dev/null \
  | grep -E "^\s+[0-9]+\.[0-9]+%\s+[0-9]+" | head -30

echo ""
echo "Full interactive report: perf report -i $PERF_DATA"
