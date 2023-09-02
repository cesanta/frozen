project(
    'frozen',
    'c',
    default_options: [
        'c_args=-Wextra -fno-builtin -pedantic',
        'c_std=c99',
        'werror=true'
    ],
    license: 'Apache-2.0',
    version: '1.7'
)

libfrozen = library(
    'frozen',
    'frozen.c',
    install: true
)
install_headers('frozen.h')

unit_test = executable('unit_test',
    'unit_test.c',
    dependencies: [libfrozen]
)
test('Frozen Unit Test', unit_test)

