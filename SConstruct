# -*- python -*-

env = Environment()

if ARGUMENTS.get('OPTIMIZE'):
    optimize = ARGUMENTS.get('OPTIMIZE')
else:
    optimize = 3

# More readable output
if not ARGUMENTS.get('VERBOSE'):
    env['CXXCOMSTR'] = 'Compiling C++ object $TARGETS'
    env['CCCOMSTR'] = 'Compiling C object $TARGETS'
    env['ARCOMSTR'] = 'Packing static library $TARGETS'
    env['RANLIBCOMSTR'] = 'Indexing static library $TARGETS'
    env['SHCCCOMSTR'] = 'Compiling shared C object $TARGETS'
    env['SHCXXCOMSTR'] = 'Compiling shared C++ object $TARGETS'
    env['LINKCOMSTR'] = 'Linking $TARGETS'
    env['SHLINKCOMSTR'] = 'Linking shared $TARGETS'

env.Append(CPPFLAGS=['-std=c++11','-Wall','-Wextra','-pedantic','-Wno-narrowing'])
env.Append(CPPPATH=['include'])
env.Append(LIBS=['png'])
env.Append(LIBS=[File('/usr/local/lib/libboost_program_options.a')])

if ARGUMENTS.get('RELEASE'):
    env.Append(CPPDEFINES=['NDEBUG'])
    env.Append(CPPFLAGS=['-s'])
else:
    env.Append(CPPFLAGS=['-g'])

if ARGUMENTS.get('PROFILE'):
    env.Append(CPPFLAGS=['-pg','-g'])
    env.Append(LINKFLAGS=['-pg'])

if optimize!='0':
    env.Append(CPPFLAGS=['-O{}'.format(optimize)])

lua_bindings = env.SConscript('lua-bindings/SConscript', 'env')
env.Append(CPPPATH = ['lua-bindings/include', 'lua-bindings/lua-5.3.0/src'] )
env.Append(LIBS = [lua_bindings])

for main in Glob('*.cc'):
    env.Program([main,Glob('src/*.cc')])
