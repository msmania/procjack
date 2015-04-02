all:
	cd "$(MAKEDIR)\pj"
	@$(MAKE) /NOLOGO
	cd "$(MAKEDIR)\spy"
	@$(MAKE) /NOLOGO

clean:
	cd "$(MAKEDIR)\pj"
	@$(MAKE) /NOLOGO clean
	cd "$(MAKEDIR)\spy"
	@$(MAKE) /NOLOGO clean
