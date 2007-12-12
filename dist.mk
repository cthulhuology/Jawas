#
# dist.mk
#

TARGETS +=  apu

commit :
	git commit -a

.PHONY: $(TARGETS)
$(TARGETS): commit
	git push $@
	ssh $@ "cd $(SITE)/Jawas && git pull /usr/pub/git/jawas.git && gmake && jawas stop $(PROD) && jawas start $(PROD)"

.PHONY: dev
dev:
	$(MAKE) $(TARGETS) SITE=dev

.PHONY: release
release:
	$(MAKE) clean
	$(MAKE) $(TARGETS) SITE=prod PROD=prod
