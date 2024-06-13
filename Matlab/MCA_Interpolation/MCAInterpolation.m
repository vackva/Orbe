% Get new sampling grid
N = 44;
denseSamplingGrid = supdeq_lebedev([], N);

% 1. import hrirs
sparseHRIRdataset_SOFA = SOFAload('pp2_HRIRs_measured.sofa');
hrtfDataset = supdeq_sofa2hrtf(sparseHRIRdataset_SOFA, N);

% MCA Interpolation
interpHRTF_mca = supdeq_interpHRTF(hrtfDataset, denseSamplingGrid,'SUpDEq','SH');

% Write SOFA file
fs = 48000;
earDistance = 0.165;
sourceDistance = 1.47;
HRIR_L = permute(interpHRTF_mca.HRIR_L,[2 1]);
HRIR_R = permute(interpHRTF_mca.HRIR_R,[2 1]);
SOFAobj = supdeq_writeSOFAobj(HRIR_L, HRIR_R, denseSamplingGrid, fs, earDistance, sourceDistance); 

% Save SOFA file
SOFAsave('pp2_HRIRs_interpolated_mca', SOFAobj);

