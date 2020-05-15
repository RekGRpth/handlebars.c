#!/usr/bin/env bash

set -e -o pipefail

# https://stackoverflow.com/a/4774063
SCRIPTPATH="$( cd "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"

HANDLEBARSC="${SCRIPTPATH}/../bin/handlebarsc"
TIME=`which time`
BC=`which bc`
RUN_COUNT=10000

cd ${SCRIPTPATH}

function strip_extension() {
	set -e -o pipefail

	filename=$(basename -- "$1")
	extension="${filename##*.}"
	filename="${filename%.*}"
	echo $filename

	return 0
}

function run_test_inner() (
	set -e -o pipefail

	TEMPLATE="templates/${1}"
	DATA="templates/${2}"
	EXTRA_OPTS="${3} ${4} --partial-loader --partial-path ./partials --partial-ext .handlebars"
	time_outputfile=`mktemp`
	actual_outputfile=`mktemp`
	expected_file=templates/$(strip_extension $TEMPLATE).expected

	# header
	echo "----- Running: ${1} -----"
	if [ ! -z "${4}" ]; then
		echo "Extra flags: ${4}"
	fi

	# execute command
	COMMAND="${TIME} --output=${time_outputfile} -p ${HANDLEBARSC} --run-count ${RUN_COUNT} ${EXTRA_OPTS} --data ${DATA} ${TEMPLATE}"
	${COMMAND} 1>${actual_outputfile}

	# print time
	real_time=`cat ${time_outputfile} | grep real | awk '{ print $2 }'`
	real_microsecs_per_run=`${BC} -l <<< "1000000 * ${real_time}/${RUN_COUNT}"`
	echo "runs ${RUN_COUNT}"
	cat ${time_outputfile}
	printf "each %g us\n" "${real_microsecs_per_run}"

	# compare with expected file
	trap "echo FAIL; echo Expected: `cat ${expected_file}`; echo Actual: `cat ${actual_outputfile}`; echo; exit 1" ERR
	diff --ignore-all-space --text ${expected_file} ${actual_outputfile}
	trap - ERR
	echo "PASS"

	echo

	rm ${time_outputfile} ${actual_outputfile}

	return 0
)

function run_test() (
	set -e -o pipefail

	run_test_inner "${1}" "${2}" "${3}"

	run_test_inner "${1}" "${2}" "${3}" "--no-convert-input"

	return 0
)

run_test "array-each.handlebars" "array-each.json"
run_test "array-each.mustache" "array-each.json" "--flags compat"

run_test "complex.handlebars" "complex.json"
run_test "complex.mustache" "complex.json" "--flags compat"

run_test "data.handlebars" "data.json"

run_test "depth-1.handlebars" "depth-1.json"
run_test "depth-1.mustache" "depth-1.json" "--flags compat"

run_test "depth-2.handlebars" "depth-2.json"
run_test "depth-2.mustache" "depth-2.json" "--flags compat"

run_test "object-mustache.handlebars" "object-mustache.json"

run_test "object.handlebars" "object.json"
run_test "object.mustache" "object.json"  "--flags compat"

run_test "partial.handlebars" "partial.json"
run_test "partial.mustache" "partial.json"  "--flags compat"

run_test "partial-recursion.handlebars" "partial-recursion.json"
run_test "partial-recursion.mustache" "partial-recursion.json"  "--flags compat --partial-ext mustache"

run_test "paths.handlebars" "paths.json"
run_test "paths.mustache" "paths.json"  "--flags compat"

run_test "string.handlebars" "string.json"
run_test "string.mustache" "string.json"  "--flags compat"

run_test "variables.handlebars" "variables.json"
run_test "variables.mustache" "variables.json"  "--flags compat"

exit 0
