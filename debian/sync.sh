#!/bin/sh

ngd=${1:-../../nvidia-graphics-drivers/nvidia-graphics-drivers}

ignore="
debian/changelog
debian/control
debian/control.models
debian/copyright
debian/gbp.conf
debian/nvidia-kernel-dkms.docs
debian/nvidia-kernel-source.docs
debian/nvidia-kernel-support.links.in
debian/rules
debian/rules.defs
debian/sync.sh
debian/watch
debian/source/lintian-overrides
debian/tests/control
debian/upstream/metadata
"

for f in $(find debian -type f)
do
	for i in $ignore
	do
		if [ "$f" = "$i" ]; then
			continue 2
		fi
	done

	if [ -f "$ngd/$f" ]; then
		if ! cmp -s "$ngd/$f" "$f" ; then
			cp -v "$ngd/$f" "$f"
		fi
	fi
done

for psrc in "$ngd/debian/patches/module"/*.patch
do
	pdst=${psrc#$ngd/}
	if ! cmp -s "$psrc" "$pdst" ; then
		cp -v "$psrc" "$pdst"
	fi
done
