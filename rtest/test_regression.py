"""
Graphviz regression tests

The test cases in this file relate to previously observed bugs. A failure of one
of these indicates that a past bug has been reintroduced.
"""

import json
import os
from pathlib import Path
import platform
import re
import shutil
import signal
import stat
import subprocess
import sys
import tempfile
from typing import List
import xml.etree.ElementTree as ET
import pytest

sys.path.append(os.path.dirname(__file__))
from gvtest import ROOT, run_c #pylint: disable=C0413

def is_ndebug_defined() -> bool:
  """
  are assertions disabled in the Graphviz build under test?
  """

  # the Windows release builds set NDEBUG
  if os.environ.get("configuration") == "Release":
    return True

  return False

# The terminology used in rtest.py is a little inconsistent. At the
# end it reports the total number of tests, the number of "failures"
# (crashes) and the number of "changes" (which is the number of tests
# where the output file did not match the reference file). However,
# for each test that detects "changes", it prints an error message
# saying "== Failed ==" which thus is not counted as a failure at the
# end.

def test_regression_subset_differences():
  """
  First run a subset of all the tests that produces equal output files
  for all platforms and fail the test if there are differences.
  """

  os.chdir(Path(__file__).resolve().parent)
  subprocess.check_call([sys.executable, "rtest.py", "tests_subset.txt"])

def test_regression_failure():
  """
  Secondly, run all tests but ignore differences and fail the test
  only if there is a crash. This will leave the differences for png
  output in rtest/nhtml/index.html for review.
  """

  os.chdir(Path(__file__).resolve().parent)
  result = subprocess.Popen([sys.executable, "rtest.py"],
                            stderr=subprocess.PIPE, universal_newlines=True)
  text = result.communicate()[1]
  print(text)
  assert "Layout failures: 0" in text
# FIXME: re-enable when all tests pass on all platforms
#    assert result.returncode == 0

@pytest.mark.xfail(strict=not is_ndebug_defined()) # FIXME
def test_14():
  """
  using ortho and twopi in combination should not cause an assertion failure
  https://gitlab.com/graphviz/graphviz/-/issues/14
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "14.dot"
  assert input.exists(), "unexpectedly missing test case"

  # process it with Graphviz
  subprocess.check_call(["dot", "-Tsvg", "-o", os.devnull, input])

def test_56():
  """
  parsing a particular graph should not cause a Trapezoid-table overflow
  assertion failure
  https://gitlab.com/graphviz/graphviz/-/issues/56
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "56.dot"
  assert input.exists(), "unexpectedly missing test case"

  # FIXME: remove this block when this #56 is fixed
  if not is_ndebug_defined() and platform.system() != "Windows":
    with pytest.raises(subprocess.CalledProcessError):
      subprocess.check_call(["dot", "-Tsvg", "-o", os.devnull, input])
    return

  # process it with Graphviz
  subprocess.check_call(["dot", "-Tsvg", "-o", os.devnull, input])

def test_131():
  """
  PIC back end should produce valid output
  https://gitlab.com/graphviz/graphviz/-/issues/131
  """

  # a basic graph
  dot = "digraph { a -> b; c -> d; }"

  # ask Graphviz to process this to PIC
  pic = subprocess.check_output(["dot", "-Tpic"], input=dot,
    universal_newlines=True)

  if shutil.which("gpic") is None:
    pytest.skip("GNU PIC not available")

  # ask GNU PIC to process the Graphviz output
  subprocess.run(["gpic"], input=pic, stdout=subprocess.DEVNULL, check=True,
    universal_newlines=True)

@pytest.mark.parametrize("testcase", ("144_no_ortho.dot", "144_ortho.dot"))
def test_144(testcase: str):
  """
  using ortho should not result in head/tail confusion
  https://gitlab.com/graphviz/graphviz/-/issues/144
  """

  # locate our associated test cases in this directory
  input = Path(__file__).parent / testcase
  assert input.exists(), "unexpectedly missing test case"

  # process the non-ortho one into JSON
  out = subprocess.check_output(["dot", "-Tjson", input],
    universal_newlines=True)
  data = json.loads(out)

  # find the two nodes, “A” and “B”
  A = [x for x in data["objects"] if x["name"] == "A"][0]
  B = [x for x in data["objects"] if x["name"] == "B"][0]

  # find the edge between them
  edge = [x for x in data["edges"]
          if x["tail"] == A["_gvid"] and x["head"] == B["_gvid"]][0]

  # the edge between them should have been routed vertically
  points = edge["_draw_"][1]["points"]
  xs = [x for x, _ in points]
  assert all(x == xs[0] for x in xs), "A->B not routed vertically"

  # determine whether it is routed down or up
  ys = [y for _, y in points]
  if ys == sorted(ys):
    routed_up = True
  elif list(reversed(ys)) == sorted(ys):
    routed_up = False
  else:
    pytest.fail("A->B seems routed neither straight up nor down")

  # determine Graphviz’ idea of which end is the head and which is the tail
  head_point = edge["_hldraw_"][2]["pt"]
  tail_point = edge["_tldraw_"][2]["pt"]
  head_is_top = head_point[1] > tail_point[1]

  # FIXME: remove when #144 is fixed
  if testcase == "144_ortho.dot":
    assert routed_up != head_is_top, "#144 fixed?"
    return

  # this should be consistent with the direction the edge is drawn
  assert routed_up == head_is_top, "heap/tail confusion"

def test_165():
  """
  dot should be able to produce properly escaped xdot output
  https://gitlab.com/graphviz/graphviz/-/issues/165
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "165.dot"
  assert input.exists(), "unexpectedly missing test case"

  # ask Graphviz to translate it to xdot
  output = subprocess.check_output(["dot", "-Txdot", input],
    universal_newlines=True)

  # find the line containing the _ldraw_ attribute
  ldraw = re.search(r"^\s*_ldraw_\s*=(?P<value>.*?)$", output, re.MULTILINE)
  assert ldraw is not None, "no _ldraw_ attribute in graph"

  # this should contain the label correctly escaped
  assert r'hello \\\" world' in ldraw.group("value"), \
    "unexpected ldraw contents"

def test_165_2():
  """
  variant of test_165() that checks a similar problem for edges
  https://gitlab.com/graphviz/graphviz/-/issues/165
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "165_2.dot"
  assert input.exists(), "unexpectedly missing test case"

  # ask Graphviz to translate it to xdot
  output = subprocess.check_output(["dot", "-Txdot", input],
    universal_newlines=True)

  # find the lines containing _ldraw_ attributes
  ldraw = re.findall(r"^\s*_ldraw_\s*=(.*?)$", output, re.MULTILINE)
  assert ldraw is not None, "no _ldraw_ attributes in graph"

  # one of these should contain the label correctly escaped
  assert any(r'hello \\\" world' in l for l in ldraw), \
    "unexpected ldraw contents"

def test_165_3():
  """
  variant of test_165() that checks a similar problem for graph labels
  https://gitlab.com/graphviz/graphviz/-/issues/165
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "165_3.dot"
  assert input.exists(), "unexpectedly missing test case"

  # ask Graphviz to translate it to xdot
  output = subprocess.check_output(["dot", "-Txdot", input],
    universal_newlines=True)

  # find the lines containing _ldraw_ attributes
  ldraw = re.findall(r"^\s*_ldraw_\s*=(.*?)$", output, re.MULTILINE)
  assert ldraw is not None, "no _ldraw_ attributes in graph"

  # one of these should contain the label correctly escaped
  assert any(r'hello \\\" world' in l for l in ldraw), \
    "unexpected ldraw contents"

def test_167():
  """
  using concentrate=true should not result in a segfault
  https://gitlab.com/graphviz/graphviz/-/issues/167
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent /  "167.dot"
  assert input.exists(), "unexpectedly missing test case"

  # process this with dot
  ret = subprocess.call(["dot", "-Tpdf", "-o", os.devnull, input])

  # Graphviz should not have caused a segfault
  assert ret != -signal.SIGSEGV, "Graphviz segfaulted"

@pytest.mark.skipif(shutil.which("gv2gxl") is None or
                    shutil.which("gxl2gv") is None,
                    reason="GXL tools not available")
def test_517():
  """
  round tripping a graph through gv2gxl should not lose HTML labels
  https://gitlab.com/graphviz/graphviz/-/issues/517
  """

  # our test case input
  input = \
    'digraph{\n' \
    '  A[label=<<TABLE><TR><TD>(</TD><TD>A</TD><TD>)</TD></TR></TABLE>>]\n' \
    '  B[label="<TABLE><TR><TD>(</TD><TD>B</TD><TD>)</TD></TR></TABLE>"]\n' \
    '}'

  # translate it to GXL
  p = subprocess.Popen(["gv2gxl"], stdin=subprocess.PIPE,
    stdout=subprocess.PIPE, universal_newlines=True)
  gxl, _ = p.communicate(input)
  assert p.returncode == 0

  # translate this back to Dot
  p = subprocess.Popen(["gxl2gv"], stdin=subprocess.PIPE,
    stdout=subprocess.PIPE, universal_newlines=True)
  dot, _ = p.communicate(gxl)
  assert p.returncode == 0

  # the result should have both expected labels somewhere
  assert \
    "label=<<TABLE><TR><TD>(</TD><TD>A</TD><TD>)</TD></TR></TABLE>>" in dot, \
    "HTML label missing"
  assert \
    'label="<TABLE><TR><TD>(</TD><TD>B</TD><TD>)</TD></TR></TABLE>"' in dot, \
    "regular label missing"

def test_793():
  """
  Graphviz should not crash when using VRML output with a non-writable current
  directory
  https://gitlab.com/graphviz/graphviz/-/issues/793
  """

  # create a non-writable directory
  with tempfile.TemporaryDirectory() as tmp:
    t = Path(tmp)
    t.chmod(t.stat().st_mode & ~stat.S_IWRITE)

    # ask the VRML back end to handle a simple graph, using the above as the
    # current working directory
    p = subprocess.Popen(["dot", "-Tvrml", "-o", os.devnull], cwd=t)
    p.communicate("digraph { a -> b; }")

  # Graphviz should not have caused a segfault
  assert p.returncode != -signal.SIGSEGV, "Graphviz segfaulted"

def test_797():
  """
  “&;” should not be considered an XML escape sequence
  https://gitlab.com/graphviz/graphviz/-/issues/797
  """

  # some input containing the invalid escape
  input = 'digraph tree {\n' \
          '"1" [shape="box", label="&amp; &amp;;", URL="a"];\n' \
          '}'

  # process this with the client-side imagemap back end
  p = subprocess.Popen(["dot", "-Tcmapx"], stdin=subprocess.PIPE,
    stdout=subprocess.PIPE, universal_newlines=True)
  output, _ = p.communicate(input)

  assert p.returncode == 0

  # the escape sequences should have been preserved
  assert "&amp; &amp;" in output

def test_1221():
  """
  assigning a node to two clusters with newrank should not cause a crash
  https://gitlab.com/graphviz/graphviz/-/issues/1221
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1221.dot"
  assert input.exists(), "unexpectedly missing test case"

  # process this with dot
  subprocess.check_call(["dot", "-Tsvg", "-o", os.devnull, input])

@pytest.mark.skipif(shutil.which("gv2gml") is None,
                    reason="gv2gml not available")
def test_1276():
  """
  quotes within a label should be escaped in translation to GML
  https://gitlab.com/graphviz/graphviz/-/issues/1276
  """

  # DOT input containing a label with quotes
  dot = 'digraph test {\n' \
        '  x[label=<"Label">];\n' \
        '}'

  # process this to GML
  p = subprocess.Popen(["gv2gml"], stdin=subprocess.PIPE,
                       stdout=subprocess.PIPE, universal_newlines=True)
  gml, _ = p.communicate(dot)

  assert p.returncode == 0, "gv2gml failed"

  # the unescaped label should not appear in the output
  assert '""Label""' not in gml, "quotes not escaped in label"

  # the escaped label should appear in the output
  assert '"&quot;Label&quot;"' in gml or '"&#34;Label&#34;"' in gml, \
    "escaped label not found in GML output"

def test_1314():
  """
  test that a large font size that produces an overflow in Pango is rejected
  https://gitlab.com/graphviz/graphviz/-/issues/1314
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1314.dot"
  assert input.exists(), "unexpectedly missing test case"

  # ask Graphviz to process it, which should fail
  try:
    subprocess.check_call(["dot", "-Tsvg", "-o", os.devnull, input])
  except subprocess.CalledProcessError:
    return

  # the execution did not fail as expected
  pytest.fail("dot incorrectly exited with success")

@pytest.mark.xfail(strict=not is_ndebug_defined()) # FIXME
def test_1408():
  """
  parsing particular ortho layouts should not cause an assertion failure
  https://gitlab.com/graphviz/graphviz/-/issues/1408
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1408.dot"
  assert input.exists(), "unexpectedly missing test case"

  # process it with Graphviz
  subprocess.check_call(["dot", "-Tsvg", "-o", os.devnull, input])

def test_1411():
  """
  parsing strings containing newlines should not disrupt line number tracking
  https://gitlab.com/graphviz/graphviz/-/issues/1411
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1411.dot"
  assert input.exists(), "unexpectedly missing test case"

  # process it with Graphviz (should fail)
  p = subprocess.Popen(["dot", "-Tsvg", "-o", os.devnull, input],
    stderr=subprocess.PIPE, universal_newlines=True)
  _, output = p.communicate()

  assert p.returncode != 0, "Graphviz accepted broken input"

  assert "syntax error in line 17 near '\\'" in output, \
    'error message did not identify correct location'

def test_1436():
  """
  test a segfault from https://gitlab.com/graphviz/graphviz/-/issues/1436 has
  not reappeared
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1436.dot"
  assert input.exists(), "unexpectedly missing test case"

  # ask Graphviz to process it, which should generate a segfault if this bug
  # has been reintroduced
  subprocess.check_call(["dot", "-Tsvg", "-o", os.devnull, input])

def test_1444():
  """
  specifying 'headport' as an edge attribute should work regardless of what
  order attributes appear in
  https://gitlab.com/graphviz/graphviz/-/issues/1444
  """

  # locate the first of our associated tests
  input1 = Path(__file__).parent / "1444.dot"
  assert input1.exists(), "unexpectedly missing test case"

  # ask Graphviz to process it
  p = subprocess.Popen(["dot", "-Tsvg", input1], stdout=subprocess.PIPE,
    stderr=subprocess.PIPE, universal_newlines=True)
  stdout1, stderr = p.communicate()

  assert p.returncode == 0, "failed to process a headport edge"

  assert stderr.strip() == "", "emitted an error for a legal graph"

  # now locate our second variant, that simply has the attributes swapped
  input2 = Path(__file__).parent / "1444-2.dot"
  assert input2.exists(), "unexpectedly missing test case"

  # process it identically
  p = subprocess.Popen(["dot", "-Tsvg", input2], stdout=subprocess.PIPE,
    stderr=subprocess.PIPE, universal_newlines=True)
  stdout2, stderr = p.communicate()

  assert p.returncode == 0, "failed to process a headport edge"

  assert stderr.strip() == "", "emitted an error for a legal graph"

  assert stdout1 == stdout2, \
    "swapping edge attributes altered the output graph"

def test_1449():
  """
  using the SVG color scheme should not cause warnings
  https://gitlab.com/graphviz/graphviz/-/issues/1449
  """

  # start Graphviz
  p = subprocess.Popen(["dot", "-Tsvg", "-o", os.devnull],
    stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
    universal_newlines=True)

  # pass it some input that uses the SVG color scheme
  _, stderr = p.communicate('graph g { colorscheme="svg"; }')

  assert p.returncode == 0, "Graphviz exited with non-zero status"

  assert stderr.strip() == "", "SVG color scheme use caused warnings"

@pytest.mark.skipif(shutil.which("gvpr") is None, reason="GVPR not available")
def test_1594():
  """
  GVPR should give accurate line numbers in error messages
  https://gitlab.com/graphviz/graphviz/-/issues/1594
  """

  # locate our associated test case in this directory
# FIXME: remove cwd workaround when
# https://gitlab.com/graphviz/graphviz/-/issues/1780 is fixed
#    input = Path(__file__).parent / "1594.gvpr"
  input = "1594.gvpr"

  # run GVPR with our (malformed) input program
  p = subprocess.Popen(["gvpr", "-f", input], stdin=subprocess.PIPE,
    cwd=os.path.join(os.path.dirname(__file__)),
    stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
  _, stderr = p.communicate()

  assert p.returncode != 0, "GVPR did not reject malformed program"

  assert "line 3:" in stderr, \
    "GVPR did not identify correct line of syntax error"

@pytest.mark.xfail() # FIXME
def test_1658():
  """
  the graph associated with this test case should not crash Graphviz
  https://gitlab.com/graphviz/graphviz/-/issues/1658
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1658.dot"
  assert input.exists(), "unexpectedly missing test case"

  # process it with Graphviz
  subprocess.check_call(["dot", "-Tpng", "-o", os.devnull, input])

def test_1676():
  """
  https://gitlab.com/graphviz/graphviz/-/issues/1676
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1676.dot"
  assert input.exists(), "unexpectedly missing test case"

  # run Graphviz with this input
  ret = subprocess.call(["dot", "-Tsvg", "-o", os.devnull, input])

  # this malformed input should not have caused Graphviz to crash
  assert ret != -signal.SIGSEGV, "Graphviz segfaulted"

def test_1724():
  """
  passing malformed node and newrank should not cause segfaults
  https://gitlab.com/graphviz/graphviz/-/issues/1724
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1724.dot"
  assert input.exists(), "unexpectedly missing test case"

  # run Graphviz with this input
  ret = subprocess.call(["dot", "-Tsvg", "-o", os.devnull, input])

  assert ret != -signal.SIGSEGV, "Graphviz segfaulted"

def test_1767():
  """
  using the Pango plugin multiple times should produce consistent results
  https://gitlab.com/graphviz/graphviz/-/issues/1767
  """

  # FIXME: Remove skip when
  # https://gitlab.com/graphviz/graphviz/-/issues/1777 is fixed
  if os.getenv("build_system") == "msbuild":
    pytest.skip("Windows MSBuild release does not contain any header files (#1777)")

  # find co-located test source
  c_src = (Path(__file__).parent / "1767.c").resolve()
  assert c_src.exists(), "missing test case"

  # find our co-located dot input
  dot = (Path(__file__).parent / "1767.dot").resolve()
  assert dot.exists(), "missing test case"

  ret, _, _ = run_c(c_src, [dot], link=["cgraph", "gvc"])
  assert ret == 0

  # FIXME: uncomment this when #1767 is fixed
  # assert stdout == "Loaded graph:clusters\n" \
  #                  "cluster_0 contains 5 nodes\n" \
  #                  "cluster_1 contains 1 nodes\n" \
  #                  "cluster_2 contains 3 nodes\n" \
  #                  "cluster_3 contains 3 nodes\n" \
  #                  "Loaded graph:clusters\n" \
  #                  "cluster_0 contains 5 nodes\n" \
  #                  "cluster_1 contains 1 nodes\n" \
  #                  "cluster_2 contains 3 nodes\n" \
  #                  "cluster_3 contains 3 nodes\n"

@pytest.mark.skipif(shutil.which("gvpr") is None, reason="GVPR not available")
@pytest.mark.skipif(platform.system() != "Windows",
  reason="only relevant on Windows")
def test_1780():
  """
  GVPR should accept programs at absolute paths
  https://gitlab.com/graphviz/graphviz/-/issues/1780
  """

  # get absolute path to an arbitrary GVPR program
  clustg = Path(__file__).resolve().parent.parent / "cmd/gvpr/lib/clustg"

  # GVPR should not fail when given this path
  subprocess.check_call(["gvpr", "-f", clustg], stdin=subprocess.DEVNULL)

def test_1783():
  """
  Graphviz should not segfault when passed large edge weights
  https://gitlab.com/graphviz/graphviz/-/issues/1783
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1783.dot"
  assert input.exists(), "unexpectedly missing test case"

  # run Graphviz with this input
  ret = subprocess.call(["dot", "-Tsvg", "-o", os.devnull, input])

  assert ret != 0, "Graphviz accepted illegal edge weight"

  assert ret != -signal.SIGSEGV, "Graphviz segfaulted"

@pytest.mark.skipif(shutil.which("gvedit") is None, reason="Gvedit not available")
def test_1813():
  """
  gvedit -? should show usage
  https://gitlab.com/graphviz/graphviz/-/issues/1813
  """

  environ_copy = os.environ.copy()
  environ_copy.pop("DISPLAY", None)
  output = subprocess.check_output(["gvedit", "-?"],
    env=environ_copy,
    universal_newlines=True)

  assert "Usage" in output, "gvedit -? did not show usage"

@pytest.mark.skipif(shutil.which("lefty") is None, reason="Lefty not available")
def test_1818():
  """
  lefty -? should show usage
  https://gitlab.com/graphviz/graphviz/-/issues/1813
  """

  output = subprocess.check_output(["lefty", "-?"],
    stderr=subprocess.STDOUT,
    universal_newlines=True)

  assert "Usage" in output, "lefty -? did not show usage"

def test_1845():
  """
  rendering sequential graphs to PS should not segfault
  https://gitlab.com/graphviz/graphviz/-/issues/1845
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1845.dot"
  assert input.exists(), "unexpectedly missing test case"

  # generate a multipage PS file from this input
  subprocess.check_call(["dot", "-Tps", "-o", os.devnull, input])

@pytest.mark.xfail(strict=True) # FIXME
def test_1856():
  """
  headports and tailports should be respected
  https://gitlab.com/graphviz/graphviz/-/issues/1856
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1856.dot"
  assert input.exists(), "unexpectedly missing test case"

  # process it into JSON
  out = subprocess.check_output(["dot", "-Tjson", input],
    universal_newlines=True)
  data = json.loads(out)

  # find the two nodes, “3” and “5”
  three = [x for x in data["objects"] if x["name"] == "3"][0]
  five  = [x for x in data["objects"] if x["name"] == "5"][0]

  # find the edge from “3” to “5”
  edge = [x for x in data["edges"]
          if x["tail"] == three["_gvid"] and x["head"] == five["_gvid"]][0]

  # The edge should look something like:
  #
  #        ┌─┐
  #        │3│
  #        └┬┘
  #    ┌────┘
  #   ┌┴┐
  #   │5│
  #   └─┘
  #
  # but a bug causes port constraints to not be respected and the edge comes out
  # more like:
  #
  #        ┌─┐
  #        │3│
  #        └┬┘
  #         │
  #   ┌─┐   │
  #   ├5̶┼───┘
  #   └─┘
  #
  # So validate that the edge’s path does not dip below the top of the “5” node.

  top_of_five = max(y for _, y in five["_draw_"][1]["points"])

  waypoints_y = [y for _, y in edge["_draw_"][1]["points"]]

  assert all(y >= top_of_five for y in waypoints_y), "edge dips below 5"

@pytest.mark.skipif(shutil.which("fdp") is None, reason="fdp not available")
def test_1865():
  """
  fdp should not read out of bounds when processing node names
  https://gitlab.com/graphviz/graphviz/-/issues/1865
  Note, the crash this test tries to provoke may only occur when run under
  Address Sanitizer or Valgrind
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1865.dot"
  assert input.exists(), "unexpectedly missing test case"

  # fdp should not crash when processing this file
  subprocess.check_call(["fdp", "-o", os.devnull, input])

@pytest.mark.skipif(shutil.which("fdp") is None, reason="fdp not available")
def test_1876():
  """
  fdp should not rename nodes with internal names
  https://gitlab.com/graphviz/graphviz/-/issues/1876
  """

  # a trivial graph to provoke this issue
  input = "graph { a }"

  # process this with fdp
  p = subprocess.Popen(["fdp"], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
    universal_newlines=True)
  output, _ = p.communicate(input)

  assert p.returncode == 0, "fdp failed to process trivial graph"

  # we should not see any internal names like "%3"
  assert "%" not in output, "internal name in fdp output"

@pytest.mark.skipif(shutil.which("fdp") is None, reason="fdp not available")
def test_1877():
  """
  fdp should not fail an assertion when processing cluster edges
  https://gitlab.com/graphviz/graphviz/-/issues/1877
  """

  # simple input with a cluster edge
  input = "graph {subgraph cluster_a {}; cluster_a -- b}"

  # fdp should be able to process this
  p = subprocess.Popen(["fdp", "-o", os.devnull], stdin=subprocess.PIPE,
    universal_newlines=True)
  p.communicate(input)
  assert p.returncode == 0

def test_1880():
  """
  parsing a particular graph should not cause a Trapezoid-table overflow
  assertion failure
  https://gitlab.com/graphviz/graphviz/-/issues/1880
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1880.dot"
  assert input.exists(), "unexpectedly missing test case"

  # FIXME: remove this block when this #1880 is fixed
  if not is_ndebug_defined() and platform.system() != "Windows":
    with pytest.raises(subprocess.CalledProcessError):
      subprocess.check_call(["dot", "-Tpng", "-o", os.devnull, input])
    return

  # process it with Graphviz
  subprocess.check_call(["dot", "-Tpng", "-o", os.devnull, input])

def test_1898():
  """
  test a segfault from https://gitlab.com/graphviz/graphviz/-/issues/1898 has
  not reappeared
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1898.dot"
  assert input.exists(), "unexpectedly missing test case"

  # ask Graphviz to process it, which should generate a segfault if this bug
  # has been reintroduced
  subprocess.check_call(["dot", "-Tsvg", "-o", os.devnull, input])

# root directory of this checkout
ROOT = Path(__file__).parent.parent.resolve()

# find all HTML files
html = set()
for (prefix, _, files) in os.walk(ROOT):
  for name in files:
    if Path(name).suffix.lower() in (".htm", ".html"):
      html.add(Path(prefix) / name)

@pytest.mark.parametrize("src", html)
@pytest.mark.skipif(shutil.which("xmllint") is None, reason="xmllint not found")
def test_html(src: Path):
  """
  check that HTML files are valid and conforming
  """

  # validate the file
  p = subprocess.Popen(["xmllint", "--nonet", "--noout", "--html", "--valid",
    src], stderr=subprocess.PIPE, universal_newlines=True)
  _, stderr = p.communicate()

  # expect it to succeed
  assert p.returncode == 0
  assert stderr == ""

@pytest.mark.parametrize("variant", [1, 2])
@pytest.mark.skipif(shutil.which("gml2gv") is None, reason="gml2gv not available")
def test_1869(variant: int):
  """
  gml2gv should be able to parse the style, outlineStyle, width and
  outlineWidth GML attributes and map them to the DOT attributes
  style and penwidth respectively
  https://gitlab.com/graphviz/graphviz/-/issues/1869
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / f"1869-{variant}.gml"
  assert input.exists(), "unexpectedly missing test case"

  # ask gml2gv to translate it to DOT
  output = subprocess.check_output(["gml2gv", input],
    universal_newlines=True)

  assert "style=dashed" in output, "style=dashed not found in DOT output"
  assert "penwidth=2" in output, "penwidth=2 not found in DOT output"

def test_1893():
  """
  an HTML label containing just a ] should work
  https://gitlab.com/graphviz/graphviz/-/issues/1893
  """

  # a graph containing a node with an HTML label with a ] in a table cell
  input = "digraph { 0 [label=<<TABLE><TR><TD>]</TD></TR></TABLE>>] }"

  # ask Graphviz to process this
  p = subprocess.Popen(["dot", "-Tsvg", "-o", os.devnull],
                       stdin=subprocess.PIPE, universal_newlines=True)
  p.communicate(input)
  assert p.returncode == 0

  # we should be able to do the same with an escaped ]
  input = "digraph { 0 [label=<<TABLE><TR><TD>&#93;</TD></TR></TABLE>>] }"

  p = subprocess.Popen(["dot", "-Tsvg", "-o", os.devnull],
                       stdin=subprocess.PIPE, universal_newlines=True)
  p.communicate(input)
  assert p.returncode == 0

def test_1906():
  """
  graphs that cause an overflow during rectangle calculation should result in
  a layout error
  https://gitlab.com/graphviz/graphviz/-/issues/1906
  """

  # one of the rtest graphs is sufficient to provoke this
  input = Path(__file__).parent / "graphs/root.gv"
  assert input.exists(), "unexpectedly missing test case"

  # use Circo to translate it to DOT
  p = subprocess.Popen(["dot", "-Kcirco", "-Tgv", "-o", os.devnull, input],
    stderr=subprocess.PIPE, universal_newlines=True)
  _, stderr = p.communicate()

  assert p.returncode != 0, "graph that generates overflow was accepted"

  assert "area too large" in stderr, "missing/incorrect error message"

@pytest.mark.skipif(shutil.which("twopi") is None, reason="twopi not available")
def test_1907():
  """
  SVG edges should have title elements that match their names
  https://gitlab.com/graphviz/graphviz/-/issues/1907
  """

  # a trivial graph to provoke this issue
  input = "digraph { A -> B -> C }"

  # generate an SVG from this input with twopi
  p = subprocess.Popen(["twopi", "-Tsvg"], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
    universal_newlines=True)
  output, _ = p.communicate(input)

  assert "<title>A&#45;&gt;B</title>" in output, \
    "element title not found in SVG"

@pytest.mark.skipif(shutil.which("gvpr") is None, reason="gvpr not available")
def test_1909():
  """
  GVPR should not output internal names
  https://gitlab.com/graphviz/graphviz/-/issues/1909
  """

  # locate our associated test case in this directory
  prog = Path(__file__).parent / "1909.gvpr"
  graph = Path(__file__).parent / "1909.dot"

  # run GVPR with the given input
  p = subprocess.Popen(["gvpr", "-c", "-f", prog, graph],
    stdout=subprocess.PIPE, universal_newlines=True)
  output, _ = p.communicate()

  assert p.returncode == 0, "gvpr failed to process graph"

  # we should have produced this graph without names like "%2" in it
  assert output == "// begin\n" \
                   "digraph bug {\n" \
                   "	a -> b;\n" \
                   "	b -> c;\n" \
                   "}\n"

def test_1910():
  """
  Repeatedly using agmemread() should have consistent results
  https://gitlab.com/graphviz/graphviz/-/issues/1910
  """

  # FIXME: Remove skip when
  # https://gitlab.com/graphviz/graphviz/-/issues/1777 is fixed
  if os.getenv("build_system") == "msbuild":
    pytest.skip("Windows MSBuild release does not contain any header files (#1777)")

  # find co-located test source
  c_src = (Path(__file__).parent / "1910.c").resolve()
  assert c_src.exists(), "missing test case"

  # run the test
  ret, _, _ = run_c(c_src, link=["cgraph", "gvc"])
  assert ret == 0

def test_1913():
  """
  ALIGN attributes in <BR> tags should be parsed correctly
  https://gitlab.com/graphviz/graphviz/-/issues/1913
  """

  # a template of a trivial graph using an ALIGN attribute
  graph = 'digraph {{\n' \
          '  table1[label=<<table><tr><td align="text">hello world' \
          '<br align="{}"/></td></tr></table>>];\n' \
          '}}'

  def run(input):
    """
    run Dot with the given input and return its exit status and stderr
    """
    p = subprocess.Popen(["dot", "-Tsvg", "-o", os.devnull],
      stdin=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
    _, stderr = p.communicate(input)
    return p.returncode, stderr

  # Graphviz should accept all legal values for this attribute
  for align in ("left", "right", "center"):

    input = align
    ret, stderr = run(graph.format(input))
    assert ret == 0
    assert stderr.strip() == ""

    # these attributes should also be valid when title cased
    input = f"{align[0].upper()}{align[1:]}"
    ret, stderr = run(graph.format(input))
    assert ret == 0
    assert stderr.strip() == ""

    # similarly, they should be valid when upper cased
    input = align.upper()
    ret, stderr = run(graph.format(input))
    assert ret == 0
    assert stderr.strip() == ""

  # various invalid things that have the same prefix or suffix as a valid
  # alignment should be rejected
  for align in ("lamp", "deft", "round", "might", "circle", "venter"):

    input = align
    _, stderr = run(graph.format(input))
    assert f"Warning: Illegal value {input} for ALIGN - ignored" in stderr

    # these attributes should also fail when title cased
    input = f"{align[0].upper()}{align[1:]}"
    _, stderr = run(graph.format(input))
    assert f"Warning: Illegal value {input} for ALIGN - ignored" in stderr

    # similarly, they should fail when upper cased
    input = align.upper()
    _, stderr = run(graph.format(input))
    assert f"Warning: Illegal value {input} for ALIGN - ignored" in stderr

def test_1931():
  """
  New lines within strings should not be discarded during parsing

  """

  # a graph with \n inside of strings
  graph = 'graph {\n'                 \
          '  node1 [label="line 1\n'  \
          'line 2\n'                  \
          '"];\n'                     \
          '  node2 [label="line 3\n'  \
          'line 4"];\n'                \
          '  node1 -- node2\n'        \
          '  node2 -- "line 5\n'      \
          'line 6"\n'                 \
          '}'

  # ask Graphviz to process this to dot output
  p = subprocess.Popen(["dot", "-Txdot"], stdin=subprocess.PIPE,
    stdout=subprocess.PIPE, universal_newlines=True)
  xdot, _ = p.communicate(graph)
  assert p.returncode == 0

  # all new lines in strings should have been preserved
  assert "line 1\nline 2\n" in xdot
  assert "line 3\nline 4" in xdot
  assert "line 5\nline 6" in xdot

@pytest.mark.skipif(shutil.which("edgepaint") is None,
                    reason="edgepaint not available")
def test_1971():
  """
  edgepaint should reject invalid command line options
  https://gitlab.com/graphviz/graphviz/-/issues/1971
  """

  # a basic graph that edgepaint can process
  input =                                                                      \
    'digraph {\n'                                                              \
    '  graph [bb="0,0,54,108"];\n'                                             \
    '  node [label="\\N"];\n'                                                  \
    '  a       [height=0.5,\n'                                                 \
    '           pos="27,90",\n'                                                \
    '           width=0.75];\n'                                                \
    '  b       [height=0.5,\n'                                                 \
    '           pos="27,18",\n'                                                \
    '           width=0.75];\n'                                                \
    '  a -> b  [pos="e,27,36.104 27,71.697 27,63.983 27,54.712 27,46.112"];\n' \
    '}'

  # run edgepaint with an invalid option, `-rabbit`, that happens to have the
  # same first character as valid options
  args = ["edgepaint", "-rabbit"]
  p = subprocess.Popen(args, stdin=subprocess.PIPE, universal_newlines=True)
  p.communicate(input)

  assert p.returncode != 0, "edgepaint incorrectly accepted '-rabbit'"

@pytest.mark.xfail(strict=not is_ndebug_defined()) # FIXME
def test_1990():
  """
  using ortho and circo in combination should not cause an assertion failure
  https://gitlab.com/graphviz/graphviz/-/issues/14
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "1990.dot"
  assert input.exists(), "unexpectedly missing test case"

  # process it with Graphviz
  subprocess.check_call(["circo", "-Tsvg", "-o", os.devnull, input])

def test_2057():
  """
  gvToolTred should be usable by user code
  https://gitlab.com/graphviz/graphviz/-/issues/2057
  """

  # FIXME: Remove skip when
  # https://gitlab.com/graphviz/graphviz/-/issues/1777 is fixed
  if os.getenv("build_system") == "msbuild":
    pytest.skip("Windows MSBuild release does not contain any header files (#1777)")

  # find co-located test source
  c_src = (Path(__file__).parent / "2057.c").resolve()
  assert c_src.exists(), "missing test case"

  # run the test
  ret, _, _ = run_c(c_src, link=["gvc"])
  assert ret == 0

def test_2078():
  """
  Incorrectly using the "layout" attribute on a subgraph should result in a
  sensible error.
  https://gitlab.com/graphviz/graphviz/-/issues/2078
  """

  # our sample graph that incorrectly uses layout
  input = "graph {\n"          \
          "  subgraph {\n"     \
          "    layout=osage\n" \
          "  }\n"              \
          "}"

  # run it through Graphviz
  p = subprocess.Popen(["dot", "-Tcanon", "-o", os.devnull],
    stdin=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
  _, stderr = p.communicate(input)

  assert p.returncode != 0, "layout on subgraph was incorrectly accepted"

  assert "layout attribute is invalid except on the root graph" in stderr, \
    "expected warning not found"

  # a graph that correctly uses layout
  input = "graph {\n"          \
          "  layout=osage\n" \
          "  subgraph {\n"     \
          "  }\n"              \
          "}"

  # ensure this one does not trigger warnings
  p = subprocess.Popen(["dot", "-Tcanon", "-o", os.devnull],
    stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
    universal_newlines=True)
  stdout, stderr = p.communicate(input)

  assert p.returncode == 0, "correct layout use was rejected"

  assert stdout.strip() == "", "unexpected output"
  assert "layout attribute is invalid except on the root graph" not in stderr, \
    "incorrect warning output"

def test_2082():
  """
  Check a bug in inside_polygon has not been reintroduced.
  https://gitlab.com/graphviz/graphviz/-/issues/2082
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "2082.dot"
  assert input.exists(), "unexpectedly missing test case"

  # ask Graphviz to process it, which should generate an assertion failure if
  # this bug has been reintroduced
  subprocess.check_call(["dot", "-Tpng", "-o", os.devnull, input])

@pytest.mark.xfail(strict=True)
@pytest.mark.parametrize("html_like_first", (False, True))
def test_2089(html_like_first: bool): # FIXME
  """
  HTML-like and non-HTML-like strings should peacefully coexist
  https://gitlab.com/graphviz/graphviz/-/issues/2089
  """

  # a graph using an HTML-like string and a non-HTML-like string
  if html_like_first:
    graph = 'graph {\n'           \
            '  a[label=<foo>];\n' \
            '  b[label="foo"];\n' \
            '}'
  else:
    graph = 'graph {\n'           \
            '  a[label="foo"];\n' \
            '  b[label=<foo>];\n' \
            '}'

  # normalize the graph
  p = subprocess.Popen(["dot", "-Tdot"], stdin=subprocess.PIPE,
                       stdout=subprocess.PIPE, universal_newlines=True)
  canonical, _ = p.communicate(graph)

  assert p.returncode == 0

  assert 'label=foo' in canonical, "non-HTML-like label not found"
  assert "label=<foo>" in canonical, "HTML-like label not found"

@pytest.mark.xfail(strict=True) # FIXME
def test_2089_2():
  """
  HTML-like and non-HTML-like strings should peacefully coexist
  https://gitlab.com/graphviz/graphviz/-/issues/2089
  """

  # find co-located test source
  c_src = (Path(__file__).parent / "2089.c").resolve()
  assert c_src.exists(), "missing test case"

  # run it
  ret, stdout, stderr = run_c(c_src, link=["cgraph"])
  sys.stdout.write(stdout)
  sys.stderr.write(stderr)
  assert ret == 0

@pytest.mark.skipif(os.environ.get("build_system") == "msbuild" and
                    os.environ.get("configuration") == "Debug",
                    reason="Graphviz built with MSBuild in Debug mode has an "
                           "insufficient stack size for this test")
def test_2095():
  """
  Exceeding 1000 boxes during computation should not cause a crash
  https://gitlab.com/graphviz/graphviz/-/issues/2095
  """

  # locate our associated test case in this directory
  input = Path(__file__).parent / "2095.dot"
  assert input.exists(), "unexpectedly missing test case"

  # ask Graphviz to process it
  subprocess.check_call(["dot", "-Tpdf", "-o", os.devnull, input])

@pytest.mark.skipif(shutil.which("gv2gml") is None,
                    reason="gv2gml not available")
def test_2131():
  """
  gv2gml should be able to process basic Graphviz input
  https://gitlab.com/graphviz/graphviz/-/issues/2131
  """

  # a trivial graph
  input = "digraph { a -> b; }"

  # ask gv2gml what it thinks of this
  p = subprocess.Popen(["gv2gml"], stdin=subprocess.PIPE,
                       universal_newlines=True)
  p.communicate(input)

  assert p.returncode == 0, "gv2gml rejected a basic graph"

@pytest.mark.skipif(shutil.which("gvpr") is None,
                    reason="gvpr not available")
@pytest.mark.parametrize("examine", ("indices", "tokens"))
def test_2138(examine: str):
  """
  gvpr splitting and tokenizing should not result in trailing garbage
  https://gitlab.com/graphviz/graphviz/-/issues/2138
  """

  # find our co-located GVPR program
  script = (Path(__file__).parent / "2138.gvpr").resolve()
  assert script.exists(), "missing test case"

  # run it with NUL input
  out = subprocess.check_output(["gvpr", "-f", script],
                                stdin=subprocess.DEVNULL)

  # Decode into text. We do this instead of `universal_newlines=True` above
  # because the trailing garbage can contain invalid UTF-8 data causing cryptic
  # failures. We want to correctly surface this as trailing garbage, not an
  # obscure UTF-8 decoding error.
  result = out.decode("utf-8", "replace")

  if examine == "indices":
    # check no indices are miscalculated
    index_re = r"^// index of space \(st\) :\s*(?P<index>-?\d+)\s*<< must " \
               r"NOT be less than -1$"
    for m in re.finditer(index_re, result, flags=re.MULTILINE):
      index = int(m.group("index"))
      assert index >= -1, "illegal index computed"

  if examine == "tokens":
    # check for text the author of 2138.gvpr expected to find
    assert "// tok[3]    >>3456789<<   should NOT include trailing spaces or " \
      "junk chars" in result, "token 3456789 not found or has trailing garbage"
    assert "// tok[7]    >>012<<   should NOT include trailing spaces or "     \
      "junk chars" in result, "token 012 not found or has trailing garbage"

def test_package_version():
  """
  The graphviz_version.h header should define a non-empty PACKAGE_VERSION
  """

  # FIXME: Remove skip when
  # https://gitlab.com/graphviz/graphviz/-/issues/1777 is fixed
  if os.getenv("build_system") == "msbuild":
    pytest.skip("Windows MSBuild release does not contain any header files (#1777)")

  # find co-located test source
  c_src = (Path(__file__).parent / "check-package-version.c").resolve()
  assert c_src.exists(), "missing test case"

  # run the test
  ret, _, _ = run_c(c_src)
  assert ret == 0

def test_user_shapes():
  """
  Graphviz should understand how to embed a custom SVG image as a node’s shape
  """

  # find our collocated test case
  input = Path(__file__).parent / "usershape.dot"
  assert input.exists(), "unexpectedly missing test case"

  # ask Graphviz to translate this to SVG
  output = subprocess.check_output(["dot", "-Tsvg", input],
    cwd=os.path.dirname(__file__), universal_newlines=True)

  # the external SVG should have been parsed and is now referenced
  assert '<image xlink:href="usershape.svg" width="62px" height="44px" ' in \
    output

def test_xdot_json():
  """
  check the output of xdot’s JSON API
  """

  # find our collocated C helper
  c_src = Path(__file__).parent / "xdot2json.c"

  # some valid xdot commands to process
  input = "c 9 -#fffffe00 C 7 -#ffffff P 4 0 0 0 36 54 36 54 0"

  # ask our C helper to process this
  try:
    ret, output, err = run_c(c_src, input=input, link=["xdot"])
  except subprocess.CalledProcessError:
    # FIXME: Remove this try-catch when
    # https://gitlab.com/graphviz/graphviz/-/issues/1777 is fixed
    if os.getenv("build_system") == "msbuild":
      pytest.skip("Windows MSBuild release does not contain any header "
                  "files (#1777)")
    raise
  assert ret == 0
  assert err == ""

  if os.getenv("build_system") == "msbuild":
    pytest.fail("Windows MSBuild unexpectedly passed compilation of a "
                  "Graphviz header. Remove the above try-catch? (#1777)")

  # confirm the output was what we expected
  assert output == '[\n'                                                   \
                   '{c : "#fffffe00"},\n'                                  \
                   '{C : "#ffffff"},\n'                                    \
                   '{P : [0.000000,0.000000,0.000000,36.000000,54.000000,' \
                     '36.000000,54.000000,0.000000]}\n'                    \
                   ']\n'

# find all .vcxproj files
VCXPROJS: List[Path] = []
for repo_root, _, files in os.walk(ROOT):
  for stem in files:
    # skip files generated by MSBuild itself
    if stem in ("VCTargetsPath.vcxproj", "CompilerIdC.vcxproj",
                "CompilerIdCXX.vcxproj"):
      continue
    full_path = Path(repo_root) / stem
    if full_path.suffix != ".vcxproj":
      continue
    VCXPROJS.append(full_path)

@pytest.mark.parametrize("vcxproj", VCXPROJS)
def test_vcxproj_inclusive(vcxproj: Path):
  """check .vcxproj files correspond to .vcxproj.filters files"""

  def fix_sep(path: str) -> str:
    """translate Windows path separators to ease running this on non-Windows"""
    return path.replace("\\", os.sep)

  # FIXME: files missing a filters file
  FILTERS_WAIVERS = (Path("lib/version/version.vcxproj"),)

  filters = Path(f"{str(vcxproj)}.filters")
  if vcxproj.relative_to(ROOT) not in FILTERS_WAIVERS:
    assert filters.exists(), \
      f"no {str(filters)} corresponding to {str(vcxproj)}"

  # namespace the MSBuild elements live in
  ns = "http://schemas.microsoft.com/developer/msbuild/2003"

  # parse XML out of the .vcxproj file
  srcs1 = set()
  tree = ET.parse(vcxproj)
  root = tree.getroot()
  for elem in root:
    if elem.tag == f"{{{ns}}}ItemGroup":
      for child in elem:
        if child.tag in (f"{{{ns}}}ClInclude", f"{{{ns}}}ClCompile"):
          filename = fix_sep(child.attrib["Include"])
          assert filename not in srcs1, \
            f"duplicate source {filename} in {str(vcxproj)}"
          srcs1.add(filename)

  # parse XML out of the .vcxproj.filters file
  if filters.exists():
    srcs2 = set()
    tree = ET.parse(filters)
    root = tree.getroot()
    for elem in root:
      if elem.tag == f"{{{ns}}}ItemGroup":
        for child in elem:
          if child.tag in (f"{{{ns}}}ClInclude", f"{{{ns}}}ClCompile"):
            filename = fix_sep(child.attrib["Include"])
            assert filename not in srcs2, \
              f"duplicate source {filename} in {str(filters)}"
            srcs2.add(filename)

    assert srcs1 == srcs2, \
      "mismatch between sources in {str(vcxproj)} and {str(filters)}"

@pytest.mark.xfail() # FIXME: fails on CentOS 7/8, macOS Autotools, MSBuild
@pytest.mark.skipif(shutil.which("gvmap") is None, reason="gvmap not available")
def test_gvmap_fclose():
  """
  gvmap should not attempt to fclose(NULL). This example will trigger a crash if
  this bug has been reintroduced and Graphviz is built with ASan support.
  """

  # a reasonable input graph
  input = 'graph "Alík: Na vlastní oči" {\n'                                  \
          '	graph [bb="0,0,128.9,36",\n'                                      \
          '		concentrate=true,\n'                                            \
          '		overlap=prism,\n'                                               \
          '		start=3\n'                                                      \
          '	];\n'                                                             \
          '	node [label="\\N"];\n'                                            \
          '	{\n'                                                              \
          '		bob	[height=0.5,\n'                                             \
          '			pos="100.95,18",\n'                                           \
          '			width=0.77632];\n'                                            \
          '	}\n'                                                              \
          '	{\n'                                                              \
          '		alice	[height=0.5,\n'                                           \
          '			pos="32.497,18",\n'                                           \
          '			width=0.9027];\n'                                             \
          '	}\n'                                                              \
          '	alice -- bob	[pos="65.119,18 67.736,18 70.366,18 72.946,18"];\n' \
          '	bob -- alice;\n'                                                  \
          '}'

  # pass this through gvmap
  p = subprocess.Popen(["gvmap"], stdin=subprocess.PIPE)
  p.communicate(input.encode("utf-8"))

  assert p.returncode == 0
