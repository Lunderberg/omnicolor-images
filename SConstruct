# -*- python -*-

env = Environment()

env.Append(CPPFLAGS=['-std=c++11','-Wno-narrowing','-O3'])
env.Append(CPPPATH=['include'])
env.Append(LIBS=['png'])

if ARGUMENTS.get('PROFILE'):
    env.Append(CPPFLAGS=['-pg','-g'])
    env.Append(LINKFLAGS=['-pg'])

for main in Glob('*.cc'):
    env.Program([main,Glob('src/*.cc')])
