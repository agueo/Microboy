project('MicroBoy', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++20'])

sdl_dep = dependency('sdl2', version:'2.0.20')
fmt_dep = dependency('fmt')
gtest_dep = dependency('gtest', main: true)

inc = include_directories('Microboy')
subdir('Microboy')
subdir('MicroBoyTests')

app = executable('MicroBoy',
  'Microboy/main.cpp',
  'Microboy/Window.cpp',
  link_with: libdmg,
  dependencies: [fmt_dep, sdl_dep],
  install : true)

test_opcodes = executable('TestOpcodes',
  'MicroBoyTests/test_opcodes.cpp',
  include_directories: [inc],
  dependencies: [gtest_dep],
  link_with: [libdmg],
  )

test_cpu = executable('TestCpu',
  'MicroBoyTests/test_cpu.cpp',
  include_directories: [inc],
  dependencies: [gtest_dep],
  link_with: [libdmg],
  )

test_bus = executable('TestBus',
  'MicroBoyTests/test_bus.cpp',
  include_directories: [inc],
  dependencies: [gtest_dep],
  link_with: [libdmg],
  )


test('Opcode Tests', test_opcodes)
test('Cpu Tests', test_cpu)
test('Bus Tests', test_bus)
