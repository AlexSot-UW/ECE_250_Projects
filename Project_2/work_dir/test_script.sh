#!/usr/bin/env bash
set -euo pipefail

# Runs ./a.out on each test_files/input/*.in
# Compares against test_files/output/*.out
#
# Normalization:
#   - CRLF -> LF
#   - strip trailing spaces/tabs per line
#   - drop trailing blank lines at EOF
#
# Workaround:
#   - If strict compare fails, try "float-tolerant" compare:
#       32.500000 == 32.5, 1.000000 == 1, -1959.000000 == -1959
#
# Usage:
#   ./test_script.sh                 # run all tests
#   ./test_script.sh test01 test11   # run specific tests by basename

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROGRAM="$SCRIPT_DIR/a.out"
IN_DIR="$SCRIPT_DIR/test_files/input"
OUT_DIR="$SCRIPT_DIR/test_files/output"

if [[ ! -x "$PROGRAM" ]]; then
  echo "Error: executable not found or not executable: $PROGRAM"
  echo "Hint: run 'make' in: $SCRIPT_DIR"
  exit 1
fi
if [[ ! -d "$IN_DIR" ]]; then
  echo "Error: input dir not found: $IN_DIR"
  exit 1
fi
if [[ ! -d "$OUT_DIR" ]]; then
  echo "Error: output dir not found: $OUT_DIR"
  exit 1
fi

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT

normalize_file() {
  # 1) remove CR at end of line
  # 2) strip trailing spaces/tabs
  # 3) remove trailing blank lines at EOF
  sed -e 's/\r$//' -e 's/[ \t]*$//' "$1" | awk '
    { lines[NR] = $0 }
    END {
      last = NR
      while (last > 0 && lines[last] == "") last--
      for (i = 1; i <= last; i++) print lines[i]
    }
  '
}

float_canonicalize() {
  # Converts decimal numbers to a canonical text form:
  #   32.500000 -> 32.5
  #   1.000000  -> 1
  #   -1959.000 -> -1959
  #
  # This is a "workaround" to make formatting differences not fail tests.
  python3 - "$@" <<'PY'
import re, sys

text = sys.stdin.read()

# number token:  -12, 3.14, +0.5000, 10.0
# (doesn't try to rewrite scientific notation; if you output that, it will be left as-is)
pat = re.compile(r'(?<![\w.])([+-]?\d+(?:\.\d+)?)(?![\w.])')

def canon(m):
    s = m.group(1)
    if '.' in s:
        # strip trailing zeros, then trailing dot
        s2 = s.rstrip('0').rstrip('.')
        if s2 == '' or s2 == '+' or s2 == '-':
            s2 = '0'
        # normalize -0 -> 0
        if s2 in ('-0', '+0'):
            s2 = '0'
        return s2
    return s

sys.stdout.write(pat.sub(canon, text))
PY
}

# Pretty printing helpers
hr() { echo "------------------------------------------------------------"; }

print_side_by_side_mismatches() {
  local expected="$1"
  local actual="$2"

  mapfile -t E < "$expected"
  mapfile -t A < "$actual"

  local e_len="${#E[@]}"
  local a_len="${#A[@]}"
  local max_len=$(( e_len > a_len ? e_len : a_len ))

  echo "Mismatched lines (line#: expected | actual):"
  for ((i=0; i<max_len; i++)); do
    local e_line=""
    local a_line=""
    [[ $i -lt $e_len ]] && e_line="${E[$i]}"
    [[ $i -lt $a_len ]] && a_line="${A[$i]}"

    if [[ "$e_line" != "$a_line" ]]; then
      printf "  %3d: %s\n" $((i+1)) "E: $e_line"
      printf "       %s\n"      "A: $a_line"
    fi
  done

  if (( e_len != a_len )); then
    echo
    echo "Note: line counts differ (expected=$e_len, actual=$a_len)."
  fi
}

run_one_test() {
  local name="$1"
  local in_file="$IN_DIR/$name.in"
  local exp_file="$OUT_DIR/$name.out"

  if [[ ! -f "$in_file" ]]; then
    echo "SKIP  $name  (missing input: $in_file)"
    return 2
  fi
  if [[ ! -f "$exp_file" ]]; then
    echo "SKIP  $name  (missing expected: $exp_file)"
    return 2
  fi

  local actual_raw="$TMP_DIR/$name.actual_raw"
  local stderr_file="$TMP_DIR/$name.stderr"

  local expected_norm="$TMP_DIR/$name.expected.norm"
  local actual_norm="$TMP_DIR/$name.actual.norm"

  local expected_float="$TMP_DIR/$name.expected.float.norm"
  local actual_float="$TMP_DIR/$name.actual.float.norm"

  # Run the program
  if ! "$PROGRAM" < "$in_file" > "$actual_raw" 2> "$stderr_file"; then
    echo "FAIL  $name  (program exited non-zero)"
    if [[ -s "$stderr_file" ]]; then
      echo "  stderr:"
      sed 's/^/    /' "$stderr_file"
    fi
    return 1
  fi

  # Normalize for strict comparison
  normalize_file "$exp_file"   > "$expected_norm"
  normalize_file "$actual_raw" > "$actual_norm"

  if diff -q "$expected_norm" "$actual_norm" >/dev/null; then
    echo "PASS  $name"
    return 0
  fi

  # Workaround: float-tolerant comparison
  float_canonicalize < "$expected_norm" > "$expected_float"
  float_canonicalize < "$actual_norm"   > "$actual_float"

  if diff -q "$expected_float" "$actual_float" >/dev/null; then
    echo "PASS* $name  (float-tolerant match)"
    return 0
  fi

  # True failure: print clean debug output
  echo "FAIL  $name"
  echo "  (Strict and float-tolerant comparisons both failed.)"
  echo
  echo "Expected (normalized):"
  nl -ba "$expected_norm" | sed 's/^/  /'
  echo
  echo "Actual (normalized):"
  nl -ba "$actual_norm" | sed 's/^/  /'
  echo
  print_side_by_side_mismatches "$expected_norm" "$actual_norm"
  echo

  # If you want, also show float-normalized mismatch view (often useful)
  echo "Float-normalized view (workaround normalization applied):"
  print_side_by_side_mismatches "$expected_float" "$actual_float"

  return 1
}

# Decide tests to run
TESTS=()
if [[ $# -gt 0 ]]; then
  TESTS=("$@")
else
  shopt -s nullglob
  for f in "$IN_DIR"/*.in; do
    TESTS+=("$(basename "$f" .in)")
  done
  shopt -u nullglob
fi

if [[ ${#TESTS[@]} -eq 0 ]]; then
  echo "No tests found."
  exit 1
fi

echo "Program:   $PROGRAM"
echo "Inputs:    $IN_DIR"
echo "Expected:  $OUT_DIR"
hr

total=0; passed=0; failed=0; skipped=0
FAILED_LIST=()

for t in "${TESTS[@]}"; do
  total=$((total + 1))
  set +e
  run_one_test "$t"
  rc=$?
  set -e

  if [[ $rc -eq 0 ]]; then
    passed=$((passed + 1))
  elif [[ $rc -eq 2 ]]; then
    skipped=$((skipped + 1))
  else
    failed=$((failed + 1))
    FAILED_LIST+=("$t")
  fi
done

hr
echo "Summary: $passed passed, $failed failed, $skipped skipped, $total total"
if [[ $failed -ne 0 ]]; then
  echo "Failed tests:"
  for t in "${FAILED_LIST[@]}"; do
    echo "  - $t"
  done
  exit 1
fi
exit 0
