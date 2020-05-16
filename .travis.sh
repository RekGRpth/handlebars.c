#!/usr/bin/env bash

set -e -o pipefail

export CC="$MYCC"
export PREFIX="$HOME/build"
export PATH="$PREFIX/bin:$PATH"
export CFLAGS="-I$PREFIX/include -I$PREFIX/include/json-c"
export LDFLAGS="-L$PREFIX/lib $LDFLAGS"
export PKG_CONFIG_PATH="$PREFIX/lib/pkgconfig:/usr/lib/$ARCH-linux-gnu/pkgconfig"
export LD_LIBRARY_PATH="$PREFIX/lib:$LD_LIBRARY_PATH"
export SUDO="sudo"

mkdir -p "${PREFIX/include}" "${PREFIX}/include/json-c" "${PREFIX}/lib/pkgconfig"

if [ -z "$ARCH" ]; then
	export ARCH=amd64
fi

if [ "$ARCH" = "i386" ]; then
	export CFLAGS="$CFLAGS -m32"
	#export LDFLAGS="$LDFLAGS -m32"
fi

function install_apt_packages() (
	set -e -o pipefail

	# we commit the generated files for these now:
	# apt-get install -y bison flex gperf re2c
	local apt_packages_to_install="${MYCC} automake pkg-config:${ARCH} gcc-multilib check:${ARCH} libpcre3-dev:${ARCH} libtalloc-dev:${ARCH} libsubunit-dev:${ARCH}"
	if [ "$COVERAGE" = "true" ]; then
		apt_packages_to_install="${apt_packages_to_install} lcov"
	fi
	if [ "$MINIMAL" != "true" ]; then
		# json-c might be having issues: https://bugs.launchpad.net/ubuntu/+source/json-c/+bug/1878738
		apt_packages_to_install="${apt_packages_to_install} libjson-c-dev:${ARCH} liblmdb-dev:${ARCH} libyaml-dev:${ARCH}"
	fi
	$SUDO apt-add-repository -y ppa:ubuntu-toolchain-r/test
	$SUDO apt-get update -y
	$SUDO apt-get purge -y bison flex gperf re2c
	$SUDO apt-get install -y ${apt_packages_to_install}
)

function install_coveralls_lcov() (
	set -e -o pipefail

	if [ "$COVERAGE" = "true" ]; then
		gem install coveralls-lcov
	fi
)

function before_install() (
	set -e -o pipefail

	# currently not using this function, add to .travis.yml instead
	install_apt_packages
	install_coveralls_lcov
)

function configure_handlebars() {
	set -e -o pipefail

	# cflags
	export CFLAGS="$CFLAGS -g -O2"

	# json-c undeprecated json_object_object_get, but the version in xenial
	# is too old, so let's silence deprecated warnings. le sigh.
	export CFLAGS="$CFLAGS -Wno-deprecated-declarations -Wno-error=deprecated-declarations"

	# does gcc-4.9 not support "#pragma GCC diagnostic ignored"?
	if [ "$CC" = "gcc-4.9" ]; then
		export CFLAGS="$CFLAGS -Wno-shadow -Wno-error=shadow -Wno-pointer-sign -Wno-error=pointer-sign -Wno-switch-default -Wno-error=switch-default"
		export CFLAGS="$CFLAGS -Wno-unused-function -Wno-error=unused-function -Wno-inline -Wno-error=inline"
		# these are apparently just broken?
		export CFLAGS="$CFLAGS -Wno-missing-braces -Wno-error=missing-braces"
	fi

	# configure flags
	local extra_configure_flags="--prefix=${PREFIX}"

	if [ -n "$ARCH" ]; then
		extra_configure_flags="${extra_configure_flags} --build=${ARCH}"
	fi

	if [ "$COVERAGE" = "true" ]; then
		extra_configure_flags="$extra_configure_flags --enable-code-coverage"
	fi

	if [ "$HARDENING" = "true" ]; then
		extra_configure_flags="$extra_configure_flags --enable-hardening"
	fi

	if [ "$LTO" = "true" ]; then
		extra_configure_flags="${extra_configure_flags} --enable-lto"
	fi

	if [ "$MINIMAL" = "true" ]; then
		extra_configure_flags="${extra_configure_flags} --disable-handlebars-memory --enable-check --disable-json --disable-lmdb --enable-pcre --disable-pthread --enable-subunit --disable-yaml"
	else
		extra_configure_flags="${extra_configure_flags} --enable-handlebars-memory --enable-check --enable-json --enable-lmdb --enable-pcre  --enable-pthread --enable-subunit --enable-yaml"
	fi

	./bootstrap

	trap "cat config.log" ERR
	./configure ${extra_configure_flags}
	trap - ERR
}

function make_handlebars() (
	set -e -o pipefail

	make clean all
)

function install_handlebars() (
	set -e -o pipefail

	make install
)

function install() (
	set -e -o pipefail

	# currently not using this function, add to .travis.yml instead
	configure_handlebars
	make_handlebars
	install_handlebars
)

function before_script() (
	set -e -o pipefail

	if [ "$COVERAGE" = "true" ]; then
		lcov --directory . --zerocounters
		lcov --directory . --capture --compat-libtool --initial --output-file coverage.info
	fi
)

function test_handlebars() (
	set -e -o pipefail

	make check
)

function run_handlebars_benchmark() (
	set -e -o pipefail

	if [ "$MINIMAL" != "true" ]; then
		./bench/run.sh
	fi
)

function script() (
	set -e -o pipefail

	# currently not using this function, add to .travis.yml instead
	test_handlebars
	run_handlebars_benchmark
)

function after_success() (
	set -e -o pipefail

	if [ "$COVERAGE" = "true" ]; then
		lcov --no-checksum --directory . --capture --compat-libtool --output-file coverage.info
		lcov --remove coverage.info "/usr*" \
			--remove coverage.info "*/tests/*" \
			--remove coverage.info "$HOME/build/json-c/*" \
			--remove coverage.info "handlebars.tab.c" \
			--remove coverage.info "handlebars.lex.c" \
			--remove coverage.info "handlebars_scanners.c" \
			--compat-libtool \
			--output-file coverage.info
		coveralls-lcov coverage.info
	fi
)

function after_failure() (
	set -e -o pipefail

	if [ "$COVERAGE" = "true" ]; then
		for i in `find tests -name "*.log" 2>/dev/null`; do
			echo "-- START ${i}";
			cat "${i}";
			echo "-- END";
		done
	fi
	if [ -f tests/test-suite.log ]; then
			echo "-- START test-suite.log";
			cat tests/test-suite.log
			echo "-- END";
	fi
)
