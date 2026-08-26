#!/usr/bin/env python
"""Builds the byProd GDExtension.

There is deliberately no dependency on the byProd SDK here: the wrapper resolves
the runtime's symbols at load time (see src/bp_api.cpp), so this builds and its
CI runs without Madrigal's proprietary files being present anywhere.
"""

import os

# SCons drops object files next to the sources it compiles, and Godot's importer
# would otherwise try to read every .obj in the tree as an OBJ mesh. src/ carries
# a committed .gdignore; the submodule cannot, so the marker is placed here — the
# build always runs before Godot can load the extension anyway.
_submodule_marker = os.path.join("godot-cpp", ".gdignore")
if os.path.isdir("godot-cpp") and not os.path.exists(_submodule_marker):
    open(_submodule_marker, "w").close()

env = SConscript("godot-cpp/SConstruct")

env.Append(CPPPATH=["src/"])
sources = Glob("src/*.cpp")

library = env.SharedLibrary(
    "addons/byprod/bin/libgdbyprod{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
    source=sources,
)

Default(library)
