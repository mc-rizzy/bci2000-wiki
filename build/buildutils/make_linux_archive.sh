#! /usr/bin/env bash

volname="BCI2000_$(./project_version.sh)"
echo Building distribution archive for ${volname} ...

echo Making temporary directory ...
rm -rf "/tmp/bci2000dist" 2>/dev/null
mkdir -p "/tmp/bci2000dist/${volname}"
echo Done.

echo Copying files ...
pushd $(dirname "$0")/../..

for i in BCI2000_Help.html GettingStarted.html; do
  cp $i "/tmp/bci2000dist/${volname}"
done

for i in batch doc parms prog python tools; do
  cp -r $i "/tmp/bci2000dist/${volname}"
done

mkdir "/tmp/bci2000dist/${volname}/data"
cp -r data/samplefiles /tmp/bci2000dist/${volname}/data

popd
echo Done.

echo Creating BCI2000Launcher alias ...
pushd "/tmp/bci2000dist/${volname}"
ln -s prog/BCI2000Launcher BCI2000Launcher
popd
echo Done.

echo Creating archive ...
tar -cvaf "../../${volname}.tar.xz" -C "/tmp/bci2000dist" "${volname}"
echo Done.

echo Removing temporary files ...
rm -rf "/tmp/bci2000dist"
echo Done.

