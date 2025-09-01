#!/usr/bin/env fish

function die
    echo "❌ $argv" >&2
    exit 1
end

echo "=== Building for x86_64 and arm64 in parallel ==="

# x86_64 build in background
begin
    echo "[x86_64] Configuring..."
    arch -x86_64 /usr/local/bin/cmake -B build_x86_64 -S . -DCMAKE_PREFIX_PATH=/usr/local \
        || exit 1
    echo "[x86_64] Building..."
    arch -x86_64 /usr/local/bin/cmake --build build_x86_64 \
        || exit 1
end &
set pid_x86 $last_pid

# arm64 build in background
begin
    echo "[arm64] Configuring..."
    arch -arm64 cmake -B build_arm64 -S . -DCMAKE_PREFIX_PATH=/opt/homebrew \
        || exit 1
    echo "[arm64] Building..."
    arch -arm64 cmake --build build_arm64 \
        || exit 1
end &
set pid_arm $last_pid

# Wait for both builds
wait $pid_x86; or die "x86_64 build failed"
wait $pid_arm; or die "arm64 build failed"

echo "=== Creating universal binary ==="
lipo -create \
    build_x86_64/UniFly-XPLM.xpl \
    build_arm64/UniFly-XPLM.xpl \
    -output UniFly-XPLM.xpl \
    || die "Universal binary creation failed"

echo "✅ Universal build complete: UniFly-XPLM.xpl"
