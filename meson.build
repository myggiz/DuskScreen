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

message('DuskScreen: Qt6 = ' + qt6_dep.version())
