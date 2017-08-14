#include "gemTreeReader.h"
#include <set>
void gemTreeReader::Begin(TTree * /*tree*/)
{
  // The Begin() function is called at the start of the query.
  // When running with PROOF Begin() is only called on the client.
  // The tree argument is deprecated (on PROOF 0 is passed).

  if (DEBUG) std::cout << "MASTER BEGIN"<< std::endl;
  TString option = GetOption();
} 

void gemTreeReader::SlaveBegin(TTree * /*tree*/)
{
  // The SlaveBegin() function is called after the Begin() function.
  // When running with PROOF SlaveBegin() is called on each slave server.
  // The tree argument is deprecated (on PROOF 0 is passed).

  if (DEBUG) std::cout << "SLAVE BEGIN"<< std::endl;
  TString option = GetOption();
  gSystem->Load("libEvent.so");

  // The file for merging
  fProofFile = new TProofOutputFile("SimpleFile.root", "M");
  TNamed *out = (TNamed *) fInput->FindObject("PROOF_OUTPUTFILE");
  if (out) fProofFile->SetOutputFileName(out->GetTitle());
  TDirectory *savedir = gDirectory;
  fFile = fProofFile->OpenFile("RECREATE");
  if (fFile && fFile->IsZombie()) SafeDelete(fFile);
  savedir->cd();

  // Cannot continue
  if (!fFile) {
     TString amsg = TString::Format("ProofSimpleFile::SlaveBegin: could not create '%s':"
                                    " instance is invalid!", fProofFile->GetName());
     Abort(amsg, kAbortProcess);
     return;
  } 
  fFile->cd();

  //VFATMap = {{{0}}};
  TObjString * diramc13 = new TObjString("AMC13-1");
  m_amc13H = new AMC13_histogram("preved", gDirectory->mkdir(diramc13->String()), "1");
  m_amc13H->bookHistograms();
  if (DEBUG) std::cout << "Slave Begin: try to get config "<< std::endl;
  TList *config_s = (TList*)fInput->FindObject("config");
  if (DEBUG) std::cout << "Slave Begin: retrieved config name " << config_s->GetName() << std::endl;
  if (DEBUG) std::cout << "Slave Begin: try to get config iterator"<< std::endl;
  TIter nextamc(config_s);
  TObject *amc;
  std::set<TString> lastNames;
  while ((amc = nextamc())) 
  {
    if (lastNames.find(amc->GetName()) != lastNames.end()) {
      if (DEBUG) std::cout << "Slave Begin: found "
		<< amc->GetName() << " in list of names, skipping" << std::endl;
      continue;
    }
    lastNames.emplace(amc->GetName());
    if (DEBUG) std::cout << "Slave Begin: found object " << amc->GetName() << std::endl;
    TString a_slot_s = (TString) amc->GetName();
    int a_slot = a_slot_s.Atoi(); // retrieve a_slot from the config somehow
    a_slot_s.Insert(0,"AMC-");
    m_amcH = new AMC_histogram("preved", gDirectory->mkdir(a_slot_s.Data()), amc->GetName());
    m_amcH->bookHistograms();
    TIter nextgeb((TList*)amc);
    TObject *geb;
    while ((geb = nextgeb())) 
    {
      if (DEBUG) std::cout << "Slave Begin: found object " << geb->GetName() << std::endl;
      TString g_slot_s = (TString)geb->GetName();
      int g_slot = g_slot_s.Atoi(); // retrieve g_slot from the config somehow
      g_slot_s.Insert(0,"GTX-");
      m_gebH = new GEB_histogram("preved", gDirectory->mkdir(g_slot_s.Data()), geb->GetName());
      m_gebH->bookHistograms();
      TMapIter nextvfat((TMap*)geb);
      TPair *vfat;
      while ((vfat = (TPair*)nextvfat())) 
      {
        TObject *chipID_s = ((TPair*)geb->FindObject(vfat))->Value();
        TString v_slot_s = (TString)vfat->GetName();
        TString s_chipID_s = (TString)chipID_s->GetName();
        if (DEBUG) std::cout << "s_chipID_s " << s_chipID_s << endl;
        //s_chipID_s = TString::BaseConvert(s_chipID_s, 16,10);
        int v_slot = v_slot_s.Atoi(); // retrieve v_slot from the config somehow
        int i_chipID = s_chipID_s.Atoi();
        VFATMap[a_slot][g_slot][v_slot] = i_chipID;
        if (DEBUG) std::cout << "Insert chip ID " << i_chipID << " in place a,g,v: "
			     << a_slot << ", " << g_slot << ", " << v_slot << endl;
        v_slot_s.Insert(0,"VFAT-");
        m_vfatH = new VFAT_histogram("preved", gDirectory->mkdir(v_slot_s.Data()), to_string(v_slot).c_str());
        m_vfatH->bookHistograms();
        m_gebH->addVFATH(m_vfatH,v_slot);
        if (DEBUG) std::cout << "Slave Begin: add vfat " << v_slot << std::endl;
        gDirectory->cd("..");   //moves back to previous directory
      } /* END VFAT LOOP */
      gDirectory->cd("..");     //moves back to previous directory
      if (DEBUG) std::cout << "Adding histograms for GEB(" << g_slot << ")" << std::endl;
      m_amcH->addGEBH(m_gebH,g_slot);
      if (DEBUG) std::cout << "Slave Begin: add geb " << g_slot << std::endl;
    } /* END GEB LOOP */
    gDirectory->cd("..");       //moves back to previous directory
    if (DEBUG) std::cout << "Adding histograms for AMC(" << a_slot << ")" << std::endl;
    m_amc13H->addAMCH(m_amcH, a_slot);
    if (DEBUG) std::cout << "Slave Begin: add amc " << a_slot << std::endl;
  } /* END AMC LOOP */

  gDirectory = savedir;
  if (DEBUG) std::cout << "SLAVE END"<< std::endl;
}

//!Fills the histograms that were book from bookAllHistograms
Bool_t gemTreeReader::Process(Long64_t entry)
{
  fChain->GetEntry(entry);
  int a13_c=0;    //counter through AMC13s
  int a_c=0;      //counter through AMCs
  int g_c=0;      //counter through GEBs
  int v_c=0;      //counter through VFATs

  v_amc13 = GEMEvents->amc13s();
  if (DEBUG) std::cout << "Get a vector of AMC13 "<< endl;
  /* LOOP THROUGH AMC13s */
  for(auto a13 = v_amc13.begin(); a13!=v_amc13.end(); a13++){
    if (DEBUG) std::cout << "Get AMC13 "<< endl;
    m_amc13H->fillHistograms(&*a13);
    if (DEBUG) std::cout << "AMC13 histograms filled "<< endl;
    v_amc = a13->amcs();
    /* LOOP THROUGH AMCs */
    for(auto a=v_amc.begin(); a!=v_amc.end(); a++){
      if (DEBUG) std::cout << "Get AMC "<< endl;
      v_geb = a->gebs();
      if (DEBUG) std::cout << "Get GEB "<< endl;
      a_c=a->AMCnum();
      if (a_c < 0 || a_c > 12) {
	std::cerr << "Invalid number of AMCs(" << a_c << ") reported, continuing" << std::endl;
	continue;
      }
      if (DEBUG) std::cout << "Trying to access histograms for AMC(" << a_c << ")" << std::endl;
      v_amcH = m_amc13H->amcsH(a_c);
      if (DEBUG) std::cout << "Get AMC H "<< endl;
      if (v_amcH) v_amcH->fillHistograms(&*a);
      if (DEBUG) std::cout << "Fill AMC histograms"<< endl;
      m_RunType = a->Rtype();
      if (m_RunType){
        m_deltaV = a->Param2() - a->Param3();
        m_Latency = a->Param1();
      }
      g_c=0;
      /* LOOP THROUGH GEBs */
      for(auto g=v_geb.begin(); g!=v_geb.end();g++){
        v_vfat = g->vfats();
        int gID = g->InputID();
	if (gID < 0 || gID > 12) {
	  std::cerr << "Invalid gID(" << gID << ") reported, continuing" << std::endl;
	  continue;
	}

	if (DEBUG) std::cout << "Trying to access histograms for GEB(" << gID << ")" << std::endl;
        v_gebH = v_amcH->gebsH(gID);
        std::map<int,int> slot_map;
        for(auto v=v_vfat.begin(); v!=v_vfat.end();v++){
          int vID = v->ChipID();
          vID = vID | 0xf000;
          int slot = slotFromMap(a_c, gID, vID);
          slot_map.insert(std::make_pair(v->ChipID(), slot));
          if (DEBUG) std::cout << "Inserted in map: chip ID " << v->ChipID() << ", slot "<< slot <<  endl;
        }
	if (DEBUG) std::cout << "v_gebH " << std::hex << std::setw(8) << std::setfill('0') << v_gebH << std::dec << std::endl;
        if (v_gebH) v_gebH->fillHistograms(&*g, slot_map);
        /* LOOP THROUGH VFATs */
        for(auto v=v_vfat.begin(); v!=v_vfat.end();v++){
          int slot = slot_map.find(v->ChipID())->second;
          if (DEBUG) std::cout << "Try get slot for chip ID " << v->ChipID() << ", retrieved slot " << slot <<  endl;
          if (slot>-1) {v_vfatH = v_gebH->vfatsH(slot);} else { continue;}
          if (v_vfatH) {
            v_vfatH->fillHistograms(&*v);
            if (m_RunType == 1 || m_RunType == 2 || m_RunType == 3){
              v_vfatH->fillScanHistograms(&*v, m_RunType, m_deltaV, m_Latency);
            }
          }
        } /* END VFAT LOOP */
      } /* END GEB LOOP */
      a_c++;
    } /* END AMC LOOP */
    a13_c++;
  } /* END AMC13 LOOP */
}

void gemTreeReader::SlaveTerminate()
{
  // The SlaveTerminate() function is called after all entries or objects
  // have been processed. When running with PROOF SlaveTerminate() is called
  // on each slave server.
  //
  // Should we fill summary canvases here?
  //
  for (unsigned int i = 0; i < 12; i++){
      if (auto a = m_amc13H->amcsH(i)){
          for (unsigned int j = 0; j < 2; j++){
              if (auto g = a->gebsH(j)){
                  g->fillSummaryCanvases();
                  for (unsigned int k = 0; k < 24; k++){
                      if (auto v = g->vfatsH(k)){
                          v->fillWarnings();
                      }
                  }
              }
          }
      }
  }
  TDirectory *savedir = gDirectory;
  fFile->cd();
  fFile->Write();
  fOutput->Add(fProofFile);
  fFile->Close();
}
void gemTreeReader::Terminate()
{
  // The Terminate() function is the last function to be called during
  // a query. It always runs on the client, it can be used to present
  // the results graphically or save the results to file.

  if ((fProofFile = dynamic_cast<TProofOutputFile*>(fOutput->FindObject("SimpleFile.root")))) {
    TString outputFile(fProofFile->GetOutputFileName());
    TString outputName(fProofFile->GetName());
    Printf("outputFile: %s", outputFile.Data());
  }
}
int gemTreeReader::slotFromMap(int a, int g, int cid)
{
  int res = -1;
  for (int i=0; i<24; i++){
    if (VFATMap[a][g][i] == cid) {res = i;}
  }
  return res;
}
void gemTreeReader::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fChain->SetMakeClass(0);

   GEMEvents = new Event();

   fChain->SetBranchAddress("GEMEvents", &GEMEvents, &b_GEMEvents);
}

Bool_t gemTreeReader::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}


