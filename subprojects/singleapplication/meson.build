project('singleapplication', 'cpp', default_options: ['cpp_std=c++17'])

qt6 = import('qt6')
# Widgets (pulls in Gui) is required because singleapplication.h defaults
# QAPPLICATION_CLASS to QApplication, which lives in QtWidgets; the brief's
# Core+Network set alone fails to compile (verified: fatal error: QApplication:
# No such file or directory).
qt_dep = dependency('qt6', modules: ['Core', 'Network', 'Widgets'])

lib = static_library('singleapplication',
  'singleapplication.cpp',
  dependencies: qt_dep)

singleapplication_dep = declare_dependency(
  link_with: lib,
  include_directories: include_directories('.'),
  dependencies: qt_dep)
