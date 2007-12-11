#
# dist.mk
#

USER = dave
TARGETS +=  apu

.PHONY: $(TARGETS)
$(TARGETS):
	git push $(TARGETS)
	ssh $(USER)@$(TARGETS) "cd dev/Jawas && git pull repo"

.PHONY: dist
dist:
	$(MAKE) clean
	$(MAKE) $(TARGETS)
