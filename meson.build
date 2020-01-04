project('vcc', ['c'],
    version: '0.0.1',
    license: 'MIT'
)

c_args = [
    '-O0',
    '-g',
    '-DENABLE_DEBUG'
]

sources = files(
    'main.c'
)

subdir('src')

dependencies = [
]

e = executable('vcc',
    sources: sources,
    c_args: c_args,
    dependencies: dependencies,
    install : false
)
