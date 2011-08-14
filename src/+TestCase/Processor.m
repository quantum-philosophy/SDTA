classdef Processor < handle
  properties (SetAccess = private)
    id
    name
    type

    ceff
    nc

    typeCount
  end

  properties
    frequency
    voltage
    ngate
  end

  methods
    function pe = Processor(id, name, type, frequency, voltage, ngate)
      pe.id = id;
      pe.name = name;
      pe.type = type;

      if nargin < 4, frequency = 2e9; end % Hz
      if nargin < 5, voltage = 0.8; end % V
      if nargin < 6, ngate = 1e7; end

      pe.frequency = frequency;
      pe.voltage = voltage;
      pe.ngate = ngate;

      pe.ceff = zeros(0, 0);
      pe.nc = zeros(0, 0);

      pe.typeCount = 0;
    end

    function addType(pe, id, ceff, nc)
      pe.ceff(id) = ceff;
      pe.nc(id) = nc;
      pe.typeCount = pe.typeCount + 1;
    end

    function inspect(pe)
      fprintf('Processor: %s %d\n', pe.name, pe.id);
      fprintf('  Frequency: %d\n', pe.frequency);
      fprintf('  Voltage: %f\n', pe.voltage);
      fprintf('  Number of gates: %d\n', pe.ngate);
      fprintf('  Number of types: %d\n', pe.typeCount);
    end
  end
end
