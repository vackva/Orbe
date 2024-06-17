% Get new sampling grid
N = 44;
denseSamplingGrid = supdeq_lebedev([], N);

% 1. import hrirs
sparseHRIRdataset_SOFA = SOFAload('pp2_HRIRs_measured.sofa');
hrtfDataset = supdeq_sofa2hrtf(sparseHRIRdataset_SOFA, N);

% MCA Interpolation
headRadius = 0.0875;

interpHRTF_sh = supdeq_interpHRTF(hrtfDataset, denseSamplingGrid,'None','SH', nan, headRadius);
interpHRTF_sh_timealign = supdeq_interpHRTF(hrtfDataset, denseSamplingGrid,'None','SH', nan, headRadius);
interpHRTF_mca_limited = supdeq_interpHRTF(hrtfDataset, denseSamplingGrid,'SUpDEq','SH', inf, headRadius);
interpHRTF_mca = supdeq_interpHRTF(hrtfDataset, denseSamplingGrid,'SUpDEq','SH', inf, headRadius);

% Write SOFA file
fs = 44100;
earDistance = 0.165;
sourceDistance = 1.47;

HRIR_L = permute(interpHRTF_sh.HRIR_L,[2 1]);
HRIR_R = permute(interpHRTF_sh.HRIR_R,[2 1]);
SOFAobj = supdeq_writeSOFAobj(HRIR_L, HRIR_R, denseSamplingGrid, fs, earDistance, sourceDistance); 
SOFAsave('pp2_HRIRs_interpolated_sh.sofa', SOFAobj);

HRIR_L = permute(interpHRTF_sh_timealign.HRIR_L,[2 1]);
HRIR_R = permute(interpHRTF_sh_timealign.HRIR_R,[2 1]);
SOFAobj = supdeq_writeSOFAobj(HRIR_L, HRIR_R, denseSamplingGrid, fs, earDistance, sourceDistance); 
SOFAsave('pp2_HRIRs_interpolated_sh_timealign.sofa', SOFAobj);

HRIR_L = permute(interpHRTF_mca.HRIR_L,[2 1]);
HRIR_R = permute(interpHRTF_mca.HRIR_R,[2 1]);
SOFAobj = supdeq_writeSOFAobj(HRIR_L, HRIR_R, denseSamplingGrid, fs, earDistance, sourceDistance); 
SOFAsave('pp2_HRIRs_interpolated_mca.sofa', SOFAobj);

