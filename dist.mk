#
# dist.mk
#

USER = dave
TARGETS +=  apu

.PHONY: $(TARGETS)
$(TARGETS):
	git push $(TARGETS)
	ssh $(USER)@$(TARGETS) "cd $(SITE)/Jawas && git pull /usr/pub/git/jawas.git && gmake"

.PHONY: dev
dev:
	$(MAKE) $(TARGETS) SITE=dev

.PHONY: release
release:
	$(MAKE) clean
	$(MAKE) $(TARGETS) SITE=prod	
