/* This printer is made to work much faster than the normal printer,
   only printing summary canvases and a select few others */


#define DEBUG 1
#include <iomanip> 
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <cstdint>
#include <TFile.h>
#include <TKey.h>
#include <TDirectory.h>
#include <TNtuple.h>
#include <TH2.h>
#include <TProfile.h>
#include <TCanvas.h>
#include <TFrame.h>
#include <TROOT.h>
#include <TSystem.h>
#include <TRandom3.h>
#include <TBenchmark.h>
#include <TInterpreter.h>
#include <TApplication.h>
#include <TString.h>
#include <Event.h>
#include <TObject.h>
#include <TClonesArray.h>
#include <TRefArray.h>
#include <TRef.h>
#include <TH1.h>
#include <TBits.h>
#include <TMath.h>
#include <TFile.h>
#include <TClassTable.h>
#include <TTree.h>
#include <TBranch.h>
#include <time.h>
#include <math.h>

#include "plotter.cxx"
#include "printer.cxx"
//#include "dqmCanvases.cxx"
//#include "GEB_summaryCanvases.cxx"

using namespace std;

int main(int argc, char** argv)
{
  cout << "--==Online Printer Main==--" << endl;
  gErrorIgnoreLevel = kError;
  if (argc!=2) 
    {
      cout << "Please provide ONE root file." << endl;
      cout << "Usage: <path>/printer input.root" << endl;
      return 0;
    }
  
  TString ifilename = argv[1];
  string ifilestring = argv[1];
  TFile *ifile;

  if(ifilestring.substr(ifilestring.length()-5,5)!=".root")
    {		
      cout << "Input File must be in .root format." << endl;
      cout << endl;
      return 0;
    }

  ifile = new TFile(ifilename, "READ");
  if(DEBUG) std::cout<<"[gtprinter]" << "ifile: " << ifile->GetName() << std::endl;

  TString iname; //name of file without .analyzed.root extension

  TString name = ifilename;
  int loc = name.Last('/')+1;
  int len = name.Length()-loc-14;
  TSubString subname = name(loc,len);
  iname = subname;
  if(DEBUG) std::cout<<"[gtprinter]"<< "iname: " << iname << std::endl;

  TString dPath = "/tmp/"+iname+"/"; 
  if(DEBUG) std::cout<<"[gtprinter]" << "dPath: " << dPath << std::endl;

  gROOT->ProcessLine(".!mkdir -p "+dPath);  

  gemTreePrintOnline(gDirectory->GetDirectory(""),dPath,iname,ifilestring);
  
  if (DEBUG) std::cout<<"Online Printer Complete. "<< std::endl;
  return 0;
}
