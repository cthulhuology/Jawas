#
# dist.mk
#

USER = dave
TARGETS +=  nexttolast.com

.PHONY: $(TARGETS)
$(TARGETS):
	rsync -avz  . $(USER)@$@:/opt/Jawas/Dev/

.PHONY: dist
dist:
	$(MAKE) clean
	$(MAKE) $(TARGETS)

.PHONY: kadath
kadath:
	$(MAKE)
	rsync -avz . $(USER)@kadath.nexttolast.com:/Users/dave/Code/Jawas/
