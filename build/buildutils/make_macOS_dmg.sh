#! /usr/bin/env bash

volname="BCI2000 $(./project_version.sh)"
echo Building distribution disk image for ${volname} ...

echo Making temporary directory ...
rm -rf "/tmp/bci2000dist" 2>/dev/null
mkdir -p "/tmp/bci2000dist/BCI2000"
echo Done.

echo Copying files ...
pushd $(dirname "$0")/../..

for i in BCI2000_Help.html GettingStarted.html; do
  cp $i "/tmp/bci2000dist/BCI2000"
done

for i in batch doc parms prog python tools; do
  cp -r $i "/tmp/bci2000dist/BCI2000"
done

mkdir "/tmp/bci2000dist/BCI2000/data"
cp -r data/samplefiles /tmp/bci2000dist/BCI2000/data

cp src/shared/gui/images/bci2000logo256.png /tmp/icon.png
sips -i /tmp/icon.png
DeRez -only icns /tmp/icon.png > /tmp/icns.rsrc
Rez -append /tmp/icns.rsrc -o $'/tmp/bci2000dist/BCI2000/Icon\r'
SetFile -a C /tmp/bci2000dist/BCI2000
SetFile -a V $'/tmp/bci2000dist/BCI2000/Icon\r'

popd
echo Done.

echo Creating disk image ...
hdiutil create /tmp/tmp.dmg -ov -volname "${volname}" -format UDRW -fs HFS+ -srcfolder "/tmp/bci2000dist"
echo Done.

echo Mounting disk image read/write ...
info=$(hdiutil mount /tmp/tmp.dmg -readwrite)
device=$(IFS=" " ; set -- $info ; echo $1)
echo Done.

# Broken in macOS 13, so we don't create a BCI2000Launcher alias
#echo Creating BCI2000Launcher alias ...
#pushd "/Volumes/${volname}/BCI2000"
#ln -s prog/BCI2000Launcher.app BCI2000Launcher
#popd
#echo Done.

echo Creating folder layout ...
echo '
  tell application "Finder"
     tell disk "'${volname}'"
           open
           set current view of container window to icon view
           set toolbar visible of container window to false
           set statusbar visible of container window to false
           set the bounds of container window to {100, 100, 400, 400}
           set position of item "BCI2000" of container window to {155, 125}
           set theViewOptions to the icon view options of container window
           set arrangement of theViewOptions to not arranged
           set icon size of theViewOptions to 100
           set the background color of theViewOptions to {57343, 57343, 57343}
           update without registering applications
           delay 5
           close
     end tell
   end tell
' | osascript

hdiutil detach $device -force
echo Done.

echo Removing temporary files ...
rm -rf "/tmp/bci2000dist"
rm /tmp/icon.png
rm /tmp/icns.rsrc
echo Done.

echo Converting disk image ...
rm ../../BCI2000.dmg
hdiutil convert /tmp/tmp.dmg -format UDZO -o ../../BCI2000.dmg -imagekey zlib-level=9
rm "/tmp/tmp.dmg"
echo Done.

