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
  end
end
