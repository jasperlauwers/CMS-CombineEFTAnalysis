import pyroot_logon
import os
import sys

from array import *

from ROOT import *
from optparse import OptionParser
from ConfigParser import SafeConfigParser

parser = OptionParser(description="%prog : A RooStats Implementation of Anomalous Triple Gauge Coupling Analysis.",
                      usage="buildWZworkspace --config=example_config.cfg")
cfgparse = SafeConfigParser()

parser.add_option("--config",dest="config",help="The name of the input configuration file.")
(options,args) = parser.parse_args()

miss_options = False

if options.config is None:
    print 'Need to specify --config'
    miss_options=True
    
if miss_options:
    exit(1)
        
cfgparse.read(options.config)
options.config = cfgparse # put the parsed config file into our options

cfg = options.config

norm_sig_sm = -1
norm_sig_sm_up = -1
norm_sig_sm_down = -1
norm_bkg = -1
norm_obs = -1

fit_sections = cfg.sections()

# read in lnN nuisances from 'Global':

NlnN = int(cfg.get('Global','NlnN'))       

lnN_name = []
for i in range(1,NlnN+1):
    lnN_name.append(cfg.get('Global','lnN%i_name'%i))
lnN_value = []
for i in range(0,NlnN):
    lnN_value.append([])
    for value in cfg.get('Global','lnN%i_value'%(i+1)).split(','):
        lnN_value[i].append(value)

lnN_for = []
for i in range(0,NlnN):
    lnN_for.append([])
    for name in cfg.get('Global','lnN%i_for'%(i+1)).split(','):
        lnN_for[i].append(name)

print '\n\t\t=============================================> lnN: ',lnN_name
print 'lnN value: ',lnN_value
print 'lnN for: ',lnN_for

dimension = int(cfg.get('Global', 'dimension'))

par1name = None
par1low = None
par1high = None

par2name = None
par2low = None
par2high = None

if dimension == 1:
    par1name = cfg.get('Global', 'par1Name')
    par1low  = float(cfg.get('Global', 'par1Low'))
    par1high = float(cfg.get('Global', 'par1High'))

elif dimension == 2:

    par1name = cfg.get('Global', 'par1Name')
    par1low  = float(cfg.get('Global', 'par1Low'))
    par1high = float(cfg.get('Global', 'par1High'))

    par2name = cfg.get('Global', 'par2Name')
    par2low  = float(cfg.get('Global', 'par2Low'))
    par2high = float(cfg.get('Global', 'par2High'))

else:
    print 'Only dimensions 1 and 2 implemented... this thing will crash.'

basepath = cfg.get('Global','basepath')     
        
fit_sections.remove('Global') #don't need to iterate over the global configuration

for section in fit_sections:

    codename = section
    f = TFile('%s/%s.root'%(basepath,codename))

    Nbkg = cfg.get(codename,'Nbkg')
    print "Nbkg= ",Nbkg
    Nbkg_int=int(Nbkg)

    bkg_name = []
    for i in range(1,Nbkg_int+1):
        bkg_name.append(cfg.get(codename,'bkg%i_name'%i))

    background = []

    for i in range(0,Nbkg_int):
        background.append(f.Get(bkg_name[i]))

    print 'backgrounds= ',background
    background_shapeSyst = []
    for i in range(0,Nbkg_int):
        background_shapeSyst.append([])
        cfg_items=cfg.items(codename)
        for cfg_item in cfg_items:
            if ('bkg%i_shape_syst'%(i+1) in cfg_item):
                for name in cfg.get(codename,'bkg%i_shape_syst'%(i+1)).split(','):
                    background_shapeSyst[i].append(name)

    background_backshapeUp = []
    background_backshapeDown = []

    for j in range(0,Nbkg_int):
        background_backshapeUp.append([])
        background_backshapeDown.append([])
        for i in range(0,len(background_shapeSyst[j])):
            background_backshapeUp[j].append(f.Get('%sUp'%background_shapeSyst[j][i]))
            background_backshapeDown[j].append(f.Get('%sDown'%background_shapeSyst[j][i]))

    # data and signal shapes
    sm_histname = cfg.get(codename, 'sm_name')
    diboson = f.Get(sm_histname)
    data_histname = cfg.get(codename, 'data_name')
    data_obs = f.Get(data_histname)

    # signal uncertainties
    doSignalShape_unc=False
    doSignalBkg_corr_unc=False
    cfg_items=cfg.items(codename)
    for cfg_item in cfg_items:
        if 'signal_shape_syst' in cfg_item:
            doSignalShape_unc = True
        if 'nsigbkg_corr_unc' in cfg_item:
            doSignalBkg_corr_unc = True

    if (doSignalShape_unc):
        diboson_up = {}
        diboson_down = {}
        norm_sig_sm_up = {}
        norm_sig_sm_down = {}
        signal_shapeSyst = [string(i) for i in cfg.get(codename,'signal_shape_syst').split(',')]
        for i in range(0,len(signal_shapeSyst)):
            print signal_shapeSyst[i]
            diboson_up[i] = f.Get('%sUp'%signal_shapeSyst[i])
            diboson_down[i] = f.Get('%sDown'%signal_shapeSyst[i])
            norm_sig_sm_up[i] = diboson_up[i].Integral()
            norm_sig_sm_down[i] = diboson_down[i].Integral()

    if (doSignalBkg_corr_unc):
        NSigBkg_corr_unc = cfg.get(codename,'NSigBkg_corr_unc')
        NSigBkg_corr_unc_int=int(NSigBkg_corr_unc)

        SignalBkg_corr_name_ws = []
        for i in range(1,NSigBkg_corr_unc_int+1):
            SignalBkg_corr_name_ws.append(cfg.get(codename,'correlated_SigBkg_unc%i_name'%i))


        SignalBkg_corr_name = {}
        for i in range(1,NSigBkg_corr_unc_int+1):
            SignalBkg_corr_name[i-1]=[string(j) for j in cfg.get(codename,'correlated_SigBkg_unc%s'%i).split(',')]

    # check if shape uncertainty name is one of those where Signal and bkg uncertainties are correlated
    def isItCorrelated(name):
        isItCorr=False
        for i in range(0,NSigBkg_corr_unc_int):
            if (name in SignalBkg_corr_name[i]):
                isItCorr=True
        return isItCorr

    def isItCorrelated_name(name):
        name_out=name
        for i in range(0,NSigBkg_corr_unc_int):
            if (name in SignalBkg_corr_name[i]):
                name_out=SignalBkg_corr_name_ws[i]
        return name_out
    
    norm_sig_sm = diboson.Integral()
    norm_bkg = []
    for i in range(0,Nbkg_int):
        norm_bkg.append(background[i].Integral())
    norm_obs = data_obs.Integral()

    # Got everything, now begin to make the RooWorkspace    
    theWS = RooWorkspace('%s' % codename, '%s' % codename)

    # The observable used to measure this thing
    theObservable = theWS.factory('%s[%f,%f]' % (codename,data_obs.GetXaxis().GetBinLowEdge(1), data_obs.GetXaxis().GetBinUpEdge(data_obs.GetNbinsX())))
    vars = RooArgList(theObservable)
    varSet = RooArgSet(theObservable)

    # The binning used in the RooWorkspace will be copied from the data histograms.
    # If you are smart, you used the same for all histograms, ok?
    # How come there is no method to initialize a RooBinning from a TAxis? Good grief...
    binning=array('d',[])
    for i in range(1, data_obs.GetNbinsX()+1):
        binning.append(data_obs.GetBinLowEdge(i))
    binning.append(data_obs.GetBinLowEdge(data_obs.GetNbinsX()+1))
    bins=RooBinning(len(binning)-1, binning)
    theObservable.setBinning(bins)

    # These are the higher dimensional operators that we are probing
    if dimension == 1:
        operator1 = theWS.factory('%s[0., %f, %f]' % (par1name, par1low, par1high))
    elif dimension == 2:
        operator1 = theWS.factory('%s[0., %f, %f]' % (par1name, par1low, par1high))
        operator2 = theWS.factory('%s[0., %f, %f]' % (par2name, par2low, par2high))
    else:
        print 'Are you still here?'

    # Some exercise to define mathematically what these things are
    #  . The data histogram is really a dataset (RooDataHist)
    #  . The diboson histogram is really a function (RooHistFunc) that will multiply some quadratic function
    #    to make a PDF (these quadratic functions are TF2 in a signal_{codename}.root file.
    #      See twiki https://twiki.cern.ch/twiki/bin/viewauth/CMS/ATGCRooStats for more instructions.
    dibosonHist = RooDataHist('SM_%s_rawshape'%codename,
                              'SM_%s_rawshape'%codename,
                              vars,
                              diboson)
    dibosonFunc = RooHistFunc('SM_%s_shape'%codename,
                              'SM_%s_shape'%codename,
                              varSet,
                              dibosonHist)
    data = RooDataHist('data_obs', 'data_obs_%s'%codename, vars, data_obs)


    print 'data integral: ',data.Print()
    bkgHist = {}
    for i in range(0,Nbkg_int):
        bkgHist[i] = RooDataHist('bkg%i_%s'%(i+1,codename),
                                 'bkg%i_%s'%(i+1,codename),
                                 vars,
                                 background[i])
        
    bkgHist_systUp = []
    bkgHist_systDown = []

    for j in range(0,Nbkg_int):
        bkgHist_systUp.append([])
        bkgHist_systDown.append([])
        for i in range(0,len(background_shapeSyst[j])):
            name_forCorr=background_shapeSyst[j][i]
            if (isItCorrelated(background_shapeSyst[j][i])):
                name_forCorr=isItCorrelated_name(background_shapeSyst[j][i])

            bkgHist_systUp[j].append(RooDataHist('bkg%i_%s_%sUp'%(j+1,codename,name_forCorr),
                                                 'bkg%i_%s_%sUp'%(j+1,codename,name_forCorr),
                                                 vars,
                                                 background_backshapeUp[j][i]))
            bkgHist_systDown[j].append(RooDataHist('bkg%i_%s_%sDown'%(j+1,codename,name_forCorr),
                                                   'bkg%i_%s_%sDown'%(j+1,codename,name_forCorr),
                                                   vars,
                                                   background_backshapeDown[j][i]))
       

    if (doSignalShape_unc):
        dibosonHist_up = {}
        dibosonHist_down = {}                       
        dibosonFunc_up = {}
        dibosonFunc_down = {}
        kappaLow = {}
        kappaHigh = {}
        dibosonPdf_norm = {}
        theta = {}
        kappaLow_sum_d = 1.
        kappaHigh_sum_d = 1.
        
        for i in range(0,len(signal_shapeSyst)):
            
            name_forCorr=signal_shapeSyst[i]
            if (isItCorrelated(signal_shapeSyst[i])):
                name_forCorr=isItCorrelated_name(signal_shapeSyst[i])

            #Get the shape variation
            dibosonHist_up[i] = RooDataHist('SM_%s_rawshape_%sUp'%(codename,name_forCorr),
                                            'SM_%s_rawshape_%sUp'%(codename,name_forCorr),
                                            vars,
                                            diboson_up[i])
            dibosonFunc_up[i] = RooHistFunc('SM_%s_shape_%sUp'%(codename,name_forCorr),
                                            'SM_%s_shape_%sUp'%(codename,name_forCorr),
                                            varSet,
                                            dibosonHist_up[i]) 
            dibosonHist_down[i] = RooDataHist('SM_%s_rawshape_%sDown'%(codename,name_forCorr),
                                              'SM_%s_rawshape_%sDown'%(codename,name_forCorr),
                                              vars,
                                              diboson_down[i])
            dibosonFunc_down[i] = RooHistFunc('SM_%s_shape_%sDown'%(codename,name_forCorr),
                                              'SM_%s_shape_%sDown'%(codename,name_forCorr),
                                              varSet,
                                              dibosonHist_down[i])

            # Nuisance parameters for the shape variation
            kappaLow[i] = RooRealVar("kappaL_%s_%s"%(i+1,codename),"kappaL_%s_%s"%(i+1,codename),norm_sig_sm_down[i]/norm_sig_sm)
            kappaLow[i].setConstant(True)
            kappaHigh[i] = RooRealVar("kappaH_%s_%s"%(i+1,codename),"kappaH_%s_%s"%(i+1,codename),norm_sig_sm_up[i]/norm_sig_sm)
            kappaHigh[i].setConstant(True)
            kappaLow_sum_d = kappaLow_sum_d*norm_sig_sm_down[i]/norm_sig_sm
            kappaHigh_sum_d = kappaHigh_sum_d*norm_sig_sm_up[i]/norm_sig_sm

            name_forCorr=signal_shapeSyst[i]
            name_forCorr=isItCorrelated_name(signal_shapeSyst[i])
            
            theWS.factory("%s[-7,7]"%name_forCorr)
            theta[i] = theWS.var("%s"%name_forCorr)
            
            dibosonPdf_norm[i] = AsymPow('dibosonPdf_%s_integral%s'%(codename,i+1),
                                         'dibosonPdf_%s_integral%s'%(codename,i+1),
                                         kappaLow[i],
                                         kappaHigh[i],
                                         theta[i])

        if (len(signal_shapeSyst)==1):
            dibosonPdf_norm_sum = dibosonPdf_norm[0]
        else:
            for i in range(0,len(signal_shapeSyst)):
                if (i==0): prodset=RooArgList(dibosonPdf_norm[i])
                else: prodset.add(RooArgList(dibosonPdf_norm[i]))
            print prodset
            dibosonPdf_norm_sum = RooProduct("dibosonPdf_norm_sum","dibosonPdf_norm_sum",prodset)

        kappaLow_sum = RooRealVar("kappaLow_sum","kappaLow_sum",kappaLow_sum_d)
        kappaHigh_sum = RooRealVar("kappaHigh_sum","kappaHigh_sum",kappaHigh_sum_d)

        dibosonPdf_norm_sum.SetNameTitle('dibosonPdf_%s_norm'%codename,
                                         'dibosonPdf_%s_norm'%codename)
                

    print 'buildWorkspace SemiAnalyticPdf...'
    #    print '%s' % codename, '%s'  %codename, theObservable,  operator1,  operator2,  dibosonFunc,  '%s/signal_%s.root' % (basepath,codename)
    
    # This is the PDF built from the functions and the diboson distribution
    if dimension == 1:
        dibosonPdf = Roo1DSemiAnalyticPdf('pdf_signal_%s' % codename,
                                          'pdf_signal_%s'  %codename,
                                          theObservable,
                                          operator1,
                                          dibosonFunc,
                                          '%s/signal_%s_%s_%s.root' % (basepath,codename,par1name)
                                          )
    elif dimension == 2:
        dibosonPdf = Roo2DSemiAnalyticPdf('pdf_signal_%s' % codename,
                                          'pdf_signal_%s'  %codename,
                                          theObservable,
                                          operator1,
                                          operator2,                                 
                                          dibosonFunc,
                                          '%s/signal_%s_%s_%s.root' % (basepath,codename,par1name,par2name)
                                          )
    else:
        print 'Only dimensions 1 and 2 implemented'
        
    if (doSignalShape_unc):
        dibosonPdf_up = {}
        dibosonPdf_down = {}
        for i in range(0,len(signal_shapeSyst)):

            name_forCorr=signal_shapeSyst[i]
            name_forCorr=isItCorrelated_name(signal_shapeSyst[i])

            if dimension == 1:                
                dibosonPdf_up[i] = Roo1DSemiAnalyticPdf('pdf_signal_%s_%sUp'%(codename,name_forCorr),
                                                        '%s'%codename,
                                                        theObservable,
                                                        operator1,
                                                        dibosonFunc_up[i],
                                                        '%s/signal_%s_%s_%s.root'%(basepath,codename,par1name)
                                                        )
                dibosonPdf_down[i] = Roo1DSemiAnalyticPdf('pdf_signal_%s_%sDown'%(codename,name_forCorr),
                                                          '%s'%codename,
                                                          theObservable,
                                                          operator1,
                                                          dibosonFunc_down[i],
                                                          '%s/signal_%s_%s_%s.root'%(basepath,codename,par1name)
                                                          )
            elif dimension == 2:
                dibosonPdf_up[i] = Roo2DSemiAnalyticPdf('pdf_signal_%s_%sUp'%(codename,name_forCorr),
                                                        '%s'%codename,
                                                        theObservable,
                                                        operator1,
                                                        operator2,                                 
                                                        dibosonFunc_up[i],
                                                        '%s/signal_%s_%s_%s.root'%(basepath,codename,par1name,par2name)
                                                        )
                dibosonPdf_down[i] = Roo2DSemiAnalyticPdf('pdf_signal_%s_%sDown'%(codename,name_forCorr),
                                                          '%s'%codename,
                                                          theObservable,
                                                          operator1,
                                                          operator2,                                 
                                                          dibosonFunc_down[i],
                                                          '%s/signal_%s_%s_%s.root'%(basepath,codename,par1name,par2name)
                                                          )
                
            else:
                print 'Only dimensions 1 and 2 implemented'
                
    print 'importing data...'
    getattr(theWS, 'import')(data)

    print 'importing bkg...'
    for i in range(0,Nbkg_int):
        getattr(theWS, 'import')(bkgHist[i])

    print 'importing bkg unc...'
    for j in range(0,Nbkg_int):
        for i in range(0,len(background_shapeSyst[j])):
            getattr(theWS, 'import')(bkgHist_systUp[j][i])
            getattr(theWS, 'import')(bkgHist_systDown[j][i])

    print 'importing dibosonPdf...'
    getattr(theWS, 'import')(dibosonPdf)

    print '  ...imported'
    if (doSignalShape_unc):
        for i in range(0,len(signal_shapeSyst)):
            getattr(theWS, 'import')(dibosonPdf_up[i])
            getattr(theWS, 'import')(dibosonPdf_down[i])
        getattr(theWS, 'import')(dibosonPdf_norm_sum)
    
    theWS.Print()

    fout = None
    if dimension == 1:
        fout = TFile('%s_%s_ws.root'%(codename,par1name), 'recreate')
    elif dimension == 2:
        fout = TFile('%s_%s_%s_ws.root'%(codename,par1name,par2name), 'recreate')
    else:
        print 'You must be kidding me'
    theWS.Write()
    fout.Close()

### make the card for this channel and plane ID
    card = """
# Simple counting experiment, with one signal and a few background processes 
imax 1  number of channels
jmax {Nbkg_int}  number of backgrounds
kmax *  number of nuisance parameters (sources of systematical uncertainties)
------------""".format(codename=codename,norm_sig_sm=norm_sig_sm,norm_bkg=norm_bkg,norm_obs=norm_obs,Nbkg_int=Nbkg_int)
    if dimension == 1:
        for i in range(0,Nbkg_int):
            card += """
            shapes bkg{Nbkg_int}_{codename}\t {codename} ./{codename}_{par1name}_{par2name}_ws.root {codename}:$PROCESS {codename}:$PROCESS_$SYSTEMATIC""".format(Nbkg_int=i+1,codename=codename,norm_sig_sm=norm_sig_sm,norm_bkg=norm_bkg,norm_obs=norm_obs,par1name=par1name,par2name=par2name)
        card += """
        shapes data_obs\t\t\t {codename} ./{codename}_{par1name}_{par2name}_ws.root {codename}:$PROCESS """.format(codename=codename,norm_sig_sm=norm_sig_sm,norm_bkg=norm_bkg,norm_obs=norm_obs,Nbkg_int=Nbkg_int,par1name=par1name,par2name=par2name)
        if (doSignalShape_unc):
            card += """   
            shapes signal_{codename}\t {codename} ./{codename}_{par1name}_{par2name}_ws.root {codename}:pdf_$PROCESS {codename}:pdf_$PROCESS_$SYSTEMATIC """.format(codename=codename,norm_sig_sm=norm_sig_sm,norm_bkg=norm_bkg,norm_obs=norm_obs,par1name=par1name,par2name=par2name)
        else:
            card += """   
            shapes signal_{codename}\t {codename} ./{codename}_{par1name}_{par2name}_ws.root {codename}:pdf_$PROCESS
            """.format(codename=codename,norm_sig_sm=norm_sig_sm,norm_bkg=norm_bkg,norm_obs=norm_obs,par1name=par1name,par2name=par2name)
    elif dimension == 2:
        for i in range(0,Nbkg_int):
            card += """
            shapes bkg{Nbkg_int}_{codename}\t {codename} ./{codename}_{par1name}_{par2name}_ws.root {codename}:$PROCESS {codename}:$PROCESS_$SYSTEMATIC""".format(Nbkg_int=i+1,codename=codename,norm_sig_sm=norm_sig_sm,norm_bkg=norm_bkg,norm_obs=norm_obs,par1name=par1name,par2name=par2name)
        card += """
        shapes data_obs\t\t\t {codename} ./{codename}_{par1name}_{par2name}_ws.root {codename}:$PROCESS """.format(codename=codename,norm_sig_sm=norm_sig_sm,norm_bkg=norm_bkg,norm_obs=norm_obs,Nbkg_int=Nbkg_int,par1name=par1name,par2name=par2name)
        if (doSignalShape_unc):
            card += """   
            shapes signal_{codename}\t {codename} ./{codename}_{par1name}_{par2name}_ws.root {codename}:pdf_$PROCESS {codename}:pdf_$PROCESS_$SYSTEMATIC """.format(codename=codename,norm_sig_sm=norm_sig_sm,norm_bkg=norm_bkg,norm_obs=norm_obs,par1name=par1name,par2name=par2name)
        else:
            card += """   
            shapes signal_{codename}\t {codename} ./{codename}_{par1name}_{par2name}_ws.root {codename}:pdf_$PROCESS
            """.format(codename=codename,norm_sig_sm=norm_sig_sm,norm_bkg=norm_bkg,norm_obs=norm_obs,par1name=par1name,par2name=par2name)
    else:
        print 'Are you serious that it did not crash?'

    card += """\n------------\nbin {codename}\nobservation {norm_obs}\n------------""".format(codename=codename,norm_obs=norm_obs)

    card += """\nbin\t\t{codename}""".format(codename=codename,norm_sig_sm=norm_sig_sm,norm_bkg=norm_bkg,norm_obs=norm_obs)
    for i in range(0,Nbkg_int):
        card += """\t{codename}""".format(codename=codename,norm_sig_sm=norm_sig_sm,norm_bkg=norm_bkg[i],norm_obs=norm_obs)

    card += """\nprocess\t\tsignal_{codename}""".format(codename=codename,norm_sig_sm=norm_sig_sm,norm_bkg=norm_bkg[i],norm_obs=norm_obs)
    for i in range(0,Nbkg_int):
        card += """\tbkg{Nbkg_int}_{codename}""".format(Nbkg_int=i+1,codename=codename,norm_sig_sm=norm_sig_sm,norm_bkg=norm_bkg[i],norm_obs=norm_obs)

    card += """\nprocess\t\t0""".format(codename=codename,norm_sig_sm=norm_sig_sm,norm_bkg=norm_bkg[i],norm_obs=norm_obs)

    for i in range(0,Nbkg_int):
        card += """\t{i}""".format(i=i+1,codename=codename,norm_sig_sm=norm_sig_sm,norm_bkg=norm_bkg[i],norm_obs=norm_obs)
        
    card += """\nrate\t\t{norm_sig_sm}""".format(codename=codename,norm_sig_sm=norm_sig_sm,norm_bkg=norm_bkg[i],norm_obs=norm_obs)

    for i in range(0,Nbkg_int):
        card += """\t{norm_bkg}""".format(codename=codename,norm_sig_sm=norm_sig_sm,norm_bkg=norm_bkg[i],norm_obs=norm_obs)

	
    card += """\n------------\n"""
    for i in range(0,NlnN):

# if found signal or codename in the list of names that are affected by lnN unc:
        if (('%s_signal'%codename in lnN_for[i]) or (any(codename in s for s in lnN_for[i]))):
            card+="""\n{lnN_name}\tlnN """.format(lnN_name=lnN_name[i])
            if ('%s_signal'%codename in lnN_for[i]):
                # if lnN syst affects signal:
                index_s=lnN_for[i].index('%s_signal'%codename)
                card+="""\t{lnN_value}""".format(lnN_value=lnN_value[i][index_s])
            else:
                card+="""\t-"""
            for j in range(0,Nbkg_int):
                name_for_lnN=codename
                name_for_lnN+='_'
                name_for_lnN+=bkg_name[j]
                if (name_for_lnN in lnN_for[i]):
                    index=lnN_for[i].index(name_for_lnN)
                    card+="""\t{lnN_value}""".format(lnN_value=lnN_value[i][index])
                else:
                    card+="""\t-"""
        
################ bkg shape syst:

    for j in range(0,Nbkg_int):
        for i in range(0,len(background_shapeSyst[j])):
            # write out only those bkg shape unc that are not correlated with signal shape syst:
            if not(isItCorrelated(background_shapeSyst[j][i])):
                card += """\n{background_shapeSyst}\tshape1""".format(codename=codename,norm_sig_sm=norm_sig_sm,norm_bkg=norm_bkg,norm_obs=norm_obs,i=i,background_shapeSyst=background_shapeSyst[j][i])
                for k in range(0,j+1):
                    card += """\t-""".format(codename=codename,norm_sig_sm=norm_sig_sm,norm_bkg=norm_bkg,norm_obs=norm_obs,i=i,background_shapeSyst=background_shapeSyst[j][i])
                card += """\t1.0""".format(codename=codename,norm_sig_sm=norm_sig_sm,norm_bkg=norm_bkg,norm_obs=norm_obs,i=i,background_shapeSyst=background_shapeSyst[j][i])
                for k in range(1,Nbkg_int-j):
                    card += """\t-""".format(codename=codename,norm_sig_sm=norm_sig_sm,norm_bkg=norm_bkg,norm_obs=norm_obs,i=i,background_shapeSyst=background_shapeSyst[j][i])

    if (doSignalShape_unc):
        for i in range(0,len(signal_shapeSyst)):
            name_forCorr=signal_shapeSyst[i]
            if (isItCorrelated(signal_shapeSyst[i])):
                name_forCorr=isItCorrelated_name(signal_shapeSyst[i])
            card += """\n{signal_shapeSyst}\tshape1\t1.0""".format(signal_shapeSyst=name_forCorr)
    
            for j in range(0,Nbkg_int):
                if (isItCorrelated(signal_shapeSyst[i])):
                    isitcorr=false
                    for k in range(0,len(background_shapeSyst[j])):
                        if (isItCorrelated(background_shapeSyst[j][k])):
                            isitcorr=true
                    if (isitcorr):
                        card += """\t1.0""".format(codename=codename,norm_sig_sm=norm_sig_sm,norm_bkg=norm_bkg,norm_obs=norm_obs)
                    if not(isitcorr):
                        card += """\t-""".format(codename=codename,norm_sig_sm=norm_sig_sm,norm_bkg=norm_bkg,norm_obs=norm_obs)
                else:
                    card += """\t-""".format(codename=codename,norm_sig_sm=norm_sig_sm,norm_bkg=norm_bkg,norm_obs=norm_obs)

    print card

    cardfile = open('%s_%s_%s.txt'%(codename,par1name,par2name),'w')
    cardfile.write(card)
    cardfile.close
