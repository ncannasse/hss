#
DIR        := bin
ifeq ($(OS), Windows_NT)
	TARGET := $(DIR)/hss.exe
else
	TARGET := $(DIR)/hss
endif
BYCODE     := $(DIR)/hss.n

release: $(TARGET)
	rm -rf *.n hss/*.n release
#	??? haxelib run xcross -console hss.n
#	mkdir -p release
#	(cd release && mv ../hss-linux hss && gzip hss && mv hss.gz hss-linux.gz && mv ../hss-osx hss && gzip hss && mv hss.gz hss-osx.gz && mv ../hss-win.exe hss.exe && zip hss-win.zip hss.exe && rm -rf hss.exe)

clean:
	rm -rf *.n hss/*.n $(BYCODE) $(TARGET) test/test.css

test: $(BYCODE)
	neko $< --append -rule test/test.rules test/test.hss

.PHONY: test release clean

$(TARGET): $(BYCODE)
	nekoc -z $<
	nekotools boot $<

%.n:%.nml
	nekoml $<

$(BYCODE): hss/Main.n hss/Ast.n hss/Lexer.n hss/Parser.n hss/Rules.n hss/Utils.n
	nekoc -link $@ hss/Main
