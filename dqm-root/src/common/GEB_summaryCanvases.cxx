#define NETA 8

class SummaryCanvases
{
public:
  SummaryCanvases(TString _outPath, long long int amcSlot, long long int gebSlot):outPath(_outPath),a_slot(amcSlot),g_slot(gebSlot){
    ClusterMultEta = new TH1I*[8];
    ClusterSizeEta = new TH1I*[8];
    VFATSlots = new TH1I;
    Totalb1010 = new TH1F;
    Totalb1100 = new TH1F;
    Totalb1110 = new TH1F;
    TotalFlag = new TH1F;
    TotalCRC = new TH1F;
    ClusterMult = new TH1I;
    ClusterSize = new TH1I;
    BeamProfile = new TH2I;
  }
  ~SummaryCanvases()
  {
    delete VFATSlots;
    delete Totalb1010;
    delete Totalb1100;
    delete Totalb1110;
    delete TotalFlag;
    delete TotalCRC;
    delete ClusterMult;
    delete ClusterSize;
    delete BeamProfile;
    
    delete[] ClusterMultEta;
    delete[] ClusterSizeEta;
  }

  TString outPath;
  long long int a_slot;
  long long int g_slot;
  
  TH1I* VFATSlots; 
  TH1F* Totalb1010;
  TH1F* Totalb1100;
  TH1F* Totalb1110;
  TH1F* TotalFlag ;
  TH1F* TotalCRC;
  TH1I* ClusterMult;
  TH1I* ClusterSize;
  TH2I* BeamProfile;

  TH1I** ClusterMultEta;
  TH1I** ClusterSizeEta;
          
  // Create Summary Canvases
  TCanvas* Integrity_canvas ;
  TCanvas* Occupancy_canvas ;
  TCanvas* ClusterSize_canvas;
  TCanvas* ClusterMult_canvas;




  
  void gtprintCanvas(TCanvas* canvas, TString opathName)
  {

    // canvas->Print(opathName+".jpg","jpg");
    // canvas->Print(opathName+".png","png");

    //Create JSON file
    ofstream jsonfile;
    jsonfile.open(opathName+".json");
    TString json = TBufferJSON::ConvertToJSON(canvas);
    jsonfile << json;
    jsonfile.close();

  }



  void fillGEBIntegrityCanvas()
  {
    TGaxis::SetMaxDigits(3);
    gStyle->SetOptStat(0000);

    Integrity_canvas->cd(1);
    Totalb1010->Draw();
    //gPad->SetLogy();
    Integrity_canvas->cd(2);
    Totalb1100->Draw();
  
    Integrity_canvas->cd(3);
    Totalb1110->Draw();

    Integrity_canvas->cd(4);
    VFATSlots->Draw();
  
    Integrity_canvas->cd(5);
    TotalFlag->Draw();

    Integrity_canvas->cd(6);
    TotalCRC->Draw();

    // ->Update() ?
  }


  void fillGEBOccupancyCanvas()
  {
    Occupancy_canvas->cd(1);
    std::stringstream ss;
    for (int nb = 1; nb < 9; nb ++)
      {
        std::string name = "eta_";
        ss.str(std::string());
        ss << 9-nb;
        name+=ss.str();
        BeamProfile->GetXaxis()->SetBinLabel(nb, name.c_str());
      }
    BeamProfile->GetYaxis()->SetTitle("Strips");
    BeamProfile->GetXaxis()->SetTitle("Pseudorapidity partitions");
    BeamProfile->Draw("colz");

    TH1D* p_temp;
    for (int p_i = 1; p_i < 9; p_i++)
      {
        Occupancy_canvas->cd(p_i+1);
        std::string title = "eta_";
        ss.str(std::string());
        ss << 9-p_i;
        title+=ss.str();
        p_temp = BeamProfile->ProjectionY(title.c_str(),p_i,p_i);
        p_temp->SetTitle(title.c_str());
        p_temp->GetYaxis()->SetTitle("Number of events");
        p_temp->Draw();
        gPad->Update();
      }
  

    // Occupancy_canvas->Update();  
    // Occupancy_canvas->WaitPrimitive();
    //  Occupancy_canvas->Write();
  
    //Occupancy_canvas->Draw();
  }


  void fillGEBClusterSizeCanvas()
  {
    gStyle->SetOptStat("emr");
    std::stringstream ss;
    ClusterSize_canvas->cd(1);
    ClusterSize->GetYaxis()->SetTitle("Number of entries");
    ClusterSize->GetXaxis()->SetTitle("Cluster size");
    ClusterSize->SetTitle("Integrated over pseudorapidity");
    ClusterSize->Draw();
    gPad->SetLogy();
    for (int p_i = 1; p_i < NETA+1; p_i++)
      {
        ClusterSize_canvas->cd(p_i+1);
        std::string title = "eta_";
        ss.str(std::string());
        ss << 9-p_i;
        title+=ss.str();
        ClusterSizeEta[NETA-p_i]->GetYaxis()->SetTitle("Number of entries");
        ClusterSizeEta[NETA-p_i]->GetXaxis()->SetTitle("Cluster size");
        ClusterSizeEta[NETA-p_i]->SetTitle(title.c_str());
        ClusterSizeEta[NETA-p_i]->Draw();
        gPad->SetLogy();
      }
 
    //clusterSize_canvas->Draw();
    //clusterSize_canvas->Update();
    //clusterSize_canvas->WaitPrimitive();
    gStyle->SetOptStat(0000);
  }

  void fillGEBClusterMultCanvas()
  {
    gStyle->SetOptStat("emr");
    std::stringstream ss;
    ClusterMult_canvas->cd(1);
    ClusterMult->GetYaxis()->SetTitle("Number of entries");
    ClusterMult->GetXaxis()->SetTitle("Cluster multiplicity");
    ClusterMult->SetTitle("Integrated over pseudorapidity");
    ClusterMult->Draw();
    gPad->SetLogy();
    for (int p_i = 1; p_i < NETA+1; p_i++)
      {
        ClusterMult_canvas->cd(p_i+1);
        std::string title = "eta_";
        ss.str(std::string());
        ss << 9-p_i;
        title+=ss.str();
        ClusterMultEta[NETA-p_i]->GetYaxis()->SetTitle("Number of entries");
        ClusterMultEta[NETA-p_i]->GetXaxis()->SetTitle("Cluster multiplicity");
        ClusterMultEta[NETA-p_i]->SetTitle(title.c_str());
        ClusterMultEta[NETA-p_i]->Draw();
        gPad->SetLogy();
      }
 
    //clusterMult->Draw();
    //clusterMult->Update();
    //clusterMult->WaitPrimitive();
    gStyle->SetOptStat(0000);
  }

  void fillGEBCanvases()
  {
 
    // Locate Summary Histograms for current GEB
    VFATSlots = (TH1I*)gDirectory->Get("VFATSlots");
    Totalb1010 = (TH1F*)gDirectory->Get("Totalb1010");
    Totalb1100 = (TH1F*)gDirectory->Get("Totalb1100");
    Totalb1110 = (TH1F*)gDirectory->Get("Totalb1110");
    TotalFlag = (TH1F*)gDirectory->Get("TotalFlag");
    TotalCRC = (TH1F*)gDirectory->Get("TotalCRC");
    ClusterMult = (TH1I*)gDirectory->Get("ClusterMult");
    ClusterSize = (TH1I*)gDirectory->Get("ClusterSize");
    BeamProfile = (TH2I*)gDirectory->Get("BeamProfile");

    for(int ie=0; ie < NETA; ie++){
      ClusterMultEta  [ie] = (TH1I*)gDirectory->Get(("ClusterMult"+std::to_string(ie)).c_str());
      ClusterSizeEta  [ie] = (TH1I*)gDirectory->Get(("ClusterSize"+std::to_string(ie)).c_str());
    }
          
    // Create Summary Canvases
    Integrity_canvas = newCanvas("GEBIntegrity",3,2,2400,1200);
    Occupancy_canvas = newCanvas("GEBOccupancy",3,3,1800,1800);
    ClusterSize_canvas = newCanvas("GEBClusterSize",3,3,1800,1800);
    ClusterMult_canvas = newCanvas("GEBClusterMult",3,3,1800,1800);

    // Fill Summary Canvases
    // fillGEBCanvases();


    // Print Summary Canvases in correct directory
    TString printPath = outPath+"AMC13-1/AMC-"+a_slot+"/GTX-"+g_slot+"/summary_canvases/";
    if (DEBUG) cout << "printPath: " << printPath << endl;
    gROOT->ProcessLine(".!mkdir -p "+printPath);

    gROOT->SetBatch(kTRUE);
    fillGEBIntegrityCanvas();
    fillGEBOccupancyCanvas();
    fillGEBClusterSizeCanvas();
    fillGEBClusterMultCanvas();
    // cout << "Writing other canvases" << endl;
    // Integrity_canvas->Write();
    // ClusterSize_canvas->Write();
    // ClusterMult_canvas->Write();

    gtprintCanvas(Integrity_canvas, printPath+"integrity");
    gtprintCanvas(Occupancy_canvas, printPath+"occupancy"); 
    gtprintCanvas(ClusterSize_canvas, printPath+"clusterSize");
    gtprintCanvas(ClusterMult_canvas, printPath+"clusterMult");
  }
};
