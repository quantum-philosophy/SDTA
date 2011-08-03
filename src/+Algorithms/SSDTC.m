classdef SSDTC < handle
  properties (SetAccess = private)
    thermalModel
    powerProfile

    coreCount
    stepCount
  end

  methods
    function ssdtc = SSDTC(thermalModel, powerProfile)
      ssdtc.thermalModel = thermalModel;
      ssdtc.powerProfile = powerProfile;

      ssdtc.coreCount = size(powerProfile, 2);
      ssdtc.stepCount = size(powerProfile, 1);
    end

    function T = solveCondensedEquation(ssdtc)
      T = ssdtc.thermalModel.solveCondensedEquation(ssdtc.powerProfile);
    end

    function [ T, it ] = solveOriginal(ssdtc, varargin)
      [ T, it ] = ssdtc.thermalModel.solveOriginal(ssdtc.powerProfile, varargin{:});
    end

    function T = solvePlainOriginal(ssdtc, repeat)
      powerFile = sprintf('cores_%d_steps_%d.ptrace', ...
        ssdtc.coreCount, ssdtc.stepCount);
      powerFile = Utils.path(powerFile);

      Utils.startTimer('Dump the power profile');
      ssdtc.dumpPowerProfile(powerFile);
      Utils.stopTimer();

      T = ssdtc.thermalModel.solvePlainOriginal(...
        powerFile, ssdtc.stepCount, repeat);
    end

    function T = solveBlockCirculant(ssdtc)
      T = ssdtc.thermalModel.solveBlockCirculant(ssdtc.powerProfile);
    end

    function dumpPowerProfile(ssdtc, file)
      Utils.dumpPowerProfile(file, ssdtc.powerProfile);
    end

    function fitPowerProfile(ssdtc, steps)
      currentSteps = ssdtc.stepCount;

      Utils.startTimer('Transform the power profile from %d to %d', ...
        currentSteps, steps);

      if steps < currentSteps
        ssdtc.powerProfile = ssdtc.powerProfile(1:steps, :);
      elseif steps > currentSteps
        repeat = floor(steps / currentSteps);
        profile = zeros(0, 0);
        for i = 1:repeat
          profile = [ profile; ssdtc.powerProfile ];
        end
        rest = steps - repeat * currentSteps;
        profile = [ profile; ssdtc.powerProfile(1:rest, :) ];

        ssdtc.powerProfile = profile;
      end

      Utils.stopTimer();

      ssdtc.stepCount = steps;
    end
  end
end
