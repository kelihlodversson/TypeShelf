#!/usr/bin/env python3
"""Exercise TypeShelf on the native pTOS libaes desktop.

Reuses the pinned pTOS desktop test's PS/2 driver and private FAT disk fixture.
Requires its desktop-tests configuration with embedded console and libaes.
"""
import argparse
import importlib.util
import os
from pathlib import Path
import re
import shutil
import subprocess
import time
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[2]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--ptos", type=Path, required=True)
    parser.add_argument("--qemu", default="qemu-system-x86_64")
    parser.add_argument("--ovmf-code", required=True)
    parser.add_argument("--ovmf-vars", required=True)
    parser.add_argument("--output", type=Path, default=ROOT / "build/emulator/x32")
    parser.add_argument("--timeout", type=int, default=180)
    args = parser.parse_args()
    args.ptos = args.ptos.resolve()
    args.output = args.output.resolve()
    args.embedded_console = True
    spec = importlib.util.spec_from_file_location(
        "ptos_desktop", args.ptos / "tests/x32_desktop/run.py")
    desktop = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(desktop)
    os.chdir(args.ptos)
    original_run = subprocess.run

    def stage(command, *positional, **keywords):
        # APPTEST retains the fixture's public desktop launch coordinates.
        if command[0] == "tools/mkhdisk.sh" and Path(command[1]).name == "system.img":
            folder = Path(command[3]).parent
            shutil.copy(ROOT / "build/x32/TYPESHLF.PRG", folder / "APPTEST.PRG")
            command = command + [str(ROOT / "build/x32/TYPESHLF.RSC")]
        return original_run(command, *positional, **keywords)

    with patch.object(desktop.subprocess, "run", side_effect=stage):
        guest = desktop.Guest(args.output, args)
    try:
        guest.ready(1)
        guest.move(34, 45)
        guest.double_click()
        guest.wait("x32 desktop window OPEN")
        guest.move(135, 8)
        time.sleep(0.25)
        guest.move(135, 44)
        guest.click()
        text = guest.wait("x32 desktop program TARGET")
        point = re.findall(r"x32 desktop program TARGET (\d+) (\d+)", text)[-1]
        guest.move(*map(int, point))
        guest.click()
        guest.key("ctrl-o")
        text = guest.wait(r"x32 desktop LAUNCH (\d+) (?:C:\\)?APPTEST.PRG",
                          regex=True)
        pid = re.findall(r"x32 desktop LAUNCH (\d+) (?:C:\\)?APPTEST.PRG", text)[-1]
        exited = "x32 desktop CH_EXIT " + pid + " "
        time.sleep(3)
        assert exited not in guest.text(), "TypeShelf exited during startup"
        guest.screenshot("startup")

        guest.key("ctrl-2")
        time.sleep(0.5)
        guest.screenshot("samples")
        guest.key("ctrl-3")
        time.sleep(0.5)
        guest.screenshot("list")
        guest.key("ctrl-n")
        time.sleep(0.5)
        guest.screenshot("two-windows")
        guest.key("ctrl-w")
        time.sleep(0.5)
        assert exited not in guest.text(), "TypeShelf exited before Quit"
        guest.key("ctrl-q")
        text = guest.wait(exited + "0000")
        assert "process fault:" not in text, text[-3000:]
        assert text.count(exited) == 1, text[-3000:]
        guest.screenshot("exited")
        print("TypeShelf x32: startup, view/window shortcuts and clean exit passed")
    finally:
        guest.close()


if __name__ == "__main__":
    main()
