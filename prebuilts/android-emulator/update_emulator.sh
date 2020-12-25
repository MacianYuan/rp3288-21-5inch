#!/bin/bash
set -e

if [ $# == 1 ]
then
build=$1
else
	echo  Usage: $0 build
	exit 1
fi

linux_zip=sdk-repo-linux-emulator-$build.zip
mac_zip=sdk-repo-darwin-emulator-$build.zip

echo Fetching Linux $build
/google/data/ro/projects/android/fetch_artifact --bid $build --target sdk_tools_linux "$linux_zip"
rm -rf linux-x86_64
unzip $linux_zip
mv emulator linux-x86_64
git add linux-x86_64

echo Fetching Mac $build
/google/data/ro/projects/android/fetch_artifact --bid $build --target sdk_tools_mac "$mac_zip"
rm -rf darwin-x86_64
unzip $mac_zip
mv emulator darwin-x86_64
git add darwin-x86_64


printf "Upgrade emulator to emu-master-dev build $build\n\n" > emulator.commitmsg


git commit -t emulator.commitmsg

rm emulator.commitmsg
rm $linux_zip
rm $mac_zip
