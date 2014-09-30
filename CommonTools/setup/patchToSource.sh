#!/bin/sh

mv CombineEFT/CommonTools/interface/Roo1DProcessScaling.h HiggsAnalysis/CombinedLimit/interface/Roo1DProcessScaling.h
mv CombineEFT/CommonTools/interface/Roo1DSemiAnalyticPdf.h HiggsAnalysis/CombinedLimit/interface/Roo1DSemiAnalyticPdf.h
mv CombineEFT/CommonTools/interface/Roo2DProcessScaling.h HiggsAnalysis/CombinedLimit/interface/Roo2DProcessScaling.h
mv CombineEFT/CommonTools/interface/Roo2DSemiAnalyticPdf.h HiggsAnalysis/CombinedLimit/interface/Roo2DSemiAnalyticPdf.h

mv CombineEFT/CommonTools/src/Roo1DProcessScaling.cc HiggsAnalysis/CombinedLimit/src/Roo1DProcessScaling.cc
mv CombineEFT/CommonTools/src/Roo1DSemiAnalyticPdf.cc HiggsAnalysis/CombinedLimit/src/Roo1DSemiAnalyticPdf.cc
mv CombineEFT/CommonTools/src/Roo2DProcessScaling.cc HiggsAnalysis/CombinedLimit/src/Roo2DProcessScaling.cc
mv CombineEFT/CommonTools/src/Roo2DSemiAnalyticPdf.cc HiggsAnalysis/CombinedLimit/src/Roo2DSemiAnalyticPdf.cc

echo '#include "../interface/Roo1DProcessScaling.h"' | cat - HiggsAnalysis/CombinedLimit/src/CombinedLimit_LinkDef.h > dummy && mv dummy HiggsAnalysis/CombinedLimit/src/CombinedLimit_LinkDef.h
echo '#include "../interface/Roo1DSemiAnalyticPdf.h"' | cat - HiggsAnalysis/CombinedLimit/src/CombinedLimit_LinkDef.h > dummy && mv dummy HiggsAnalysis/CombinedLimit/src/CombinedLimit_LinkDef.h
echo '#include "../interface/Roo2DProcessScaling.h"' | cat - HiggsAnalysis/CombinedLimit/src/CombinedLimit_LinkDef.h > dummy && mv dummy HiggsAnalysis/CombinedLimit/src/CombinedLimit_LinkDef.h
echo '#include "../interface/Roo2DSemiAnalyticPdf.h"' | cat - HiggsAnalysis/CombinedLimit/src/CombinedLimit_LinkDef.h > dummy && mv dummy HiggsAnalysis/CombinedLimit/src/CombinedLimit_LinkDef.h

sed 's/#endif/#pragma link C++ class Roo1DProcessScaling+;\n#endif/' < HiggsAnalysis/CombinedLimit/src/CombinedLimit_LinkDef.h > dummy && mv dummy HiggsAnalysis/CombinedLimit/src/CombinedLimit_LinkDef.h
sed 's/#endif/#pragma link C++ class Roo1DSemiAnalyticPdf+;\n#endif/' < HiggsAnalysis/CombinedLimit/src/CombinedLimit_LinkDef.h > dummy && mv dummy HiggsAnalysis/CombinedLimit/src/CombinedLimit_LinkDef.h
sed 's/#endif/#pragma link C++ class Roo2DProcessScaling+;\n#endif/' < HiggsAnalysis/CombinedLimit/src/CombinedLimit_LinkDef.h > dummy && mv dummy HiggsAnalysis/CombinedLimit/src/CombinedLimit_LinkDef.h
sed 's/#endif/#pragma link C++ class Roo2DSemiAnalyticPdf+;\n#endif/' < HiggsAnalysis/CombinedLimit/src/CombinedLimit_LinkDef.h > dummy && mv dummy HiggsAnalysis/CombinedLimit/src/CombinedLimit_LinkDef.h
