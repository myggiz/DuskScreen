# NOTE: duskscreen_core carries no qrc resources — they live in the app executable
# (Qt strips resource objects out of static libs). The current tests need none. A
# future test that constructs a resource-using widget must add the qrc to this
# target's sources or call Q_INIT_RESOURCE explicitly, or it will link fine and hit
# a missing resource at runtime.
gtest_dep = dependency('gtest', required: get_option('tests'))

if gtest_dep.found()
  # One entry per line so adding a test suite is a clean one-line diff.
  test_sources = [
    'main.cpp',
    'ukeysequence_test.cpp',
    'naming_test.cpp',
    'screenshot_test.cpp',
    'os_x11guard_test.cpp',
  ]

  duskscreen_tests = executable('duskscreen_tests',
    sources: test_sources,
    link_with: duskscreen_core,
    dependencies: [qt6_dep, gtest_dep, uglobalhotkey_dep],
    include_directories: include_directories('..'))

  test('unit', duskscreen_tests, env: {'QT_QPA_PLATFORM': 'offscreen'})
endif
