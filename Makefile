all:
	nekoml hss/*.nml
	nekoc -link hss.n hss/Main
	nekotools boot hss.n
	rm -rf *.n hss/*.n
