clear all;
clc;

sweep = Sweep.Power('001_030', 1:1:100, 81e-6);
sweep.perform();
sweep.draw();
