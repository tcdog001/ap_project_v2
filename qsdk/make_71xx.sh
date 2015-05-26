#/bin/sh

#
#$1:hisitopdir
#
main () {
	local file_version=files/etc/.version
	local hisitopdir="$1"
	local opt="$2"
	local dir_md=${hisitopdir}/custom/image
	local sysupgrade_image=openwrt-ar71xx-generic-ap147-16M-squashfs-sysupgrade.bin

	if [ "" == "${hisitopdir}" ]; then
		echo "you must export hisitopdir!!!"
		exit 1
	fi
	cp -f ${hisitopdir}/version ${file_version}
	
	rm -fr build_dir/target-*/autelan-*
	rm -fr build_dir/target-*/thirdpart-*
#	rm -fr build_dir/target-*/myapp
#	rm -fr build_dir/target-*/mylib
	rm -fr build_dir/target-*/capp

	#make V=s
	make ${opt}
	echo "version=$(cat staging_dir/target-mips_r2_uClibc-0.9.33.2/root-ar71xx/etc/.version)"

	pushd bin/ar71xx/
	md5sum ${sysupgrade_image} > sysupgrade.md5
	tar zcvf lte-openwrt.img ${sysupgrade_image} sysupgrade.md5

	cp -f ${sysupgrade_image} ${dir_md}/sysupgrade.bin
	popd
}

main $@
