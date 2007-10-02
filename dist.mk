#
# dist.mk
#

USER = dave
#TARGETS +=  kadath.nexttolast.com
TARGETS +=  nexttolast.com

.PHONY: $(TARGETS)
$(TARGETS):
	rsync -avz  -e "ssh -i $$HOME/.ssh/rsync-key" . $(USER)@$@:/opt/Jawas/Code/Jawas
	ssh -i $$HOME/.ssh/rsync-key $(USER)@$@ 'cd /opt/Jawas/Code/Jawas && make prod'
# prod'

.PHONY: dist
dist:
	$(MAKE) clean
	$(MAKE) $(TARGETS)
