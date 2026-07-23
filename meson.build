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

app_sources = [
  'main.cpp', 'duskscreenwindow.cpp',
  'dialogs/areadialog.cpp', 'dialogs/namingdialog.cpp',
  'dialogs/optionsdialog.cpp', 'dialogs/updaterdialog.cpp',
  'tools/os.cpp', 'tools/screenshot.cpp', 'tools/screenshotmanager.cpp',
  'tools/windowpicker.cpp', 'updater/updater.cpp', 'widgets/hotkeywidget.cpp',
]

app_deps = [qt6_dep, singleapplication_dep, uglobalhotkey_dep]

if host_machine.system() == 'linux'
  app_deps += meson.get_compiler('cpp').find_library('X11')
endif

executable('duskscreen',
  sources: app_sources + moc + ui + qrc,
  dependencies: app_deps,
  include_directories: include_directories('.'),
  win_subsystem: 'windows',
  install: true)
