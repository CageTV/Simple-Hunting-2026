"""Compile the two optimised Simple Hunting Overhaul scripts with the official Papyrus compiler.

  python tools/build_scripts.py     src/scripts -> build/scripts

Import order: our scripts first, then the original mod sources (unpacked in H:/Hunting), then SKSE and vanilla.
"""
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
GAME = r"E:\Tabula Rasa\Stock Game"
MODS = r"E:\Tabula Rasa\mods"
ORIGINAL = r"H:\Modlists\Tabula\mods\Simple Hunting Overhaul\source\scripts"
SRC = os.path.join(HERE, "src", "scripts")
OUT = os.path.join(HERE, "build", "scripts")
COMPILER = os.path.join(GAME, "Papyrus Compiler", "PapyrusCompiler.exe")
FLAGS = os.path.join(GAME, "Data", "Source", "Scripts", "TESV_Papyrus_Flags.flg")
IMPORTS = [
    SRC,
    ORIGINAL,
    os.path.join(MODS, "Skyrim Script Extender (SKSE64)", "Scripts", "Source"),
    os.path.join(GAME, "Data", "Source", "Scripts"),
]
for p in [COMPILER, FLAGS, SRC] + IMPORTS:
    if not os.path.exists(p):
        raise SystemExit(f"missing: {p}")
os.makedirs(OUT, exist_ok=True)
log, rc = "", 0
for name in sorted(n for n in os.listdir(SRC) if n.lower().endswith(".psc")):
    r = subprocess.run([COMPILER, name, f"-f={FLAGS}", "-i=" + ";".join(IMPORTS), f"-o={OUT}"], capture_output=True, text=True, cwd=SRC, timeout=180)
    log += (r.stdout or "") + (r.stderr or "")
    rc = rc or r.returncode
open(os.path.join(HERE, "build", "compile.log"), "w", encoding="utf-8").write(log)
errors = [l for l in log.splitlines() if ".psc(" in l]
print(f"compiler exit {rc}: {len([n for n in os.listdir(OUT) if n.endswith('.pex')])} .pex; {len(errors)} error lines")
for l in errors[:20]:
    print("  " + l)
sys.exit(0 if rc == 0 and not errors else 1)
