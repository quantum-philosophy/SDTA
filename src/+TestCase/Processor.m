classdef Processor < handle
  properties (SetAccess = private)
    name
    id

    frequency
    voltage

    ceff
    nc

    typeCount
  end

  methods
    function pe = Processor(name, id)
      pe.name = name;
      pe.id = id;
      pe.ceff = zeros(0, 0);
      pe.nc = zeros(0, 0);
      pe.typeCount = 0;
    end

    function setFrequency(pe, value)
      pe.frequency = value;
    end

    function setVoltage(pe, value)
      pe.voltage = value;
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
      fprintf('  Number of types: %d\n', pe.typeCount);
    end
  end
end
