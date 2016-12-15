#ifndef gemTreeReader_h
#define gemTreeReader_h

#ifndef DEBUG
#define DEBUG 1
#endif
#include <TChain.h>
#include <TProofOutputFile.h>
#include <TSelector.h>

#include <iomanip> 
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <TSystem.h>

#include "AMC13_histogram.cxx"

class gemTreeReader: public TSelector {
public :
  TTree          *fChain;   //!pointer to the analyzed TTree or TChain
  
  // Declaration of leaf types
  Event           *GEMEvents;
  // List of branches
  TBranch        *b_GEMEvents;   //!
  gemTreeReader(TTree * /*tree*/ =0) : fChain(0) { }
  virtual ~gemTreeReader() { }
  virtual Int_t   Version() const { return 2; }
  virtual void    Begin(TTree *tree);
  virtual void    SlaveBegin(TTree *tree);
  virtual void    Init(TTree *tree);
  virtual Bool_t  Notify();
  virtual Bool_t  Process(Long64_t entry);
  virtual Int_t   GetEntry(Long64_t entry, Int_t getall = 0) { return fChain ? fChain->GetTree()->GetEntry(entry, getall) : 0; }
  virtual void    SetOption(const char *option) { fOption = option; }
  virtual void    SetObject(TObject *obj) { fObject = obj; }
  virtual void    SetInputList(TList *input) { fInput = input; }
  virtual TList  *GetOutputList() const { return fOutput; }
  virtual void    SlaveTerminate();
  virtual void    Terminate();

  int slotFromMap(int a, int g, int cid);

  vector<AMC13Event> v_amc13;    ///<Vector of AMC13Event
  vector<AMCdata> v_amc;         ///<Vector of AMCdata
  vector<GEBdata> v_geb;         ///<Vector of GEBdata
  vector<VFATdata> v_vfat;       ///Vector of VFATdata

  AMC_histogram * v_amcH;        ///<Vector of AMC_histogram
  GEB_histogram * v_gebH;        ///<Vector of GEB_histogram
  VFAT_histogram * v_vfatH;      ///<Vector of VFAT_histogram

  int VFATMap[12][2][24];

  AMC13_histogram * m_amc13H;
  AMC_histogram * m_amcH;
  GEB_histogram * m_gebH;
  VFAT_histogram * m_vfatH;

  int m_RunType;
  int m_deltaV;
  int m_Latency;
  TDirectory* onlineHistsDir;

  TFile *fFile;
  TProofOutputFile *fProofFile;

  ClassDef(gemTreeReader,2);
};

#endif
