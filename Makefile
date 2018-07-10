all:
	@pushd spy & nmake /nologo & popd
	@pushd pj & nmake /nologo & popd

clean:
	@pushd spy & nmake /nologo clean & popd
	@pushd pj & nmake /nologo clean & popd
