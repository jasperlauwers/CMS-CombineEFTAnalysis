#!/bin/bash

# Set cards to use:
cardVersion=fixV1  # v1 or v2

############################################
# Cards and fits in public
cardDir=/afs/cern.ch/user/j/jlauwers/public/TP/Cards/${cardVersion}/*.txt
anoFitDir=/afs/cern.ch/user/j/jlauwers/public/TP/EFTfits/${cardVersion}

# Other scripts in repository
buildWorkspace=$CMSSW_BASE/src/CombineEFT/CommonTools/test/buildWorkspace.py
save1DPlot=$CMSSW_BASE/src/CombineEFT/CommonTools/scripts/save1DPlot.C
save2DPlot=$CMSSW_BASE/src/CombineEFT/CommonTools/scripts/save2DPlot.C
convertDC2EFT=$CMSSW_BASE/src/CombineEFT/CommonTools/python/convertDC2EFT.py
build1DInterval=$CMSSW_BASE/src/CombineEFT/CommonTools/python/build1DInterval.py

# Results directory
finalDir=$CMSSW_BASE/src/CombineEFT/CommonTools/test/Results_${cardVersion}
############################################

anaName=WWVBS_mll
declare -a minScanValues=(1.15 3.8 0.8 1.05 1.7 1.55 0.08 0.02 0.042) # minus is added later
declare -a maxScanValues=(1.15 3.8 0.8 1.05 1.7 1.55 0.08 0.036 0.125)
counter=0

if [ ! -d $finalDir ]; then mkdir $finalDir; fi
cd $finalDir

for param in LS0 LS1 LM0 LM1 LM6 LM7 LT0 LT1 LT2 LS0_LS1
do 
    categString=""

    for card in $cardDir
    do
        categ=`echo $card | rev | cut -d "_" -f1 | rev | cut -d "." -f1`
        categString=`echo $categString ${categ}"=WWVBS_mll_"${categ}_$param".txt"` 
        if [ "$param" = LS0_LS1 ]
        then
            python $convertDC2EFT -i $card -o $finalDir/config_mll_$categ -p LS0:-9:9:LS0\;LS1:-30:30:LS1 -a ${anaName}_$categ
        else
            python $convertDC2EFT -i $card -o $finalDir/config_mll_$categ -p $param:-${minScanValues[counter]}:${maxScanValues[counter]}:$param -a ${anaName}_$categ
        fi
        cp $anoFitDir/signal_${anaName}_$param.root $finalDir/signal_${anaName}_${categ}_$param.root
        python $buildWorkspace --config=$finalDir/config_mll_$categ
    done

    combineCards.py $categString > ${anaName}_$param.txt
    
    # Run EFT framework and get results
    text2workspace.py ${anaName}_$param.txt -o VBS_SS_Combine.root -P CombineEFT.CommonTools.HigherDimensionalOperators:$param
    
    if [ "$param" = LS0_LS1 ]
    then       
        combine VBS_SS_Combine.root -M MultiDimFit -P LS0 -P LS1 --floatOtherPOIs=0 --algo=grid --points=10000 --minimizerStrategy=2 -n test_VBS_$param -t -1 --expectSignal=1
        root -l -b -q ${save2DPlot}+
    else
        combine VBS_SS_Combine.root -M MultiDimFit -P $param --floatOtherPOIs=0 --algo=grid --points=1000 --minimizerStrategy=2 -n test_VBS_$param -t -1 --expectSignal=1
        python $build1DInterval ${minScanValues[counter]} ${maxScanValues[counter]} higgsCombinetest_VBS_$param.MultiDimFit.mH120.root $param >> Limits1D.txt
        root -l -b -q ${save1DPlot}+'("'$param'")'
    fi
    
    counter=$((counter + 1))
done

cd -

