import subprocess
import sys
import tempfile
from pathlib import Path


executable = Path(sys.argv[1]).resolve()
repository = Path(__file__).resolve().parents[1]
checks = 0


with tempfile.TemporaryDirectory(prefix="badd analyzer regression.") as directory:
    scratch = Path(directory)

    def run(source, expected=0, marker=""):
        global checks
        path = scratch / "input file.ba"
        path.write_bytes(source.encode("utf-8"))
        result = subprocess.run([str(executable), str(path)], cwd=scratch,
                                capture_output=True, text=True, input="", timeout=10)
        report = (scratch / "myscript.lx").read_text(encoding="utf-8") if (scratch / "myscript.lx").exists() else ""
        assert result.returncode == expected, (result.returncode, report, result.stderr)
        assert marker in " ".join((report + result.stderr).split()), (marker, report, result.stderr)
        checks += 1
        return report

    report = run("int item1. move 20 to item1. add -5 to item1. sub 2 from item1. out item1.")
    assert "item1 13" in report
    assert "'from' is a keyword" in report
    report = run("int x. move 0 to x. loop 3 times [add 1 to x.]. out x.")
    assert "x 3" in report
    run("", marker="")
    run("int x.\tmove 2 to x.\t\nout x.", marker="x 2")
    run("{first}{" + "comment" * 100 + "}int x.", marker="Comment Line:")
    run("int.", 1, "Identifier is expected")
    run("move 1.", 1, "Keyword 'to' is expected")
    run("int x", 1, "End of line is expected")
    run('out "hello".', 1, "declared identifier")
    run("int x. int x.", 1, "Duplicate identifier")
    run("int x. move - to x.", 1, "not an integer")
    run("int x. move 2147483648 to x.", 1, "not an integer")
    run("int x. move 2147483647 to x. add 1 to x.", 1, "range exceeded")
    run("int x. move -2147483648 to x. sub 1 from x.", 1, "range exceeded")
    run("{unterminated", 1, "not completed")
    run("int x. loop 0 times add 1 to x.", 1, "Positive loop count")
    run("int x. loop 1 times out " + "x " * 60 + ".", 1, "Loop body length")
    run("out 1." * 1000, marker="Integer Value")
    run("x." * 1001, 1, "Statement limit")
    run("x " * 1000 + ".", 1, "Word limit")
    run("x" * 1048577, 1, "1 MiB limit")
    run("int x.\0out x.", 1, "NUL byte")
    for name in ["myscript.ba", "custom-test.ba"]:
        run((repository / "Only Source Codes" / name).read_text(encoding="utf-8"), 1, "myVar is not an identifier")
    run((repository / "Only Source Codes/standart-test.ba").read_text(encoding="utf-8"), 1, "declared identifier")
    result = subprocess.run([str(executable)], cwd=scratch, input="x" * 2000 + "\n",
                            capture_output=True, text=True, timeout=10)
    assert result.returncode == 1 and "too long" in result.stderr
    checks += 1
    for filename, marker in [("x" * 2000, "too long"), ("missing.ba", ""), ("wrong.txt", "extension")]:
        result = subprocess.run([str(executable), filename], cwd=scratch,
                                capture_output=True, text=True, input="", timeout=10)
        assert result.returncode == 1 and marker in result.stderr
        checks += 1

assert (repository / "Only Source Codes/main.c").read_bytes() == (
    repository / "Executable File and Codes/main.c").read_bytes()
print(f"Passed {checks} analyzer regression runs and duplicate-source check.")
