# Meson overlay for itay-grudev/SingleApplication (upstream ships CMake + qmake only).
# Copied into the fetched source tree at configure time via the wrap's patch_directory.
project('singleapplication', 'cpp', default_options: ['cpp_std=c++17'])

qt6 = import('qt6')

# SingleApplication is templated on the Qt application class via QAPPLICATION_CLASS.
# DuskScreen is a QApplication (Widgets) app; this define MUST be identical for the
# library build, its moc, AND every consumer that includes singleapplication.h — so
# it is re-exported through declare_dependency(compile_args:) below.
qapp_def = '-DQAPPLICATION_CLASS=QApplication'

qt_dep = dependency('qt6', modules: ['Core', 'Network', 'Widgets'])

moc = qt6.compile_moc(
  headers: ['singleapplication.h', 'singleapplication_p.h'],
  dependencies: qt_dep,
  include_directories: include_directories('.'),
  extra_args: [qapp_def],
)

lib = static_library('singleapplication',
  ['singleapplication.cpp', 'singleapplication_p.cpp', moc],
  dependencies: qt_dep,
  cpp_args: [qapp_def],
)

singleapplication_dep = declare_dependency(
  link_with: lib,
  include_directories: include_directories('.'),
  dependencies: qt_dep,
  compile_args: [qapp_def],
)
