# -*- python -*-

env = Environment()

if ARGUMENTS.get('OPTIMIZE'):
    optimize = ARGUMENTS.get('OPTIMIZE')
else:
    optimize = 3

env.Append(CPPFLAGS=['-std=c++11','-Wno-narrowing'])
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

for main in Glob('*.cc'):
    env.Program([main,Glob('src/*.cc')])
