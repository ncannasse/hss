all:
	nekoml hss/*.nml
	mkdir -p bin
	nekoc -link bin/hss.n hss/Main
	nekotools boot bin/hss.n
	rm -rf *.n hss/*.n

release:
	nekoml hss/*.nml
	nekoc -link hss.n hss/Main
	haxelib run xcross -console hss.n
	rm -rf *.n hss/*.n release
	mkdir -p release
	(cd release && mv ../hss-linux hss && gzip hss && mv hss.gz hss-linux.gz && mv ../hss-osx hss && gzip hss && mv hss.gz hss-osx.gz && mv ../hss-win.exe hss.exe && zip hss-win.zip hss.exe && rm -rf hss.exe)

.PHONY: all release
