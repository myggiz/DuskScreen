project('uglobalhotkey', 'cpp', default_options: ['cpp_std=c++17'])

qt6 = import('qt6')
cc = meson.get_compiler('cpp')

# Widgets is required in addition to the brief's Core/Gui: UGlobalHotkeys derives from
# QWidget (uglobalhotkeys.h) so it can receive native/win events — the original
# uglobalhotkey.pro also pulled in `QT += widgets`.
deps = [dependency('qt6', modules: ['Core', 'Gui', 'Widgets'])]
extra_args = ['-DUGLOBALHOTKEY_NOEXPORT']

if host_machine.system() == 'linux'
  deps += [cc.find_library('xcb'), cc.find_library('xcb-keysyms')]
  # NOTE: no gui-private needed — uglobalhotkeys.cpp uses the public QX11Application
  # native interface instead of the private QPA platformNativeInterface() API.
elif host_machine.system() == 'windows'
  deps += cc.find_library('user32')
endif

lib = static_library('uglobalhotkey',
  ['ukeysequence.cpp', 'uglobalhotkeys.cpp'],
  dependencies: deps,
  cpp_args: extra_args)

uglobalhotkey_dep = declare_dependency(
  link_with: lib,
  include_directories: include_directories('.'),
  dependencies: deps,
  compile_args: extra_args)
