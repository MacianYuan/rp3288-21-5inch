#!/bin/bash
set -e

manual_mode=false
version=3.10

while getopts "mv:" opt; do
    case $opt in
	m) manual_mode=true
	    ;;
	v) version=$OPTARG
	   ;;
	?) echo "Usage: $0 [-m] [-v version]"
	   echo "   -m: manually specify build numbers"
	   echo "   -v: specify kernel version [default 3.10]"
	   exit 1
	   ;;
    esac
done

if [[ "$version" != "3.10" && "$version" != "3.18" ]]
then
	echo "kernel version must be 3.10 or 3.18"
	exit 1
fi

fetchtool='sso_client -location -connect_timeout 60 -request_timeout 60 -url'
build_server='https://android-build-uber.corp.google.com'
branch_prefix='kernel-n-dev-android-goldfish-'

# kernel_img[branch]="build_server_output local_file_name"
declare -A kernel_img

kernel_img[3.10-arm]="zImage arm/ranchu/kernel-qemu"
kernel_img[3.10-arm64]="Image arm64/kernel-qemu"
kernel_img[3.10-mips]="vmlinux mips/ranchu/kernel-qemu"
kernel_img[3.10-mips64]="vmlinux mips64/kernel-qemu"
kernel_img[3.10-x86]="bzImage x86/ranchu/kernel-qemu"
kernel_img[3.10-x86_64]="bzImage x86_64/ranchu/kernel-qemu"
kernel_img[3.10-x86_64-qemu1]="bzImage x86_64/kernel-qemu"
kernel_img[3.18-arm]="zImage arm/3.18/kernel-qemu2"
kernel_img[3.18-arm64]="Image arm64/3.18/kernel-qemu2"
kernel_img[3.18-mips]="vmlinux mips/3.18/kernel-qemu2"
kernel_img[3.18-mips64]="vmlinux mips64/3.18/kernel-qemu2"
kernel_img[3.18-x86]="bzImage x86/3.18/kernel-qemu2"
kernel_img[3.18-x86_64]="bzImage x86_64/3.18/kernel-qemu2"

printf "Upgrade emulator kernels $version\n\n" > emu_kernel.commitmsg

for key in "${!kernel_img[@]}"
do
	if [[ $key != $version* ]]
	then
		continue
	fi

	branch=$branch_prefix$key
	branch_url=$build_server/builds/$branch-linux-kernel

	# Find the latest build by searching for highest build number since
	# build server doesn't provide the "latest" link.
	build=`$fetchtool $branch_url | \
			sed -rn "s/<li><a href=".*">([0-9]+)<\/a><\/li>/\1/p" | \
			sort -nr | head -n 1`

	if $manual_mode
	then
		read -p "Enter build number for $branch: [$build]" input
		build="${input:-$build}"
	fi

	echo Fetching build $build from branch $branch

	# file_info[0] - kernel image on build server
	# file_info[1] - kernel image in local tree
	file_info=(${kernel_img[$key]})

	$fetchtool $branch_url/$build/${file_info[0]} > ${file_info[1]}

	git add ${file_info[1]}

	printf "$branch - build: $build\n" >> emu_kernel.commitmsg
done

last_commit=`git log | \
	sed -rn "s/.*Upgrade $version kernel images to ([a-z0-9]+).*/\1/p" | \
	head -n 1`

if [ ! -d goldfish_cache ]
then
	mkdir goldfish_cache
	git clone https://android.googlesource.com/kernel/goldfish goldfish_cache
fi

pushd goldfish_cache

git fetch origin

git checkout remotes/origin/android-goldfish-$version
tot_commit=`git log --oneline -1 | cut -d' ' -f1`
printf "\nUpgrade $version kernel images to ${tot_commit}\n" >> ../emu_kernel.commitmsg
git log --oneline HEAD...${last_commit} >> ../emu_kernel.commitmsg

popd

git commit -t emu_kernel.commitmsg

rm emu_kernel.commitmsg

