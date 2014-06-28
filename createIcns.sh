#!/bin/sh

IMG=$1

if ! [ -f "$IMG" ]; then
 echo "$IMG is not a file"
 exit 1
fi

DIR=$(mktemp -d /tmp/icns.XXXXXX)
ICONSET="$DIR/${IMG%.*}.iconset"
mkdir -p "$ICONSET"

sips -z 16 16     "$IMG" --out "$ICONSET/icon_16x16.png"
sips -z 32 32     "$IMG" --out "$ICONSET/icon_16x16@2x.png"
sips -z 32 32     "$IMG" --out "$ICONSET/icon_32x32.png"
sips -z 64 64     "$IMG" --out "$ICONSET/icon_32x32@2x.png"
sips -z 128 128   "$IMG" --out "$ICONSET/icon_128x128.png"
sips -z 256 256   "$IMG" --out "$ICONSET/icon_128x128@2x.png"
sips -z 256 256   "$IMG" --out "$ICONSET/icon_256x256.png"
sips -z 512 512   "$IMG" --out "$ICONSET/icon_256x256@2x.png"
sips -z 512 512   "$IMG" --out "$ICONSET/icon_512x512.png"
sips -z 1024 1024 "$IMG" --out "$ICONSET/icon_512x512@2x.png"
iconutil -c icns "$ICONSET"
mv "$DIR/${IMG%.*}.icns" ./
rm -r "$DIR"

