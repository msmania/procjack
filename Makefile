all:
	@pushd spy & nmake /nologo & popd
	@pushd pj & nmake /nologo & popd
	@pushd expeb & nmake /nologo & popd
	@pushd clove & nmake /nologo & popd
	@pushd tests & nmake /nologo & popd

clean:
	@pushd spy & nmake /nologo clean & popd
	@pushd pj & nmake /nologo clean & popd
	@pushd expeb & nmake /nologo clean & popd
	@pushd clove & nmake /nologo clean & popd
	@pushd tests & nmake /nologo clean & popd
