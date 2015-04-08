#!/usr/bin/env python
import sys, re, os, os.path
from optparse import OptionParser
from manipDataCard import card as cardTools
import ROOT
from ROOT import *


# ------------------------------------------------------- MAIN --------------------------------------------

parser = OptionParser(usage="usage: %prog [options]")

parser.add_option("-a", "--AnaName"    , dest="AnaName"    , help="Analysis Name"       , default="WWVBS_mll", type='string' )
parser.add_option("-i", "--InputFile"  , dest="InputFile"  , help="Input datacard file" , default="None" , type='string' )
parser.add_option("-o", "--OutputFile" , dest="OutputFile" , help="Output EFT file"     , default="None" , type='string' )
#parser.add_option("-d", "--dimension"  , dest="dimension"  , help="EFT Param Dimension" , default=1      , type='float'  )
parser.add_option("-p", "--params"     , dest="params"     , help="EFT parameter(s)"    , default='cw:-50.:50.:c_{w}' , type='string')

(options, args) = parser.parse_args()

print "Input Data Card : ",options.InputFile
print "Output EFT File : ",options.OutputFile

if options.InputFile == 'None' :
   print 'Undefined Input File -> exit()'
   exit()
if options.OutputFile == 'None' :
   print 'Undefined Output File -> exit()'
   exit() 
if options.InputFile == options.OutputFile :
   print 'Input and Output files are the same !!! -> exit()'
   exit()

# Open Files
dc=cardTools(options.InputFile)
f = open(options.OutputFile,'w')

# [Global] Section

f.write('[Global] \n')

# ... EFT parameters
Params=options.params.split(';')
f.write('dimension = '+str(len(Params))+'\n')

nParam=0
for iParam in Params :
  nParam+=1
  vParam = iParam.split(':')
  f.write('par'+str(nParam)+'Name = ' + vParam[0] + '\n')
  f.write('par'+str(nParam)+'Low  = ' + vParam[1] + '\n')
  f.write('par'+str(nParam)+'High = ' + vParam[2] + '\n')
  f.write('par'+str(nParam)+'PlotName = ' + vParam[3] + '\n')

# ... Path
f.write('basepath = ./ \n')

# ... Find Signal and Backgrounds
iPos=0
Nbkg=0
BkgNames=[]
BkgPos=[]
Nsig=0
SigName=''
for iProc in dc.content['block2']['processId']:
  if float(iProc)>0 : 
     Nbkg+=1
     BkgNames.append(dc.content['block2']['process'][iPos])
     BkgPos.append(iPos)
  else: 
     Nsig+=1
     SigName=dc.content['block2']['process'][iPos]
     SigPos=iPos
  iPos+=1

# ... Find Histograms
for iDesc in dc.content['header2']:
  if dc.content['header2'][iDesc][1] == 'data_obs' :
    DataFile=dc.content['header2'][iDesc][3]
    DataHist=dc.content['header2'][iDesc][4]
  if dc.content['header2'][iDesc][1] == '*' and dc.content['header2'][iDesc][2] == '*' :
    MCFile=dc.content['header2'][iDesc][3]
    MCHist = dc.content['header2'][iDesc][4]
    if len(dc.content['header2'][iDesc]) == 6 : MCHisSyst = dc.content['header2'][iDesc][5]
    else: MCHisSyst ='None'

#print DataFile, DataHist
#print MCFile  , MCHist , MCHisSyst

# ... lnN systematics
if 'lnN' in dc.content['systs']:
  NlnN = len(dc.content['systs']['lnN'])
  f.write('NlnN = ' + str(NlnN) + '\n')
  counter = 1
  #print dc.content['systs']
  #print NlnN
  for systName,systVal in dc.content['systs']['lnN'].items():
    f.write('lnN'+ str(counter) + '_name = ' + systName + '\n')
    # get non-zero names and values
    syst = [[ systVal[i],dc.content['block2']['process'][i] ] for i in range(len(systVal)) if systVal[i] != '-']
    #print syst
    f.write('lnN'+ str(counter) + '_value = ')
    for j,s in enumerate(syst) :
      if j != 0 : 
        f.write(',')
      f.write(s[0])
    f.write('\n')
    
    f.write('lnN'+ str(counter) + '_for = ')
    for j,s in enumerate(syst) :
      if j != 0 : 
        f.write(',')
      if s[1] == SigName :
        f.write(options.AnaName + '_signal')
      else :
        f.write(options.AnaName + '_background_' + s[1])  
    f.write('\n')
    
    counter+=1
else:
  f.write('NlnN = 0\n')


# [AnaName] Section
f.write('\n')
f.write('['+options.AnaName+']\n')

# ... Find Signal and Backgrounds
f.write('Nbkg='+str(Nbkg)+'\n')

# ... Signal Name
f.write('sm_name='+SigName+' \n')
# ... Data Name
f.write('data_name=data_obs \n') 

# Signal shape syst
SYST=[]
for iSystType in dc.content['systs']: 
  if 'shape' in iSystType:
    for iSyst in dc.content['systs'][iSystType]:
      if dc.content['systs'][iSystType][iSyst][SigPos] == '1' :
        SYST.append('signal_'+SigName+'_'+iSyst)
if len(SYST)>0 :
  f.write('signal_shape_syst=')  
  for i in range (0,len(SYST)) : 
    f.write(SYST[i])
    if i < len(SYST)-1 : f.write(',')
  f.write('\n')

# Bkgd Shape syst
for iBkg in range(0,len(BkgPos)) : 
  iPos=BkgPos[iBkg]
  SYST=[]
  for iSystType in dc.content['systs']:
    if 'shape' in iSystType:
      for iSyst in dc.content['systs'][iSystType]:
        if dc.content['systs'][iSystType][iSyst][iPos] == '1' :
          SYST.append('background_'+BkgNames[iBkg]+'_'+iSyst)
  f.write('bkg'+str(iBkg+1)+'_name=background_'+BkgNames[iBkg]+'\n')
  if len(SYST)>0 :
    f.write('bkg'+str(iBkg+1)+'_shape_syst=')
    for i in range (0,len(SYST)) :
      f.write(SYST[i])
      if i < len(SYST)-1 : f.write(',')
    f.write('\n')

#

f.write('NSigBkg_corr_unc = 0 \n')

# Start writing histogram file

#if '.txt' in options.OutputFile : rootFile = options.OutputFile.replace('.txt','.root')
#else                            : rootFile = options.OutputFile + '.root'
rootFile = os.path.dirname(options.OutputFile) + '/' + options.AnaName + '.root' # necessary for EFT framework
print 'RootFile  : ',rootFile 

fOut = TFile.Open(rootFile,'RECREATE')

# ... Data
print DataFile, DataHist

fIn = TFile.Open(os.path.dirname(options.InputFile)+'/'+DataFile,'READ')
fOut.cd()
hTmp = fIn.Get(DataHist).Clone("data_obs")
hTmp.Write()
fIn.Close()

# ... Signal
print MCFile  , MCHist , MCHisSyst

fIn = TFile.Open(os.path.dirname(options.InputFile)+'/'+MCFile,'READ')
fOut.cd()
hTmp = fIn.Get(MCHist.replace('$PROCESS',SigName)).Clone(SigName)
hTmp.Write()
for iSystType in dc.content['systs']:
  if 'shape' in iSystType:
    for iSyst in dc.content['systs'][iSystType]:
      if dc.content['systs'][iSystType][iSyst][SigPos] == '1' :
        hTmp = fIn.Get(MCHisSyst.replace('$PROCESS',SigName).replace('$SYSTEMATIC',iSyst)+'Up').Clone('signal_'+SigName+'_'+iSyst+'Up')
        hTmp.Write()       
        hTmp = fIn.Get(MCHisSyst.replace('$PROCESS',SigName).replace('$SYSTEMATIC',iSyst)+'Down').Clone('signal_'+SigName+'_'+iSyst+'Down')
        hTmp.Write()       
fIn.Close()

# ... Background

fIn = TFile.Open(os.path.dirname(options.InputFile)+'/'+MCFile,'READ')
fOut.cd()
for iBkg in range(0,len(BkgPos)) :
  iPos=BkgPos[iBkg]
  hTmp = fIn.Get(MCHist.replace('$PROCESS',BkgNames[iBkg])).Clone('background_'+BkgNames[iBkg])
  hTmp.Write()
  for iSystType in dc.content['systs']:
    if 'shape' in iSystType:
      for iSyst in dc.content['systs'][iSystType]:
        if dc.content['systs'][iSystType][iSyst][iPos] == '1' :
          hTmp = fIn.Get(MCHisSyst.replace('$PROCESS',BkgNames[iBkg]).replace('$SYSTEMATIC',iSyst)+'Up').Clone('background_'+BkgNames[iBkg]+'_'+iSyst+'Up')
          hTmp.Write() 
          hTmp = fIn.Get(MCHisSyst.replace('$PROCESS',BkgNames[iBkg]).replace('$SYSTEMATIC',iSyst)+'Down').Clone('background_'+BkgNames[iBkg]+'_'+iSyst+'Down')
          hTmp.Write() 
fIn.Close()

# close Files
f.close()
fOut.Close()
