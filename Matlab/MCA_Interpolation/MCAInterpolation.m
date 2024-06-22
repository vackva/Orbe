% COORDINATE STATES

% 1. HUTUBS SOFA FILE (azim: [0, 360], elev [-90, 90])
% 2. sparseHRIRdataset_SOFA (azim: [0, 360], elev [-90, 90]) [retrieved by sparseHRIRdataset.SourcePosition]
% 3. hrtfDataset (azim: [0, 360], elev [0, 180]) [retrieved by hrtfDataset.samplingGrid]
% 4. denseSamplingGrid (azim: [0, 360], elev [0, 180])
% 5. interpHRTF (azim: [0, 360], elev [0, 180]) [retrieved by interpHRTF.samplingGrid]
% 6. SOFAobj (azim: [0, 360], elev [-90, 90]) [retrieved by SOFAobj.SourcePosition]


% CONCLUSIONS

% supdeq_sofa2hrtf transforms elevation to colatitude
% Code Snippet:
% 0110     %Transform samplingGrid to SH coordinate system
% 0111     samplingGrid(:,1) = mod(samplingGrid(:,1),360);
% 0112     samplingGrid(:,2) = 90-samplingGrid(:,2);

% supdeq_writeSOFAobj re-transforms colatitude to elevation
% Code Snippet:
% 0091 %Transform grid to SOFA format
% 0092 samplingGrid(:,2) = 90-samplingGrid(:,2);

% Get new sampling grid
N = 44;
denseSamplingGrid = supdeq_lebedev([], N);

% remove weights
%denseSamplingGrid(:,3) = [1];

% 1. import hrirs
sparseHRIRdataset_SOFA = SOFAload('pp2_HRIRs_measured.sofa');
hrtfDataset = supdeq_sofa2hrtf(sparseHRIRdataset_SOFA, 12);
sparseSamplingGrid = hrtfDataset.samplingGrid;

% MCA Interpolation
headRadius = 0.0875;

interpHRTF_sh = supdeq_interpHRTF(hrtfDataset, denseSamplingGrid,'None','SH', nan, headRadius);
interpHRTF_sh_timealign = supdeq_interpHRTF(hrtfDataset, denseSamplingGrid,'None','SH', nan, headRadius);
%interpHRTF_mca_limited = supdeq_interpHRTF(hrtfDataset, denseSamplingGrid,'SUpDEq','SH', inf, headRadius);
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

