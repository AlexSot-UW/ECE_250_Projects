#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROGRAM="$SCRIPT_DIR/a.out"
IN_DIR="$SCRIPT_DIR/test_files/input"
OUT_DIR="$SCRIPT_DIR/test_files/output"

SHOW_ALL=0
if [[ $# -gt 0 && "$1" == "--show-all" ]]; then
  SHOW_ALL=1
  shift
fi

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT

hr() { echo "------------------------------------------------------------"; }
die() { echo "Error: $1"; exit 1; }

[[ -x "$PROGRAM" ]] || die "executable not found or not executable: $PROGRAM"
[[ -d "$IN_DIR" ]]   || die "input dir not found: $IN_DIR"
[[ -d "$OUT_DIR" ]]  || die "output dir not found: $OUT_DIR"

normalize_file() {
  sed -e 's/\r$//' -e 's/[ \t]*$//' "$1" | awk '
    { lines[NR] = $0 }
    END {
      last = NR
      while (last > 0 && lines[last] == "") last--
      for (i = 1; i <= last; i++) print lines[i]
    }
  '
}

# FIXED: uses -c so stdin is available for the data stream
float_canonicalize() {
  python3 -c '
import re, sys
text = sys.stdin.read()
pat = re.compile(r"(?<![\\w.])([+-]?\\d+(?:\\.\\d+)?)(?![\\w.])")

def canon(m):
    s = m.group(1)
    if "." in s:
        s2 = s.rstrip("0").rstrip(".")
        if s2 in ("", "+", "-"):
            s2 = "0"
        if s2 in ("-0", "+0"):
            s2 = "0"
        return s2
    return s

sys.stdout.write(pat.sub(canon, text))
'
}

print_file_block() {
  local title="$1"
  local path="$2"
  echo "$title"
  hr
  if [[ -f "$path" ]]; then
    if [[ -s "$path" ]]; then
      cat "$path"
    else
      echo "(empty)"
    fi
  else
    echo "(missing file)"
  fi
  hr
}

run_one_test() {
  local name="$1"
  local in_file="$IN_DIR/$name.in"
  local exp_file="$OUT_DIR/$name.out"

  [[ -f "$in_file" ]] || { echo "SKIP  $name  (missing input)"; return 2; }
  [[ -f "$exp_file" ]] || { echo "SKIP  $name  (missing expected output)"; return 2; }

  local actual_raw="$TMP_DIR/$name.actual_raw"
  local stderr_file="$TMP_DIR/$name.stderr"

  local expected_norm="$TMP_DIR/$name.expected.norm"
  local actual_norm="$TMP_DIR/$name.actual.norm"

  local expected_float="$TMP_DIR/$name.expected.float.norm"
  local actual_float="$TMP_DIR/$name.actual.float.norm"

  if ! "$PROGRAM" < "$in_file" > "$actual_raw" 2> "$stderr_file"; then
    echo "FAIL  $name  (program exited non-zero)"
    [[ -s "$stderr_file" ]] && { echo "stderr:"; sed 's/^/  /' "$stderr_file"; }
    if [[ $SHOW_ALL -eq 1 ]]; then
      echo
      print_file_block "RAW EXPECTED ($name.out):" "$exp_file"
      print_file_block "RAW ACTUAL (program output):" "$actual_raw"
    fi
    return 1
  fi

  normalize_file "$exp_file"   > "$expected_norm"
  normalize_file "$actual_raw" > "$actual_norm"

  local strict_ok=0
  diff -q "$expected_norm" "$actual_norm" >/dev/null && strict_ok=1

  local float_ok=0
  if [[ $strict_ok -eq 0 ]]; then
    float_canonicalize < "$expected_norm" > "$expected_float"
    float_canonicalize < "$actual_norm"   > "$actual_float"
    diff -q "$expected_float" "$actual_float" >/dev/null && float_ok=1
  fi

  if [[ $SHOW_ALL -eq 1 ]]; then
    echo
    echo "TEST: $name"
    print_file_block "RAW EXPECTED ($name.out):" "$exp_file"
    print_file_block "RAW ACTUAL (program output):" "$actual_raw"
    print_file_block "NORMALIZED EXPECTED (used for strict compare):" "$expected_norm"
    print_file_block "NORMALIZED ACTUAL (used for strict compare):" "$actual_norm"
    if [[ $strict_ok -eq 0 ]]; then
      print_file_block "FLOAT-NORMALIZED EXPECTED (used for PASS* compare):" "$expected_float"
      print_file_block "FLOAT-NORMALIZED ACTUAL (used for PASS* compare):" "$actual_float"
    fi
  fi

  if [[ $strict_ok -eq 1 ]]; then
    echo "PASS  $name"
    return 0
  fi
  if [[ $float_ok -eq 1 ]]; then
    echo "PASS* $name  (float-tolerant match)"
    return 0
  fi

  echo "FAIL  $name"
  echo "  Strict and float-tolerant comparisons both failed."
  echo "  Tip: re-run with --show-all to see the compared files."
  return 1
}

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
[[ ${#TESTS[@]} -gt 0 ]] || die "No tests found."

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