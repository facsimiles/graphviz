#!/usr/bin/env python3

"""
Graphviz regression test driver

TODO:
 Report differences with shared version and with new output.
"""

import filecmp
import os
import re
import shutil
import subprocess
import sys
import platform
import argparse
import atexit

TESTFILE = os.path.join(os.path.dirname(__file__), "tests.txt")
                                      # Test specifications
GRAPHDIR = os.path.join(os.path.dirname(__file__), "graphs")
                                      # Directory of input graphs and data
OUTDIR = "ndata"                      # Directory for test output
OUTHTML = "nhtml"                     # Directory for html test report
REFDIR = os.environ.get("REFDIR", "") # Directory for expected test output
GENERATE = False                      # If set, generate test data
VERBOSE = False                       # If set, give verbose output
NOOP = False                          # If set, just print list of tests
DOT = os.environ.get("DOT", shutil.which("dot"))
DIFFIMG = os.environ.get("DIFFIMG", shutil.which("diffimg"))

CRASH_CNT = 0
DIFF_CNT = 0
TOT_CNT = 0
TESTTYPES = {}
TMPINFILE = f"tmp{os.getpid()}.gv"
TMPFILE1 = f"tmpnew{os.getpid()}"
TMPFILE2 = f"tmpref{os.getpid()}"

def readLine():
  """
  Read single line, storing it in LINE.
  Returns the line on success, else returns None
  """
  LINE = f3.readline()
  if LINE != "":
    return LINE.strip()
  return None

def skipLines():
  """
  Skip blank lines and comments (lines starting with #)
  Use first real line as the test name
  """
  while True:
    LINE = readLine()
    if LINE is None:
      return None
    if LINE and not LINE.startswith("#"):
      return LINE

def readSubtests():
  """
  Subtests have the form: layout format optional_flags
  Store the 3 parts in the arrays ALG, FMT, FLAGS.
  Stop at a blank line
  """
  while True:
    LINE = readLine()
    if LINE == "":
      return
    if not LINE.startswith("#"):
      ALG0, FMT0, *FLAGS0 = LINE.split(" ")
      yield {
              "ALG": ALG0,
              "FMT": FMT0,
              "FLAGS": FLAGS0,
            }

def readTest():
  """
  Read and parse a test.
  """
  # read test name
  LINE = skipLines()
  if LINE is not None:
    TESTNAME = LINE
  else:
    return None

  # read input graph
  LINE = skipLines()
  if LINE is not None:
    GRAPH = LINE
  else:
    return None

  SUBTESTS = list(readSubtests())
  return {
      "TESTNAME": TESTNAME,
      "GRAPH": GRAPH,
      "SUBTESTS": SUBTESTS,
      }

def doDiff(OUTFILE, testname, subtest_index, fmt):
  """
  Compare old and new output and report if different.
   Args: testname index fmt
  """
  global DIFF_CNT
  FILE1 = os.path.join(OUTDIR, OUTFILE)
  FILE2 = os.path.join(REFDIR, OUTFILE)
  F = fmt.split(":")[0]
  # FIXME: Remove when https://gitlab.com/graphviz/graphviz/-/issues/1789 is
  # fixed
  if platform.system() == "Windows" and \
     F in ["ps", "gv"] and \
     testname in ["clusters", "compound", "rootlabel"]:
    print("Warning: Skipping {0} output comparison for test {1}:{2} : format "
          "{3} because the order of clusters in gv or ps output is not "
          "stable on Windows (#1789)"
          .format(F, testname, subtest_index, fmt),
          file=sys.stderr)
    return
  if F in ["ps", "ps2"]:

    with open(FILE1, "rt") as src:
      with open(TMPFILE1, "wt") as dst:
        done_setup = False
        for line in src:
          if done_setup:
            dst.write(line)
          else:
            done_setup = re.match(r"%%End.*Setup", line) is not None

    with open(FILE2, "rt") as src:
      with open(TMPFILE2, "wt") as dst:
        done_setup = False
        for line in src:
          if done_setup:
            dst.write(line)
          else:
            done_setup = re.match(r"%%End.*Setup", line) is not None

    returncode = 0 if filecmp.cmp(TMPFILE1, TMPFILE2) else -1
  elif F == "svg":
    with open(FILE1) as f:
      a = re.sub(r"^<!--.*-->$", "", f.read(), flags=re.MULTILINE)
    with open(FILE2) as f:
      b = re.sub(r"^<!--.*-->$", "", f.read(), flags=re.MULTILINE)
    returncode = 0 if a.strip() == b.strip() else -1
  elif F == "png":
    # FIXME: remove when https://gitlab.com/graphviz/graphviz/-/issues/1788 is fixed
    if os.environ.get("build_system") == "cmake":
      print(f"Warning: Skipping PNG image comparison for test {testname}:"
            f"{subtest_index} : format: {fmt} because CMake builds does not "
             "contain the diffimg utility (#1788)",
            file=sys.stderr)
      return
    returncode = subprocess.call(
      [DIFFIMG, FILE1, FILE2, os.path.join(OUTHTML, f"dif_{OUTFILE}")],
    )
    if returncode != 0:
      with open(os.path.join(OUTHTML, "index.html"), mode="a") as fd:
        fd.write("<p>\n")
        shutil.copyfile(FILE2, os.path.join(OUTHTML, f"old_{OUTFILE}"))
        fd.write(f'<img src="old_{OUTFILE}" width="192" height="192">\n')
        shutil.copyfile(FILE1, os.path.join(OUTHTML, f"new_{OUTFILE}"))
        fd.write(f'<img src="new_{OUTFILE}" width="192" height="192">\n')
        fd.write(f'<img src="dif_{OUTFILE}" width="192" height="192">\n')
    else:
      os.unlink(os.path.join(OUTHTML, f"dif_{OUTFILE}"))
  else:
    with open(FILE2) as a:
      with open(FILE1) as b:
        returncode = 0 if a.read().strip() == b.read().strip() else -1
  if returncode != 0:
    print(f"Test {testname}:{subtest_index} : == Failed == {OUTFILE}", file=sys.stderr)
    DIFF_CNT += 1
  else:
    if VERBOSE:
      print(f"Test {testname}:{subtest_index} : == OK == {OUTFILE}", file=sys.stderr)

def genOutname(name, alg, fmt):
  """
  Generate output file name given 3 parameters.
    testname layout format
  If format ends in :*, remove this, change the colons to underscores,
  and append to basename
  If the last two parameters have been used before, add numeric suffix.
  """
  global TESTTYPES
  fmt_split = fmt.split(":")
  if len(fmt_split) >= 2:
    F = fmt_split[0]
    XFMT = f'_{"_".join(fmt_split[1:])}'
  else:
    F=fmt
    XFMT=""

  IDX = alg + XFMT + F
  j = TESTTYPES.get(IDX, 0)
  if j == 0:
    TESTTYPES[IDX] = 1
    J = ""
  else:
    TESTTYPES[IDX]= j + 1
    J = str(j)
  OUTFILE = f"{name}_{alg}{XFMT}{J}.{F}"
  return OUTFILE

def doTest(test):
  """
  Run a single test.
  """
  global TOT_CNT
  global CRASH_CNT
  global TESTTYPES
  TESTNAME = test["TESTNAME"]
  SUBTESTS = test["SUBTESTS"]
  if len(SUBTESTS) == 0:
    return
  GRAPH = test["GRAPH"]
  if GRAPH == "=":
    INFILE = os.path.join(GRAPHDIR, f"{TESTNAME}.gv")
  elif GRAPH.startswith("graph") or GRAPH.startswith("digraph"):
    with open(TMPINFILE, mode="w") as fd:
      fd.write(GRAPH)
    INFILE = TMPINFILE
  elif os.path.splitext(GRAPH)[1] == ".gv":
    INFILE = os.path.join(GRAPHDIR, GRAPH)
  else:
    print(f"Unknown graph spec, test {TESTNAME} - ignoring",
          file=sys.stderr)
    return

  for i, SUBTEST in enumerate(SUBTESTS):
    TOT_CNT += 1
    OUTFILE = genOutname(TESTNAME, SUBTEST["ALG"], SUBTEST["FMT"])
    OUTPATH = os.path.join(OUTDIR, OUTFILE)
    KFLAGS = SUBTEST["ALG"]
    TFLAGS = SUBTEST["FMT"]
    if KFLAGS:
      KFLAGS = f"-K{KFLAGS}"
    if TFLAGS:
      TFLAGS = f"-T{TFLAGS}"
    testcmd = [DOT]
    if KFLAGS:
      testcmd += [KFLAGS]
    if TFLAGS:
      testcmd += [TFLAGS]
    testcmd += SUBTEST["FLAGS"] + ["-o", OUTPATH, INFILE]
    if VERBOSE:
      print(" ".join(testcmd))
    if NOOP:
      continue
    # FIXME: Remove when https://gitlab.com/graphviz/graphviz/-/issues/1786 is
    # fixed
    if os.environ.get("build_system") == "cmake" and \
       SUBTEST["FMT"] == "png:gd":
      print(f'Skipping test {TESTNAME}:{i} : format {SUBTEST["FMT"]} because '
             "CMake builds does not support format png:gd (#1786)",
            file=sys.stderr)
      continue
    # FIXME: Remove when https://gitlab.com/graphviz/graphviz/-/issues/1269 is
    # fixed
    if platform.system() == "Windows" and \
       os.environ.get("build_system") == "msbuild" and \
       "-Goverlap=false" in SUBTEST["FLAGS"]:
      print(f"Skipping test {TESTNAME}:{i} : with flag -Goverlap=false because "
             "it fails with Windows MSBuild builds which are not built with "
             "triangulation library (#1269)",
            file=sys.stderr)
      continue
    # FIXME: Remove when https://gitlab.com/graphviz/graphviz/-/issues/1787 is
    # fixed
    if platform.system() == "Windows" and \
       os.environ.get("build_system") == "msbuild" and \
       os.environ.get("configuration") == "Debug" and \
       TESTNAME == "user_shapes":
      print(f"Skipping test {TESTNAME}:{i} : using shapefile because it fails "
             "with Windows MSBuild Debug builds (#1787)",
            file=sys.stderr)
      continue
    # FIXME: Remove when https://gitlab.com/graphviz/graphviz/-/issues/1790 is
    # fixed
    if platform.system() == "Windows" and \
       TESTNAME == "ps_user_shapes":
      print(f"Skipping test {TESTNAME}:{i} : using PostScript shapefile "
             "because it fails with Windows builds (#1790)",
            file=sys.stderr)
      continue

    result = subprocess.Popen(
       testcmd,
       universal_newlines=True,
       stderr = subprocess.PIPE,
    )
    _, errout = result.communicate()
    RVAL = result.returncode

    if errout:
      print(errout)

    if RVAL != 0 or not os.path.exists(OUTPATH):
      CRASH_CNT += 1
      print(f"Test {TESTNAME}:{i} : == Layout failed ==", file=sys.stderr)
      print(f'  {" ".join(testcmd)}', file=sys.stderr)
    elif GENERATE:
      continue
    elif os.path.exists(os.path.join(REFDIR, OUTFILE)):
      doDiff(OUTFILE, TESTNAME, i, SUBTEST["FMT"])
    else:
      sys.stderr.write(f"Test {TESTNAME}:{i} : == No file {REFDIR}/{OUTFILE} "
                       "for comparison ==\n")

  # clear TESTTYPES
  TESTTYPES = {}

def cleanup():
  """
  Delete temporary files.
  """
  shutil.rmtree(TMPFILE1, ignore_errors=True)
  shutil.rmtree(TMPFILE2, ignore_errors=True)
  shutil.rmtree(TMPINFILE, ignore_errors=True)
atexit.register(cleanup)

# Set REFDIR
REFDIR = os.environ.get("REFDIR")
if not REFDIR:
  if platform.system() == "Linux":
    REFDIR = "linux.x86"
  elif platform.system() == "Darwin":
    REFDIR = "macosx"
  elif platform.system() == "Windows":
    REFDIR = "nshare"
  else:
    print('Unrecognized system "{0}"'.format(platform.system()), file=sys.stderr)
    REFDIR = "nshare"

parser = argparse.ArgumentParser(description="Run regression tests.")
parser.add_argument("-g",
                    dest="generate",
                    action="store_true",
                    help="generate test data"
)
parser.add_argument("-v",
                    dest="verbose",
                    action="store_true",
                    help="verbose"
)
parser.add_argument("-n",
                    dest="noop",
                    action="store_true",
                    help="noop"
)
parser.add_argument("testfile",
                    nargs="?",
                    help="test files"
)
args = parser.parse_args()
VERBOSE = args.verbose or args.noop
NOOP = args.noop
GENERATE = args.generate
if GENERATE:
  if not os.path.isdir(REFDIR):
    os.mkdir(OUTDIR)
  OUTDIR = REFDIR

if args.testfile:
  if os.path.exists(args.testfile):
    TESTFILE = args.testfile
  else:
    print(f"Test file {args.testfile} does not exist", file=sys.stderr)
    sys.exit(1)

# Check environment and initialize

if not NOOP:
  if not os.path.isdir(REFDIR):
    print(f"Test data directory {REFDIR} does not exist",
          file=sys.stderr)
    sys.exit(1)

if not os.path.isdir(OUTDIR):
  os.mkdir(OUTDIR)

if not os.path.isdir(OUTHTML):
  os.mkdir(OUTHTML)
for filename in os.listdir(OUTHTML):
  os.unlink(os.path.join(OUTHTML, filename))

if not DOT:
  print("Could not find a value for DOT", file=sys.stderr)
  sys.exit(1)
if not os.path.isfile(DOT) or not os.access(DOT, os.X_OK):
  print(f"{DOT} program is not executable")
  sys.exit(1)

if not GENERATE:
  if DIFFIMG:
    if not os.path.isfile(DIFFIMG) or not os.access(DIFFIMG, os.X_OK):
      print(f"{DIFFIMG} program is not executable")
      sys.exit(1)
  else:
    print("Could not find a value for DIFFIMG", file=sys.stderr)
    # FIXME: Remove workaround for missing diffimg when
    # https://gitlab.com/graphviz/graphviz/-/issues/1788 is fixed
    if os.environ.get("build_system") != "cmake":
      sys.exit(1)
#    sys.exit(1)


f3 = open(TESTFILE)
while True:
  TEST = readTest()
  if TEST is None:
    break
  doTest(TEST)
if NOOP:
  print(f"No. tests: {TOT_CNT}", file=sys.stderr)
elif GENERATE:
  print(f"No. tests: {TOT_CNT} Layout failures: {CRASH_CNT}", file=sys.stderr)
else:
  print(f"No. tests: {TOT_CNT} Layout failures: {CRASH_CNT} Changes: "
        f"{DIFF_CNT}", file=sys.stderr)
EXIT_STATUS = CRASH_CNT + DIFF_CNT
sys.exit(EXIT_STATUS)
