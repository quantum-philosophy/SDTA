function generate(varargin)
  if nargin > 0
    prefix = '';
    testCases = struct();
    for i = 1:nargin
      testCases(i).name = varargin{i};
    end
  else
    prefix = 'test_cases/';
    fprintf('Processing all test cases in %s\n', Utils.path('test_cases'));
    testCases = dir(Utils.path([ prefix, '*.tgffopt' ]));
  end

  for i = 1:length(testCases)
    name = strrep(testCases(i).name, '.tgffopt', '');

    fprintf('%d. Generate a test case: %s\n', i, name);

    floorplan = Utils.path([ prefix, name, '.flp' ]);
    config = Utils.path([ prefix, name ]);

    % Generate a test case
    result = Utils.run('tgff', config, true);

    if result ~= 0, error('Cannot run TGFF'); end

    tgff = TestCase.TGFF([ config, '.tgff' ]);

    if length(tgff.graphs) ~= 1
      error('Wrong number of task graphs.');
    end

    % Generate a floorplan
    Utils.generateFloorplan(floorplan, length(tgff.pes));

    systemConfig = Utils.compactTaskGraph(tgff.graphs{1}, tgff.pes);
    systemConfig.type = systemConfig.type - 1;

    Utils.dumpObject(systemConfig, [ config, '.txt' ]);
  end
end
