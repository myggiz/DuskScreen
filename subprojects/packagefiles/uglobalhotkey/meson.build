# Meson overlay for ckaiser/UGlobalHotkey (upstream ships qmake .pro/.pri only).
# Copied into the fetched source tree at configure time via the wrap's patch_directory,
# alongside the three Qt 6-ported source files in this same overlay. Full patch
# breakdown lives one level up in subprojects/packagefiles/uglobalhotkey.PROVENANCE.md
# (kept outside this patch_directory so it is not copied into the fetched build tree).
project('uglobalhotkey', 'cpp', default_options: ['cpp_std=c++17'])

qt6 = import('qt6')
cc = meson.get_compiler('cpp')

# Widgets is required in addition to Core/Gui: UGlobalHotkeys derives from QWidget
# (uglobalhotkeys.h) so it can receive native/win events — the original
# uglobalhotkey.pro also pulled in `QT += widgets`.
deps = [dependency('qt6', modules: ['Core', 'Gui', 'Widgets'])]
extra_args = ['-DUGLOBALHOTKEY_NOEXPORT']

if host_machine.system() == 'linux'
  deps += [cc.find_library('xcb'), cc.find_library('xcb-keysyms')]
  # NOTE: no gui-private needed — the overlaid uglobalhotkeys.cpp uses the public
  # QX11Application native interface instead of the private QPA
  # platformNativeInterface() API.
elif host_machine.system() == 'windows'
  deps += cc.find_library('user32')
endif

# uglobalhotkeys.h and ukeysequence.h both declare Q_OBJECT (signals/slots),
# so they need moc'd like any other QObject-derived header.
moc = qt6.compile_moc(headers: ['uglobalhotkeys.h', 'ukeysequence.h'], dependencies: deps,
  include_directories: include_directories('.'), extra_args: extra_args)

lib = static_library('uglobalhotkey',
  ['ukeysequence.cpp', 'uglobalhotkeys.cpp', moc],
  dependencies: deps,
  cpp_args: extra_args)

uglobalhotkey_dep = declare_dependency(
  link_with: lib,
  include_directories: include_directories('.'),
  dependencies: deps,
  compile_args: extra_args)
