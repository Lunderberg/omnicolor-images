Import("env")

env.Tool("compilation_db")
env["COMPILATIONDB_USE_ABSPATH"] = True
env.CompilationDatabase()

env.Append(LIBS=["png", "boost_program_options"])

env.CompileFolderDWIM(".", requires=["lua-bindings"])
