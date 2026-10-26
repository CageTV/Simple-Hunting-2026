"""Lays out the Simple Hunting Overhaul 2026 packages (staging in package/, nothing outside the workspace is touched).

  package/Simple Hunting Overhaul 2026            the layer for a quick install: new SimpleHunting.dll + README + LICENSE
  package/Simple Hunting Overhaul 2026 - Installer(.zip)   FOMOD: pick the DLL build (new / older)
  package/Simple Hunting Overhaul 2026 - Optimised Scripts(.zip)   the two rebuilt JaySerpa scripts, GitHub ONLY (never uploaded to Nexus)

Needs plugin/build/release/SimpleHunting.dll (older, plugin/build.cmd), E:/WorkSpace/ng-build/build/release/out/SimpleHunting/SimpleHunting.dll
(new, ng-build/build.cmd SimpleHunting) and build/scripts (tools/build_scripts.py).

Usage: python tools/package.py [version]
"""
import os
import shutil
import sys
import xml.dom.minidom
import zipfile

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
VERSION = sys.argv[1] if len(sys.argv) > 1 else "1.0.0"
OLD = os.path.join(HERE, "plugin", "build", "release", "SimpleHunting.dll")
NEW = r"E:\WorkSpace\ng-build\build\release\out\SimpleHunting\SimpleHunting.dll"
SCRIPTS = os.path.join(HERE, "build", "scripts")
for p in (OLD, NEW, SCRIPTS):
    assert os.path.exists(p), f"missing {p}"
PEX = ["SHO_PickUpCarCass.pex", "SHO_PlayerAliasScript.pex"]
PSC = ["SHO_PickUpCarCass.psc", "SHO_PlayerAliasScript.psc"]


def put(src, dst):
    os.makedirs(os.path.dirname(dst), exist_ok=True)
    shutil.copyfile(src, dst)


def scripts_into(root):
    for n in PEX:
        put(os.path.join(SCRIPTS, n), os.path.join(root, "scripts", n))
    for n in PSC:
        put(os.path.join(HERE, "src", "scripts", n), os.path.join(root, "scripts", "source", n))


# ---- the quick layer --------------------------------------------------------------------------------------------------------------
quick = os.path.join(HERE, "package", "Simple Hunting Overhaul 2026")
if os.path.exists(quick):
    shutil.rmtree(quick)
put(NEW, os.path.join(quick, "SKSE", "Plugins", "SimpleHunting.dll"))
put(os.path.join(HERE, "docs", "README.txt"), os.path.join(quick, "Simple Hunting Overhaul 2026 README.txt"))
put(os.path.join(HERE, "LICENSE.txt"), os.path.join(quick, "Simple Hunting Overhaul 2026 LICENSE.txt"))

# ---- the installer ----------------------------------------------------------------------------------------------------------------
dest = os.path.join(HERE, "package", "Simple Hunting Overhaul 2026 - Installer")
if os.path.exists(dest):
    shutil.rmtree(dest)
put(NEW, os.path.join(dest, "new", "SKSE", "Plugins", "SimpleHunting.dll"))
put(OLD, os.path.join(dest, "older", "SKSE", "Plugins", "SimpleHunting.dll"))
put(os.path.join(HERE, "docs", "README.txt"), os.path.join(dest, "README.txt"))
put(os.path.join(HERE, "LICENSE.txt"), os.path.join(dest, "LICENSE.txt"))
NEW_VER = "1.6.1170.0"
config = f"""<config xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="http://qconsulting.ca/fo3/ModConfig5.0.xsd">
  <moduleName>Simple Hunting Overhaul 2026</moduleName>
  <requiredInstallFiles>
    <file source="README.txt" destination="Simple Hunting Overhaul 2026 README.txt"/>
    <file source="LICENSE.txt" destination="Simple Hunting Overhaul 2026 LICENSE.txt"/>
  </requiredInstallFiles>
  <installSteps order="Explicit">
    <installStep name="Game version">
      <optionalFileGroups order="Explicit">
        <group name="SimpleHunting.dll" type="SelectExactlyOne">
          <plugins order="Explicit">
            <plugin name="New (Skyrim 1.6.1170 and newer)">
              <description>The new build. Works on Skyrim SE/AE 1.6.1170 and on every later version, including 1.7.x. Pre-selected when the installer sees 1.6.1170 or newer.</description>
              <files><folder source="new" destination=""/></files>
              <typeDescriptor><dependencyType><defaultType name="Optional"/><patterns>
                <pattern><dependencies><gameDependency version="{NEW_VER}"/></dependencies><type name="Recommended"/></pattern>
              </patterns></dependencyType></typeDescriptor>
            </plugin>
            <plugin name="Older (Skyrim VR and 1.6.1130 and older)">
              <description>The older build, for Skyrim VR and SE/AE 1.6.1130 and older. On 1.6.1170 either build works.</description>
              <files><folder source="older" destination=""/></files>
              <typeDescriptor><dependencyType><defaultType name="Optional"/><patterns>
                <pattern><dependencies><gameDependency version="{NEW_VER}"/></dependencies><type name="Optional"/></pattern>
              </patterns></dependencyType></typeDescriptor>
            </plugin>
          </plugins>
        </group>
      </optionalFileGroups>
    </installStep>
  </installSteps>
</config>
"""
info = (f"<fomod>\n  <Name>Simple Hunting Overhaul 2026</Name>\n  <Author>CageTV (Simple Hunting Overhaul by JaySerpa)</Author>\n  <Version>{VERSION}</Version>\n"
        "  <Website>https://github.com/CageTV/Simple-Hunting-2026</Website>\n"
        "  <Description>Simple Hunting Overhaul's settings, Immersive Hunting Animations' options and hunting hotkeys in SKSE Menu Framework, replacing both MCM add-ons.</Description>\n</fomod>\n")
xml.dom.minidom.parseString(config)
xml.dom.minidom.parseString(info)
os.makedirs(os.path.join(dest, "fomod"), exist_ok=True)
open(os.path.join(dest, "fomod", "ModuleConfig.xml"), "w", encoding="utf-8", newline="\n").write(config)
open(os.path.join(dest, "fomod", "info.xml"), "w", encoding="utf-8", newline="\n").write(info)
# ---- the GitHub-only scripts add-on ----------------------------------------------------------------------------------------------
SNAME = "Simple Hunting Overhaul 2026 - Optimised Scripts"
sdest = os.path.join(HERE, "package", SNAME)
if os.path.exists(sdest):
    shutil.rmtree(sdest)
scripts_into(sdest)
open(os.path.join(sdest, "README.txt"), "w", encoding="utf-8", newline="\n").write("""Simple Hunting Overhaul 2026 - Optimised Scripts
Rebuilt from SHO_PickUpCarCass.psc and SHO_PlayerAliasScript.psc of Simple Hunting Overhaul (Nexus 95943) by JaySerpa. Needs the original mod. GitHub only: it is
not on Nexus, because it contains modified copies of the original author's scripts. It stays up while the author has no objection and comes down on request.

WHAT IT CHANGES
---------------
SHO_PickUpCarCass reads the animal's voice type once (it did up to twelve native calls) and no longer errors when you press the pick-up key with nothing in the
crosshair. SHO_PlayerAliasScript looks the animation mod's globals up once instead of on every skinning, and reads the actor and the looting mode once.
Nothing about how skinning plays changes.

INSTALL
-------
Install it after (below) the original Simple Hunting Overhaul in MO2's left pane so the scripts win. Uninstall it to go back to the original.

CREDITS AND LICENSE
-------------------
Simple Hunting Overhaul and the original scripts are by JaySerpa. These rebuilt copies are shared with credit and without any claim over the original work.
""")
szip = sdest + ".zip"
if os.path.exists(szip):
    os.remove(szip)
with zipfile.ZipFile(szip, "w", zipfile.ZIP_DEFLATED) as z:
    for dp, _, fs in os.walk(sdest):
        for fn in fs:
            pth = os.path.join(dp, fn)
            z.write(pth, os.path.relpath(pth, sdest))
print("scripts add-on:", szip)

zpath = dest + ".zip"
if os.path.exists(zpath):
    os.remove(zpath)
with zipfile.ZipFile(zpath, "w", zipfile.ZIP_DEFLATED) as z:
    for dp, _, fs in os.walk(dest):
        for fn in fs:
            p = os.path.join(dp, fn)
            z.write(p, os.path.relpath(p, dest))
    names = z.namelist()
print(f"{zpath}: {len(names)} entries, {os.path.getsize(zpath)//1024} KB")
for n in sorted(names):
    print("  ", n)
