# Meson overlay for ckaiser/UGlobalHotkey (upstream ships qmake .pro/.pri only).
# This is the ONLY file in the patch_directory overlay; it is copied into the fetched
# source tree at configure time. The Qt 6 port + operator[] fix are applied separately
# as a real patch via the wrap's diff_files (0001-qt6-port.patch), after this overlay.
# Full patch breakdown lives one level up in
# subprojects/packagefiles/uglobalhotkey.PROVENANCE.md.
# Upstream ckaiser/UGlobalHotkey publishes no tags or releases, so there is no
# semver to adopt. The version encodes the pinned commit from uglobalhotkey.wrap
# (revision 231b10144741b29037f0128bb7a1cd7176529f74) so `meson setup` reports the
# exact fetched revision instead of "undefined".
project('uglobalhotkey', 'cpp', version: '0-git.231b101',
  default_options: ['cpp_std=c++17'])

qt6 = import('qt6')
cc = meson.get_compiler('cpp')

# Widgets is required in addition to Core/Gui: UGlobalHotkeys derives from QWidget
# (uglobalhotkeys.h) so it can receive native/win events — the original
# uglobalhotkey.pro also pulled in `QT += widgets`.
deps = [dependency('qt6', modules: ['Core', 'Gui', 'Widgets'])]
extra_args = ['-DUGLOBALHOTKEY_NOEXPORT']

if host_machine.system() == 'linux'
  deps += [cc.find_library('xcb'), cc.find_library('xcb-keysyms')]
  # NOTE: no gui-private needed — the patched uglobalhotkeys.cpp uses the public
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
