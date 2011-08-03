function generate()
  testCases = dir(Utils.path('test_cases/*.tgffopt'));

  for i = 1:length(testCases)
    name = strrep(testCases(i).name, '.tgffopt', '');

    floorplan = Utils.path([ 'test_cases/', name, '.flp' ]);
    config = Utils.path([ 'test_cases/', name ]);

    % Generate a test case
    Utils.startTimer('Generate a test case: %s', name);
    result = Utils.run('tgff', config, true);
    Utils.stopTimer();

    if result ~= 0, error('Cannot run TGFF'); end

    tgff = TestCase.TGFF([ config, '.tgff' ]);
    cores = length(tgff.pes);

    % Generate a floorplan
    Utils.startTimer('Generate a floorplan for %d cores', cores);
    Utils.generateFloorplan(floorplan, cores);
    Utils.stopTimer();
  end
end
