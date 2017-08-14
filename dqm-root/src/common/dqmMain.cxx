#include "TApplication.h"
#include "TProof.h"
#include "TList.h"
#include "TChain.h"
#include "TError.h"

#include"db_interface.cxx"

#include <iostream>

#define DEBUG 0

using namespace std;

TList *getConfig(string filename)
{
  MYSQL *Database;
  Database = connectDB();
  string RunName;
  if(filename.find("chunk") == string::npos) {
    RunName = filename.substr(filename.find("run"),filename.find(".raw.root")-filename.find("run"));
  }
  else {
    RunName = filename.substr(filename.find("run"),filename.find("_chunk")-filename.find("run"));
  }
  // only care about the run number
  RunName = filename.substr(filename.find("run"),9);

  TList * config = new TList();
  TList * amc = new TList();
  TMap * geb = new TMap();
  TObjString * slot = new TObjString();
  TObjString * chipID = new TObjString();
  config->SetName("config");
  string AMC13Query = "select id from ldqm_db_run where Name like '";
  AMC13Query += RunName;
  AMC13Query += "%'";

  string RunIDstr = stringFromChar(simpleDBQuery(Database, AMC13Query));
  
  string AMCQuery = "select amc_id from ldqm_db_run_amcs where run_id like '"+RunIDstr+"'";
  std::cout << "Exceuting query: " << RunIDstr << std::endl;
  vector<string> AMCs = manyDBQuery(Database,AMCQuery);
  std::cout << "Number of AMCs: " << AMCs.size() << endl;
  for ( int iamc = 0; iamc < AMCs.size(); iamc++ )
  {
    string currentAMCid = stringFromChar(AMCs.at(iamc).c_str());
    string AMCBidQuery = "select BoardID from ldqm_db_amc where id like '"+currentAMCid+"'";
    char* AMCBidchar = simpleDBQuery(Database,AMCBidQuery);
    string AMCBidstr(AMCBidchar);
    string aslot_ch = AMCBidstr.substr(AMCBidstr.find("-")+1,AMCBidstr.size()-2);
    long long int a_slot = atoi(aslot_ch.c_str());
    amc->SetName(to_string(a_slot).c_str());
    string GEBQuery = "select geb_id from ldqm_db_amc_gebs where amc_id like '"+currentAMCid+"'";
    vector<string> GEBs = manyDBQuery(Database,GEBQuery);
    std::cout << "Number of GEBs: " << GEBs.size() << endl;
    for ( int igeb = 0; igeb < GEBs.size(); igeb++ )
    {
      string currentGEBid = stringFromChar(GEBs.at(igeb).c_str());
      string GEBBidQuery = "select ChamberID from ldqm_db_geb where id like '"+currentGEBid+"'";
      char* GEBBidchar = simpleDBQuery(Database,GEBBidQuery);
      string GEBBidstr(GEBBidchar);
      string gslot_ch = GEBBidstr.substr(GEBBidstr.find("-")+1,GEBBidstr.size());
      long long int g_slot = atoi(gslot_ch.c_str());
      geb->SetName(to_string(g_slot).c_str());
      string VFATQuery = "select vfat_id from ldqm_db_geb_vfats where geb_id like '"+currentGEBid+"'";
      vector<string> VFATs = manyDBQuery(Database,VFATQuery);
      std::cout << "Number of VFATs: " << VFATs.size() << endl;
      for ( int ivfat = 0; ivfat < VFATs.size(); ivfat++ )
      {
        string currentVFATid = stringFromChar(VFATs.at(ivfat).c_str());
        string VFATSlotQuery = "select Slot from ldqm_db_vfat where id like '"+currentVFATid+"'";
        char* VFATSlotchar = simpleDBQuery(Database,VFATSlotQuery);
        long long int v_slot = atoi(VFATSlotchar);
        string VFATdirname = "VFAT-";
        VFATdirname += to_string(v_slot);
        string VFATChipQuery = "select ChipID from ldqm_db_vfat where id like '"+currentVFATid+"'";
        char* VFATChipchar = simpleDBQuery(Database,VFATChipQuery);
        int v_chipid = strtol(VFATChipchar, NULL, 16);

        slot->SetString(to_string(v_slot).c_str());
        chipID->SetString(to_string(v_chipid).c_str());
        geb->Add((TObject*)slot->Clone(),(TObject*)chipID->Clone());
      } /* END VFAT LOOP */
      amc->Add(geb->Clone());
      geb->Clear();
    } /* END GEB LOOP */
    config->Add(amc->Clone());
    amc->Clear();
    //cout << "Add AMC histograms to AMC13 for amc in " << a_slot << endl;
  } /* END AMC LOOP */

  return config;
}

int main(int argc, char** argv)
{
  gErrorIgnoreLevel = kError;
  std::cout << "--==DQM Main==--" << endl;
  if (argc<2) 
    {
      std::cout << "Please provide input filenames" << endl;
      std::cout << "Usage: <path>/rundqm inputFile.root" << endl;
      return 0;
    }
  string ifilename = argv[1];
  string ofilename;
  string RunName;
  bool print_hist = false;
  if (argc > 2){
    string option = argv[2];
    if ((option == "--with-print") || (option == "-p")) {
      print_hist = true;
      std::cout << "[MAIN]: will print histograms" << std::endl;
    }
  }

  if(ifilename.find("chunk") == string::npos) {
    RunName = ifilename.substr(ifilename.find("run"),ifilename.find(".raw.root")-ifilename.find("run"));
  }
  else {
    RunName = ifilename.substr(ifilename.find("run"),ifilename.find("_chunk")-ifilename.find("run"));
  }
  std::string tmp = ifilename.substr(ifilename.size()-9, ifilename.size());
  if (tmp != ".raw.root") throw std::runtime_error("Wrong input filename (should end with '.raw.root'): "+ifilename);
  ofilename = ifilename.substr(0,ifilename.size()-9);
  ofilename += ".analyzed.root";
 
  gSystem->Load("libEvent.so");
  TChain *ch = new TChain ("GEMtree","chain");
  ch->Add(ifilename.c_str());

  TList * m_config = new TList();
  m_config = getConfig(ifilename);

  TProof::Open("workers=4");
  gProof->AddInput(m_config);
  gProof->AddInput(new TNamed("PROOF_OUTPUTFILE", ofilename.c_str()));
  gProof->GetInputList()->Print();
  ch->SetProof();;
  string sel;
  string path;
  sel = getenv("BUILD_HOME");
  path = sel+"/gem-light-dqm/proof_packages/gemevent.par";
  gProof->UploadPackage(path.c_str());
  gProof->ShowPackages(kTRUE);
  gProof->EnablePackage("gemevent");
  gProof->ShowPackages(kTRUE);

  path = sel+"/gem-light-dqm/dqm-root/include";
  gProof->AddIncludePath(path.c_str());
  path = sel+"/gem-light-dqm/dqm-root/src/common";
  gProof->AddIncludePath(path.c_str());
  path = sel+"/gem-light-dqm/gemtreewriter/include";
  gProof->AddIncludePath(path.c_str());

  gSystem->SetIncludePath("-I$BUILD_HOME/gem-light-dqm/dqm-root/include -I$BUILD_HOME/gem-light-dqm/dqm-root/src/common -I$BUILD_HOME/gem-light-dqm/gemtreewriter/include");

  sel += "/gem-light-dqm/dqm-root/src/common/gemTreeReader.cxx++g";
  ch->Process(sel.c_str());
  //TProofLite::Mgr("__lite__")->GetSessionLogs()->Display("*");

  std::cout << "DQM Complete." << endl;
  return 0;
}
