#
# dist.mk
#

TARGETS +=  freya

commit :
	git commit -a

.PHONY: $(TARGETS)
$(TARGETS):
	git push
	ssh $@ "cd ~/Jawas && git pull && gmake && sudo jawas stop $(PROD) && cp jawasd /usr/local/Jawas/ && sudo jawas start $(PROD)"

.PHONY: dev
dev:
	$(MAKE) $(TARGETS) SITE=dev

.PHONY: release
release:
	$(MAKE) clean
	$(MAKE) $(TARGETS) SITE=prod PROD=prod
