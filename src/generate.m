function generate(varargin)
  if nargin > 0
    testCases = struct();
    for i = 1:nargin
      testCases(i).name = varargin{i};
    end
  else
    fprintf('Processing all test cases in %s\n', Utils.path('test_cases'));
    testCases = dir(Utils.path('test_cases/*.tgffopt'));
  end

  for i = 1:length(testCases)
    name = strrep(testCases(i).name, '.tgffopt', '');

    fprintf('%d. Generate a test case: %s\n', i, name);

    floorplan = Utils.path([ 'test_cases/', name, '.flp' ]);
    config = Utils.path([ 'test_cases/', name ]);

    % Generate a test case
    result = Utils.run('tgff', config, true);

    if result ~= 0, error('Cannot run TGFF'); end

    tgff = TestCase.TGFF([ config, '.tgff' ]);
    cores = length(tgff.pes);

    % Generate a floorplan
    Utils.generateFloorplan(floorplan, cores);
  end
end
