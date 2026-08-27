#!/usr/bin/env python
"""Builds the byProd GDExtension.

There is deliberately no dependency on the byProd SDK here: the wrapper resolves
the runtime's symbols at load time (see src/bp_api.cpp), so this builds and its
CI runs without Madrigal's proprietary files being present anywhere.
"""

import os

# Godot's importer walks the whole project, and two directories here are full of
# things it would try to read as assets: the submodule's object files (as OBJ
# meshes) and an unpacked SDK. src/ carries a committed .gdignore; these two are
# not in the repository, so the marker is dropped here instead — the build always
# runs before Godot can load the extension anyway.
for _directory in ("godot-cpp", "vendor"):
    _marker = os.path.join(_directory, ".gdignore")
    if os.path.isdir(_directory) and not os.path.exists(_marker):
        open(_marker, "w").close()

env = SConscript("godot-cpp/SConstruct")

env.Append(CPPPATH=["src/"])
sources = Glob("src/*.cpp")

library = env.SharedLibrary(
    "addons/byprod/bin/libgdbyprod{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
    source=sources,
)

Default(library)
