#!/bin/bash

VALGRIND=''
if [ "$1" = '--valgrind' ]; then
    VALGRIND='valgrind --leak-check=full --error-exitcode=1'
fi

MODE="${DEBUG:+debug}"
MODE="${MODE:-release}"

make tests

DIST_DIR='target'

TOTAL=0
PASSED=0

run_test() {
    local name=$1
    local binary="${DIST_DIR}/${name}.test.${MODE}"

    TOTAL=$((TOTAL + 1))
    echo -n "[$name] "

    if $VALGRIND "$binary" > /dev/null 2>&1; then
        echo 'PASS'
        PASSED=$((PASSED + 1))
    else
        echo 'FAIL'
        $VALGRIND "$binary" || true
    fi
}

run_interactive() {
    local name=$1
    local binary="${DIST_DIR}/${name}.test.${MODE}"
    local base="${name#_interactive_}"

    TOTAL=$((TOTAL + 1))
    echo -n "[$name] "

    if [ ! -f "tests/_interactive/inputs/$base" ]; then
        echo 'SKIP (no input)'
        return
    fi

    actual=$($VALGRIND "$binary" < "tests/_interactive/inputs/$base" 2>&1)
    actual_trimmed=$(echo "$actual" | xargs echo -n)

    if [ -f "tests/_interactive/outputs/$base" ]; then
        expected=$(cat "tests/_interactive/outputs/$base")
        expected_trimmed=$(echo "$expected" | xargs echo -n)

        if [ "$actual_trimmed" = "$expected_trimmed" ]; then
            echo 'PASS'
            PASSED=$((PASSED + 1))
        else
            echo -e 'FAIL\n-------- EXPECTED --------'
            echo "$expected"
            echo '--------- ACTUAL ---------'
            echo "$actual"
        fi
    else
        if [ $? -eq 0 ]; then
            echo 'PASS'
            PASSED=$((PASSED + 1))
        else
            echo 'FAIL'
        fi
    fi
}


for test in $(ls -p tests | grep .cpp); do
    test="${test%.*}"

    if [[ "$test" == '_interactive_'* ]]; then
        run_interactive $test
    else
        run_test $test
    fi
done


echo ""
echo "$PASSED/$TOTAL passed"
