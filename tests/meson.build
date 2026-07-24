gtest_dep = dependency('gtest', required: get_option('tests'))

if gtest_dep.found()
  duskscreen_tests = executable('duskscreen_tests',
    sources: ['main.cpp', 'ukeysequence_test.cpp'],
    link_with: duskscreen_core,
    dependencies: [qt6_dep, gtest_dep, uglobalhotkey_dep],
    include_directories: include_directories('..'))

  test('unit', duskscreen_tests, env: {'QT_QPA_PLATFORM': 'offscreen'})
endif
