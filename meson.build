project('duskscreen', 'cpp',
  version: '1.0.3',
  meson_version: '>=1.1',
  default_options: ['cpp_std=c++17', 'warning_level=2'])

qt6 = import('qt6')
qt6_dep = dependency('qt6',
  modules: ['Core', 'Gui', 'Widgets', 'Network', 'Multimedia'],
  main: true,
  method: 'auto')

app_url = get_option('app_url')
add_project_arguments(
  '-DAPP_VERSION="' + meson.project_version() + '"',
  '-DAPP_URL="' + app_url + '"',
  language: 'cpp')

if host_machine.system() == 'windows'
  # Qt 6 CMake sets these transitively; the Meson qt6 module does not.
  # Match qmake/Qt 6 CMake so Win32 macros resolve to the wide-char variants.
  add_project_arguments('-DUNICODE', '-D_UNICODE', language: 'cpp')
endif

singleapplication_dep = subproject('singleapplication').get_variable('singleapplication_dep')
uglobalhotkey_dep = subproject('uglobalhotkey').get_variable('uglobalhotkey_dep')

message('DuskScreen: Qt6 = ' + qt6_dep.version())

moc_headers = [
  'dialogs/areadialog.h', 'dialogs/namingdialog.h', 'dialogs/optionsdialog.h',
  'dialogs/updaterdialog.h', 'duskscreenwindow.h',
  'tools/screenshot.h', 'tools/screenshotmanager.h', 'tools/windowpicker.h',
  'updater/updater.h', 'widgets/hotkeywidget.h',
]
ui_files = ['dialogs/namingdialog.ui', 'dialogs/optionsdialog.ui', 'duskscreenwindow.ui']

moc = qt6.compile_moc(headers: moc_headers, dependencies: qt6_dep,
  include_directories: include_directories('.'))
ui  = qt6.compile_ui(sources: ui_files)
qrc = qt6.compile_resources(sources: 'duskscreen.qrc')

core_sources = [
  'duskscreenwindow.cpp',
  'dialogs/areadialog.cpp', 'dialogs/namingdialog.cpp',
  'dialogs/optionsdialog.cpp', 'dialogs/updaterdialog.cpp',
  'tools/os.cpp', 'tools/screenshot.cpp', 'tools/screenshotmanager.cpp',
  'tools/windowpicker.cpp', 'updater/updater.cpp', 'widgets/hotkeywidget.cpp',
]

app_deps = [qt6_dep, singleapplication_dep, uglobalhotkey_dep]

if host_machine.system() == 'linux'
  app_deps += meson.get_compiler('cpp').find_library('X11')
endif

# Generate the versioned Windows resource script from project(version:) so the
# version lives in exactly one place. Run unconditionally (it is cheap) so the
# substitution is exercised on every platform, not only on Windows.
# NOTE: assumes project(version:) is numeric MAJOR.MINOR.PATCH (VERSIONINFO
# FILEVERSION requires integer fields). A pre-release suffix (e.g. 1.0.3-rc1)
# would need this split adjusted.
ver_parts = meson.project_version().split('.')
rc_conf = configuration_data()
rc_conf.set('VER_COMMA', ver_parts[0] + ',' + ver_parts[1] + ',' + ver_parts[2] + ',0')
rc_conf.set('VER_DOT', meson.project_version())
duskscreen_rc = configure_file(
  input: 'platform/win/duskscreen.rc.in',
  output: 'duskscreen.rc',
  configuration: rc_conf)

platform_sources = []

if host_machine.system() == 'windows'
  windows = import('windows')
  # include_directories('.') lets the resource compiler resolve the icon/manifest
  # paths (relative to the project source root) even though the generated .rc
  # lives in the build directory — portable across windres and MSVC rc.exe.
  platform_sources += windows.compile_resources(duskscreen_rc,
    include_directories: include_directories('.'),
    depend_files: files('images/LS.ico', 'platform/win/duskscreen.manifest'))
  cc = meson.get_compiler('cpp')
  foreach l : ['gdi32', 'user32', 'ole32', 'shell32', 'shlwapi', 'comctl32']
    app_deps += cc.find_library(l)
  endforeach
endif

duskscreen_core = static_library('duskscreen_core',
  sources: core_sources + moc + ui,     # NOT qrc — resources stay in the exe
  dependencies: app_deps,
  include_directories: include_directories('.'))

executable('duskscreen',
  # ui is listed here too (not compiled twice - qt6.compile_ui only generates
  # headers) purely so Meson adds duskscreen_core's private build dir to this
  # target's include path; main.cpp includes ui_duskscreenwindow.h directly.
  sources: ['main.cpp'] + ui + qrc + platform_sources,
  link_with: duskscreen_core,
  dependencies: app_deps,
  include_directories: include_directories('.'),
  win_subsystem: 'windows',
  install: true)

if not get_option('tests').disabled()
  subdir('tests')
endif
