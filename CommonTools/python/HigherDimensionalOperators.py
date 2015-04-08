from CombineEFT.CommonTools.AnomalousCouplingModel import *
import ROOT as r
import os

basepath = '/uscms_data/d3/rclsa/ATGC_limits/CMSSW_6_1_1/src/InputPreparation/'

#this model is in the equal couplings scenario of HISZ or something similar
#it does the old style limits of setting the other parameter to zero
class Operators2D(AnomalousCouplingModel):
    def __init__(self, op1, min1, max1, op2, min2, max2, process, channel):
        
        AnomalousCouplingModel.__init__(self)
        self.processes = [process]
        self.channels  = [channel]
        self.pois      =  [op1, op2]
        self.anomCoupSearchWindows = {op1:[min1,max1],
                                      op2:[min2,max2]}

        
        self.verbose = False

    def buildScaling(self,process,channel):        
        scalerName = '%s_%s'%(process,channel)
        
              
        f = r.TFile('%s/%s_%s.root'% (basepath, process, channel),'READ')
        SM_diboson_shape = f.Get('diboson').Clone('shape_for_scale')
        SM_diboson_shape.SetDirectory(0)
        f.Close()

        self.modelBuilder.out._import(SM_diboson_shape)
        SM_diboson_shape_dhist = r.RooDataHist('DHIST_shape_for_scale',
                                               'DHIST_shape_for_scale',
                                               r.RooArgList(self.modelBuilder.out.var('theObservable')),
                                               self.modelBuilder.out.obj('shape_for_scale'))
        self.modelBuilder.out._import(SM_diboson_shape_dhist)        
        self.modelBuilder.factory_('RooHistFunc::Scaling_base_pdf_%s({theObservable},DHIST_shape_for_scale)'%(scalerName))              
        self.modelBuilder.factory_('Roo2DProcessScaling::Scaling_%s(theObservable,operator1,operator2,Scaling_base_pdf_%s,"%s")'%(scalerName,scalerName,filename))

        return scalerName

class Operators1D(AnomalousCouplingModel):
    def __init__(self, op1, min1, max1, process, channel):
        
        AnomalousCouplingModel.__init__(self)
        self.processes = [process]
        self.channels  = [channel]
        self.pois      =  [op1]
        self.anomCoupSearchWindows = {op1:[min1,max1]}
        
        self.verbose = False

    def buildScaling(self,process,channel):        
        scalerName = '%s_%s'%(process,channel)
        
              
        f = r.TFile('%s/%s_%s.root'% (basepath, process, channel),'READ')
        SM_diboson_shape = f.Get('diboson').Clone('shape_for_scale')
        SM_diboson_shape.SetDirectory(0)
        f.Close()

        self.modelBuilder.out._import(SM_diboson_shape)
        SM_diboson_shape_dhist = r.RooDataHist('DHIST_shape_for_scale',
                                               'DHIST_shape_for_scale',
                                               r.RooArgList(self.modelBuilder.out.var('theObservable')),
                                               self.modelBuilder.out.obj('shape_for_scale'))
        self.modelBuilder.out._import(SM_diboson_shape_dhist)        
        self.modelBuilder.factory_('RooHistFunc::Scaling_base_pdf_%s({theObservable},DHIST_shape_for_scale)'%(scalerName))              
        self.modelBuilder.factory_('Roo1DProcessScaling::Scaling_%s(theObservable,operator1,operator2,Scaling_base_pdf_%s,"%s")'%(scalerName,scalerName,filename))

        return scalerName


CWWWCW = Operators2D('cwww', '-50.', '50.', 'cw', '-50.', '50.', 'signal_WW2L2Nu0jets', 'WW2L2Nu0jets')
CWWWCB = Operators2D('cwww', '-50.', '50.', 'cb', '-150.', '150.', 'signal_WW2L2Nu0jets', 'WW2L2Nu0jets')
CWCB   = Operators2D('cw', '-50.', '50.', 'cb', '-150.', '150.', 'signal_WW2L2Nu0jets', 'WW2L2Nu0jets')

CWWW = Operators1D('cwww', '-50.', '50.', 'signal_WW2L2Nu0jets', 'WW2L2Nu0jets')
CW   = Operators1D('cw', '-50.', '50.', 'signal_WW2L2Nu0jets', 'WW2L2Nu0jets')
CB   = Operators1D('cb', '-150.', '150.', 'signal_WW2L2Nu0jets', 'WW2L2Nu0jets')

LT0 = Operators1D('LT0', '-0.4', '0.4', 'signal_WWVBS_mll', 'WWVBS_mll')
LT1 = Operators1D('LT1', '-0.15', '0.2', 'signal_WWVBS_mll', 'WWVBS_mll')
LT2 = Operators1D('LT2', '-0.4', '0.6', 'signal_WWVBS_mll', 'WWVBS_mll')
LT3 = Operators1D('LT3', '-10.', '10.', 'signal_WWVBS_mll', 'WWVBS_mll')
LT4 = Operators1D('LT4', '-10.', '10.', 'signal_WWVBS_mll', 'WWVBS_mll')
LT5 = Operators1D('LT5', '-10.', '10.', 'signal_WWVBS_mll', 'WWVBS_mll')
LT6 = Operators1D('LT6', '-10.', '10.', 'signal_WWVBS_mll', 'WWVBS_mll')
LT7 = Operators1D('LT7', '-10.', '10.', 'signal_WWVBS_mll', 'WWVBS_mll')
LT8 = Operators1D('LT8', '-10.', '10.', 'signal_WWVBS_mll', 'WWVBS_mll')
LT9 = Operators1D('LT9', '-10.', '10.', 'signal_WWVBS_mll', 'WWVBS_mll')

LS0 = Operators1D('LS0', '-6.', '6.', 'signal_WWVBS_mll', 'WWVBS_mll')
LS1 = Operators1D('LS1', '-18.', '18.', 'signal_WWVBS_mll', 'WWVBS_mll')
LS0_LS1 = Operators2D('LS0', '-35.', '35.', 'LS1', '-100.', '100.', 'signal_WWVBS_mll', 'WWVBS_mll') 

LM0 = Operators1D('LM0', '-4.', '4.', 'signal_WWVBS_mll', 'WWVBS_mll')
LM1 = Operators1D('LM1', '-5.', '5.', 'signal_WWVBS_mll', 'WWVBS_mll')
LM2 = Operators1D('LM2', '-100.', '100.', 'signal_WWVBS_mll', 'WWVBS_mll')
LM3 = Operators1D('LM3', '-100.', '100.', 'signal_WWVBS_mll', 'WWVBS_mll')
LM4 = Operators1D('LM4', '-100.', '100.', 'signal_WWVBS_mll', 'WWVBS_mll')
LM5 = Operators1D('LM5', '-100.', '100.', 'signal_WWVBS_mll', 'WWVBS_mll')
LM6 = Operators1D('LM6', '-8.', '8.', 'signal_WWVBS_mll', 'WWVBS_mll')
LM7 = Operators1D('LM7', '-8.', '8.', 'signal_WWVBS_mll', 'WWVBS_mll')


