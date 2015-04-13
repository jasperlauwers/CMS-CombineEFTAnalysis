# CMS-CombineEFTAnalysis
Extraction of EWK higher-dimensional operators using combine

VBS for TP version

## Set-up:

```
cmsrel CMSSW_7_3_0
cd CMSSW_7_3_0/src/
cmsenv
git clone https://github.com/cms-analysis/HiggsAnalysis-CombinedLimit.git HiggsAnalysis/CombinedLimit
git clone https://github.com/jasperlauwers/CMS-CombineEFTAnalysis.git CombineEFT
source CombineEFT/CommonTools/setup/patchToSource.sh
scram b -j
```

## Run code:
This will convert datacards to EFT config cards, run the EFT framework, combine and calculate and draw the limits from the deltaLL profile likelihood

`source CommonTools/scripts/makeEFTcards.sh`

