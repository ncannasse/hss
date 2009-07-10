all:
	nekoml hss/*.nml
	-mkdir bin
	nekoc -link bin/hss.n hss/Main
	nekotools boot bin/hss.n
	rm -rf *.n hss/*.n

release:
	nekoml hss/*.nml
	nekoc -link hss.n hss/Main
	haxelib run xcross -console hss.n
	rm -rf *.n hss/*.n release
	mkdir release
	mv hss-win.exe release/hss.exe
	mv hss-linux release/hss
	(cd release && gzip hss && mv hss.gz hss-linux.gz && mv ../hss-osx hss && gzip hss && mv hss.gz hss-osx.gz && zip hss-win.zip hss.exe && rm -rf hss.exe)

.PHONY: all release
