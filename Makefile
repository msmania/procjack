all:
	@pushd expeb & nmake /nologo & popd
	@pushd spy & nmake /nologo & popd
	@pushd pj & nmake /nologo & popd

clean:
	@pushd expeb & nmake /nologo clean & popd
	@pushd spy & nmake /nologo clean & popd
	@pushd pj & nmake /nologo clean & popd
