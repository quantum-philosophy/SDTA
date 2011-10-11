classdef Thickness < Sweep.Basic
  properties (SetAccess = private)
    processorThickness
  end

  methods
    function sweep = Thickness(test, processorThickness)
      sweep = sweep@Sweep.Basic(test);
      sweep.variable = 'Thickness, mm';

      sweep.processorThickness = processorThickness;
    end
  end

  methods (Access = protected)
    function result = continueStep(sweep, i)
      if i > length(sweep.processorThickness), result = false;
      else result = true;
      end
    end

    function config = setupStep(sweep, i)
      thickness = sweep.processorThickness(i);
      config = { 'hotspot', [ 't_chip ', num2str(thickness) ] };
    end

    function [ value, retake ] = valueStep(sweep, i, Tce, Tss, power)
      value = sweep.processorThickness(i) * 1e3;
      retake = false;
    end
  end
end
